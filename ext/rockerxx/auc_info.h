#ifndef AUC_INFO_H
# define AUC_INFO_H

#include <string>
#include <sstream>
#include <iostream>

typedef unsigned int  uint;


using std::ostringstream;
using std::string;
using std::ostream;

const string AUC_COLUMNS = "(experiment_id, \"column\", auc, pr_area, true_positives, false_positives, true_negatives, false_negatives, threshold)";

class auc_info {
public:
    float auc;
    float auprc;
    uint tp;
    uint fp;
    uint tn;
    uint fn;
    float threshold;

    // Constructor
    auc_info(float threshold_ = 0.0, float area_under_curve = 0, float area_under_pr_curve = 0, uint true_positives = 0, uint false_positives = 0, uint true_negatives = 0, uint false_negatives = 0)
    : auc(area_under_curve), auprc(area_under_pr_curve), tp(true_positives), fp(false_positives), tn(true_negatives), fn(false_negatives), threshold(threshold_) { }

    ~auc_info() { }

    // Convert to a portion of a SQL insertion (string)
    string to_s() const {
        ostringstream s;
        s << auc << ", " << auprc << ", " << tp << ", " << fp << ", " << tn << ", " << fn << ", " << threshold;
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