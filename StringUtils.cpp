#include "StringUtils.hpp"

void StringUtils::split(const string &str, const char split, vector<string> &res) {
    if (str == "") {
        return;
    }
    string strs = str + split;
    size_t pos = strs.find(split);
    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(split);
    }
}

void StringUtils::split(const string &str, const string &splits, vector<string> &res) {
    if (str == "") {
        return;
    }
    string strs = str + splits;
    size_t pos = strs.find(splits);
    int step = splits.size();
    while (pos != strs.npos) {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos + step, strs.size());
        pos = strs.find(splits);
    }
}

string StringUtils::strcat(const std::vector<string> &data, const string &split) {
    string strs = "";
    if (data.empty()) {
        return strs;
    }
    //
    for (auto iter = data.begin(); iter != data.end(); iter++) {
        if (iter != data.begin()) {
            strs += split;
        }
        //
        strs += *iter;
    }
    //
    return strs;
}