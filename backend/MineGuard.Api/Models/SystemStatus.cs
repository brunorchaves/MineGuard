namespace MineGuard.Api.Models;

public class SystemStatus
{
    public bool SimulatorConnected { get; set; }
    public int TotalVehicles { get; set; }
    public int OnlineVehicles { get; set; }
    public int ActiveAlerts { get; set; }
    public int TotalAlertsReceived { get; set; }
    public long UptimeSeconds { get; set; }
    public long LastTelemetryTimestamp { get; set; }
}
