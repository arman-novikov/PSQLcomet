#pragma once
#include "psql_manager.h"
#include "ws_server.h"
#include <boost/atomic.hpp>

/*!
*   \class UMAcomet
*   \brief provides API to gain access to Postgres (WEB-server simulation) throu WebSocket
*
*   needs implementation of workerIPv4 and WS_server_start
*/
class PSQLcomet: public PSQLmanager,
                public WS_server
{
public:
    void start(size_t manager_threads = 1U);        /// API for launching PSQLcomet
    template<typename T>
    void add_channels(const T& channel_ids);

    ~PSQLcomet() override
        { workON.store(false, std::memory_order_release); }
private:
    int workerIPv4(std::string&, const int) final;
    void data_updator();
    void WS_server_start(size_t managers_thread = 1U) final;
    struct channel_t {
        std::string fresh_data;
        std::mutex mx;
    };
    std::unordered_map <std::string, channel_t> channels;
};

/*!
*   \brief adds channels to get and share info
*   \param container with channels' IDs
*/
template <typename T>
void PSQLcomet::add_channels(const T& channel_ids)
{
    for (auto i: channel_ids) {
        this->channels[i].fresh_data = "";
    }
}
