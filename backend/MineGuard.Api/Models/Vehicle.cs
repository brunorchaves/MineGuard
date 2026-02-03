namespace MineGuard.Api.Models;

public class Vehicle
{
    public string Id { get; set; } = string.Empty;
    public int VehicleType { get; set; }
    public int CycleState { get; set; }
    public Position Position { get; set; } = new();
    public VehicleTelemetry Telemetry { get; set; } = new();
    public long LastUpdate { get; set; }
    public bool Online { get; set; }

    public string VehicleTypeName => VehicleType switch
    {
        0 => "HaulTruck",
        1 => "Excavator",
        2 => "LightVehicle",
        _ => "Unknown"
    };

    public string CycleStateName => CycleState switch
    {
        0 => "IDLE",
        1 => "LOADING",
        2 => "HAULING",
        3 => "DUMPING",
        4 => "RETURNING",
        _ => "UNKNOWN"
    };
}
