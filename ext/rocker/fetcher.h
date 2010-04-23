#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <pqxx/transactor.hxx>
#include <pqxx/result.hxx>

using std::cout;
using std::cerr;
using std::endl;
using std::set;
using std::vector;
using std::string;
using std::ostringstream;
using pqxx::transactor;
using pqxx::result;

typedef unsigned int  uint;


class Fetcher : public transactor <> {
public:
    Fetcher() : transactor<>("Fetcher") {}

    uint matrix_id;
    uint experiment_id;
    vector< set<uint> > known_correct;
    string query;

    void operator()(argument_type &T) {
        result R;
        query = make_known_correct_query().c_str();

        try {
            R = T.exec(query);

            vector< set<uint> > known(R.size());

            // Get the row and add it to the results set
            for (result::const_iterator it = R.begin(); it != R.end(); ++it) {
                uint i; uint j;
                (*it)[1].to(j); // Get column
                (*it)[2].to(i); // Get gene
                known[j].insert(i);
            }

            known_correct = known;

        } catch (pqxx::sql_error e) {
            cerr << "SQL error in Fetcher transactor." << endl;
            cerr << "Query: " << e.query() << endl;
            cerr << "Error: " << e.what()  << endl;
        }
    }

protected:
    string make_known_correct_query() const {
        ostringstream q;
        q << "SELECT id, j, i FROM entries WHERE matrix_id = " << matrix_id
          << " AND type = 'Cell' ORDER BY j,i;";
        return q.str();
    }
};
