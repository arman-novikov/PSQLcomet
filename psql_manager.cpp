#include "psql_manager.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <error.h>

/*!
* \brief sets initials values of the class atributes
*/
PSQLmanager::PSQLmanager()
{
    postgresConnection.reset(nullptr);
    std::for_each(dbopenParams.begin(), dbopenParams.end(),
                    [](std::string &param ){param = "";});
}

/*!
* \brief requests a new Postgres connetion from Postmaster
* \param filename an absolute name of file with a database name, username, password
* \return int == 0 if ok else error code
*/
int PSQLmanager::openDB(const char *filename)
{
    std::fstream in(filename); ///< open conf file with parameters to access to DB

    if (in.is_open()) {
        getline(in, dbopenParams.at(dbnamePos)); ///< get DB name
        getline(in, dbopenParams.at(usernamePos)); ///< get username
        getline(in, dbopenParams.at(passwordPos)); ///< get password
    } else {
        std::cout << "no conf.ini found" << std::endl;
        return -ENOENT;
    }
#define connectiontoken "dbname = "+dbopenParams[dbnamePos]+ \
                        " user = "+dbopenParams[usernamePos]+ \
                        " password = "+dbopenParams[passwordPos]+ \
                        " hostaddr = 127.0.0.1 port = 5432"
    try {
        postgresConnection.reset(new pqxx::connection(connectiontoken)); ///< create a new connection with PostgreSQL
#undef connectiontoken
        if (postgresConnection->is_open()) {
            std::cout << "Opened database successfully: "
                      << postgresConnection->dbname() << std::endl;
            return 0;
        } else {
            std::cout << "Can't open database" << std::endl;
            return -ECONNREFUSED;
        }
    }
    catch (const std::exception &e) {
       std::cerr << e.what() << std::endl;
       return -ECONNREFUSED;
    }

    return 0;
}
/*!
* \brief a draft function to implement the request's result transmitting
* \param string containing request
* \param string containing result
* \return int == 0 if ok else error code
*/
int PSQLmanager::SQLfunc(const std::string& request, std::string& result)
{
    return 0;
}
/*!
* \brief a draft function to implement transactioanal requests
* \param string containing request
* \return int == 0 if ok else error code
*/
int PSQLmanager::SQLproc(const std::string& request)
{
    return 0;
}

PSQLmanager::~PSQLmanager()
{
    if (postgresConnection && postgresConnection->is_open())
        postgresConnection->disconnect();
}
