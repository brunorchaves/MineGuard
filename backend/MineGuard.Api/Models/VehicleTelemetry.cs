using System.Text.Json.Serialization;

namespace MineGuard.Api.Models;

public class VehicleTelemetry
{
    [JsonPropertyName("speed")]
    public double Speed { get; set; }

    [JsonPropertyName("heading")]
    public double Heading { get; set; }

    [JsonPropertyName("payload")]
    public double Payload { get; set; }

    [JsonPropertyName("fuel_level")]
    public double FuelLevel { get; set; }

    [JsonPropertyName("engine_rpm")]
    public double EngineRpm { get; set; }
}
