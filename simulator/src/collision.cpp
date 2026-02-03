#include "collision.hpp"
#include <cmath>
#include <chrono>
#include <limits>

namespace mineguard {

static constexpr double DEG_TO_RAD = M_PI / 180.0;
static constexpr double EARTH_RADIUS = 6371000.0;

CollisionDetector::CollisionDetector() {}

// ============================================================
// Checa todos os pares de veiculos (N*(N-1)/2 combinacoes)
// ============================================================

std::vector<CollisionAlert> CollisionDetector::check_all(
    const std::vector<std::unique_ptr<Vehicle>>& vehicles
) {
    std::vector<CollisionAlert> alerts;

    for (size_t i = 0; i < vehicles.size(); i++) {
        for (size_t j = i + 1; j < vehicles.size(); j++) {
            if (!vehicles[i]->is_active() || !vehicles[j]->is_active()) continue;

            CollisionAlert alert = check_pair(*vehicles[i], *vehicles[j]);
            if (alert.priority != AlertPriority::NONE) {
                alerts.push_back(alert);
            }
        }
    }

    return alerts;
}

// ============================================================
// Algoritmo de deteccao para um par de veiculos
//
// 1. Verifica distancia atual
// 2. Projeta posicoes futuras (path prediction)
// 3. Encontra o Closest Point of Approach (CPA)
// 4. Se CPA < raio de seguranca, gera alerta com TTI
// ============================================================

CollisionAlert CollisionDetector::check_pair(const Vehicle& v1, const Vehicle& v2) {
    CollisionAlert no_alert{};
    no_alert.priority = AlertPriority::NONE;

    // Se ambos estao praticamente parados, sem risco
    bool v1_moving = v1.telemetry().speed > MIN_SPEED_THRESHOLD;
    bool v2_moving = v2.telemetry().speed > MIN_SPEED_THRESHOLD;
    if (!v1_moving && !v2_moving) return no_alert;

    double safety_radius = combined_safety_radius(v1, v2);

    // Checar distancia atual primeiro
    double current_dist = calculate_distance(v1.position(), v2.position());

    // Se ja esta muito longe, nem precisa projetar
    // (a 60 km/h em 15s percorre ~250m, entao 500m e um bom corte)
    if (current_dist > 500.0) return no_alert;

    // Se ja esta dentro do raio agora
    if (current_dist < safety_radius) {
        using namespace std::chrono;
        int64_t ts = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();

        return CollisionAlert{
            .vehicle_id_1 = v1.id(),
            .vehicle_id_2 = v2.id(),
            .priority = AlertPriority::CRITICAL,
            .type = classify_alert(v1, v2),
            .time_to_impact = 0.0,
            .distance = current_dist,
            .timestamp = ts
        };
    }

    // Path Prediction: projeta posicoes no futuro
    double min_distance = current_dist;
    double tti = -1.0;

    for (double t = PREDICTION_STEP; t <= MAX_PREDICTION_TIME; t += PREDICTION_STEP) {
        Position pos1 = v1.predict_position(t);
        Position pos2 = v2.predict_position(t);

        double dist = calculate_distance(pos1, pos2);

        if (dist < min_distance) {
            min_distance = dist;
            tti = t;
        }

        // Se as trajetorias estao divergindo, para cedo
        if (dist > current_dist * 1.5 && t > 3.0) break;
    }

    // Se o CPA esta dentro do raio de seguranca, gera alerta
    if (min_distance < safety_radius && tti > 0) {
        AlertPriority priority = priority_from_tti(tti);

        using namespace std::chrono;
        int64_t ts = duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();

        return CollisionAlert{
            .vehicle_id_1 = v1.id(),
            .vehicle_id_2 = v2.id(),
            .priority = priority,
            .type = classify_alert(v1, v2),
            .time_to_impact = tti,
            .distance = min_distance,
            .timestamp = ts
        };
    }

    return no_alert;
}

// ============================================================
// Classificacao do tipo de alerta
// ============================================================

AlertType CollisionDetector::classify_alert(const Vehicle& v1, const Vehicle& v2) const {
    double h1 = v1.telemetry().heading;
    double h2 = v2.telemetry().heading;

    // Diferenca de heading
    double diff = std::abs(h1 - h2);
    if (diff > 180.0) diff = 360.0 - diff;

    // Mesma direcao (< 30 graus) = tailgating
    if (diff < 30.0) {
        return AlertType::TAILGATING;
    }

    // Direcoes opostas (> 150 graus) = approach frontal
    if (diff > 150.0) {
        return AlertType::APPROACH;
    }

    // Quase perpendicular (60-120 graus) = crossing
    if (diff > 60.0 && diff < 120.0) {
        return AlertType::CROSSING;
    }

    // Restante = blind spot
    return AlertType::BLIND_SPOT;
}

// ============================================================
// Prioridade baseada no Time to Impact
// ============================================================

AlertPriority CollisionDetector::priority_from_tti(double tti) const {
    if (tti < 3.0)  return AlertPriority::CRITICAL;
    if (tti < 5.0)  return AlertPriority::HIGH;
    if (tti < 10.0) return AlertPriority::MEDIUM;
    if (tti < 15.0) return AlertPriority::LOW;
    return AlertPriority::NONE;
}

// ============================================================
// Raio de seguranca combinado
// ============================================================

double CollisionDetector::combined_safety_radius(const Vehicle& v1, const Vehicle& v2) const {
    // Usa o maior raio + margem
    double r1 = v1.safety_radius();
    double r2 = v2.safety_radius();
    return r1 + r2;
}

// ============================================================
// Distancia entre duas posicoes em metros (Haversine simplificado)
// ============================================================

double CollisionDetector::calculate_distance(const Position& a, const Position& b) const {
    double dlat = (b.latitude - a.latitude) * DEG_TO_RAD;
    double dlon = (b.longitude - a.longitude) * DEG_TO_RAD;
    double cos_lat = std::cos(a.latitude * DEG_TO_RAD);

    double dx = dlon * cos_lat * EARTH_RADIUS;
    double dy = dlat * EARTH_RADIUS;

    return std::sqrt(dx * dx + dy * dy);
}

} // namespace mineguard
