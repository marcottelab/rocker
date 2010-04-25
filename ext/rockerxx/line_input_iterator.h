
#include <iterator>
#include <istream>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <utility>
// #include <boost/lexical_cast.hpp>

// using boost::lexical_cast;

template <class StringT = std::string>
class LineInputIterator :
    public std::iterator<std::input_iterator_tag, StringT, std::ptrdiff_t, const StringT*, const StringT&>
{
public:
    typedef typename StringT::value_type char_type;
    typedef typename StringT::traits_type traits_type;
    typedef std::basic_istream<char_type, traits_type> istream_type;

    LineInputIterator() : is(NULL) { }
    LineInputIterator(istream_type& is): is(&is) { }

    const StringT& operator*() const { return value; }
    const StringT* operator->() const { return &value; }
    
    LineInputIterator<StringT>& operator++() {
        assert(is != NULL);
        if (is && !std::getline(*is, value)) {
            is = NULL;
        }
        return *this;
    }

    LineInputIterator<StringT> operator++(int) {
        LineInputIterator<StringT> prev(*this);
        ++*this;
        return prev;
    }

    bool operator!=(const LineInputIterator<StringT>& other) const {
        return is != other.is;
    }

    bool operator==(const LineInputIterator<StringT>& other) const {
        return !(*this != other);
    }

protected:
    istream_type* is;
    StringT value;
};


template <typename GeneT = unsigned int, typename ScoreT = double>
class GeneScoreIterator : public LineInputIterator<std::string> {
public:
    typedef typename std::pair<GeneT,ScoreT> pair_type;
    typedef typename std::string::value_type char_type;
    typedef typename std::string::traits_type traits_type;
    typedef std::basic_istream<char_type, traits_type> istream_type;

    GeneScoreIterator() : is(NULL) { }
    GeneScoreIterator(istream_type& is): is(&is) {
        ++*this; // Priming read.
    }

    GeneScoreIterator<GeneT,ScoreT>& operator++() {
        assert(is != NULL);

        if (is) {
            if (std::getline(*is, value)) {

                // Cast the contents of the string
                std::istringstream in(value, std::istringstream::in);
                in >> value_gene_score.first;
                in >> value_gene_score.second;
            } else {
                is = NULL;
            }
        }
        return *this;
    }

    GeneScoreIterator<GeneT,ScoreT> operator++(int) {
        GeneScoreIterator<std::string> prev(*this);
        ++*this;
        return prev;
    }

    // De-reference
    const pair_type operator*() const {
        return value_gene_score;
    }
    const pair_type* operator->() const {
        return &value_gene_score;
    }

    bool operator!=(const GeneScoreIterator<GeneT,ScoreT>& other) const {
        return is != other.is;
    }

    bool operator==(const GeneScoreIterator<GeneT,ScoreT>& other) const {
        return !(*this != other);
    }

    const std::string test_value() const { return value; }

protected:
    istream_type* is;
    std::string value;
    pair_type value_gene_score;
};
