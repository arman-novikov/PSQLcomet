#ifndef WS_SERVER_H_INCLUDED
#define WS_SERVER_H_INCLUDED

#include <iostream>
#include <map>
#include <string>
#include <atomic>
#include <mutex>

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
    int initWebServerIPv4(); ///< make a server socket ready to accept web requests from clients
    virtual void WS_server_start() = 0; ///< implement it in a derived class
protected:
    int acceptorIPv4(); ///< accepts clients web requests in a loop while workON is true
    int managerIPv4();  ///< handles clients web requests in a loop while workON is true
    virtual int workerIPv4(std::string&, const int) = 0; ///< processes each request: adjust it in a derived class
    std::string webSocketHandshaker(std::string&, std::string::size_type); ///< implements WebSocket handshake procedure on a new connection

    std::map<int, struct sockaddr_in> IPv4_clients; ///< contains clients socket descriptors as keys and socket_ipv4 struct as value ! critical section
    std::map<int, struct sockaddr_in6> IPv6_clients; ///< contains clients socket descriptors as keys and socket_ipv6 struct as value ! critical section
    std::mutex IPv4_clients_mutex; ///< to avoid IPv4_clients dirty r/w ops
    std::mutex IPv6_clients_mutex; ///< to avoid IPv6_clients dirty r/w ops

    std::atomic<bool> workON; ///< flag indicating whether to continue accept and handle requests
    int listener;///< Server's socket descriptor
    struct sockaddr_in  addrIPv4;///< Server's socket_ipv4 struct
    struct sockaddr_in6 addrIPv6;///< Server's socket_ipv6 struct
private:
    WS_server(const WS_server&) = delete;
};

const unsigned FIN = 0b1000'0000; ///< indicates the last fragment of a frame
const unsigned TEXT_FRAME = 0b1; ///< indicates text frame
const unsigned BINARY_FRAME = 0b10; ///< indicates binary frame
const unsigned BIG_FRAME = 0b1111'1110; ///< indicates a frame of length [2^8-1:2^16-1] bytes
const unsigned EXTRA_BIG_FRAME = 0b1111'1111; ///< indicates a frame of length [2^16:2^64-1] bytes
const unsigned COMMON_FRAME_OFFSET = 2; // just flags
const unsigned BIG_FRAME_OFFSET = 4; // 4 = 2 of flags + 2 of extra length
const unsigned EXTRA_BIG_FRAME_OFFSET = 10; //10 = 2 of flags + 8 of extra length
const unsigned IS_MASK = 0b1000'0000; ///> indicates whether the frame is masked
const unsigned MASK_SIZE = 4;

extern int getClientIPv4(const struct sockaddr_in&); ///< see ws_server_utils.cpp
extern int sender(const std::string &, const int, const unsigned); ///< see ws_server_utils.cpp
extern std::string decoder(const std::string &msg); ///< see ws_server_utils.cpp

const unsigned short port = 3334;
const std::string GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; ///< const according to the WebSocket protocol
const std::string WebSockReplayBase =   "HTTP/1.1 101 Switching Protocols\r\n"\
                                        "Connection: Upgrade\r\n"\
                                        "Upgrade: websocket\r\n"\
                                        "Sec-WebSocket-Accept: "; ///< here is to be accept key returned by get_webSocket_accept()
const std::string WebSockReplayEnding = "\r\n\r\n";
const unsigned WebSockReplaySize = 160; //for std::string::reserve (with extra place: potentially possible to reduce)
const unsigned hashStringSize = 64; ///<used for reserve in webSocketHandshaker() method of class ws_server

const unsigned rBufferSIZE = 1024; ///< size of a buffer for recv()

const std::string requestKeyIndicator = "Sec-WebSocket-Key: "; ///< after this token key begins (used in get_webSocket_accept())
const unsigned requestKeyLen = 24;

#endif // WS_SERVER_H_INCLUDED
