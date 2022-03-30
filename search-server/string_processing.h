//Вставьте сюда своё решение из урока «‎Очередь запросов».‎
#pragma once
#include <iostream>

#include "document.h"

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first_(begin)
        , last_(end)
        , size_(distance(first_, last_)) {
    }
    Iterator begin() const {
        return first_;
    }
    Iterator end() const {
        return last_;
    }
    size_t size() const {
        return size_;
    }
private:
    Iterator first_, last_;
    size_t size_;
};

template <typename Iterator>
std::ostream& std::operator<<(ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

