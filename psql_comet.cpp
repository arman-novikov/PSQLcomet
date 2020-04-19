#include "psql_comet.h"

#include <thread>
#include <future>
#include <cstdlib>
#include <ctime>
#include <chrono>

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
int PSQLcomet::workerIPv4(std::string& request, const int client_sock)
{
    namespace ws = ws_server_ns;
    std::string::size_type reqKeyPos = request.find(ws::requestKeyIndicator); //todo: add pos arg

    if (reqKeyPos != std::string::npos) {                                      ///< if we found a request for handshake
        request = webSocketHandshaker(request, reqKeyPos);                     ///< generate a handshake answer
        if (!request.empty()) {                                                ///< not empty means we got an answer for a new client
            send(client_sock, request.data(), request.size(), 0);              ///< we can send it
            IPv4_clients[client_sock].is_valid = true;                         ///< connected
        } else {
            // todo: add a handler of empty res of webSocketHandshaker
        }
    } else { //here is to be a special func        

        std::string decoded_request = ws::decoder(request);
        std::string res = SQLfunc(decoded_request);
std::cerr << "decoded request: " << decoded_request << std::endl;
        ws::sender(res, client_sock, ws::TEXT_FRAME);
    }
    return 0;
}

/*!
*   \brief launches WebSocket server
*
*   \details threads are on while atomic<bool> workON is true
*/

void PSQLcomet::WS_server_start(size_t managers_thread)
{
    typedef std::unique_ptr<std::thread> manager_t;
    std::vector<manager_t> managers;

    initWebServerIPv4();

    std::thread acceptThread(&PSQLcomet::acceptorIPv4, this);

    for  (size_t i = 0; i < managers_thread; ++i) {
        managers.push_back(std::make_unique<std::thread>(&PSQLcomet::managerIPv4, this));
        managers.back()->detach();
    }

    acceptThread.join();
}

/*!
*   \brief launches PSQLcomet
*
*   implements launching API
*   waits for any input to stop working
*/

void PSQLcomet::start(size_t manager_threads)
{
    openDB(paramsFileName);                         /// implemented in psql_manager ///paramsFileName is a global const
    PSQLcomet::WS_server_start(manager_threads);
}

/*!
 * \brief updates channels' data
*/
void PSQLcomet::data_updator()
{
    std::unordered_map<std::string, std::future<std::string> > fresh_data_cache;

    for (const auto &i: this->channels)
        fresh_data_cache[i.first] = std::future<std::string>();

    for (const auto &i: fresh_data_cache) {
        std::string channel_id = i.first;
        fresh_data_cache[channel_id] =
            std::async(std::launch::async, &PSQLcomet::SQLfunc, this, std::ref(channel_id));

    }

    while(this->workON.load(std::memory_order_acquire)) {
        const int rand_sleep = std::rand() % 1000 + 1000;
        for (auto &i: fresh_data_cache) {
            if (i.second.valid()) {
                channel_t &channel = this->channels[i.first];
                channel.fresh_data = i.second.get();
                std::lock_guard<std::mutex> lguard(channel.mx);
                fresh_data_cache[i.first] =
                    std::async(std::launch::async, &PSQLcomet::SQLfunc, this, std::ref(i.first));

            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(rand_sleep));
    }
}
