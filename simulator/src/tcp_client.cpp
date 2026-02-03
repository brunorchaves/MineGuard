#include "tcp_client.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace mineguard {

TcpClient::TcpClient(const std::string& host, uint16_t port)
    : host_(host)
    , port_(port)
    , socket_fd_(-1)
    , connected_(false)
{
}

TcpClient::~TcpClient() {
    disconnect();
}

// ============================================================
// Conectar ao servidor
// ============================================================

bool TcpClient::connect_to_server() {
    if (connected_) return true;

    // Criar socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::cerr << "[TCP] Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Resolver hostname
    struct hostent* server = gethostbyname(host_.c_str());
    if (!server) {
        std::cerr << "[TCP] Failed to resolve host: " << host_ << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    // Configurar endereco
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Conectar
    if (connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[TCP] Failed to connect to " << host_ << ":" << port_
                  << " - " << strerror(errno) << std::endl;
        close(socket_fd_);
        socket_fd_ = -1;
        return false;
    }

    connected_ = true;
    std::cout << "[TCP] Connected to " << host_ << ":" << port_ << std::endl;
    return true;
}

// ============================================================
// Desconectar
// ============================================================

void TcpClient::disconnect() {
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
    connected_ = false;
}

// ============================================================
// Reconectar
// ============================================================

bool TcpClient::reconnect() {
    disconnect();
    return connect_to_server();
}

// ============================================================
// Enviar mensagem com length-prefix
//
// Protocolo: [4 bytes tamanho big-endian][payload JSON]
// ============================================================

bool TcpClient::send_message(const std::string& json) {
    if (!connected_) return false;

    uint32_t length = static_cast<uint32_t>(json.size());
    uint32_t length_be = htonl(length); // big-endian

    // Enviar header (4 bytes com o tamanho)
    if (!send_bytes(&length_be, sizeof(length_be))) {
        std::cerr << "[TCP] Failed to send length header, disconnecting" << std::endl;
        disconnect();
        return false;
    }

    // Enviar payload JSON
    if (!send_bytes(json.data(), json.size())) {
        std::cerr << "[TCP] Failed to send payload, disconnecting" << std::endl;
        disconnect();
        return false;
    }

    return true;
}

// ============================================================
// Enviar bytes com loop (garante envio completo)
// ============================================================

bool TcpClient::send_bytes(const void* data, size_t length) {
    const char* ptr = static_cast<const char*>(data);
    size_t remaining = length;

    while (remaining > 0) {
        ssize_t sent = send(socket_fd_, ptr, remaining, MSG_NOSIGNAL);
        if (sent <= 0) {
            return false;
        }
        ptr += sent;
        remaining -= sent;
    }

    return true;
}

} // namespace mineguard
