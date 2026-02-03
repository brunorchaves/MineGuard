#include "fleet.hpp"
#include "collision.hpp"
#include "tcp_client.hpp"
#include "json_serializer.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstring>

using namespace mineguard;

// Flag pra shutdown graceful
static volatile bool running = true;

void signal_handler(int) {
    running = false;
}

// ============================================================
// Helpers de output pro modo local (console)
// ============================================================

static const char* vehicle_type_str(int type) {
    switch (type) {
        case 0: return "HaulTruck";
        case 1: return "Excavator";
        case 2: return "LightVehicle";
        default: return "Unknown";
    }
}

static const char* cycle_state_str(int state) {
    switch (state) {
        case 0: return "IDLE";
        case 1: return "LOADING";
        case 2: return "HAULING";
        case 3: return "DUMPING";
        case 4: return "RETURNING";
        default: return "UNKNOWN";
    }
}

static const char* priority_str(AlertPriority p) {
    switch (p) {
        case AlertPriority::LOW:      return "LOW";
        case AlertPriority::MEDIUM:   return "MEDIUM";
        case AlertPriority::HIGH:     return "HIGH";
        case AlertPriority::CRITICAL: return "CRITICAL";
        default: return "NONE";
    }
}

static const char* alert_type_str(AlertType t) {
    switch (t) {
        case AlertType::APPROACH:   return "APPROACH";
        case AlertType::CROSSING:   return "CROSSING";
        case AlertType::TAILGATING: return "TAILGATING";
        case AlertType::BLIND_SPOT: return "BLIND_SPOT";
        default: return "UNKNOWN";
    }
}

void print_telemetry(const std::vector<TelemetryPacket>& packets) {
    std::cout << "\033[2J\033[H"; // limpa tela
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║               MINEGUARD SIMULATOR - LOCAL MODE                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";

    for (const auto& p : packets) {
        std::printf("  %-8s [%-12s] %-10s | Lat: %10.6f  Lon: %11.6f  Alt: %6.1fm\n",
            p.vehicle_id.c_str(),
            vehicle_type_str(p.vehicle_type),
            cycle_state_str(p.cycle_state),
            p.position.latitude,
            p.position.longitude,
            p.position.altitude
        );
        std::printf("  %8s Speed: %5.1f km/h  Heading: %5.1f°  Fuel: %5.1f%%  Payload: %5.1ft  RPM: %.0f\n\n",
            "",
            p.telemetry.speed,
            p.telemetry.heading,
            p.telemetry.fuel_level,
            p.telemetry.payload,
            p.telemetry.engine_rpm
        );
    }
}

void print_alerts(const std::vector<CollisionAlert>& alerts) {
    if (alerts.empty()) {
        std::cout << "  [ALERTS] No active alerts\n";
    } else {
        std::cout << "  ┌─────────────────── COLLISION ALERTS ───────────────────┐\n";
        for (const auto& a : alerts) {
            std::printf("  │ %-8s ↔ %-8s  %-10s  %-12s  TTI: %4.1fs  Dist: %5.1fm │\n",
                a.vehicle_id_1.c_str(),
                a.vehicle_id_2.c_str(),
                priority_str(a.priority),
                alert_type_str(a.type),
                a.time_to_impact,
                a.distance
            );
        }
        std::cout << "  └──────────────────────────────────────────────────────────┘\n";
    }
    std::cout << "\n  Press Ctrl+C to stop\n";
}

// ============================================================
// Uso
// ============================================================

void print_usage(const char* prog) {
    std::cout << "MineGuard Simulator v0.1\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " --local              Run in local mode (console output)\n";
    std::cout << "  " << prog << " --host <ip> --port <p>  Connect to backend via TCP\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --local          Console output only, no network\n";
    std::cout << "  --host <addr>    Backend hostname/IP (default: localhost)\n";
    std::cout << "  --port <port>    Backend port (default: 5000)\n";
    std::cout << "  --help           Show this message\n";
}

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {
    bool local_mode = false;
    std::string host = "localhost";
    uint16_t port = 5000;

    // Parse argumentos
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--local") == 0) {
            local_mode = true;
        }
        else if (std::strcmp(argv[i], "--host") == 0 && i + 1 < argc) {
            host = argv[++i];
        }
        else if (std::strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoi(argv[++i]));
        }
        else if (std::strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        else {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // Se nenhum modo foi especificado
    if (!local_mode && argc == 1) {
        print_usage(argv[0]);
        return 0;
    }

    // Registrar signal handler pra Ctrl+C
    std::signal(SIGINT, signal_handler);

    // Inicializar fleet e collision detector
    FleetManager fleet;
    fleet.initialize();

    CollisionDetector collision;

    // Conectar ao backend se nao for modo local
    std::unique_ptr<TcpClient> tcp;
    if (!local_mode) {
        tcp = std::make_unique<TcpClient>(host, port);
        std::cout << "[SIM] Connecting to backend at " << host << ":" << port << "...\n";

        if (!tcp->connect_to_server()) {
            std::cerr << "[SIM] Failed to connect. Retrying in background...\n";
        }
    } else {
        std::cout << "[SIM] Running in local mode (console output)\n";
    }

    // ========================================================
    // Loop principal - 1 Hz (1 update por segundo)
    // ========================================================

    int tick = 0;
    int reconnect_counter = 0;
    constexpr double DELTA_TIME = 1.0; // 1 segundo

    while (running) {
        auto tick_start = std::chrono::steady_clock::now();

        // 1. Update da frota (movimentacao, navegacao, ciclo)
        fleet.update(DELTA_TIME);

        // 2. Coleta de telemetria
        auto packets = fleet.collect_telemetry();

        // 3. Deteccao de colisao
        auto alerts = collision.check_all(fleet.vehicles());

        // 4. Output
        if (local_mode) {
            // Modo local: imprime no console
            print_telemetry(packets);
            print_alerts(alerts);
        } else {
            // Modo rede: serializa e envia via TCP
            std::string json = JsonSerializer::serialize_batch(packets, alerts);

            if (tcp->is_connected()) {
                if (!tcp->send_message(json)) {
                    std::cerr << "[SIM] Send failed at tick " << tick << "\n";
                }
            } else {
                // Tenta reconectar a cada 5 segundos
                reconnect_counter++;
                if (reconnect_counter >= 5) {
                    std::cout << "[SIM] Attempting reconnection...\n";
                    tcp->reconnect();
                    reconnect_counter = 0;
                }
            }
        }

        tick++;

        // Esperar ate completar 1 segundo
        auto tick_end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tick_end - tick_start);
        auto sleep_time = std::chrono::milliseconds(1000) - elapsed;
        if (sleep_time.count() > 0) {
            std::this_thread::sleep_for(sleep_time);
        }
    }

    std::cout << "\n[SIM] Shutting down after " << tick << " ticks.\n";
    return 0;
}
