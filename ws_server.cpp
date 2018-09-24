#include "ws_server.h"
#include "webSocketCrypto.h"

/*!
* \brief sets initials values of the class atributes
*/
WS_server::WS_server()
{
    addrIPv4 = {0};
    addrIPv6 = {0};
    listener = -1;
    workON.store(false, std::memory_order_relaxed);
}
/*!
* \brief make a server socket ready to accept web requests from clients
*/
int WS_server::initWebServerIPv4()
{
    this->listener = socket(AF_INET, SOCK_STREAM, 0); ///< try to get socket desriptor for the server
    if (listener < 0) {
        perror("socket");
        workON.store(false, std::memory_order_relaxed);
        return -EPIPE;
    }

    addrIPv4.sin_family = AF_INET;
    addrIPv4.sin_port = htons(port);
    addrIPv4.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addrIPv4, sizeof(addrIPv4)) < 0) { ///< try to bind socket struct and descriptor
        perror("bind");
        workON.store(false, std::memory_order_relaxed);
        return -EPIPE;
    }

    listen(listener, 1); ///< now we can start to accept requests

    workON.store(true, std::memory_order_relaxed); ///< indicates all is ok
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
    std::string res(""); ///< here is to be a accept token
    std::string forHashing; ///< here is to be a (Sec-WebSocket-Accept)'s value
    res.reserve(WebSockReplaySize);
    forHashing.reserve(hashStringSize);

    keyBegin += requestKeyIndicator.length(); ///< to step on (Sec-WebSocket-Key)'s value

    try {///< try to get (Sec-WebSocket-Key)'s value
        forHashing = request.substr(keyBegin, requestKeyLen);
    } catch (std::out_of_range) {
        return res;
    }
    ///> process the gotten data according to the WebSocket handshake procedure protocol
    forHashing += GUID;

    forHashing = get_webSocket_accept(forHashing);

    res += WebSockReplayBase;
    res += forHashing;
    res += WebSockReplayEnding;

    return res;
}
/*!
* \brief accepts clients web requests in a loop while workON is true
*/
int WS_server::acceptorIPv4()
{
    while (workON.load(std::memory_order_acquire)){ ///< while workON is true accept()
        struct sockaddr_in clientSockAddr = {0}; ///< to keep clients address
        socklen_t clientaddr_size = sizeof(clientSockAddr); ///< neccessary for accept()

        int sock = accept(listener, (struct sockaddr *)&clientSockAddr, &clientaddr_size);
        if (sock < 0) { ///< if we got an error
            std::lock_guard<std::mutex> lguard(IPv4_clients_mutex); ///< take control over the map
            close(sock);
            if (IPv4_clients.find(sock) != IPv4_clients.end())
                IPv4_clients.erase(sock); ///< if clients in map delete it
        }
std::cout<<"***** socket int: "<<sock;
getClientIPv4(clientSockAddr);

        std::lock_guard<std::mutex> lguard(IPv4_clients_mutex);//todo: make a buffer
        try { ///> if client's data is not in map
            IPv4_clients.at(sock);
        } catch (std::out_of_range) {
            IPv4_clients[sock] = clientSockAddr; ///< save it
        }
    }

    return 0;
}
/*!
* \brief handles clients web requests in a loop while workON is true
*/
int WS_server::managerIPv4()
{
    while(workON.load(std::memory_order_acquire)) { ///< while workON is true accept()
        std::lock_guard<std::mutex> lguard(IPv4_clients_mutex); ///< take control over the map

        for(auto &it : IPv4_clients) { ///< and try to receive requests from clients
            char rBuffer[rBufferSIZE] = {0}; ///< buffer to read data
            int bytes_read = recv(it.first, rBuffer, rBufferSIZE, MSG_DONTWAIT);
            if (bytes_read < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) { ///< may be there is just no requests still from the client
                    errno = 0; // neccessary?
                    continue;
                } else { ///< means client connection is broken
                    errno = 0; // neccessary?
                    close(it.first); ///< close
                    IPv4_clients.erase(it.first); ///< and delete it
                }
            } else { ///< handle client's request
std::string dbgstr(rBuffer);
if(!dbgstr.empty())
std::cout<<"msgFromClient:\n"<<dbgstr<<"\nsize = "<<dbgstr.size()<<"\n^^^^^^^^^^^^^^^^^^^^^"<<std::endl;
                std::string client_msg(rBuffer);
                workerIPv4(client_msg, it.first); //todo add errors handling
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
