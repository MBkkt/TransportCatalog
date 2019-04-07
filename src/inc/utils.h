#ifndef __UTILS_H__
#define __UTILS_H__
#pragma once

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

template<typename It>
class Range {
public:
    using ValueType = typename std::iterator_traits<It>::value_type;

    Range(It begin, It end): begin_(begin), end_(end) {
    }

    It begin() const {
        return begin_;
    }

    It end() const {
        return end_;
    }

private:
    It begin_;
    It end_;
};

template<typename C>
auto asRange(C const & container) {
    return Range {std::begin(container), std::end(container)};
}

template<typename It>
size_t computeUniqueItemsCount(Range<It> range) {
    return std::unordered_set<typename Range<It>::ValueType> {
        range.begin(), range.end()
    }.size();
}

template<typename K, typename V>
V const * getValuePointer(std::unordered_map<K, V> const & map, K const & key) {
    if (auto it = map.find(key); it != end(map)) {
        return &it->second;
    } else {
        return nullptr;
    }
}

std::string_view strip(std::string_view line);

#endif /* __UTILS_H__ */
