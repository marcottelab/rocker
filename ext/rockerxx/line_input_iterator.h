#ifndef LINE_INPUT_ITERATOR_H_
# define LINE_INPUT_ITERATOR_H_

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


#endif
