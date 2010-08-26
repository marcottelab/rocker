#ifndef AUC_INFO_H
# define AUC_INFO_H

#include <string>
#include <sstream>
#include <iostream>

typedef unsigned int  uint;


using std::ostringstream;
using std::string;
using std::ostream;

const string RESULTS_COLUMNS = "(experiment_id, \"column\", roc_area, pr_area, gene_count)";

class auc_info {
public:
    float roc_area;
    float pr_area;
    uint gene_count;

    // Constructor
    auc_info(float area_under_curve = 0, float area_under_pr_curve = 0, uint gene_count_ = 0)
    : roc_area(area_under_curve), pr_area(area_under_pr_curve), gene_count(gene_count_) { }

    ~auc_info() { }

    // Convert to a portion of a SQL insertion (string)
    string to_s() const {
        ostringstream s;
        s << roc_area << ", " << pr_area << ", " << gene_count;
        return s.str();
    }

    string entry(uint experiment_id, uint j) const {
        ostringstream s;
        s << '(' << experiment_id << ", " << j << ", " << to_s() << ')';
        return s.str();
    }

    
};

// Probably not necessary.
std::ostream& operator<<(std::ostream& out, const auc_info& rhs) {
    out << rhs.to_s();
    return out;
}

#endif