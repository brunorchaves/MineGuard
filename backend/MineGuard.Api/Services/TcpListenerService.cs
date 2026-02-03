using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using MineGuard.Api.Models;

namespace MineGuard.Api.Services;

public class TcpListenerService : BackgroundService
{
    private readonly FleetStateService _fleetState;
    private readonly ILogger<TcpListenerService> _logger;
    private const int Port = 5000;

    public TcpListenerService(FleetStateService fleetState, ILogger<TcpListenerService> logger)
    {
        _fleetState = fleetState;
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        var listener = new TcpListener(IPAddress.Any, Port);
        listener.Start();
        _logger.LogInformation("[TCP] Listening on port {Port}", Port);

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                var client = await listener.AcceptTcpClientAsync(stoppingToken);
                _logger.LogInformation("[TCP] Simulator connected from {Endpoint}", client.Client.RemoteEndPoint);
                _fleetState.SetSimulatorConnected(true);

                // Processa cliente em background
                _ = Task.Run(() => HandleClient(client, stoppingToken), stoppingToken);
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "[TCP] Error accepting connection");
            }
        }

        listener.Stop();
    }

    private async Task HandleClient(TcpClient client, CancellationToken ct)
    {
        using var stream = client.GetStream();
        var headerBuffer = new byte[4];

        try
        {
            while (!ct.IsCancellationRequested && client.Connected)
            {
                // 1. Ler header de 4 bytes (tamanho do payload, big-endian)
                if (!await ReadExact(stream, headerBuffer, 4, ct))
                {
                    break;
                }

                // Converter big-endian para int
                if (BitConverter.IsLittleEndian)
                    Array.Reverse(headerBuffer);
                int payloadLength = BitConverter.ToInt32(headerBuffer, 0);

                if (payloadLength <= 0 || payloadLength > 1_000_000)
                {
                    _logger.LogWarning("[TCP] Invalid payload length: {Length}", payloadLength);
                    break;
                }

                // 2. Ler payload JSON
                var payloadBuffer = new byte[payloadLength];
                if (!await ReadExact(stream, payloadBuffer, payloadLength, ct))
                {
                    break;
                }

                string json = Encoding.UTF8.GetString(payloadBuffer);

                // 3. Processar JSON
                ProcessMessage(json);
            }
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "[TCP] Error handling client");
        }
        finally
        {
            _logger.LogInformation("[TCP] Simulator disconnected");
            _fleetState.SetSimulatorConnected(false);
            client.Close();
        }
    }

    private void ProcessMessage(string json)
    {
        try
        {
            var batch = JsonSerializer.Deserialize<BatchPacket>(json);
            if (batch == null) return;

            // Atualizar estado dos veiculos
            foreach (var packet in batch.Telemetry)
            {
                _fleetState.UpdateVehicle(packet);
            }

            // Atualizar alertas
            _fleetState.UpdateAlerts(batch.Alerts);
        }
        catch (JsonException ex)
        {
            _logger.LogWarning("[TCP] Failed to parse JSON: {Error}", ex.Message);
        }
    }

    private static async Task<bool> ReadExact(NetworkStream stream, byte[] buffer, int count, CancellationToken ct)
    {
        int totalRead = 0;
        while (totalRead < count)
        {
            int read = await stream.ReadAsync(buffer.AsMemory(totalRead, count - totalRead), ct);
            if (read == 0) return false; // conexao fechada
            totalRead += read;
        }
        return true;
    }
}
