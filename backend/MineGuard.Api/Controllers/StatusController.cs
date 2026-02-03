using Microsoft.AspNetCore.Mvc;
using MineGuard.Api.Services;

namespace MineGuard.Api.Controllers;

[ApiController]
[Route("api/[controller]")]
public class StatusController : ControllerBase
{
    private readonly FleetStateService _fleetState;

    public StatusController(FleetStateService fleetState)
    {
        _fleetState = fleetState;
    }

    [HttpGet]
    public IActionResult Get()
    {
        var status = _fleetState.GetStatus();
        return Ok(status);
    }
}
