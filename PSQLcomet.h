#ifndef PSQLCOMET_H_INCLUDED
#define PSQLCOMET_H_INCLUDED

#include "psql_manager.h"
#include "ws_server.h"

/*!
*   \class PSQLcomet
*   \brief provides API to gain access to Postgres throu WebSocket
*
*   needs implementation of workerIPv4 and WS_server_start
*/
class PSQLcomet: public PSQLmanager,
                 public WS_server
{
public:
    void start(); /// API for launching PSQLcomet
    ~PSQLcomet() /// todo: use join of threads
        { workON.store(false, std::memory_order_release); }
private:
    virtual int workerIPv4(std::string&, const int) override final;
    virtual void WS_server_start() override final;
};

#endif // PSQLCOMET_H_INCLUDED
