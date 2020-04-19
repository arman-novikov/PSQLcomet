#include "ws_server.h"

#include <arpa/inet.h> //for inet_ntoa()

/*
 * @warning need to use ntohs() and ntohl()
*/
uint64_t  ws_server_ns::ws_get_msg_size(const uint8_t *data)
{
    namespace ws = ws_server_ns;
    uint64_t res = 0ULL;
    const uint8_t *data_ptr = data + 2; /// < here the size starts if BIG or EXTRA_BIG
    uint8_t size_byte = *reinterpret_cast<const uint8_t*>(data+1);
    size_byte &= ~ws::IS_MASK; // !!! hardcode
    switch (size_byte) {
    case ws::BIG_FRAME:
        res = *reinterpret_cast<const uint16_t*>(data_ptr);
        break;
    case ws::EXTRA_BIG_FRAME:
        res = *reinterpret_cast<const uint64_t*>(data_ptr);
        break;
    default:
        res = size_byte;
        break;
    }
    return res;
}

/*!
*   \brief prints IPv4 address of an accepted client
*   \param [in] struct sockaddr_in
*   \return 0
*   is being used to debug
*/
int ws_server_ns::getClientIPv4(const struct sockaddr_in& clientSockAddr)
{
    char dest[INET6_ADDRSTRLEN] = {0};

    inet_ntop(AF_INET, &(clientSockAddr.sin_addr), dest, INET_ADDRSTRLEN);
    //todo: return value check
std::cout<<"client addr: "<<dest<<":"<<clientSockAddr.sin_port<<std::endl;

    return 0;
}

/*!
*   \brief envelops msg and sendsto sock
*   \param [in] msg a string containing data to transmit
*   \param [in] sock a client descriptor
*   \param [in] frame_type a binary or text frame
*   \return result of the unix send()
*   \todo add big and extra big frame support
*/
ssize_t ws_server_ns::sender(const std::string &msg,
           const int sock,
           const uint8_t frame_type)
{
    namespace ws = ws_server_ns;
    if (msg.size() < ws::BIG_FRAME) {
        char header = static_cast<char>(ws::FIN | frame_type);
        char len = static_cast<char>(msg.size());
        std::string for_send;
        for_send.reserve(128); //todo: remove hardcode

        for_send += header;
        for_send += len;
        for_send += msg;

        return send(sock, for_send.data(), for_send.size(), 0);
    }

    return -1;
}

/*!
*   \brief decodes a client message
*   \param [in] msg a string containing data to decode
*   \return string containing data of clients' frame
*/
std::string ws_server_ns::decoder(const std::string &msg)
{
    namespace ws = ws_server_ns;
    static constexpr auto min_size = static_cast<size_t>(ws::MINIMAL_RCV_MSG_SIZE);
    char mask[ws::MASK_SIZE] = {0};
    size_t dataPos;                                             ///< place, where data bytes start
    std::string res("");
    bool is_masked;
    if (msg.size() < min_size) {
std::cerr << "decoder: msg is too small to decode" <<std::endl;
        return res;
    }
    is_masked = (msg.at(1) & ws::IS_MASK) ? true: false;        ///< first bit of the second byte is the mask flag
    uint8_t msg_size = static_cast<uint8_t>(msg.at(1));         ///< the size is in the 1st byte
    msg_size &= ~ws::IS_MASK;
    switch (msg_size) {
    case ws::BIG_FRAME:
        dataPos = ws::BIG_FRAME_OFFSET;
        break;
    case ws::EXTRA_BIG_FRAME:
        dataPos = ws::EXTRA_BIG_FRAME_OFFSET;
        break;
    default:
        dataPos = ws::COMMON_FRAME_OFFSET;
    }

    try {
        if (is_masked) {
            size_t mask_pos = dataPos;                  ///< if masked there is a mask before the data
            dataPos += ws::MASK_SIZE;                   ///< data is after the mask
            ///get the mask
            for (unsigned i = 0; i < ws::MASK_SIZE; ++i)
                mask[i] = msg.at(mask_pos+i);
            ///begin to decode the data
            for (unsigned i = 0; i < msg.size() - dataPos; ++i) {
                char x = msg.at(dataPos+i) ^ mask[i%4];
                res += x;
            }
        } else {///if data is not masked just get it
            res.append(msg, dataPos, msg.size() - dataPos);
        }
    } catch (const std::out_of_range& oor) {
        std::cerr << "while decoding: " << oor.what() << std::endl;
    }
    return res;
}
