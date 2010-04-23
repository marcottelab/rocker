// DATABASE CONSTANTS AND INCLUDES
#include <string>
#include <sstream>
#include <pqxx/connection.hxx>
#include <pqxx/transaction.hxx>


// DATABASE CONSTANTS
const std::string DBNAME             = "crossval_development";
const std::string USER               = "jwoods";
const std::string PASSWORD           = "youwish1";
const std::string READ_TRANSACTION   = "ReadTransaction";
const std::string WRITE_TRANSACTION  = "WriteTransaction";

// RESULTS CONSTANTS
const std::string ROCKER_VERSION = "0.0.1";

class database_string {
public:
    database_string(std::string dbn = DBNAME, std::string u = USER, std::string p = PASSWORD) : dbname(dbn), user(u), password(p) { }
    ~database_string() { }

    std::string dbname;
    std::string user;
    std::string password;

    std::string operator()() {
        std::ostringstream arg;
        arg << "dbname=" << dbname << " user=" << user << " password=" << password;
        return arg.str();
    }
};


// USED TO GENERATE DBARG CONSTANT:
std::string make_db_argument(const std::string& dbname, const std::string& user, const std::string& password) {
    database_string dbstr(dbname, user, password);
    return dbstr();
}
