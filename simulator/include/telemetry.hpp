#pragma once

#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <string>
#include <cstdint>
#include <chrono>

namespace mineguard {

struct Position {
    double latitude;
    double longitude;
    double altitude;
};

struct Telemetry {
    double speed;        // km/h
    double heading;      // degrees (0-360, 0=North)
    double payload;      // tonnes (for haul trucks)
    double fuel_level;   // percentage (0-100)
    double engine_rpm;
};

enum class AlertPriority {
    NONE = 0,
    LOW,        // 10-15s TTI
    MEDIUM,     // 5-10s TTI
    HIGH,       // 3-5s TTI
    CRITICAL    // <3s TTI
};

enum class AlertType {
    APPROACH,
    CROSSING,
    TAILGATING,
    BLIND_SPOT
};

struct CollisionAlert {
    std::string vehicle_id_1;
    std::string vehicle_id_2;
    AlertPriority priority;
    AlertType type;
    double time_to_impact;   // seconds
    double distance;         // meters
    int64_t timestamp;
};

struct TelemetryPacket {
    std::string vehicle_id;
    int64_t timestamp;       // epoch milliseconds
    Position position;
    Telemetry telemetry;
    int vehicle_type;        // maps to VehicleType enum
    int cycle_state;         // maps to CycleState enum

    int64_t now_ms() const {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()
        ).count();
    }
};

} // namespace mineguard

#endif // TELEMETRY_HPP
