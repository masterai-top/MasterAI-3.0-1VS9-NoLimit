#include "InfoNode.hpp"

InfoNode::InfoNode() {
    regretNumber = 0;
    strategyNumber = 0;
    infoSet = "";
}

InfoNode::InfoNode(const std::set<std::string> validActions): mValidActions(validActions) {
    for (auto action : validActions) {
        regretSum.insert({action, 0.0});
        strategySum.insert({action, 0.0});
        strategy.insert({action, 0.0});
    }
    //
    regretNumber = 1;
    strategyNumber = 0;
    infoSet = "";
}

InfoNode::~InfoNode() {

}

std::unordered_map<std::string, double> InfoNode::getAverageStrategy() {
    std::unordered_map<std::string, double> avgStrategy;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        //
        float normSum = 0;
        for (auto action : mValidActions) {
            normSum += strategySum[action];
        }
        //
        int numActions = mValidActions.size();
        for (auto action : mValidActions) {
            if (normSum > 0) {
                avgStrategy[action] = strategySum[action] / normSum;
            } else {
                avgStrategy[action] = 1.0 / numActions;
            }
        }
    }
    //
    return avgStrategy;
}

std::unordered_map<std::string, double> InfoNode::getStrategy(const std::set<std::string> validActions) {
    std::unordered_map<std::string, double> avgStrategy;
    //
    if (true) {
        //std::unique_lock<std::shared_mutex> sm(regretMutex);
        //
        float normSum = 0;
        for (auto action : validActions) {
            strategy[action] = regretSum[action] > 0 ? regretSum[action] : 0;
            normSum += strategy[action];
        }
        //
        int numActions = validActions.size();
        for (auto action : validActions) {
            if (normSum > 0) {
                strategy[action] /= normSum;
            } else {
                strategy[action] = 1.0 / numActions;
            }
        }
        //
        for (auto iter = strategy.begin(); iter != strategy.end(); iter++) {
            avgStrategy.emplace((*iter).first, (*iter).second);
        }
    }
    //
    return avgStrategy;
}

std::unordered_map<std::string, double> InfoNode::getStrategy() {
    std::unordered_map<std::string, double> results;
    //
    if (true) {
        std::unique_lock<std::shared_mutex> sm(regretMutex);
        //
        float normSum = 0;
        for (auto action : mValidActions) {
            strategy[action] = regretSum[action] > 0 ? regretSum[action] : 0;
            normSum += strategy[action];
        }
        //
        int numActions = mValidActions.size();
        for (auto action : mValidActions) {
            if (normSum > 0) {
                strategy[action] /= normSum;
            } else {
                strategy[action] = 1.0 / numActions;
            }
        }
        //
        for (auto iter = strategy.begin(); iter != strategy.end(); iter++) {
            results.emplace(iter->first, iter->second);
        }
    }
    //
    return results;
}

std::unordered_map<std::string, double> InfoNode::getRegretSum() {
    std::unordered_map<std::string, double> results;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        for (auto iter = regretSum.begin(); iter != regretSum.end(); iter++) {
            results.emplace((*iter).first, (*iter).second);
        }
    }
    //
    return results;
}

std::unordered_map<std::string, double> InfoNode::getStrategySum() {
    std::unordered_map<std::string, double> results;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        for (auto iter = strategySum.begin(); iter != strategySum.end(); iter++) {
            results.emplace((*iter).first, (*iter).second);
        }
    }
    //
    return results;
}

std::set<std::string> InfoNode::getValidActions() {
    std::set<std::string> results;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        for (auto iter = mValidActions.begin(); iter != mValidActions.end(); iter++) {
            results.emplace(*iter);
        }
    }
    //
    return results;
}

int InfoNode::incRegretNumber() {
    return ++regretNumber;
}

int InfoNode::getRegretNumber() {
    int result = 0;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        result = regretNumber;
    }
    //
    return result;
}

int InfoNode::incStrategyNumber() {
    return ++strategyNumber;
}

int InfoNode::getStrategyNumber() {
    int result = 0;
    //
    if (true) {
        std::shared_lock<std::shared_mutex> sm(regretMutex);
        result = strategyNumber;
    }
    //
    return result;
}

std::string InfoNode::getInfoSet() {
    return infoSet;
}

int InfoNode::setInfoSet(std::string key) {
    infoSet = key;
    return 0;
}

int InfoNode::discount(double factor) {
    if (true) {
        std::unique_lock<std::shared_mutex> sm(regretMutex);
        //
        for (auto action : mValidActions) {
            regretSum.at(action) *= factor;
            strategySum.at(action) *= factor;
        }
    }
    //
    return 0;
}

int InfoNode::updateStrategy() {
    if (true) {
        std::unique_lock<std::shared_mutex> sLock(regretMutex);
        //
        std::random_device randDevice;
        std::mt19937 mt(randDevice());
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto strategy = getStrategy(mValidActions);
        for (auto map : strategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mt);
        strategySum.at(actions[action]) += 1;
        incStrategyNumber();
    }
    //
    return 0;
}

int InfoNode::incRegretSum(const std::unordered_map<std::string, double> &temp) {
    if (true) {
        std::unique_lock<std::shared_mutex> uLock(regretMutex);
        //
        for (auto iter = temp.begin(); iter != temp.end(); iter++) {
            auto first = (*iter).first;
            auto second = (*iter).second;
            auto found = regretSum.find(first);
            if (found != regretSum.end()) {
                regretSum[first] += second;
            }
        }
        //
        incRegretNumber();
    }
    return 0;
}

int InfoNode::incRegretSum2(const std::unordered_map<std::string, double> &temp) {
    if (true) {
        std::unique_lock<std::shared_mutex> uLock(regretMutex);
        //
        if (true) {
            for (auto iter = temp.begin(); iter != temp.end(); iter++) {
                auto first = (*iter).first;
                auto second = (*iter).second;
                auto found = regretSum.find(first);
                if (found != regretSum.end()) {
                    regretSum[first] += second;
                }
            }
            //
            incRegretNumber();
        }
        //
        auto found = infoSet.find_last_of("leaf");
        if (found == std::string::npos) {
            std::random_device randDevice;
            std::mt19937 mt(randDevice());
            //
            std::vector<std::string> actions;
            std::vector<double> probabilities;
            auto strategy = getStrategy(mValidActions);
            for (auto map : strategy) {
                actions.push_back(map.first);
                probabilities.push_back(map.second);
            }
            //
            std::discrete_distribution<> random_choice(probabilities.begin(), probabilities.end());
            auto action = random_choice(mt);
            strategySum.at(actions[action]) += 1;
            incStrategyNumber();
        }
    }
    return 0;
}
