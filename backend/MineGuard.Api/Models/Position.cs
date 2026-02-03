using System.Text.Json.Serialization;

namespace MineGuard.Api.Models;

public class Position
{
    [JsonPropertyName("latitude")]
    public double Latitude { get; set; }

    [JsonPropertyName("longitude")]
    public double Longitude { get; set; }

    [JsonPropertyName("altitude")]
    public double Altitude { get; set; }
}
