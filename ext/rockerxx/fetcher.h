#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <pqxx/transactor.hxx>
#include <pqxx/nontransaction.hxx>
#include <pqxx/result.hxx>

using std::cout;
using std::cerr;
using std::endl;
using std::set;
using std::vector;
using std::map;
using std::string;
using std::ostringstream;
using pqxx::transactor;
using pqxx::nontransaction;
using pqxx::result;

typedef unsigned int  uint;


class Fetcher : public transactor<nontransaction> {
public:
    Fetcher(uint m_id, map<uint, set<uint> >* known)
    : transactor<nontransaction>("Fetcher"), matrix_id(m_id), known_correct(known)
    {
//        cerr << "Constructed" << endl;
    }

    Fetcher(const Fetcher& rhs)
    : query(rhs.query), matrix_id(rhs.matrix_id), known_correct(rhs.known_correct)
    {
//        cerr << "Copy constructed" << endl;
    }

    ~Fetcher() {
//        cerr << "De-allocate" << endl;
    }

    void operator()(argument_type &T) {
        result R;
        query = make_known_correct_query();

        try {
            R = T.exec(query);

            // Get the row and add it to the results set
            for (result::const_iterator it = R.begin(); it != R.end(); ++it) {
                uint i; uint j;
                (*it)[1].to(j); // Get column
                (*it)[2].to(i); // Get gene
                (*known_correct)[j].insert(i);
            }

        } catch (pqxx::sql_error e) {
            cerr << "SQL error in Fetcher transactor." << endl;
            cerr << "Query: " << e.query() << endl;
            cerr << "Error: " << e.what()  << endl;
        }
    }

    set<uint>& operator[](size_t idx) {
        return (*known_correct)[idx];
    }

    set<uint> operator[](size_t idx) const {
        return (*known_correct).find(idx)->second;
    }

protected:
    string query;
    uint matrix_id;
    
    map<uint, set<uint> >* known_correct;

    string make_known_correct_query() const {
        ostringstream q;
        q << "SELECT id, j, i FROM entries WHERE matrix_id = " << matrix_id
          << " AND type = 'Cell' ORDER BY j,i;";
        return q.str();
    }
};
