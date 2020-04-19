#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <utility>
#include <condition_variable>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>

/*!
*   \class WS_server
*   \brief WebSocket server based on TCP/IP
*
*   handles clients web requests
*/
class WS_server
{
public:
    WS_server();
    virtual ~WS_server();
    int initWebServerIPv4();                                ///< make a server socket ready to accept web requests from clients
    virtual void WS_server_start(size_t) = 0;               ///< implement it in a derived class
protected:
    int acceptorIPv4();                                     ///< accepts clients web requests in a loop while workON is true
    int managerIPv4();                                      ///< handles clients web requests in a loop while workON is true
    virtual int workerIPv4(std::string& request,
                           const int client_sock) = 0;      ///< processes each request: adjust it in a derived class
    std::string webSocketHandshaker(std::string&,
                                    std::string::size_type); ///< implements WebSocket handshake procedure on a new connection

    struct IPv4_client_t {
        std::mutex mtx;
        struct sockaddr_in addr;
        bool is_valid;
        uint8_t __padding__[sizeof(void*) - sizeof(bool)];
    };
    struct IPv6_client_t {
        std::mutex mtx;
        struct sockaddr_in6 addr;
        bool is_valid;
        uint8_t __padding__[sizeof(void*) - sizeof(bool) + 4];
    };

    std::unordered_map<int, IPv4_client_t> IPv4_clients;    ///< contains clients socket descriptors as keys and socket_ipv4 struct as value ! critical section
    std::unordered_map<int, IPv6_client_t> IPv6_clients;    ///< contains clients socket descriptors as keys and socket_ipv6 struct as value ! critical section
    std::mutex IPv4_clients_mutex;                          ///< to avoid IPv4_clients dirty r/w ops
    std::mutex IPv6_clients_mutex;                          ///< to avoid IPv6_clients dirty r/w ops

    std::atomic<bool> workON;                               ///< flag indicating whether to continue accept and handle requests

    int listener;                                           ///< Server's socket descriptor
    struct sockaddr_in  addrIPv4;                           ///< Server's socket_ipv4 struct
    struct sockaddr_in6 addrIPv6;                           ///< Server's socket_ipv6 struct
private:
    WS_server(const WS_server&) = delete;
};

namespace ws_server_ns {
    extern int getClientIPv4(const struct sockaddr_in&);                    ///< see ws_server_utils.cpp
    extern ssize_t sender(const std::string&, const int, const uint8_t);    ///< see ws_server_utils.cpp
    extern std::string decoder(const std::string &msg);                     ///< see ws_server_utils.cpp
    extern uint64_t ws_get_msg_size(const uint8_t *);

    constexpr ssize_t MINIMAL_RCV_MSG_SIZE = 6;
    constexpr uint8_t FIN = 0x80U;                   ///< indicates the last fragment of a frame
    constexpr uint8_t TEXT_FRAME = 0x1U;             ///< indicates text frame
    constexpr uint8_t BINARY_FRAME = 0x2U;           ///< indicates binary frame
    constexpr uint8_t CLOSING_FRAME = 0x8U;          ///< indicates closing frame
    constexpr uint8_t BIG_FRAME = 0x7EU;             ///< indicates a frame of length [2^8-1:2^16-1] bytes
    constexpr uint8_t EXTRA_BIG_FRAME = 0x7FU;       ///< indicates a frame of length [2^16:2^64-1] bytes
    constexpr uint8_t COMMON_FRAME_OFFSET = 2U;      ///< just flags
    constexpr uint8_t BIG_FRAME_OFFSET = 4U;         ///< 4 = 2 of flags + 2 of extra length
    constexpr uint8_t EXTRA_BIG_FRAME_OFFSET = 10U;  ///< 10 = 2 of flags + 8 of extra length
    constexpr uint8_t IS_MASK = 0b10000000;          ///< indicates whether the frame is masked
    constexpr uint8_t MASK_SIZE = 4U;

    constexpr uint16_t port = 3334U;
    const std::string GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; ///< const according to the WebSocket protocol
    const std::string WebSockReplayBase =   "HTTP/1.1 101 Switching Protocols\r\n"\
                                            "Connection: Upgrade\r\n"\
                                            "Upgrade: websocket\r\n"\
                                            "Sec-WebSocket-Accept: "; ///< here is to be accept key returned by get_webSocket_accept()
    const std::string WebSockReplayEnding = "\r\n\r\n";
    constexpr uint8_t WebSockReplaySize = 160U; //for std::string::reserve (with extra place: potentially possible to reduce)
    constexpr uint16_t hashStringSize = 64U; ///<used for reserve in webSocketHandshaker() method of class ws_server

    constexpr size_t rBufferSIZE = 1024U; ///< size of a buffer for recv() // todo: increase it to 65535

    const std::string requestKeyIndicator = "Sec-WebSocket-Key: "; ///< after this token key begins (used in get_webSocket_accept())
    constexpr uint8_t requestKeyLen = 24U;
}
