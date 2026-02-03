#include "vehicle.hpp"
#include <cmath>
#include <chrono>

namespace mineguard {

// Constantes para conversao de coordenadas
static constexpr double EARTH_RADIUS = 6371000.0;       // metros
static constexpr double DEG_TO_RAD = M_PI / 180.0;
static constexpr double RAD_TO_DEG = 180.0 / M_PI;
static constexpr double KMH_TO_MS = 1.0 / 3.6;
static constexpr double ACCEL_RATE = 2.0;               // m/s^2
static constexpr double BRAKE_RATE = 4.0;               // m/s^2

// --- VehicleSpec defaults por tipo ---

VehicleSpec Vehicle::default_spec(VehicleType type) {
    switch (type) {
        case VehicleType::HAUL_TRUCK:
            return VehicleSpec{
                .max_speed = 45.0,           // km/h
                .max_payload = 220.0,        // tonnes (CAT 793 class)
                .fuel_capacity = 3800.0,     // liters
                .fuel_consumption = 180.0,   // liters/hour
                .safety_radius = 30.0,       // metros
                .length = 13.0,
                .width = 8.0
            };
        case VehicleType::EXCAVATOR:
            return VehicleSpec{
                .max_speed = 5.0,
                .max_payload = 0.0,
                .fuel_capacity = 2500.0,
                .fuel_consumption = 120.0,
                .safety_radius = 25.0,
                .length = 15.0,
                .width = 7.0
            };
        case VehicleType::LIGHT_VEHICLE:
            return VehicleSpec{
                .max_speed = 60.0,
                .max_payload = 0.0,
                .fuel_capacity = 80.0,
                .fuel_consumption = 12.0,
                .safety_radius = 10.0,
                .length = 5.0,
                .width = 2.2
            };
    }
    return VehicleSpec{};
}

// --- Constructor ---

Vehicle::Vehicle(const std::string& id, VehicleType type, Position start_pos)
    : id_(id)
    , type_(type)
    , cycle_state_(CycleState::IDLE)
    , spec_(default_spec(type))
    , position_(start_pos)
    , telemetry_{0.0, 0.0, 0.0, 100.0, 800.0}
    , target_speed_(0.0)
    , active_(true)
{
}

// --- Update principal (chamado a cada tick) ---

void Vehicle::update(double delta_time) {
    if (!active_) return;

    // Acelera/desacelera em direcao a target_speed
    double current_ms = telemetry_.speed * KMH_TO_MS;
    double target_ms = target_speed_ * KMH_TO_MS;
    double diff = target_ms - current_ms;

    if (std::abs(diff) > 0.01) {
        double rate = (diff > 0) ? ACCEL_RATE : -BRAKE_RATE;
        current_ms += rate * delta_time;

        // Clamp
        if (rate > 0 && current_ms > target_ms) current_ms = target_ms;
        if (rate < 0 && current_ms < target_ms) current_ms = target_ms;
        if (current_ms < 0) current_ms = 0;

        telemetry_.speed = current_ms / KMH_TO_MS;
    }

    apply_payload_effects();
    update_position(delta_time);
    update_fuel(delta_time);

    // RPM proporcional a velocidade
    double speed_ratio = telemetry_.speed / spec_.max_speed;
    telemetry_.engine_rpm = 800.0 + speed_ratio * 1400.0;
}

// --- Movimentacao baseada em heading e velocidade ---

void Vehicle::update_position(double dt) {
    double speed_ms = telemetry_.speed * KMH_TO_MS;
    if (speed_ms < 0.01) return;

    // Distancia percorrida neste tick
    double distance = speed_ms * dt;

    // Converter heading para radianos (0=North, clockwise)
    double heading_rad = telemetry_.heading * DEG_TO_RAD;

    // Deslocamento em metros
    double dx = distance * std::sin(heading_rad);  // leste
    double dy = distance * std::cos(heading_rad);  // norte

    // Converter metros para graus de lat/lon
    double dlat = dy / EARTH_RADIUS * RAD_TO_DEG;
    double dlon = dx / (EARTH_RADIUS * std::cos(position_.latitude * DEG_TO_RAD)) * RAD_TO_DEG;

    position_.latitude += dlat;
    position_.longitude += dlon;
}

// --- Consumo de combustivel ---

void Vehicle::update_fuel(double dt) {
    if (telemetry_.speed < 0.1 && cycle_state_ == CycleState::IDLE) return;

    // Consumo base em litros/segundo
    double consumption_per_sec = spec_.fuel_consumption / 3600.0;

    // Fator de carga: mais consumo em movimento e com payload
    double load_factor = 0.3; // idle
    if (telemetry_.speed > 0.1) {
        load_factor = 0.6 + 0.4 * (telemetry_.speed / spec_.max_speed);
    }
    if (telemetry_.payload > 0 && spec_.max_payload > 0) {
        load_factor += 0.3 * (telemetry_.payload / spec_.max_payload);
    }

    double consumed = consumption_per_sec * load_factor * dt;
    double fuel_liters = (telemetry_.fuel_level / 100.0) * spec_.fuel_capacity;
    fuel_liters -= consumed;
    if (fuel_liters < 0) fuel_liters = 0;

    telemetry_.fuel_level = (fuel_liters / spec_.fuel_capacity) * 100.0;
}

// --- Efeito de payload na velocidade maxima ---

void Vehicle::apply_payload_effects() {
    if (spec_.max_payload <= 0 || telemetry_.payload <= 0) return;

    // Reduz velocidade maxima efetiva com carga
    double load_ratio = telemetry_.payload / spec_.max_payload;
    double effective_max = spec_.max_speed * (1.0 - 0.3 * load_ratio);

    if (target_speed_ > effective_max) {
        target_speed_ = effective_max;
    }
}

// --- Predicao de posicao futura ---

Position Vehicle::predict_position(double seconds_ahead) const {
    double speed_ms = telemetry_.speed * KMH_TO_MS;
    if (speed_ms < 0.01) return position_;

    double distance = speed_ms * seconds_ahead;
    double heading_rad = telemetry_.heading * DEG_TO_RAD;

    double dx = distance * std::sin(heading_rad);
    double dy = distance * std::cos(heading_rad);

    double dlat = dy / EARTH_RADIUS * RAD_TO_DEG;
    double dlon = dx / (EARTH_RADIUS * std::cos(position_.latitude * DEG_TO_RAD)) * RAD_TO_DEG;

    return Position{
        position_.latitude + dlat,
        position_.longitude + dlon,
        position_.altitude
    };
}

// --- Gerar pacote de telemetria ---

TelemetryPacket Vehicle::generate_packet() const {
    using namespace std::chrono;
    int64_t ts = duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();

    return TelemetryPacket{
        .vehicle_id = id_,
        .timestamp = ts,
        .position = position_,
        .telemetry = telemetry_,
        .vehicle_type = static_cast<int>(type_),
        .cycle_state = static_cast<int>(cycle_state_)
    };
}

// --- Setters ---

void Vehicle::set_target_speed(double speed) {
    if (speed < 0) speed = 0;
    if (speed > spec_.max_speed) speed = spec_.max_speed;
    target_speed_ = speed;
}

void Vehicle::set_heading(double heading) {
    // Normaliza para 0-360
    while (heading < 0) heading += 360.0;
    while (heading >= 360.0) heading -= 360.0;
    telemetry_.heading = heading;
}

} // namespace mineguard
