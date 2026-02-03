using System.Text.Json.Serialization;

namespace MineGuard.Api.Models;

public class TelemetryPacket
{
    [JsonPropertyName("vehicle_id")]
    public string VehicleId { get; set; } = string.Empty;

    [JsonPropertyName("timestamp")]
    public long Timestamp { get; set; }

    [JsonPropertyName("vehicle_type")]
    public int VehicleType { get; set; }

    [JsonPropertyName("cycle_state")]
    public int CycleState { get; set; }

    [JsonPropertyName("position")]
    public Position Position { get; set; } = new();

    [JsonPropertyName("telemetry")]
    public VehicleTelemetry Telemetry { get; set; } = new();
}
