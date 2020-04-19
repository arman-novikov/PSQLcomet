#include "ws_server.h"
#include "web_socket_crypto.h"

/*!
* \brief sets initials values of the class atributes
*/
WS_server::WS_server():
    listener(-1),
    addrIPv4{},
    addrIPv6{}
{
    workON.store(false, std::memory_order_relaxed);
}
/*!
* \brief make a server socket ready to accept web requests from clients
*/
int WS_server::initWebServerIPv4()
{
    namespace ws = ws_server_ns;
    this->listener = socket(AF_INET, SOCK_STREAM, 0);                          ///< try to get socket desriptor for the server
    if (listener < 0) {
        perror("socket");
        workON.store(false, std::memory_order_relaxed);
        return -EPIPE;
    }

    addrIPv4.sin_family = AF_INET;
    addrIPv4.sin_port = htons(ws::port);
#pragma GCC diagnostic ignored "-Wold-style-cast"
    addrIPv4.sin_addr.s_addr = htonl(INADDR_ANY);
#pragma GCC diagnostic pop
    struct sockaddr* skaddr = reinterpret_cast<struct sockaddr*>(&addrIPv4);
    if(bind(listener, skaddr, sizeof(addrIPv4)) < 0) {                         ///< try to bind socket struct and descriptor
        perror("bind");
        workON.store(false, std::memory_order_relaxed);
        return -EPIPE;
    }

    listen(listener, 1); ///< now we can start to accept requests

    workON.store(true, std::memory_order_relaxed);                             ///< indicates all is ok
    return 0;
}

/*!
* \brief implements WebSocket handshake procedure on a new connection
* \param [in] string with client's request
* \param [in] pos of (Sec-WebSocket-Key)
* \return (Sec-WebSocket-Accept)'s value
*/
std::string WS_server::webSocketHandshaker(std::string &request,
                                        std::string::size_type keyBegin)
{
    namespace ws = ws_server_ns;
    std::string res("");                        ///< here is to be a accept token
    std::string forHashing;                     ///< here is to be a (Sec-WebSocket-Accept)'s value
    res.reserve(ws::WebSockReplaySize);
    forHashing.reserve(ws::hashStringSize);

    keyBegin += ws::requestKeyIndicator.length();   ///< to step on (Sec-WebSocket-Key)'s value

    try {                                           ///< try to get (Sec-WebSocket-Key)'s value
        forHashing = request.substr(keyBegin, ws::requestKeyLen);
    } catch (std::out_of_range) {
        return res;
    }
    ///> process the gotten data according to the WebSocket handshake procedure protocol
    forHashing += ws::GUID;

    forHashing = get_webSocket_accept(forHashing);

    res += ws::WebSockReplayBase;
    res += forHashing;
    res += ws::WebSockReplayEnding;

    return res;
}
/*!
* \brief accepts clients web requests in a loop while workON is true
*/
int WS_server::acceptorIPv4()
{
    while (workON.load(std::memory_order_acquire)) {                    ///< while workON is true accept()
        struct sockaddr_in clientSockAddr{};                            ///< to keep clients address
        socklen_t clientaddr_size = sizeof(clientSockAddr);             ///< neccessary for accept()
        struct sockaddr *skad = reinterpret_cast
                <struct sockaddr*>(&clientSockAddr);
        int sock = accept(listener, skad, &clientaddr_size);
        std::lock_guard<std::mutex> lguard(IPv4_clients_mutex);          ///< take control over the map
        if (sock < 0) {                                                 ///< if we got an error            
            close(sock);
            if (IPv4_clients.find(sock) != IPv4_clients.end())
                IPv4_clients.erase(sock);                               ///< if clients in map delete it
        }
std::cout << "***** socket descriptor: " << sock << std::endl;
ws_server_ns::getClientIPv4(clientSockAddr);
        try {                                                           ///> if client's data is not in map
            IPv4_clients.at(sock);
        } catch (const std::out_of_range&) {
            IPv4_clients[sock].addr = clientSockAddr;                  ///< save it
            IPv4_clients[sock].is_valid = false;
        }
    }

    return 0;
}
/*!
* \brief handles clients web requests in a loop while workON is true
*/
int WS_server::managerIPv4()
{
    namespace ws = ws_server_ns;
    while(workON.load(std::memory_order_acquire)) {                     ///< while workON is true accept() // todo: use condvar
        std::lock_guard<std::mutex> lguard(IPv4_clients_mutex);         ///< take control over the map
        for(auto &it : IPv4_clients) {                                  ///< and try to receive requests from clients
            char rBuffer[ws::rBufferSIZE] = {0};                            ///< buffer to read data // todo: pull it up
            ssize_t bytes_read = 0;
            std::unique_lock<std::mutex> lock(it.second.mtx, std::try_to_lock);
            if(!lock.owns_lock()){
                std::cerr << "\tmanagerIPv4 locked" << std::endl;
                continue;
            }

            if (it.second.is_valid) {                                   /// < if client already passed handshaking
                bytes_read = recv(it.first, rBuffer,
                                  ws::MINIMAL_RCV_MSG_SIZE, MSG_PEEK | MSG_DONTWAIT);
                if (bytes_read < ws::MINIMAL_RCV_MSG_SIZE)                  ///< no data is available to start proccessing
                    continue;

                const uint8_t *row_data = reinterpret_cast<const uint8_t*>(rBuffer);
                uint64_t incoming_msg_size = ws::ws_get_msg_size(row_data);
                const size_t read_count = static_cast
                        <size_t>(incoming_msg_size) + ws::MINIMAL_RCV_MSG_SIZE;
                if (read_count < ws::rBufferSIZE) {
                    bytes_read = recv(it.first, rBuffer, read_count, MSG_DONTWAIT);
                } else  {
                    // add code for processing big frames
                }
            } else {
                bytes_read = recv(it.first, rBuffer, ws::rBufferSIZE, MSG_DONTWAIT);
            }
            if (bytes_read < ws::MINIMAL_RCV_MSG_SIZE) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {          ///< may be there are just no requests still from the client
                    errno = 0;
                    continue;
                } else {                                                ///< means client connection is broken
                    errno = 0;
                    close(it.first);                                    ///< close
                    IPv4_clients.erase(it.first);                       ///< and delete it
                }
            } else {                                                    ///< handle client's request
                std::string client_msg(rBuffer);    // todo: optimize it with move semantics
                workerIPv4(client_msg, it.first);   // todo add errors handling
            }
        }
    }

    return 0;
}

WS_server::~WS_server()
{
    if (listener != -1)
        close(listener);
}
