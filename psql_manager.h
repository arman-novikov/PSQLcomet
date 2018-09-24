#ifndef PSQL_MANAGER_H_INCLUDED
#define PSQL_MANAGER_H_INCLUDED

#include <memory>
#include <array>
#include <string>
#include <pqxx/pqxx>

const unsigned short dbopenParamsNum = 3;
const char paramsFileName[] = "conf.ini";
const unsigned int dbnamePos = 0;
const unsigned int usernamePos = 1;
const unsigned int passwordPos = 2;

/*!
*   \class PSQLmanager
*   \brief provides API to gain access to Postgres for WebSocket server
*/
class PSQLmanager
{
public:
    PSQLmanager();
    ~PSQLmanager();
    int openDB(const char *); ///< requests a new Postgres connetion from Postmaster
    int SQLproc(const std::string&); ///< draft
    int SQLfunc(const std::string&, std::string&); ///< draft
protected:
    std::unique_ptr<pqxx::connection> postgresConnection; ///< contains a postgres connection
    std::array <std::string, dbopenParamsNum> dbopenParams; ///< neccessary to open a postgres connection
};

#endif // PSQL_MANAGER_H_INCLUDED
