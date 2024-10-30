#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <bitset>
#include <future>
#include <random>
#include "Log.hpp"
//
using namespace std;

//
template<typename Numeric, typename Generator = std::mt19937>
Numeric random(Numeric from, Numeric to) {
    thread_local static Generator gen(std::random_device{}());
    using dist_type = typename std::conditional<std::is_integral<Numeric>::value,
          std::uniform_int_distribution<Numeric>,
          std::uniform_real_distribution<Numeric>>::type;
    thread_local static dist_type dist;
    return dist(gen, typename dist_type::param_type{from, to});
}
//
class StringUtils {
public:
    //
    static void split(const string &str, const char split, vector<string> &res);
    //
    static void split(const string &str, const string &splits, vector<string> &res);
    //
    static string strcat(const std::vector<string> &data, const string &split);
};

#endif
