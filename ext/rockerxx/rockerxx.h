#ifndef ROCKERXX_H_
# define ROCKERXX_H_

#include "constants.h"
#include "gene_score_iterator.h"
#include "fetcher.h"
#include "updater.h"
#include "confusion_matrix.h"

#include <string>
#include <utility>
#include <sstream>
#include <list>
#include <vector>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#ifdef RICE
#include <rice/Exception.hpp>
#endif

using boost::filesystem::exists;
using boost::filesystem::ifstream;
using boost::lexical_cast;
using std::string;
using std::list;
using std::vector;
using std::ostringstream;

// typedef unordered_map<uint, double> gene_score_map;
typedef std::pair<uint, double>                gene_score;
typedef std::pair<std::set<uint>,double>       genes_score;
typedef list<genes_score >                     genes_score_list;
typedef GeneScoreIterator<unsigned int,double> gene_score_iterator;
typedef vector<size_t>                         size_vec;



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
        update.results = process_results(threshold);
        c.perform(update);
    }


    // Return the mean AUC calculated -- requires that process_results was called,
    // which happens in the constructor, so it's okay.
    double roc_area() const { return update.mean_roc_area; }
    double pr_area() const {  return update.mean_pr_area;  }
    
    // Go through the results directory
    map<uint,auc_info> process_results(float threshold) {
        using namespace boost::filesystem;
        map<uint, auc_info> rocs;
        
        double temp_auc_accum = 0.0; // Keep track of AUCs so we can get a mean
        double temp_pr_accum  = 0.0; // Keep track of AU-PR-Cs so we can get a mean
        size_t divide_by      = 1;
        
        // Look at all files in the directory
        for (basic_directory_iterator<path> jit(path(".")); jit != directory_iterator(); ++jit) {
            uint j = 0;
            if (path_to_uint(jit->path(), j)) {
                // Read the file and calculate AUCs.
                rocs[j] = calculate_statistic(j);
                temp_auc_accum += rocs[j].roc_area;
                temp_pr_accum  += rocs[j].pr_area;
                ++divide_by;
                
                cout << "ROC area: " << rocs[j].roc_area << "\t\tPR area: " << rocs[j].pr_area << endl;
            }
        }

        // Calculate the mean AUC
        if (divide_by > 0) {
            update.mean_roc_area = temp_auc_accum / (double)(divide_by);
            update.mean_pr_area = temp_pr_accum / (double)(divide_by);
        } else {
            update.mean_roc_area = 0;
            update.mean_pr_area = 0;
        }

        return rocs;
    }


    // Get genes with a specific phenotype association (phenotype id = j).
    set<uint> fetch_column(uint j) const {
        return (*known_)[j];
    }


    // For some phenotype j, determine:
    // * Sensitivity/recall/TPR  (TP/(TP+FN))
    // * Fallout/FPR (FP/(FP+TN))
    // * precision (TP/(TP+FP))
    confusion_matrix calculate_plots(uint j) const {
        using std::pair;

        pair<genes_score_list,size_t> candidates;
        try {
            candidates = read_candidates(j); // bins (first) and total number of genes (second)
        }
#ifdef RICE
        catch (Rice::Exception e) {
            string err("rockerxx.h: calculate_plots: column ");
            err += lexical_cast<string>(j) + " has no predictions";
            throw Rice::Exception(rb_eArgError, err.c_str());
        }
#else
        catch (...) { // Just continue to throw an exception.
            throw;
        }
#endif
        set<uint> known_correct                  = fetch_column(j);
        
        confusion_matrix cm(candidates.first.size(), candidates.second, known_correct.size());

        for (genes_score_list::const_iterator i = candidates.first.begin(); i != candidates.first.end(); ++i) {

            // Get the number of genes in the current class that are "correct" and aren't "correct"
            std::list<uint> i_t; // correct

            std::set_intersection(known_correct.begin(), known_correct.end(),
                                  i->first.begin(), i->first.end(),
                                  std::insert_iterator<std::list<uint> >(i_t,i_t.begin()));

            // Add to the confusion matrix.
            cm.push_back(i->second, i->first.size(), i_t.size());
        }
        
        return cm;
    }

    // For some phenotype j, determine AUC, fp, tp, fn, tn, etc.
    auc_info calculate_statistic(uint j) const {
        return calculate_plots(j).summary(0.0);
    }


    // Assume we're in the correct directory and read the correct phenotype file
    // First two lines are comment.
    // Assumes the files are pre-sorted by sortall.pl (by column 2 descending).
    // column 1 is the gene, column 2 is the prediction score (higher is better).
    // Returns a list of binned genes (by score) and the total number of genes.
    std::pair<genes_score_list,size_t> read_candidates(uint j) const {
        ostringstream fn; fn << j;
        boost::filesystem::path filepath(fn.str());
        //cerr << "Opening file: '" << filepath << "'" << endl;

        if (!exists(filepath)) {
            //cerr << "Error: File '" << filepath << "' does not exist." << endl;
#ifdef RICE
            string err("rockerxx.h: read_candidates: file '");
            err += filepath.string() + "' does not exist";
            throw Rice::Exception(rb_eIOError, err.c_str());
#else
            throw;
#endif
        }

        genes_score_list res;

        // Open a filestream
        ifstream fin(filepath);

        // Ignore two header lines
        fin.ignore(500, '\n');
        fin.ignore(500, '\n');
        //cout << "Next character is: '" << fin.peek() << "'" << endl;

        genes_score genes_s;  // store set of genes and a score here.
        size_t num_genes = 0; // keep track of total number of genes in file.

        gene_score_iterator gsit(fin); // priming read

        genes_s.second = gsit->second;
        genes_s.first.insert(gsit->first);
        
        // Iterate through the gene-score pairs in the file
        while (++gsit != gene_score_iterator()) {
            gene_score gs(*gsit);

            // If we come upon a score change, insert the previous row/rows, and
            // start a new collection.
            if (genes_s.second != gs.second) {
                res.push_back(genes_s);
                genes_s.second = gs.second;
                genes_s.first.clear();
            }
            genes_s.first.insert(gs.first);

            ++num_genes;
        }
        // Insert anything that remains once the file is at its end.
        res.push_back(genes_s);

        fin.close();

        return make_pair(res,num_genes+1);
    }
    
    
protected:
    pqxx::connection c;
    map<uint, set<uint> >* known_;
    Fetcher fetch;
    Updater update;
};

#endif
