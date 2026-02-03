using System.Collections.Concurrent;
using MineGuard.Api.Models;

namespace MineGuard.Api.Services;

public class FleetStateService
{
    private readonly ConcurrentDictionary<string, Vehicle> _vehicles = new();
    private readonly ConcurrentBag<CollisionAlert> _activeAlerts = new();
    private readonly ConcurrentBag<CollisionAlert> _alertHistory = new();
    private readonly DateTime _startTime = DateTime.UtcNow;

    private long _lastTelemetryTimestamp;
    private int _totalAlertsReceived;
    private bool _simulatorConnected;

    public void UpdateVehicle(TelemetryPacket packet)
    {
        var vehicle = new Vehicle
        {
            Id = packet.VehicleId,
            VehicleType = packet.VehicleType,
            CycleState = packet.CycleState,
            Position = packet.Position,
            Telemetry = packet.Telemetry,
            LastUpdate = packet.Timestamp,
            Online = true
        };

        _vehicles.AddOrUpdate(packet.VehicleId, vehicle, (_, _) => vehicle);
        _lastTelemetryTimestamp = packet.Timestamp;
    }

    public void UpdateAlerts(List<CollisionAlert> alerts)
    {
        // Limpa alertas ativos e substitui pelos novos
        while (_activeAlerts.TryTake(out _)) { }

        foreach (var alert in alerts)
        {
            _activeAlerts.Add(alert);
            _alertHistory.Add(alert);
            Interlocked.Increment(ref _totalAlertsReceived);
        }
    }

    public void SetSimulatorConnected(bool connected)
    {
        _simulatorConnected = connected;
    }

    public List<Vehicle> GetAllVehicles()
    {
        return _vehicles.Values.ToList();
    }

    public Vehicle? GetVehicle(string id)
    {
        _vehicles.TryGetValue(id, out var vehicle);
        return vehicle;
    }

    public List<CollisionAlert> GetActiveAlerts()
    {
        return _activeAlerts.ToList();
    }

    public List<CollisionAlert> GetAlertHistory()
    {
        return _alertHistory.ToList();
    }

    public SystemStatus GetStatus()
    {
        return new SystemStatus
        {
            SimulatorConnected = _simulatorConnected,
            TotalVehicles = _vehicles.Count,
            OnlineVehicles = _vehicles.Values.Count(v => v.Online),
            ActiveAlerts = _activeAlerts.Count,
            TotalAlertsReceived = _totalAlertsReceived,
            UptimeSeconds = (long)(DateTime.UtcNow - _startTime).TotalSeconds,
            LastTelemetryTimestamp = _lastTelemetryTimestamp
        };
    }
}
