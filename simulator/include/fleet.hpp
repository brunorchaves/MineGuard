#pragma once

#ifndef FLEET_HPP
#define FLEET_HPP

#include "vehicle.hpp"
#include "telemetry.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace mineguard {

// Waypoints nomeados da mina
struct MineLayout {
    std::unordered_map<std::string, Position> waypoints;

    static MineLayout create_default();
};

// Rota: sequencia de nomes de waypoints
struct Route {
    std::vector<std::string> waypoint_names;
};

// Estado de navegacao de um veiculo
struct NavigationState {
    Route current_route;
    size_t current_waypoint_index;
    double arrival_threshold;     // metros - distancia pra considerar "chegou"
    double wait_timer;            // segundos restantes de espera (loading/dumping)
    bool waiting;
};

class FleetManager {
public:
    FleetManager();

    void initialize();
    void update(double delta_time);

    const std::vector<std::unique_ptr<Vehicle>>& vehicles() const { return vehicles_; }
    std::vector<TelemetryPacket> collect_telemetry() const;

private:
    void create_fleet();
    void setup_routes();
    void update_navigation(Vehicle& vehicle, NavigationState& nav, double dt);
    void advance_cycle(Vehicle& vehicle, NavigationState& nav);
    void handle_route_complete(Vehicle& vehicle, NavigationState& nav);

    double calculate_heading(const Position& from, const Position& to) const;
    double calculate_distance(const Position& a, const Position& b) const;

    Route get_haul_route() const;
    Route get_return_route() const;

    MineLayout mine_;
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::unordered_map<std::string, NavigationState> nav_states_;

    // Velocidades por estado do ciclo (km/h)
    static constexpr double HAUL_SPEED = 35.0;
    static constexpr double RETURN_SPEED = 40.0;
    static constexpr double APPROACH_SPEED = 10.0;
    static constexpr double LV_PATROL_SPEED = 45.0;
    static constexpr double LOADING_TIME = 120.0;    // segundos
    static constexpr double DUMPING_TIME = 45.0;     // segundos
};

} // namespace mineguard

#endif // FLEET_HPP
