using System.Text.Json;
using MineGuard.Api.Services;

var builder = WebApplication.CreateBuilder(args);

// Services
builder.Services.AddControllers()
    .AddJsonOptions(options =>
    {
        // API responde em camelCase pro dashboard JS
        options.JsonSerializerOptions.PropertyNamingPolicy = JsonNamingPolicy.CamelCase;
    });
builder.Services.AddSingleton<FleetStateService>();
builder.Services.AddHostedService<TcpListenerService>();

// CORS - permitir dashboard acessar a API
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
    {
        policy.AllowAnyOrigin()
              .AllowAnyMethod()
              .AllowAnyHeader();
    });
});

// Configurar porta HTTP diferente da TCP (TCP usa 5000)
builder.WebHost.UseUrls("http://0.0.0.0:5100");

var app = builder.Build();

app.UseCors();

// Servir dashboard como static files (da pasta wwwroot)
app.UseDefaultFiles();
app.UseStaticFiles();

app.MapControllers();

app.Run();
