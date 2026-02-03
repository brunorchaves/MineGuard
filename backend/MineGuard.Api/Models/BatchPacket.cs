using System.Text.Json.Serialization;

namespace MineGuard.Api.Models;

public class BatchPacket
{
    [JsonPropertyName("type")]
    public string Type { get; set; } = string.Empty;

    [JsonPropertyName("telemetry")]
    public List<TelemetryPacket> Telemetry { get; set; } = new();

    [JsonPropertyName("alerts")]
    public List<CollisionAlert> Alerts { get; set; } = new();
}
