using System.Text.Json.Serialization;

namespace MineGuard.Api.Models;

public class CollisionAlert
{
    [JsonPropertyName("vehicle_id_1")]
    public string VehicleId1 { get; set; } = string.Empty;

    [JsonPropertyName("vehicle_id_2")]
    public string VehicleId2 { get; set; } = string.Empty;

    [JsonPropertyName("priority")]
    public int Priority { get; set; }

    [JsonPropertyName("alert_type")]
    public int AlertType { get; set; }

    [JsonPropertyName("time_to_impact")]
    public double TimeToImpact { get; set; }

    [JsonPropertyName("distance")]
    public double Distance { get; set; }

    [JsonPropertyName("timestamp")]
    public long Timestamp { get; set; }

    public string PriorityName => Priority switch
    {
        1 => "LOW",
        2 => "MEDIUM",
        3 => "HIGH",
        4 => "CRITICAL",
        _ => "NONE"
    };

    public string AlertTypeName => AlertType switch
    {
        0 => "APPROACH",
        1 => "CROSSING",
        2 => "TAILGATING",
        3 => "BLIND_SPOT",
        _ => "UNKNOWN"
    };
}
