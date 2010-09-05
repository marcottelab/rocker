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
    Updater(uint exp_id) : transactor<>("Updater"), experiment_id(exp_id) {}

    map<uint,auc_info> results;
    double mean_roc_area;
    double mean_pr_area;

    void operator()(argument_type &T) {
        result R;

        if (results.size() == 0) {
            cerr << "No updates necessary." << endl;
            return;
        }

        query = make_insertion_sql() + "\n" + make_update_result_sql();

        try {
            // Insert the results
            // Update the average PR area and ROC area
            R = T.exec(query);
            cout << "Query:" << endl;
            cout << query << endl;

            T.commit();
        } catch (pqxx::sql_error e) {
            cerr << "SQL error in Updater transactor." << endl;
            cerr << "Query: " << e.query() << endl;
            cerr << "Error: " << e.what()  << endl;
#ifdef RICE
            throw Rice::Exception(rb_eArgError, "SQL error in Updater transactor.");
#else
            throw;
#endif
        }
    }

protected:
    uint experiment_id;
    string query;

    string make_insertion_sql() const {
        ostringstream q;
        q << "INSERT INTO results " << RESULTS_COLUMNS << " VALUES ";
        list<string> insertions;
        for (map<uint,auc_info>::const_iterator i = results.begin(); i != results.end(); ++i) {
            insertions.push_back( i->second.entry(experiment_id, i->first) );
        }
        q << join(insertions, ", ") << ';';
        return q.str();
    }

    string make_update_result_sql() const {
        ostringstream q;
        q << "UPDATE experiments SET mean_auroc = " << mean_roc_area
          << ", mean_auprc = " << mean_pr_area
          << " WHERE experiments.id = " << experiment_id << ';';
        return q.str();
    }
};
