#pragma once

#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <string>
#include <cstdint>

namespace mineguard {

class TcpClient {
public:
    TcpClient(const std::string& host, uint16_t port);
    ~TcpClient();

    // Nao copiavel
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    bool connect_to_server();
    void disconnect();
    bool is_connected() const { return connected_; }

    // Envia string JSON com length-prefix (4 bytes big-endian + payload)
    bool send_message(const std::string& json);

    // Tenta reconectar se desconectado
    bool reconnect();

private:
    bool send_bytes(const void* data, size_t length);

    std::string host_;
    uint16_t port_;
    int socket_fd_;
    bool connected_;
};

} // namespace mineguard

#endif // TCP_CLIENT_HPP
