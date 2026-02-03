using Microsoft.AspNetCore.Mvc;
using MineGuard.Api.Services;

namespace MineGuard.Api.Controllers;

[ApiController]
[Route("api/[controller]")]
public class VehiclesController : ControllerBase
{
    private readonly FleetStateService _fleetState;

    public VehiclesController(FleetStateService fleetState)
    {
        _fleetState = fleetState;
    }

    [HttpGet]
    public IActionResult GetAll()
    {
        var vehicles = _fleetState.GetAllVehicles();
        return Ok(vehicles);
    }

    [HttpGet("{id}")]
    public IActionResult GetById(string id)
    {
        var vehicle = _fleetState.GetVehicle(id);
        if (vehicle == null)
            return NotFound(new { error = $"Vehicle '{id}' not found" });

        return Ok(vehicle);
    }
}
