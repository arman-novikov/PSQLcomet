#pragma once

#include <memory>
#include <array>
#include <string>
#include <pqxx/pqxx>

constexpr unsigned short dbopenParamsNum = 3;
constexpr char paramsFileName[] = "conf.ini";
constexpr unsigned int dbnamePos = 0;
constexpr unsigned int usernamePos = 1;
constexpr unsigned int passwordPos = 2;

/*!
*   \class PSQLmanager
*   \brief provides API to gain access to Postgres for WebSocket server
*/
class PSQLmanager
{
public:
    PSQLmanager();
    virtual ~PSQLmanager();
    int openDB(const char*);                                ///< requests a new Postgres connetion from Postmaster
    int SQLproc(const std::string&);                        ///< draft
    std::string SQLfunc(const std::string&);                ///< draft
protected:
    std::unique_ptr<pqxx::connection> postgresConnection;   ///< contains a postgres connection
    std::array <std::string, dbopenParamsNum> dbopenParams; ///< neccessary to open a postgres connection
};
