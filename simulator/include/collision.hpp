#pragma once

#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "vehicle.hpp"
#include "telemetry.hpp"
#include <vector>
#include <memory>

namespace mineguard {

class CollisionDetector {
public:
    CollisionDetector();

    // Checa todas as combinacoes de veiculos e retorna alertas ativos
    std::vector<CollisionAlert> check_all(
        const std::vector<std::unique_ptr<Vehicle>>& vehicles
    );

private:
    // Checa um par de veiculos
    CollisionAlert check_pair(const Vehicle& v1, const Vehicle& v2);

    // Calcula distancia entre duas posicoes (metros)
    double calculate_distance(const Position& a, const Position& b) const;

    // Classifica o tipo de alerta baseado nas trajetorias
    AlertType classify_alert(const Vehicle& v1, const Vehicle& v2) const;

    // Determina prioridade baseado no TTI
    AlertPriority priority_from_tti(double tti) const;

    // Raio de seguranca combinado de dois veiculos
    double combined_safety_radius(const Vehicle& v1, const Vehicle& v2) const;

    // Configuracao
    static constexpr double MAX_PREDICTION_TIME = 15.0;  // segundos
    static constexpr double PREDICTION_STEP = 0.5;       // segundos
    static constexpr double MIN_SPEED_THRESHOLD = 1.0;   // km/h - ignora veiculos parados
};

} // namespace mineguard

#endif // COLLISION_HPP
