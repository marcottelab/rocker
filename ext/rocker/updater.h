#include "auc_info.h"

#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <sstream>
#include <pqxx/transactor.hxx>
#include <pqxx/result.hxx>
#include <boost/algorithm/string/join.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::set;
using std::vector;
using std::string;
using std::list;
using std::map;
using std::ostringstream;
using pqxx::transactor;
using pqxx::result;
using boost::algorithm::join;

typedef unsigned int  uint;


//std::string join(const SequenceT<std::string>& strings, std::string join_str = "") {
//    std::ostringstream o;
//
//    SequenceT<string>::const_iterator i = strings.begin();
//    o << *i;
//    ++i;
//
//    for (; i != strings.end(); ++i) {
//        o << join_str << *i;
//    }
//
//    return o.str();
//}



class Updater : public transactor <> {
public:
    Updater() : transactor<>("Updater") {}

    uint experiment_id;
    map<uint,auc_info> aucs;
    string query;

    void operator()(argument_type &T) {
        result R;

        if (aucs.size() == 0) {
            cerr << "No updates necessary." << endl;
            return;
        }

        query = make_known_correct_query().c_str();

        try {
            //R = T.exec(query);
            cout << "Query:" << endl;
            cout << query << endl;
        } catch (pqxx::sql_error e) {
            cerr << "SQL error in Fetcher transactor." << endl;
            cerr << "Query: " << e.query() << endl;
            cerr << "Error: " << e.what()  << endl;
        }
    }

protected:
    string make_known_correct_query() const {
        ostringstream q;
        q << "INSERT INTO rocs " << AUC_COLUMNS << " VALUES \n";
        list<string> insertions;
        for (map<uint,auc_info>::const_iterator i = aucs.begin(); i != aucs.end(); ++i) {
            insertions.push_back( i->second.entry(experiment_id, i->first) );
        }
        q << join(insertions, ",\n") << ';';
        return q.str();
    }
};
