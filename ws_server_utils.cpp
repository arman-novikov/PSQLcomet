#include "ws_server.h"

#include <arpa/inet.h> //for inet_ntoa()

/*!
*   \brief prints IPv4 address of an accepted client
*   \param [in] struct sockaddr_in
*   \return 0
*   is being used to debug
*/
int getClientIPv4(const struct sockaddr_in& clientSockAddr)
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
int sender(const std::string &msg,
           const int sock,
           const unsigned frame_type)
{
    if (msg.size() < BIG_FRAME) {
        char header = FIN | frame_type;
        char len = msg.size(); // todo: dynamic_cast
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
std::string decoder(const std::string &msg)
{
    char mask[MASK_SIZE];
    size_t dataPos; ///< place, where data bytes start
    std::string res("");
    bool is_masked = (msg.at(1) & IS_MASK) ? true: false; ///< first bit of the second byte is the mask flag

    try {
        if (msg.at(2) == BIG_FRAME) /// last 7 bits of the second byte indicates len of the data fragment
            dataPos = BIG_FRAME_OFFSET;
        else if (msg.at(2) == EXTRA_BIG_FRAME)
            dataPos = EXTRA_BIG_FRAME_OFFSET;
        else
            dataPos = COMMON_FRAME_OFFSET;

        if (is_masked) {
            unsigned mask_pos = dataPos; ///< if masked there is a mask before the data
            dataPos += MASK_SIZE; /// data is after the mask
            ///get the mask
            for (unsigned i = 0; i < MASK_SIZE; ++i )
            mask[i] = msg.at(mask_pos+i);
            ///begin to decode the data
            for (unsigned i = 0; i < msg.size() - dataPos; ++i) {
                char x = msg.at(dataPos+i) ^ mask[i%4];
                res += x;
            }
        } else {///if data is not masked just get it
            res.append(msg, dataPos, msg.size() - dataPos);
std::cout<<"/n!!! decoder_else !!!/n";
        }
    } catch (std::out_of_range) {std::cout<<"/n!!! decoder_except !!!/n";} // just an empty res for return
std::cout<<"++++++ decoded msg:\n"<<res<<"\n+++++++++++\n";
    return res;
}
