#include "PSQLcomet.h"

#include <thread>

/*!
*   \brief implements main logic of requests to DB processing
*   \param [in] string containing request
*   \param [in] int client socket descriptor
*   \return int == 0 or error code
*
*   adjust it for you use
*   here is an example given to make a handshake procedure if a new client
*   or reply any message of accepted clients with the result of
*   "SELECT text FROM texts WHERE id = 1"
*/
int PSQLcomet::workerIPv4(std::string& strBuffer, const int client_sock)
{
    std::string::size_type reqKeyPos = strBuffer.find(requestKeyIndicator); //todo: add pos arg
    if (reqKeyPos != std::string::npos) { /// if we found a request for handshake
        strBuffer = webSocketHandshaker(strBuffer, reqKeyPos); /// generate a handshake answer
        if (!strBuffer.empty()) /// not empty means we got an answer for a new client
            send(client_sock, strBuffer.data(), strBuffer.size(), 0); // we can send it
        else
            ; ///todo add a handler of empty res of webSocketHandshaker

    } else { //here is to be a special func
        if (!postgresConnection) /// Postgres availability testing
            return -ECONNREFUSED;

        decoder(strBuffer); /// implemented in the class ws_server
        std::string res; /// here is to be result of sql request
        pqxx::nontransaction N(*postgresConnection); /// prepare a nontransaction object to execute it
        pqxx::result R( N.exec("SELECT text FROM texts WHERE id = 1")); /// gain the result of our sql request
        for (pqxx::result::const_iterator c = R.begin(); c != R.end(); ++c) {
            res = c[0].as<std::string>();
        }
        sender(res, client_sock, TEXT_FRAME); /// implemented in ws_server_utils.cpp

    }
    return 0;
}

/*!
*   \brief launches WebSocket server
*
*   uses 2 detached threads:
*   1st for accepting connections
*   2nd for handling requests
*
*   threads are on while atomic<bool> workON is true
*/

void PSQLcomet::WS_server_start()
{
    initWebServerIPv4();

    std::thread acceptThread(&PSQLcomet::acceptorIPv4, this);
    acceptThread.detach();

    std::thread managerThread(&PSQLcomet::managerIPv4, this);
    managerThread.detach();
}

/*!
*   \brief launches PSQLcomet
*
*   implements launching API
*   waits for any input to stop working
*/

void PSQLcomet::start()
{
    openDB(paramsFileName); /// implemented in psql_manager ///paramsFileName is a global const
    PSQLcomet::WS_server_start(); ///

    while (true) {
        int x;
        std::cin >> x;
    }
}
