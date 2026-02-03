using Microsoft.AspNetCore.Mvc;
using MineGuard.Api.Services;

namespace MineGuard.Api.Controllers;

[ApiController]
[Route("api/[controller]")]
public class AlertsController : ControllerBase
{
    private readonly FleetStateService _fleetState;

    public AlertsController(FleetStateService fleetState)
    {
        _fleetState = fleetState;
    }

    [HttpGet]
    public IActionResult GetActive()
    {
        var alerts = _fleetState.GetActiveAlerts();
        return Ok(alerts);
    }

    [HttpGet("history")]
    public IActionResult GetHistory()
    {
        var history = _fleetState.GetAlertHistory();
        return Ok(history);
    }
}
