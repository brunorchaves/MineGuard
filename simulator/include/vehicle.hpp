#pragma once

#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include "telemetry.hpp"
#include <string>

namespace mineguard {

enum class VehicleType {
    HAUL_TRUCK = 0,
    EXCAVATOR,
    LIGHT_VEHICLE
};

enum class CycleState {
    IDLE = 0,
    LOADING,
    HAULING,
    DUMPING,
    RETURNING
};

struct VehicleSpec {
    double max_speed;         // km/h
    double max_payload;       // tonnes (0 for non-haulers)
    double fuel_capacity;     // liters
    double fuel_consumption;  // liters/hour
    double safety_radius;     // meters
    double length;            // meters
    double width;             // meters
};

class Vehicle {
public:
    Vehicle(const std::string& id, VehicleType type, Position start_pos);

    void update(double delta_time);
    Position predict_position(double seconds_ahead) const;
    TelemetryPacket generate_packet() const;

    // Getters
    const std::string& id() const { return id_; }
    VehicleType type() const { return type_; }
    CycleState cycle_state() const { return cycle_state_; }
    const Position& position() const { return position_; }
    const Telemetry& telemetry() const { return telemetry_; }
    double safety_radius() const { return spec_.safety_radius; }
    bool is_active() const { return active_; }

    // Setters
    void set_target_speed(double speed);
    void set_heading(double heading);
    void set_active(bool active) { active_ = active; }
    void set_cycle_state(CycleState state) { cycle_state_ = state; }

    // Specs por tipo de veiculo
    static VehicleSpec default_spec(VehicleType type);

private:
    void update_position(double dt);
    void update_fuel(double dt);
    void apply_payload_effects();

    std::string id_;
    VehicleType type_;
    CycleState cycle_state_;
    VehicleSpec spec_;
    Position position_;
    Telemetry telemetry_;
    double target_speed_;
    bool active_;
};

} // namespace mineguard

#endif // VEHICLE_HPP
