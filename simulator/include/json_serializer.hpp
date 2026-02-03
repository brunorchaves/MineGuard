#pragma once

#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include "telemetry.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace mineguard {

class JsonSerializer {
public:
    static std::string serialize(const TelemetryPacket& packet) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6);

        ss << "{";
        ss << "\"type\":\"telemetry\",";
        ss << "\"vehicle_id\":\"" << packet.vehicle_id << "\",";
        ss << "\"timestamp\":" << packet.timestamp << ",";
        ss << "\"vehicle_type\":" << packet.vehicle_type << ",";
        ss << "\"cycle_state\":" << packet.cycle_state << ",";

        ss << "\"position\":{";
        ss << "\"latitude\":" << packet.position.latitude << ",";
        ss << "\"longitude\":" << packet.position.longitude << ",";
        ss << "\"altitude\":" << std::setprecision(1) << packet.position.altitude;
        ss << "},";

        ss << std::setprecision(2);
        ss << "\"telemetry\":{";
        ss << "\"speed\":" << packet.telemetry.speed << ",";
        ss << "\"heading\":" << packet.telemetry.heading << ",";
        ss << "\"payload\":" << packet.telemetry.payload << ",";
        ss << "\"fuel_level\":" << packet.telemetry.fuel_level << ",";
        ss << "\"engine_rpm\":" << packet.telemetry.engine_rpm;
        ss << "}";

        ss << "}";
        return ss.str();
    }

    static std::string serialize(const CollisionAlert& alert) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "{";
        ss << "\"type\":\"alert\",";
        ss << "\"vehicle_id_1\":\"" << alert.vehicle_id_1 << "\",";
        ss << "\"vehicle_id_2\":\"" << alert.vehicle_id_2 << "\",";
        ss << "\"priority\":" << static_cast<int>(alert.priority) << ",";
        ss << "\"alert_type\":" << static_cast<int>(alert.type) << ",";
        ss << "\"time_to_impact\":" << alert.time_to_impact << ",";
        ss << "\"distance\":" << alert.distance << ",";
        ss << "\"timestamp\":" << alert.timestamp;
        ss << "}";

        return ss.str();
    }

    static std::string serialize_batch(
        const std::vector<TelemetryPacket>& packets,
        const std::vector<CollisionAlert>& alerts
    ) {
        std::ostringstream ss;

        ss << "{";
        ss << "\"type\":\"batch\",";

        // Telemetria
        ss << "\"telemetry\":[";
        for (size_t i = 0; i < packets.size(); i++) {
            if (i > 0) ss << ",";
            ss << serialize(packets[i]);
        }
        ss << "],";

        // Alertas
        ss << "\"alerts\":[";
        for (size_t i = 0; i < alerts.size(); i++) {
            if (i > 0) ss << ",";
            ss << serialize(alerts[i]);
        }
        ss << "]";

        ss << "}";
        return ss.str();
    }
};

} // namespace mineguard

#endif // JSON_SERIALIZER_HPP
