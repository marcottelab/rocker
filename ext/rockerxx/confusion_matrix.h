#ifndef CONFUSION_MATRIX_H_
# define CONFUSION_MATRIX_H_

#include <vector>
#include <utility>

using std::vector;
using std::pair;
using std::make_pair;


typedef vector<float>                          rate_vec;
typedef vector<size_t>                         size_vec;


class confusion_matrix {
public:
    confusion_matrix(size_t total_bins, size_t total_genes_, size_t known_)
    : roc_area_(0.0)
    {
        // Create contingency table
        tp.reserve(total_bins+1);     tp.push_back(0);
        p.reserve(total_bins+1);       p.push_back(0);
        tn.reserve(total_bins+1);     tn.push_back(total_genes_ - known_);
        n.reserve(total_bins+1);       n.push_back(total_genes_);
        
        threshold[1.0] = 0;
    }

    void push_back(float bin_score, size_t predicted, size_t correctly_predicted) {
        threshold[bin_score] = p.size();

        size_t last_i = p.size() - 1, i = p.size();

        p.push_back(p[last_i] + predicted);
        n.push_back(n[last_i] - predicted);

        tp.push_back(tp[last_i] + correctly_predicted);
        tn.push_back(tn[last_i] - (predicted - correctly_predicted));

        float delta_tpr = tpr(i)  -  tpr(last_i);
        float delta_fpr = fpr(i)  -  fpr(last_i);

        roc_area_ += (tpr(last_i) + 0.5*delta_tpr) * delta_fpr;
    }

    // Given some bin 'i', what is the true positive rate? (sensitivity/recall)
    float tpr(size_t i) const {
        return tp[i] / (tp[i] + n[i] - tn[i] + 0.0);
    }

    // Given some bin 'i', what is the false positive rate? (fallout)
    float fpr(size_t i) const {
        return (p[i] - tp[i]) / (p[i] - tp[i] + tn[i] + 0.0);
    }

    float actual_precision(size_t i) const {
        return tp[i] / (float)(p[i]);
    }

    rate_vec tpr_axis() const {
        rate_vec x(p.size());
        for (uint i = 0; i < p.size(); ++i)
            x[i] = tpr(i);
        return x;
    }

    rate_vec fpr_axis() const {
        rate_vec x(p.size());
        for (uint i = 0; i < p.size(); ++i)
            x[i] = fpr(i);
        return x;
    }

    pair<rate_vec, float> precision_axis_and_area() const {
        rate_vec pre(p.size());

        float area = 0.0;

        int i = p.size() - 1;
        float max = pre[i] = actual_precision(i); --i;

        for (; i >= 0; --i) {
            max = pre[i] = std::max(max, actual_precision(i));
            area += (tpr(i+1) - tpr(i)) * max;
        }
        pre[0] = 1.0;

        return make_pair(pre, area);
    }

    rate_vec precision_axis() const {
        return precision_axis_and_area().first;
    }

    
    size_vec tp_axis() const { return tp; }
    size_vec p_axis() const { return p; }
    size_vec tn_axis() const { return tn; }
    size_vec n_axis() const { return n; }

    float roc_area() const { return roc_area_; }

    // Summary is what gets put into the database.
    auc_info summary(float thresh) const {
        size_t idx = 0;
        if (thresh == 0.0 || thresh == 1.0) idx = threshold.find(thresh)->second;
        else {
            // TODO: Find a more efficient data structure for this!
            for (map<float,size_t>::const_iterator it = threshold.begin(); it != threshold.end(); ++it)
                if (it->first >= thresh) idx = it->second;
        }

        // Only calculate this so we can get the precision.
        pair<rate_vec,float> tmp(precision_axis_and_area());

        auc_info a(thresh);
        a.tp = tp[idx];
        a.fp = p[idx] - tp[idx];
        a.tn = tn[idx];
        a.fn = n[idx] - tn[idx];
        a.auc = roc_area_;
        a.auprc = tmp.second;
        
        return a;
    }

    size_t bins() const {  return p.size() - 1; }
    size_t size() const {  return p.size();     }
    size_t genes() const { return n[0];         }
    size_t known() const { return n[0] - tn[0]; }
protected:
    map<float,size_t> threshold;
    size_vec tp, p, tn, n; // p = predictions, n = not predicted
    float roc_area_; // "AUC" for ROC (accumulator)
};

#endif
