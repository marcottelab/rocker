#ifndef AUC_INFO_H
# define AUC_INFO_H

#include <string>
#include <sstream>
#include <iostream>

typedef unsigned int  uint;


using std::ostringstream;
using std::string;
using std::ostream;

const string AUC_COLUMNS = "(experiment_id, \"column\", auc, true_positives, false_positives, true_negatives, false_negatives)";

class auc_info {
public:
    double auc;
    uint tp;
    uint fp;
    uint tn;
    uint fn;

    // Constructor
    auc_info(double area_under_curve = 0, uint true_positives = 0, uint false_positives = 0, uint true_negatives = 0, uint false_negatives = 0)
    : auc(area_under_curve), tp(true_positives), fp(false_positives), tn(true_negatives), fn(false_negatives) { }

    ~auc_info() { }

    // Convert to a portion of a SQL insertion (string)
    string to_s() const {
        ostringstream s;
        s << auc << ", " << tp << ", " << fp << ", " << tn << ", " << fn;
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