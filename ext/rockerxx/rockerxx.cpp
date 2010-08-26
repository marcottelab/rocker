/* Rocker gem C++ extension
 * Part of crossval, in the Phenolog project
 * (C) John O. Woods, The Marcotte Lab, 2010
 *
 * Requires boost, boost_filesystem, rice (Ruby in C++ Extension), and of course
 * Ruby (1.8). Use at your own risk if you're not a lab member!
 *
 * To compile, run irb or script/console and do:
 *  require 'extconf'
 *
 * Then leave the console, and type:
 *  make
 *
 * You will then load the module from within the ruby shell using:
 *  require 'rocker'
 *
 * Instantiate using:
 *  Rocker.new("dbname=crossval_production username=youruser password=yourpass", 1, 167)
 *
 * (That connects to the specified PostgreSQL database and autoloads matrix 1
 *  data. Updates will be made to experiment 167. This is just an example.)
 */

// g++ -I/usr/include -I/usr/local/include -L/usr/lib -L/usr/local/lib -lpqxx -lboost_filesystem rocker.cpp -o rocker
#include <rice/Data_Type.hpp>
#include <rice/Array.hpp>
#include <rice/Hash.hpp>
#include <rice/Constructor.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
using std::cout;
using std::endl;
using namespace Rice;

#include "rockerxx.h"

//#include "line_input_iterator.h"

typedef LineInputIterator<std::string> line_input_iterator;


// CONVERSION FUNCTIONS: Allow non-basic types in C++ to be converted into Ruby
// objects for users to peruse.

template <>
Object to_ruby<rate_vec >(rate_vec const & d) {
    Array ary;
    for (rate_vec::const_iterator it = d.begin(); it != d.end(); ++it) {
        ary.push(*it);
    }
    return ary;
}


template <>
Object to_ruby<size_vec >(size_vec const & d) {
    Array ary;
    for (size_vec::const_iterator it = d.begin(); it != d.end(); ++it) {
        ary.push(*it);
    }
    return ary;
}

template <>
Object to_ruby<auc_info>(auc_info const & d) {
    Hash h;
    h[Symbol("roc_area")]  = to_ruby<float>(d.roc_area);
    h[Symbol("pr_area")]   = to_ruby<float>(d.pr_area);
    h[Symbol("gene_count")]= to_ruby<uint>(d.gene_count);

    return h;
}

template <>
Object to_ruby<confusion_matrix>(confusion_matrix const & d) {
    Hash h;
    h[Symbol("tp_axis")] = to_ruby<size_vec >(d.tp_axis());
    h[Symbol("p_axis")] = to_ruby<size_vec >(d.p_axis());
    h[Symbol("n_axis")] = to_ruby<size_vec >(d.n_axis());
    h[Symbol("tn_axis")] = to_ruby<size_vec >(d.tn_axis());
    
    h[Symbol("tpr_axis")] = to_ruby<rate_vec >(d.tpr_axis());
    h[Symbol("fpr_axis")] = to_ruby<rate_vec >(d.fpr_axis());
    pair<rate_vec, float> pre_and_pr_area = d.precision_axis_and_area();
    h[Symbol("precision_axis")] = to_ruby<rate_vec >(pre_and_pr_area.first);
    h[Symbol("roc_area")]  = to_ruby<float>(d.roc_area());
    h[Symbol("pr_area")]   = to_ruby<float>(pre_and_pr_area.second);

    return h;
}


template <>
Object to_ruby< map<uint,auc_info> >(map<uint,auc_info> const & d) {
    Hash h;
    for (map<uint,auc_info>::const_iterator it = d.begin(); it != d.end(); ++it) {
        h[to_ruby<uint>(it->first)] = to_ruby<auc_info>(it->second);
    }
    return h;
}



// INIT FUNCTIONS: For exposing C++ objects directly to Ruby.

extern "C"
void Init_rockerxx() {

    // Expose Rocker class to Ruby
    //database_string dbarg(DBNAME, USER, PASSWORD);
    Data_Type<Rocker> rb_cRockerxx =
            define_class<Rocker>("Rockerxx")
             .define_constructor(Constructor<Rocker,std::string,uint,uint>())
             .define_method("process_results", &Rocker::process_results)
             .define_method("acquire_results", &Rocker::acquire_results)
             .define_method("fetch_column", &Rocker::fetch_column, (Arg("j")))
             .define_method("calculate_plots", &Rocker::calculate_plots)
             .define_method("calculate_statistic",
                            &Rocker::calculate_statistic,
                            (Arg("j"), Arg("threshold") = (double)(0.0)))
             .define_method("read_candidates", &Rocker::read_candidates, (Arg("j")))
             .define_method("pr_area", &Rocker::pr_area)
             .define_method("roc_area", &Rocker::roc_area);

}

extern "C"
void Init_DatabaseString() {
    Data_Type<database_string> rb_cDatabaseString =
        define_class<database_string>("DatabaseString")
         .define_constructor(Constructor<database_string,std::string,std::string,std::string>())
         .define_method("to_s", &database_string::operator());
}

// Uncomment to test -- probably unnecessary now that the Ruby extensions are
// built.
/*
int main(int argc, char* argv[]) {
    database_string dbarg(DBNAME, USER, PASSWORD);
    Rocker rocker(dbarg(), 1, 167);

    uint j = 0, m = 0, x = 0;
    cout << "Args: " << argc << endl;
    if (argc < 4) {
        cerr << "Please provide matrix id, experiment id, and column as arguments." << endl;
        return EXIT_FAILURE;
    } else {
        // Read command line arguments.
        m = atoi(argv[1]);
        x = atoi(argv[2]);
        j = atoi(argv[3]);
    }

    set<uint> gene_ids = rocker.fetch(42);
    for (set<uint>::const_iterator i = gene_ids.begin(); i != gene_ids.end(); ++i)
        cout << *i << endl;

    cout << "---" << endl;
    gene_score_list candidates = rocker.read_candidates(42);
    for (gene_score_list::iterator i = candidates.begin(); i != candidates.end(); ++i)
        cout << "hi:\t" << i->first << '\t' << i->second << endl;

    cout << "---" << endl;
    auc_info test = rocker.calculate_statistic(42);
    cout << "auc\t=" << test.auc << endl;
    cout << " tp\t=" << test.tp << endl;
    cout << " fp\t=" << test.fp << endl;
    cout << " tn\t=" << test.tn << endl;
    cout << " fn\t=" << test.fn << endl;

    cerr << "m=" << m << endl;
    cerr << "x=" << x << endl;
    cerr << "j=" << j << endl;

    rocker.process_results();

    return EXIT_SUCCESS;
}
*/