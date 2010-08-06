#ifndef ROCKERXX_H_
# define ROCKERXX_H_

#include "constants.h"
#include "line_input_iterator.h"
#include "fetcher.h"
#include "updater.h"
#include "auc_info.h"

#include <string>
#include <utility>
#include <sstream>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

using boost::filesystem::exists;
using boost::filesystem::ifstream;
using std::string;
using std::list;
using std::ostringstream;

// typedef unordered_map<uint, double> gene_score_map;
typedef list<std::pair<uint,double> > gene_score_list;
typedef GeneScoreIterator<unsigned int,double> gene_score_iterator;


string path_to_s(const boost::filesystem::path& p) {
    return p.string().substr(2);
}

bool path_to_uint(const boost::filesystem::path& p, uint& n) {
    using boost::lexical_cast;
    using boost::bad_lexical_cast;
    
    string s = path_to_s(p);
    try {
        n = lexical_cast<uint>(s);
    } catch(bad_lexical_cast &) {
        cerr << "Unable to read phenotype file '" << s << "': not numeric" << endl;
        return false;
    }
    return true;
}


class Rocker {
public:

    // Connect to the database and create the read transaction.
    // This also creates the place where query results from the fetch operation will
    // be stored. It won't work if the fetcher creates those results itself, since
    // perform() apparently creates a copy and therefore doesn't write to the
    // original's var.
    Rocker(string dbarg, uint m_id, uint e_id)
    : c(dbarg), known_(new map<uint, set<uint> >), fetch(m_id, known_), update(e_id)
    {
        c.perform(fetch);
    }


    // Delete data that has been loaded.
    ~Rocker() {
        delete known_;
    }


    // Acquire results, compare them to what's in the database, and add the data to the DB.
    // The argument allows us to choose a value at which to set the T/F cutoff.
    void acquire_results(float threshold) {
        update.aucs = process_results(threshold);
        c.perform(update);
    }


    // Return the mean AUC calculated -- requires that process_results was called,
    // which happens in the constructor, so it's okay.
    double mean_auc() { return update.mean_auc; }
    
    // Go through the results directory
    map<uint,auc_info> process_results(float threshold) {
        using namespace boost::filesystem;
        map<uint, auc_info> rocs;
        
        double temp_auc_accum = 0.0; // Keep track of AUCs so we can get a mean
        size_t divide_by      = 1;
        
        // Look at all files in the directory
        for (basic_directory_iterator<path> jit(path(".")); jit != directory_iterator(); ++jit) {
            uint j = 0;
            if (path_to_uint(jit->path(), j)) {
                // Read the file and calculate AUCs.
                rocs[j] = calculate_statistic(j, threshold);
                temp_auc_accum += rocs[j].auc;
                ++divide_by;
                
                cout << "AUC: " << rocs[j].auc << endl;
            }
        }

        // Calculate the mean AUC
        if (divide_by > 0)
            update.mean_auc = temp_auc_accum / (double)(divide_by);
        else
            update.mean_auc = 0;

        return rocs;
    }


    // Get genes with a specific phenotype association (phenotype id = j).
    set<uint> fetch_column(uint j) const {
        return (*known_)[j];
    }


    // For some phenotype j, determine AUC, fp, tp, fn, tn, etc.
    auc_info calculate_statistic(uint j, float threshold) const {
        set<uint> known_correct     = fetch_column(j);
        gene_score_list candidates  = read_candidates(j);
        auc_info                      result(threshold);

        if (known_correct.size() == 0) {
            result.auc = 0;
            return result;
        }

        // Attempted transcription of code from Ruby into C++, after having taken
        // it from Python the first time.
        // No guarantees!
        vector<size_t> t,f; // trues and falses
        t.reserve(candidates.size()+1); t.push_back(0);
        f.reserve(candidates.size()+1); f.push_back(0);

        for (gene_score_list::const_iterator i = candidates.begin(); i != candidates.end(); ++i) {
            if (known_correct.find(i->first) != known_correct.end()) {
                t.push_back( *(t.rbegin()) + 1 );
                f.push_back( *(f.rbegin()) );

                // Update true positives / false negatives
                if (i->second > result.threshold) result.tp++;
                else                              result.fn++;
                
            } else {
                t.push_back( *(t.rbegin()) );
                f.push_back( *(f.rbegin()) + 1 );

                // Update false positives / true negatives
                if (i->second > result.threshold) result.fp++;
                else                              result.tn++;
            }

        }

        vector<double> tpl; tpl.reserve(candidates.size()+1);
        // vector<double> fpl = tpl;
        size_t last_f = 0;
        for (size_t i = 0; i < t.size(); ++i) {
            if (f[i] > last_f) {
                tpl.push_back(t[i]);
                // fpl.push_back(f[i]);
                last_f = f[i];
            }
        }

        size_t last_t = *(t.rbegin());
        double sum = 0.0;
        // Divide each by the last item in that array
        // Also keep track of the sum for calculating the final AUC value
        for (size_t i = 0; i < tpl.size(); ++i) {
            // tpl[i] /= (double)(last_t);
            // fpl[i] /= (double)(last_f);
            sum += tpl[i];
        }

        result.auc = (sum / (double)(last_t)) / (double)(tpl.size());
        if (tpl.size() == 0) result.auc = 0; // prevent NaN return.

        return result;
    }


    // Assume we're in the correct directory and read the correct phenotype file
    // First two lines are comment.
    // Assumes the files are pre-sorted by sortall.pl (by column 2 descending).
    // column 1 is the gene, column 2 is the prediction score (higher is better).
    gene_score_list read_candidates(uint j) const {
        ostringstream fn; fn << j;
        boost::filesystem::path filepath(fn.str());
        //cerr << "Opening file: '" << filepath << "'" << endl;

        if (!exists(filepath)) {
            //cerr << "Error: File '" << filepath << "' does not exist." << endl;
            throw;
        }

        gene_score_list res;

        // Open a filestream
        ifstream fin(filepath);

        // Ignore two header lines
        fin.ignore(500, '\n');
        fin.ignore(500, '\n');
        //cout << "Next character is: '" << fin.peek() << "'" << endl;

        // Iterate through the gene-score pairs in the file
        for (gene_score_iterator gsit(fin); gsit != gene_score_iterator(); ++gsit) {
            //cerr << "Adding " << gsit->first << '\t' << gsit->second << endl;
            res.push_back(*gsit);
        }

        fin.close();

        return res;
    }
    
    
protected:
    pqxx::connection c;
    map<uint, set<uint> >* known_;
    Fetcher fetch;
    Updater update;
};

#endif
