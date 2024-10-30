#include "Pluribus.hpp"
#include "Log.hpp"
#include "Game.hpp"
#include "GamePool.hpp"
#include "Configure.hpp"

//
bool Pluribus::isRunning = true;
//
Pluribus::Pluribus(): mEngine(nullptr),
    mNumPlayers(Configure::GetInstance().mNumPlayers),
    mGameState(nullptr) {
    //
    std::random_device mRd;
    randseed = mRd();
    //
    if (true) {
        mNumPlayers = Configure::GetInstance().mNumPlayers;
        mRegretMinimum = Configure::GetInstance().mMinRegret;
        mStrategyInterval = Configure::GetInstance().mStrategyInterval;
        mPruneThreshold = Configure::GetInstance().mPruneThreshold;
        mDiscountInterval = Configure::GetInstance().mDiscountInterval;
        mLCFRThreshold =  Configure::GetInstance().mCFRThreshold;
        mSerializeInterval = Configure::GetInstance().mSerializeInterval;
        mMaxIterations = Configure::GetInstance().mMaxIterations;
    }
    //
    for (auto i = 0; i < AGENT_MAX_NUM; i++) {
        mAgentBuckets[i] = nullptr;
    }
}

Pluribus::~Pluribus() {
    if (nullptr != mEngine) {
        delete mEngine;
        mEngine = nullptr;
    }
    //
    for (auto i = 0; i < AGENT_MAX_NUM; i++) {
        delete mAgentBuckets[i];
        mAgentBuckets[i] = nullptr;
    }
}

void Pluribus::resetEng() {
    mActionEng = std::mt19937(mRd());
}

void Pluribus::print(bool isPrint) {

}

void Pluribus::trainChoseBlueprint(State &state, int player) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::valarray<float> nodeLeafUtil(UNLEAGLE, 1326 * state.mNumPlayers);
    std::valarray<std::unordered_map<std::string, double>> utilitiesLeaf(1326);
    std::valarray<int> validLeafIndexs(1, 1326);
    std::valarray<float> returned;
    std::set<std::string> mLeafActions;
    mLeafActions.emplace("NULL");
    mLeafActions.emplace("FOLD");
    mLeafActions.emplace("CALL");
    mLeafActions.emplace("RAISE");
    auto validActions = mLeafActions;
    //
    auto current = state.getTurnIndex();
    int firstAction = 1;
    int actionIndex = 0;
    auto target = state.getEngine()->getPlayers(player);
    auto card1 = *(target->getHand()[0]);
    auto card2 = *(target->getHand()[1]);
    for (auto action : validActions) {
        if (true) {
            State st(state);
            //
            std::vector<int> biasOther(6, 0);
            biasOther[player] = actionIndex;
            //
            returned = rollout(st, player, biasOther);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto legal = true;
                if (returned[index * 6] == UNLEAGLE) {
                    if (validLeafIndexs[index] == 1) {
                        if (firstAction != 1) {
                            for (int z = 0; z < 6; z++) {
                                nodeLeafUtil[index * 6 + z] = UNLEAGLE;
                            }
                        }
                        //
                        validLeafIndexs[index] = 0 ;
                    }
                    //
                    legal = false;
                } else {
                    if (validLeafIndexs[index] == 0) {
                        legal = false;
                    }
                }
                //
                if (!legal) {
                    continue;
                }
                //
                target->getHand()[0]->setCard(Card::fastId2str(i));
                target->getHand()[1]->setCard(Card::fastId2str(j));
                //
                auto infoSet = state.infoSetdepthLimit(player) + "leaf";
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                utilitiesLeaf[index][action] = returned[index * 6 + current];
                //
                auto validStrategy = node->getStrategy();
                auto validValue = validStrategy.at(action);
                for (int k = 0; k < 6; k++) {
                    if (firstAction == 1) {
                        nodeLeafUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                    } else {
                        nodeLeafUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                    }
                }
            }
        }
        //
        firstAction = 0;
        actionIndex += 1;
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j);
            auto invalid = false;
            for (auto action : validActions) {
                auto um = &utilitiesLeaf[index];
                if (um->find(action) == um->end() || um->at(action) == UNLEAGLE) {
                    invalid = true;
                    break;
                }
            }
            //
            if (invalid) {
                continue;
            }
            //
            target->getHand()[0]->setCard(Card::fastId2str(i));
            target->getHand()[1]->setCard(Card::fastId2str(j));
            //
            auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
            auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
            //
            // float maxprob = 0.000000000000001;
            // float allprob = 0;
            // auto ranges = &state.handRanges[player];
            // for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
            //     if (iter->second > maxprob) {
            //         maxprob = iter->second;
            //         allprob += iter->second;
            //     }
            // }
            // //
            // float nowprob = 0;
            // auto find = ranges->find(comb1);
            // if (find != ranges->end()) {
            //     nowprob = find->second;
            // }
            // //
            // find = ranges->find(comb2);
            // if (find != ranges->end()) {
            //     nowprob = find->second;
            // }
            //
            auto infoSet = state.infoSetdepthLimit(player) + "leaf";
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = utilitiesLeaf[index].at(action) - nodeLeafUtil[index * 6 + current];
                // temp.emplace(action, regret * nowprob / maxprob);
                temp.emplace(action, regret);
            }
            //
            node->incRegretSum(temp);
        }
    }
    //
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    LOG_DEBUG("@trainChoseBluePrint: Player" << player << ", duration=" << duration / pow(10, 6));
}

void Pluribus::train(int iterations) {
    auto tt1 = std::chrono::high_resolution_clock::now();
    //
    std::random_device rd;
    std::mt19937 randEng(rd());
    //
    for (int i = 1; i <= iterations; i++) {
        //
        if (!isRunning) {
            break;
        }
        //
        LOG_DEBUG("@iterations=" << i << "begin...");
        //
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int player = 0; player < mNumPlayers; player++) {
            if (true) {
                State state(mNumPlayers);
                auto t1 = std::chrono::high_resolution_clock::now();
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        mccfr2(state, player, i, false, 0);
                    else
                        mccfr2(state, player, i, true, 0);
                } else {
                    mccfr2(state, player, i, false, 0);
                }
                //
                auto t2 = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
                LOG_DEBUG("@Calculate, Player" << player << ", duration=" << duration / pow(10, 6));
            }
        }
        //
        if (i % mStrategyInterval == 0) {
            auto t1 = std::chrono::high_resolution_clock::now();
            agentStrategy();
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            LOG_INFO("@UpdateStrategy: iter=" << i << ", interval=" << mStrategyInterval
                     << ", duration=" << (duration / pow(10, 6)));
        }
        //
        auto t3 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t0).count();
        LOG_INFO("@iterations=" << i << " duration=" << duration / pow(10, 6));
        //
        if ((i < mLCFRThreshold) && (i % mDiscountInterval == 0)) {
            auto t1 = std::chrono::high_resolution_clock::now();
            LOG_INFO("@Discount begin");
            //
            float factor = (i / mDiscountInterval) / ((i / mDiscountInterval) + 1.);
            agentDiscount(factor);
            //
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            LOG_INFO("@Discount over: factor=" << factor << ", duration=" << duration / pow(10, 6));
        }
        //
        if (0 == (i % 500)) {
            GamePool::GetInstance().status();
        }
    }
    //
    auto tt2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
    LOG_INFO("train duration " << duration / pow(10, 6));
}

void Pluribus::trainConsume(int i) {
    auto tt1 = std::chrono::high_resolution_clock::now();
    for (int player = 0; player < mNumPlayers; player++) {
        if (true) {
            State state = State(mNumPlayers);
            auto t1 = std::chrono::high_resolution_clock::now();
            if (false) {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        mccfr2(state, player, i, false, 0);
                    else
                        mccfr2(state, player, i, true, 0);
                } else {
                    mccfr2(state, player, i, false, 0);
                }
            } else {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        mccfr4(state, player, i, false, 0);
                    else
                        mccfr4(state, player, i, true, 0);
                } else {
                    mccfr4(state, player, i, false, 0);
                }
            }
            //
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            LOG_DEBUG("@Calculate, Player" << player << ", duration=" << duration / pow(10, 6));
        }
    }
    //
    auto tt2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
    LOG_DEBUG("@Consume i=" << i << ", duration " << duration / pow(10, 6));
}

void Pluribus::searchConsume(int i) {
    if (nullptr == mGameState) {
        LOG_ERROR("mGameState is nullptr");
        return;
    }
    //
    if (nullptr == mGameState->getEngine()) {
        LOG_ERROR("train task failed: i=" << i);
        return;
    }
    //
    int useMCCFR = Configure::GetInstance().mResearchUseCFR;
    //
    auto tt1 = std::chrono::high_resolution_clock::now();
    for (int player = 0; player < mNumPlayers; player++) {
        if (mGameState->getEngine()->getPlayers(player)->isFold())
            continue;
        //
        if (true) {
            // Current game state
            State state = State(mGameState, false);
            // Deal cards by hand probability
            //int ret = Evaluate::dealCards3(i, mGameState->boardCards, mGameState->handRanges, state.getEngine());
            int ret = Evaluate::dealCards(i, mGameState->boardCards, mGameState->handRanges, state.getEngine());
            if (ret < 0) {
                LOG_ERROR("Evaluate::dealCards() failed: i=" << i << ", ret=" << ret);
                return;
            }
            //
            auto t1 = std::chrono::high_resolution_clock::now();
            //
            if (1 == useMCCFR) {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        subgameSolve(state, player, i, false);
                    else
                        subgameSolve(state, player, i, true);
                } else {
                    subgameSolve(state, player, i, false);
                }
            } else if (2 == useMCCFR) {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        subgameSolve2(state, player, i, false);
                    else
                        subgameSolve2(state, player, i, true);
                } else {
                    subgameSolve2(state, player, i, false);
                }
            } else if (3 == useMCCFR) {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        subgameSolve3(state, player, i, false, 0);
                    else
                        subgameSolve3(state, player, i, true, 0);
                } else {
                    subgameSolve3(state, player, i, false, 0);
                }
            } else if (4 == useMCCFR) {
                if (i > mPruneThreshold) {
                    float prune = (float) rand() / (float)RAND_MAX;
                    if (prune < 0.05)
                        subgameSolve4(state, player, i, false, 0);
                    else
                        subgameSolve4(state, player, i, true, 0);
                } else {
                    subgameSolve4(state, player, i, false, 0);
                }
            } else if (5 == useMCCFR) {
                std::vector<float> reachProbs(6, 1.0);
                lcfr(state, player, i, 0, reachProbs);
            }
            //
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            LOG_DEBUG("@Calculate, Player" << player << ", duration=" << duration / pow(10, 6));
        }
    }
    //
    auto tt2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
    LOG_DEBUG("@Consume i=" << i << ", duration " << duration / pow(10, 6));
}

void Pluribus::sample(State &state, int blueprint) {
    while (!state.isTerminal()) {
        auto isRandom = true;
        auto infoSet = state.infoSet();
        auto stage = atoi(infoSet.substr(0, 1).c_str());
        auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
        if (node != nullptr) {
            auto strategySum = node->getStrategySum();
            if (strategySum.empty()) {
                strategySum = state.initialRegret();
            }
            //if (!strategySum.empty()) {
            if (true) {
                std::vector<double> probs;
                std::vector<string> actions;
                for (auto kv : strategySum) {
                    auto key = kv.first;
                    auto val = (kv.second > 0.0) ? kv.second : 0.0;
                    //
                    if (blueprint == 1) {
                        if ("FOLD" == key)
                            val *= 10.0;
                    } else if (blueprint == 2) {
                        if ("CHECK" == key)
                            val *= 10.0;
                        else if ("CALL" == key)
                            val *= 10.0;
                    } else if (blueprint == 3) {
                        if (("ALLIN" != key) && ("FOLD" != key) && ("CHECK" != key) && ("CALL" != key))
                            val *= 10.0;
                    }
                    //
                    actions.push_back(key);
                    probs.push_back(val);
                }
                //
                auto sumProbs = std::accumulate(probs.begin(), probs.end(), 0.0);
                for (int v = 0; v < probs.size(); v++) {
                    probs[v] /= sumProbs;
                }
                //
                isRandom = false;
                std::discrete_distribution<int> random_choice(probs.begin(), probs.end());
                auto action = actions[random_choice(mActionEng)];
                state.Command(action, false);
                LOG_DEBUG("DepthLimit(1): infoSet=" << infoSet << ", action=" << action);
            }
        }
        //
        if (isRandom) {
            auto actions = state.validActions();
            auto it(actions.begin());
            advance(it, rand() % actions.size());
            state.Command(*it, false);
            LOG_DEBUG("DepthLimit(2): infoSet=" << infoSet << ", action=" << *it);
        }
    }
}

std::unordered_map<std::string, std::valarray<float>> Pluribus::samplePayoff(State &state, int player) {
    std::unordered_map<std::string, std::valarray<float>> avgResults;
    //
    for (auto i = 0; i < 4; i++) {
        if (true) {
            State st(state);
            sample(st, i);
            //
            auto results = st.payoffMatrix2(player);
            for (auto iter = results.begin(); iter != results.end(); iter++) {
                auto found = avgResults.find(iter->first);
                if (found == avgResults.end()) {
                    avgResults.emplace(iter->first, iter->second);
                } else {
                    auto sz = iter->second.size();
                    for (auto p = 0; p < sz; p++) {
                        found->second[p] += iter->second[p];
                    }
                }
            }
        }
    }
    //
    for (auto iter = avgResults.begin(); iter != avgResults.end(); iter++) {
        auto sz = iter->second.size();
        for (auto p = 0; p < sz; p++) {
            iter->second[p] /= 4;
        }
    }
    //
    return avgResults;
}

std::valarray<float> Pluribus::samplePayoff2(State &state , int player) {
    std::valarray<float> avg(UNLEAGLE, 1326 * mNumPlayers);
    //
    for (auto s = 0; s < 4; s++) {
        if (true) {
            State st(state);
            sample(st, s);
            //
            auto results = st.payoffMatrix(player);
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j) * mNumPlayers;
                    if (results[index] == UNLEAGLE) {
                        if (s != 0) {
                            for (int p = 0; p < mNumPlayers; p++) {
                                avg[index + p] = UNLEAGLE;
                            }
                        }
                    } else {
                        if (s == 0) {
                            for (int p = 0; p < mNumPlayers; p++) {
                                avg[index + p] = results[index + p];
                            }
                        } else {
                            if (avg[index] != UNLEAGLE) {
                                for (int p = 0; p < mNumPlayers; p++) {
                                    avg[index + p] += results[index + p];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j) * mNumPlayers;
            auto skip = false;
            for (int p = 0; p < mNumPlayers; p++) {
                if (UNLEAGLE == avg[index + p]) {
                    skip = true;
                    break;
                }
            }
            //
            if (skip) {
                continue;
            }
            //
            for (int p = 0; p < mNumPlayers; p++) {
                avg[index + p] /= 4;
            }
        }
    }
    //
    return avg;
}

std::valarray<float> Pluribus::samplePayoff3(State &state , int player) {
    std::valarray<float> avg(UNLEAGLE, 1326 * mNumPlayers);
    std::unordered_map<std::string, std::size_t> holeCards;
    //
    auto blueprintNum = 4;
    for (auto i = 0; i < blueprintNum; i++) {
        if (true) {
            State st(state);
            sample(st, i);
            //
            auto results = st.payoffMatrix(player);
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j) * mNumPlayers;
                    auto skip = false;
                    for (int p = 0; p < mNumPlayers; p++) {
                        if (UNLEAGLE == results[index + p]) {
                            skip = true;
                            break;
                        }
                    }
                    //
                    if (skip) {
                        continue;
                    }
                    //
                    if (UNLEAGLE == avg[index + 0]) {
                        for (int p = 0; p < mNumPlayers; p++) {
                            avg[index + p] = results[index + p];
                        }
                    } else {
                        for (int p = 0; p < mNumPlayers; p++) {
                            avg[index + p] += results[index + p];
                        }
                    }
                    //
                    auto combo = Card::fastId2str(j) + Card::fastId2str(i);
                    auto found = holeCards.find(combo);
                    if (found == holeCards.end()) {
                        holeCards.emplace(combo, 1);
                    } else {
                        found->second++;
                    }
                }
            }
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j) * mNumPlayers;
            auto combo = Card::fastId2str(j) + Card::fastId2str(i);
            auto found = holeCards.find(combo);
            if (found == holeCards.end() || found->second < blueprintNum) {
                // Restore initial data
                for (int p = 0; p < mNumPlayers; p++) {
                    avg[index + p] = UNLEAGLE;
                }
                //
                continue;
            }
            //
            bool skip = false;
            for (int p = 0; p < mNumPlayers; p++) {
                if (UNLEAGLE == avg[index + p]) {
                    skip = true;
                    break;
                }
            }
            //
            if (skip) {
                // invalid data
                continue;
            }
            //
            for (int p = 0; p < mNumPlayers; p++) {
                avg[index + p] /= blueprintNum;
            }
        }
    }
    //
    return avg;
}

std::vector<std::size_t> Pluribus::searchBoardCards(State &state) {
    std::vector<std::size_t> knownCards;
    //
    auto pEngine = state.getEngine();
    //
    if (pEngine->getRound() > GAME_ROUND_PREFLOP) {
        knownCards.push_back(Card::str2fastId(pEngine->getFlopCard()[0]->strCluster()));
        knownCards.push_back(Card::str2fastId(pEngine->getFlopCard()[1]->strCluster()));
        knownCards.push_back(Card::str2fastId(pEngine->getFlopCard()[2]->strCluster()));
    }
    //
    if (pEngine->getRound() > GAME_ROUND_FLOP) {
        knownCards.push_back(Card::str2fastId(pEngine->getTurnCard()->strCluster()));
    }
    //
    if (pEngine->getRound() > GAME_ROUND_TURN) {
        knownCards.push_back(Card::str2fastId(pEngine->getRiverCard()->strCluster()));
    }
    //
    return knownCards;
}

std::valarray<float> Pluribus::mccfr1(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoff();
    }
    //Number of effective hands
    auto infoSet = state.infoSet();
    auto agent = getAgent(state.getRound(), infoSet);
    if (nullptr == agent) {
        LOG_ERROR("invalid agent: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    //
    auto node = agent->GetNode(infoSet, validActions);
    if (nullptr == node) {
        LOG_ERROR("invalid node: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validStrategy = node->getStrategy();
    auto validRegret = node->getRegretSum();
    // Depth finite search
    if (Configure::GetInstance().mResearchDepth > depth) {
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        state.Command(actions[action], false);
        return mccfr1(state, player, iter, prune, depth + 1);
    }
    //
    if (current == player) {
        std::valarray<float> nodeUtil(state.mNumPlayers);
        std::unordered_map<std::string, double> utilities;
        std::valarray<float> returned;
        float regret = 0.0;
        //
        if (prune) {
            std::set<std::string> explored;
            for (auto action : validActions) {
                if (validRegret.at(action) > mRegretMinimum) {
                    if (true) {
                        State st(state, action);
                        returned = mccfr1(st, player, iter, prune, depth + 1);
                    }
                    //
                    utilities[action] = returned[current];
                    nodeUtil += returned * (float)validStrategy.at(action);
                    explored.insert(action);
                }
            }
            //
            if (true) {
                std::unordered_map<std::string, double> temp;
                //
                for (auto action : validActions) {
                    auto search = explored.find(action);
                    if (search != explored.end()) {
                        regret = utilities.at(action) - nodeUtil[current];
                        temp.emplace(action, regret);
                    }
                }
                //
                node->incRegretSum(temp);
            }
        } else {
            for (auto action : validActions) {
                if (true) {
                    State st(state, action);
                    returned = mccfr1(st, player, iter, prune, depth + 1);
                }
                //
                utilities[action] = returned[current];
                nodeUtil += returned * (float)validStrategy.at(action);
            }
            //
            if (true) {
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    regret = utilities.at(action) - nodeUtil[current];
                    temp.emplace(action, regret);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        return nodeUtil;
    } else {
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr1(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr2(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoff();
    }
    //
    auto infoSet = state.infoSet();
    auto agent = getAgent(state.getRound(), infoSet);
    if (nullptr == agent) {
        LOG_ERROR("invalid agent: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    //
    auto node = agent->GetNode(infoSet, validActions);
    if (nullptr == node) {
        LOG_ERROR("invalid node: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validStrategy = node->getStrategy();
    auto validRegret = node->getRegretSum();
    //
    if (current == player) {
        std::valarray<float> nodeUtil(state.mNumPlayers);
        std::unordered_map<std::string, double> utilities;
        std::valarray<float> returned;
        float regret = 0.0;
        //
        if (prune) {
            std::set<std::string> explored;
            for (auto action : validActions) {
                if (validRegret.at(action) > mRegretMinimum) {
                    if (true) {
                        State st(state, action);
                        returned = mccfr2(st, player, iter, prune, depth + 1);
                    }
                    //
                    utilities[action] = returned[current];
                    nodeUtil += returned * (float)validStrategy.at(action);
                    explored.insert(action);
                }
            }
            //
            if (true) {
                std::unordered_map<std::string, double> temp;
                //
                for (auto action : validActions) {
                    auto search = explored.find(action);
                    if (search != explored.end()) {
                        regret = utilities.at(action) - nodeUtil[current];
                        temp.emplace(action, regret);
                    }
                }
                //
                node->incRegretSum(temp);
            }
        } else {
            for (auto action : validActions) {
                if (true) {
                    State st(state, action);
                    returned = mccfr2(st, player, iter, prune, depth + 1);
                }
                //
                utilities[action] = returned[current];
                nodeUtil += returned * (float)validStrategy.at(action);
            }
            //
            if (true) {
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    regret = utilities.at(action) - nodeUtil[current];
                    temp.emplace(action, regret);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        return nodeUtil;
    } else {
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr2(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr3(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    auto infoSet = state.infoSet();
    auto agent = getAgent(state.getRound(), infoSet);
    if (nullptr == agent) {
        LOG_ERROR("invalid agent: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    //
    auto node = agent->GetNode(infoSet, validActions);
    if (nullptr == node) {
        LOG_ERROR("invalid node: infoSet=" << infoSet);
        exit(-1);
    }
    //
    auto validStrategy = node->getStrategy();
    auto validRegret = node->getRegretSum();
    auto target = state.getEngine()->getPlayers(player);
    //
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        //
        int firstAction = 1;
        int actionIndex = 0;
        //auto last = State(state);
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        std::vector<std::size_t> boardCards;
        for (auto action : validActions) {
            boardCards.clear();
            //
            if (true) {
                State st(state, action);
                returned = mccfr3(st, player, iter, prune, depth + 1);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0 ;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (legal) {
                        //
                        target->getHand()[0]->setCard(Card::fastId2str(i));
                        target->getHand()[1]->setCard(Card::fastId2str(j));
                        //
                        infoSet = state.infoSet();
                        auto agent = getAgent(state.getRound(), infoSet);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSet, validActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        utilities[index][action] = returned[index * 6 + current];
                        //
                        auto validStrategy = node->getStrategy();
                        auto validValue = validStrategy.at(action);
                        for (int k = 0; k < 6; k++) {
                            if (firstAction == 1) {
                                nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                            } else {
                                nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                            }
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand()[0]->setCard(&card1);
            target->getHand()[1]->setCard(&card2);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto invalid = false;
                for (auto action : validActions) {
                    if ( utilities[index].find(action) == utilities[index].end() || utilities[index].at(action) == UNLEAGLE) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                target->getHand()[0]->setCard(Card::fastId2str(i));
                target->getHand()[1]->setCard(Card::fastId2str(j));
                //
                string comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                string comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto iter = state.handRanges[player].begin();
                for (; iter != state.handRanges[player].end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find = state.handRanges[player].find(comb1);
                if (find !=  state.handRanges[player].end()) {
                    nowprob = find->second;
                }
                //
                find = state.handRanges[player].find(comb2);
                if (find !=  state.handRanges[player].end()) {
                    nowprob = find->second;
                }
                //
                infoSet = state.infoSet();
                //
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        return nodeUtil;
    } else {
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr3(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr4(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff3(state, player);
        }
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                returned = mccfr4(st, player, iter, prune, depth + 1);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0 ;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (legal) {
                        target->getHand()[0]->setCard(Card::fastId2str(i));
                        target->getHand()[1]->setCard(Card::fastId2str(j));
                        //
                        auto infoSet = state.infoSet();
                        auto agent = getAgent(state.getRound(), infoSet);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSet, validActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        utilities[index][action] = returned[index * 6 + current];
                        //
                        auto validStrategy = node->getStrategy();
                        auto validValue = validStrategy.at(action);
                        for (int k = 0; k < 6; k++) {
                            if (firstAction == 1) {
                                nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                            } else {
                                nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                            }
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand()[0]->setCard(&card1);
            target->getHand()[1]->setCard(&card2);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto invalid = false;
                for (auto action : validActions) {
                    if (utilities[index].find(action) == utilities[index].end() || utilities[index].at(action) == UNLEAGLE) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                target->getHand()[0]->setCard(Card::fastId2str(i));
                target->getHand()[1]->setCard(Card::fastId2str(j));
                //
                string comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                string comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto iter = state.handRanges[player].begin();
                for (; iter != state.handRanges[player].end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find = state.handRanges[player].find(comb1);
                if (find !=  state.handRanges[player].end()) {
                    nowprob = find->second;
                }
                //
                find = state.handRanges[player].find(comb2);
                if (find !=  state.handRanges[player].end()) {
                    nowprob = find->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node ) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr4(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr5(State &state, int iter, int depth) {
    auto actions = state.validActions();
    auto player = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    //
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff3(state, player);
        }
    }
    //
    std::valarray<std::unordered_map<std::string, double>> utilities(1326);
    std::valarray<float> returned;
    //
    auto card1 = *target->getHand(0);
    auto card2 = *target->getHand(1);
    //
    std::unordered_map<std::string, std::size_t> kvStat;
    for (auto action : actions) {
        if (true) {
            State st(state, action);
            returned = mccfr5(st, iter, depth + 1);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                if (returned[index * 6] == UNLEAGLE)
                    continue;
                //
                utilities[index][action] = returned[index * 6 + player];
                //
                auto comb = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                auto found = kvStat.find(comb);
                if (found == kvStat.end())
                    kvStat.emplace(comb, 1);
                else
                    found->second++;
            }
        }
    }
    //
    std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j);
            auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
            auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
            //
            auto found = kvStat.find(comb2);
            if (found == kvStat.end() || found->second != actions.size()) {
                continue;
            }
            //
            target->getHand(0)->setCard(Card::fastId2str(i));
            target->getHand(1)->setCard(Card::fastId2str(j));
            //
            float maxProb = 0.000000000000001;
            float allProb = 0;
            auto ranges = &state.handRanges[player];
            for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                if (iter->second > maxProb) {
                    maxProb = iter->second;
                    allProb += iter->second;
                }
            }
            //
            float nowProb = 0;
            auto find1 = state.handRanges[player].find(comb1);
            if (find1 != state.handRanges[player].end()) {
                nowProb = find1->second;
            }
            //
            auto find2 = state.handRanges[player].find(comb2);
            if (find2 != state.handRanges[player].end()) {
                nowProb = find2->second;
            }
            //
            auto infoSet = state.infoSet();
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, actions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto validStrategy = node->getStrategy();
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : actions) {
                auto prob = validStrategy.at(action);
                for (int p = 0; p < 6; p++) {
                    if (nodeUtil[index * 6] == UNLEAGLE) {
                        nodeUtil[index * 6 + p] = returned[index * 6 + p] * prob;
                    } else {
                        nodeUtil[index * 6 + p] += returned[index * 6 + p] * prob;
                    }
                }
                //
                auto regret = utilities[index].at(action) - nodeUtil[index * 6 + player];
                temp.emplace(action, regret * nowProb / maxProb);
            }
            //
            node->incRegretSum(temp);
        }
    }
    //
    target->getHand(0)->setCard(&card1);
    target->getHand(1)->setCard(&card2);
    //
    return nodeUtil;
}

std::unordered_map<std::string, std::valarray<float>> Pluribus::mccfr6(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix2(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff(state, player);
        }
    }
    //
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::unordered_map<std::string, std::valarray<float>> nodeUtil;
        std::unordered_map<std::string, std::valarray<float>> returned;
        std::unordered_map<std::string, std::unordered_map<std::string, double>> utilities;
        //
        auto card1 = *(target->getHand(0));
        auto card2 = *(target->getHand(1));
        //
        auto validActions = state.validActions();
        for (auto action : validActions) {
            //
            if (true) {
                State st(state, action);
                returned = mccfr6(st, player, iter, prune, depth + 1);
            }
            //
            for (auto iter = returned.begin(); iter != returned.end(); iter++) {
                std::vector<std::string> vHands;
                StringUtils::split(iter->first, "-", vHands);
                //
                target->getHand(0)->setCard(vHands[0]);
                target->getHand(1)->setCard(vHands[1]);
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto strategy = node->getStrategy();
                auto values = iter->second * strategy.at(action);
                nodeUtil.emplace(iter->first, values);
                //
                auto param1 = iter->second[current];
                auto found1 = utilities.find(iter->first);
                if (found1 == utilities.end()) {
                    std::unordered_map<std::string, double> mp;
                    mp.emplace(action, param1);
                    utilities.emplace(iter->first, mp);
                } else {
                    found1->second.emplace(action, param1);
                }
            }
            //
            target->getHand(0)->setCard(&card1);
            target->getHand(1)->setCard(&card2);
        }
        //
        auto validActionNum = validActions.size();
        for (auto iter = utilities.begin(); iter != utilities.end(); iter++) {
            auto first = iter->first;
            auto second = iter->second;
            if (second.size() != validActionNum)
                continue;
            //
            std::vector<std::string> vHands;
            StringUtils::split(first, "-", vHands);
            //
            target->getHand(0)->setCard(vHands[0]);
            target->getHand(1)->setCard(vHands[1]);
            //
            float maxProb = 0.000000000000001;
            auto ranges = &state.handRanges[player];
            for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                if (iter->second > maxProb) {
                    maxProb = iter->second;
                }
            }
            //
            float nowProb = 0;
            auto comb1 = vHands[0] + "-" + vHands[1];
            auto find1 = ranges->find(comb1);
            if (find1 != ranges->end()) {
                nowProb = find1->second;
            }
            //
            auto comb2 = vHands[1] + "-" + vHands[0];
            auto find2 = ranges->find(comb2);
            if (find2 != ranges->end()) {
                nowProb = find2->second;
            }
            //
            auto infoSet = state.infoSet();
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = utilities[first].at(action) - nodeUtil[first][current];
                temp.emplace(action, regret * nowProb / maxProb);
            }
            node->incRegretSum(temp);
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto validActions = state.validActions();
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::unordered_map<std::string, std::valarray<float>> results;
        //
        if (true) {
            State st(state, actions[action]);
            results = mccfr6(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::unordered_map<std::string, std::valarray<float>> Pluribus::mccfr7(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix2(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff(state, player);
        }
    }
    //
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::unordered_map<std::string, std::size_t> filter;
        //
        std::unordered_map<std::string, std::valarray<float>> nodeUtil;
        std::unordered_map<std::string, std::valarray<float>> returned;
        std::unordered_map<std::string, std::unordered_map<std::string, double>> utilities;
        //
        auto card1 = *(target->getHand(0));
        auto card2 = *(target->getHand(1));
        //
        auto validActions = state.validActions();
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                returned = mccfr7(st, player, iter, prune, depth + 1);
            }
            //
            for (auto iter = returned.begin(); iter != returned.end(); iter++) {
                std::vector<std::string> vHands;
                StringUtils::split(iter->first, "-", vHands);
                //
                target->getHand(0)->setCard(vHands[0]);
                target->getHand(1)->setCard(vHands[1]);
                //
                auto infoSet = state.infoSet();
                auto found0 = filter.find(infoSet);
                if (found0 == filter.end()) {
                    filter.emplace(infoSet, 1);
                } else {
                    found0->second += 1;
                }
                //
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto strategy = node->getStrategy();
                auto values = iter->second * strategy.at(action);
                nodeUtil.emplace(iter->first, values);
                //
                auto param1 = iter->second[current];
                auto found1 = utilities.find(iter->first);
                if (found1 == utilities.end()) {
                    std::unordered_map<std::string, double> mp;
                    mp.emplace(action, param1);
                    utilities.emplace(iter->first, mp);
                } else {
                    found1->second.emplace(action, param1);
                }
            }
            //
            target->getHand(0)->setCard(&card1);
            target->getHand(1)->setCard(&card2);
        }
        //
        auto validActionNum = validActions.size();
        for (auto iter = utilities.begin(); iter != utilities.end(); iter++) {
            auto first = iter->first;
            auto second = iter->second;
            if (second.size() != validActionNum)
                continue;
            //
            std::vector<std::string> vHands;
            StringUtils::split(first, "-", vHands);
            //
            target->getHand(0)->setCard(vHands[0]);
            target->getHand(1)->setCard(vHands[1]);
            //
            float maxProb = 0.000000000000001;
            auto ranges = &state.handRanges[player];
            for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                if (iter->second > maxProb) {
                    maxProb = iter->second;
                }
            }
            //
            float nowProb = 0;
            auto comb1 = vHands[0] + "-" + vHands[1];
            auto find1 = ranges->find(comb1);
            if (find1 != ranges->end()) {
                nowProb = find1->second;
            }
            //
            auto comb2 = vHands[1] + "-" + vHands[0];
            auto find2 = ranges->find(comb2);
            if (find2 != ranges->end()) {
                nowProb = find2->second;
            }
            //
            auto infoSet = state.infoSet();
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = utilities[first].at(action) - nodeUtil[first][current];
                temp.emplace(action, regret * nowProb / maxProb);
            }
            node->incRegretSum(temp);
            node->updateStrategy();
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto validActions = state.validActions();
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node ) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::unordered_map<std::string, std::valarray<float>> results;
        //
        if (true) {
            State st(state, actions[action]);
            results = mccfr7(st, player, iter, prune, depth + 1);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr8(State &state, int player, int iter, bool prune, int depth,  std::unordered_set<std::string> &dict) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff2(state, player);
        }
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *target->getHand(0);
        auto card2 = *target->getHand(1);
        //
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                returned = mccfr8(st, player, iter, prune, depth + 1, dict);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0 ;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (legal) {
                        target->getHand(0)->setCard(Card::fastId2str(i));
                        target->getHand(1)->setCard(Card::fastId2str(j));
                        //
                        auto infoSet = state.infoSet();
                        auto agent = getAgent(state.getRound(), infoSet);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSet, validActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        utilities[index][action] = returned[index * 6 + current];
                        //
                        auto validStrategy = node->getStrategy();
                        auto validValue = validStrategy.at(action);
                        for (int k = 0; k < 6; k++) {
                            if (firstAction == 1) {
                                nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                            } else {
                                nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                            }
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand(0)->setCard(&card1);
            target->getHand(1)->setCard(&card2);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto invalid = false;
                for (auto action : validActions) {
                    auto found = utilities[index].find(action);
                    if (found == utilities[index].end()) {
                        invalid = true;
                        break;
                    }
                    //
                    if (found->second == UNLEAGLE) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                target->getHand(0)->setCard(Card::fastId2str(i));
                target->getHand(1)->setCard(Card::fastId2str(j));
                //
                auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto ranges = &state.handRanges[player];
                for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find1 = state.handRanges[player].find(comb1);
                if (find1 !=  state.handRanges[player].end()) {
                    nowprob = find1->second;
                }
                //
                auto find2 = state.handRanges[player].find(comb2);
                if (find2 !=  state.handRanges[player].end()) {
                    nowprob = find2->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
                //
                auto found3 = dict.find(infoSet);
                if (found3 == dict.end()) {
                    dict.emplace(infoSet);
                }
            }
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node ) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr8(st, player, iter, prune, depth + 1, dict);
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::mccfr9(State &state, int player, int iter, bool prune, int depth,  std::unordered_set<std::string> &dict) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    if (Configure::GetInstance().mResearchOpening > 0) {
        if (depth >= Configure::GetInstance().mResearchDepth) {
            return samplePayoff2(state, player);
        }
    }
    //
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<float> returned;
        //
        auto card1 = *target->getHand(0);
        auto card2 = *target->getHand(1);
        //
        std::unordered_map<std::string, std::size_t> kvStat;
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                returned = mccfr9(st, player, iter, prune, depth + 1, dict);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    if (returned[index * 6] == UNLEAGLE)
                        continue;
                    //
                    utilities[index][action] = returned[index * 6 + current];
                    //
                    auto comb = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                    auto found = kvStat.find(comb);
                    if (found == kvStat.end())
                        kvStat.emplace(comb, 1);
                    else
                        found->second++;
                }
            }
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                auto found = kvStat.find(comb2);
                if (found == kvStat.end() || found->second != validActions.size()) {
                    continue;
                }
                //
                target->getHand(0)->setCard(Card::fastId2str(i));
                target->getHand(1)->setCard(Card::fastId2str(j));
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto ranges = &state.handRanges[player];
                for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find1 = state.handRanges[player].find(comb1);
                if (find1 != state.handRanges[player].end()) {
                    nowprob = find1->second;
                }
                //
                auto find2 = state.handRanges[player].find(comb2);
                if (find2 != state.handRanges[player].end()) {
                    nowprob = find2->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto validStrategy = node->getStrategy();
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto prob = validStrategy.at(action);
                    for (int p = 0; p < 6; p++) {
                        if (nodeUtil[index * 6] == UNLEAGLE) {
                            nodeUtil[index * 6 + p] = returned[index * 6 + p] * prob;
                        } else {
                            nodeUtil[index * 6 + p] += returned[index * 6 + p] * prob;
                        }
                    }
                    //
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
                //
                auto found3 = dict.find(infoSet);
                if (found3 == dict.end()) {
                    dict.emplace(infoSet);
                }
            }
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node ) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = mccfr9(st, player, iter, prune, depth + 1, dict);
        }
        //
        return results;
    }
}

void Pluribus::terminal() {
    std::vector<int> cards;
    int numRound = 4;
    int numPlayers = mNumPlayers;
    int numGames = 1000;
    for (int i = 0; i < numGames; i++) {
        if (true) {
            State state(numPlayers);
            while (!state.isTerminal()) {
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet << ", round=" << state.getRound());
                    exit(-1);
                }
                //
                auto validActions = state.validActions();
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet << ", round=" << state.getRound());
                    exit(-1);
                }
                //
                std::vector<std::string> actions;
                std::vector<double> probabilities;
                //
                auto strategy = node->getAverageStrategy();
                if (!strategy.empty()) {
                    std::ostringstream os;
                    for (auto map : strategy) {
                        actions.push_back(map.first);
                        probabilities.push_back(map.second);
                        //
                        os.str("");
                        os << " prob="        << map.second;
                        os << " regretSum="   << node->regretSum[map.first];
                        os << " strategySum=" << node->strategySum[map.first];
                        os << " action="      << map.first;
                        LOG_DEBUG(os.str());
                    }
                } else {
                    LOG_DEBUG("infoset=" << infoSet << " empty!!");
                    for (auto str : validActions) {
                        actions.push_back(str);
                        probabilities.push_back(1.0 / validActions.size());
                    }
                }
                //
                std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                auto action = random_choice(mActionEng);
                auto engine = state.getEngine();
                LOG_DEBUG("@MCCFR: next random_choice: turn=" << engine->getTurnIndex() << endl );
                LOG_DEBUG("action =" <<  actions[action] << " infoSet=" << infoSet << endl);
                LOG_DEBUG("@Choice: turn=" << engine->getTurnIndex() << ", last=" << engine->getLastIndex()
                          << ", action=" << action << ", raiseCount=" << engine->getRaiseCount());
                //
                state.Command(actions[action]);
            }
        }
        //
        std::string line;
        std::cin >> line;
        transform(line.begin(), line.end(), line.begin(), ::tolower);
        if (line == "quit") {
            break;
        }
    }
}

std::string Pluribus::getDateTime(bool onlynum, time_t timestamp) {
    char buffer[20] = {0};
    struct tm *info = localtime(&timestamp);
    if (onlynum) {
        strftime(buffer, sizeof buffer, "%Y%m%d%H%M%S", info);
    } else {
        strftime(buffer, sizeof buffer, "%Y/%m/%d_%H:%M:%S", info);
    }
    return string(buffer);
}

std::string Pluribus::getMSDateTime() {
    string sTimestamp;
    char acTimestamp[24];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm = localtime(&tv.tv_sec);
    sprintf(acTimestamp, "%04d-%02d-%02d_%02d:%02d:%02d.%03d",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec,
            (int)(tv.tv_usec / 1000));
    sTimestamp = acTimestamp;
    return sTimestamp;
}

void Pluribus::load(bool isResearch) {
    LOG_DEBUG("@Load: begin, loadPath=" << Configure::GetInstance().mLoadPath);
    std::stringstream prefix;
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        int stage = i / AGENT_MAX_NUM;
        int index = i % AGENT_MAX_NUM;
        prefix.str("");
        prefix << "BluePrint-" << stage << "-" << setw(4) << setfill('0') << index;
        //
        auto agent = new Agent(prefix.str());
        if (!isResearch) {
            agent->Load();
        }
        mAgentBuckets[i] = agent;
    }
    LOG_DEBUG("@Load: over, loadPath=" << Configure::GetInstance().mLoadPath);
}

void Pluribus::save(bool isResearch) {
    LOG_INFO("@Save: begin, savePath=" << Configure::GetInstance().mSavePath);
    std::string suffix = getDateTime() + "/";
    std::string mkDir = Configure::GetInstance().mSavePath + suffix;
    if (0 != access(mkDir.c_str(), 0)) {
        if (mkdir(mkDir.c_str(), 0771) < 0 ) {
            LOG_ERROR("mkdir(" << mkDir << ") failed, errno=" << errno);
        }
    }
    //
    if (!isResearch) {
        for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
            mAgentBuckets[i]->Save(suffix);
        }
    }
    //
    LOG_INFO("@Save: over, savePath=" << Configure::GetInstance().mSavePath);
}

void Pluribus::agentView() {
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        mAgentBuckets[i]->View();
    }
}

void Pluribus::agentDiscount(double factor) {
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        mAgentBuckets[i]->Discount(factor);
    }
}

void Pluribus::agentStrategy() {
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        mAgentBuckets[i]->UpdateStrategy();
    }
}

Agent *Pluribus::getAgent(const int stage, const std::string &infoSet) {
    uint32_t hashCode = common::Hash((void *)infoSet.c_str(), infoSet.size()) % AGENT_MAX_NUM;
    if ((stage >= 1) && (stage <= 4)) {
        uint32_t bucketIdx = ((stage - 1) * AGENT_MAX_NUM) + hashCode;
        return mAgentBuckets[bucketIdx];
    }
    //
    LOG_ERROR("invalid stage: " << stage);
    return nullptr;
}

void Pluribus::setAgent(const int stage, const std::string &infoSet, InfoNode *nodePtr) {
    uint32_t hashCode = common::Hash((void *)infoSet.c_str(), infoSet.size()) % AGENT_MAX_NUM;
    if ((stage >= 1) && (stage <= 4)) {
        uint32_t bucketIdx = ((stage - 1) * AGENT_MAX_NUM) + hashCode;
        auto agent = mAgentBuckets[bucketIdx];
        if (nullptr != agent) {
            agent->SetNode(infoSet, nodePtr);
        } else {
            LOG_ERROR("error ptr: infoSet=" << infoSet << ", stage=" << stage);
            exit(-1);
        }
    }
}

InfoNode *Pluribus::getAgentNode(const int stage, const std::string &infoSet, const std::set<string> &validActions) {
    InfoNode *node = nullptr;
    //
    auto agent = getAgent(stage, infoSet);
    if (nullptr != agent) {
        node = agent->GetNode(infoSet, validActions);
    }
    //
    return node;
}

InfoNode *Pluribus::getAgentNode(const std::string &infoSet, const std::set<string> &validActions) {
    InfoNode *node = nullptr;
    //
    auto stage = atoi(infoSet.substr(0, 1).c_str());
    auto agent = getAgent(stage, infoSet);
    if (nullptr != agent) {
        node = agent->GetNode(infoSet, validActions);
    }
    //
    return node;
}

InfoNode *Pluribus::findAgentNode(const int stage, const std::string &infoSet) {
    InfoNode *node = nullptr;
    //
    auto agent = getAgent(stage, infoSet);
    if (nullptr != agent) {
        node = agent->GetNode2(infoSet);
    }
    //
    return node;
}

InfoNode *Pluribus::findAgentNode(const std::string &infoSet) {
    InfoNode *node = nullptr;
    //
    auto stage = atoi(infoSet.substr(0, 1).c_str());
    auto agent = getAgent(stage, infoSet);
    if (nullptr != agent) {
        node = agent->GetNode2(infoSet);
    }
    //
    return node;
}

void Pluribus::redis() {
    LOG_INFO("@Redis: upload begin");
    RedisMgr mgr = RedisMgr();
    mgr.init();
    mgr.start();
    //
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        LOG_INFO("@Redis" << i << endl);
        mAgentBuckets[i]->UploadToRedis(mgr);
    }
    //
    mgr.wait();
    mgr.final();
    LOG_INFO("@Redis: upload exited");
}

void Pluribus::convert() {
    std::vector<Agent *> v;
    LOG_INFO("@Load: begin, loadPath=" << Configure::GetInstance().mLoadPath);
    std::stringstream prefix;
    int oldAgentNum = 256;
    for (int i = 0; i < (GAME_STAGE_NUM * oldAgentNum); i++) {
        int stage = i / oldAgentNum;
        int index = i % oldAgentNum;
        prefix.str("");
        prefix << "BluePrint-" << stage << "-" << setw(3) << setfill('0') << index;
        //
        auto agent = new Agent(prefix.str());
        agent->Load();
        v.push_back(agent);
    }
    /*
    LOG_INFO("@Convert: model begin");
    //
    std::vector<Agent *> v;
    //
    std::ostringstream os;
    int oldAgentNum = 256;
    for (int i = 0; i < oldAgentNum; i++) {
        os.str("");
        os << "BluePrint" << std::setw(3) << std::setfill('0') << i;
        auto agent = new Agent(os.str());
        agent->Load();
        v.push_back(agent);
    }*/
    //
    int processed = 0;
    for (auto iter = v.begin(); iter != v.end(); iter++) {
        auto agent = (*iter);
        bool convRes = false;
        LOG_INFO("@Convent: agent begin, processed=" << processed);
        if (nullptr != agent) {
            std::unique_lock<std::shared_mutex> lock(agent->mtxNode);
            auto nodes = agent->GetNodes();
            for (auto it = nodes.begin(); it != nodes.end(); it++) {
                auto infoSet = (*it).first;
                auto node = (*it).second;
                if ("" == infoSet || nullptr == node) {
                    LOG_ERROR("@Convent: agent failed: infoSet=" << infoSet << ", node=" << node);
                    exit(-1);
                }
                //
                int stage = atoi(infoSet.substr(0, 1).c_str());
                if ((stage >= 1) && (stage <= 4)) {
                    setAgent(stage, infoSet, node);
                } else {
                    LOG_INFO("@Convent: agent failed, processed=" << processed);
                    exit(-1);
                }
            }
            //
            convRes = true;
        }
        LOG_INFO("@Convent: agent over, processed=" << processed << ", result=" << convRes);
        //
        processed++;
    }
    //
    LOG_INFO("@Convert: model over");
}

Agent *Pluribus::getAgentPointer(int index) {
    if ((index < 0) || (index >= getAgentSize())) {
        LOG_ERROR("invalid index: " << index);
        return nullptr;
    }
    //
    return mAgentBuckets[index];
}

size_t Pluribus::getAgentSize() {
    return AGENT_MAX_NUM * GAME_STAGE_NUM;
}

size_t Pluribus::getAgentNodeSize() {
    std::size_t nodeNum = 0;
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        nodeNum += mAgentBuckets[i]->nodeNum();
    }
    return nodeNum;
}

void Pluribus::printAgentNodeSize() {
    for (int i = 0; i < (GAME_STAGE_NUM * AGENT_MAX_NUM); i++) {
        auto nodeNum = mAgentBuckets[i]->nodeNum();
        TEST_I("round=" << i / AGENT_MAX_NUM << ", agent=" << i % AGENT_MAX_NUM << ", nodeNum=" << nodeNum);
    }
}

std::valarray<float > Pluribus::rollout(State &state, int player, std::vector<int> bias) {
    auto t1 = std::chrono::high_resolution_clock::now();
    //
    std::valarray<float> avg(UNLEAGLE, 1326 * mNumPlayers);
    std::unordered_map<std::string, std::size_t> holeCards;
    //
    auto rolloutNum = 3;
    vector<int> randList;
    for (int s = 0; s < 1326; s++) {
        size_t i = 0, j = 0;
        pair_conv(s , i , j);
        //
        auto combo1 = Card::fastId2str(j) + '-' + Card::fastId2str(i);
        auto combo2 = Card::fastId2str(i) + '-' + Card::fastId2str(j);
        auto ranges = &state.handRanges[player];
        if (ranges->find(combo1) == ranges->end() && ranges->find(combo2) == ranges->end())
            continue;
        //
        randList.push_back(s);
        //target->getHand()[0]->setCard(Card::fastId2str(i));
        //target->getHand()[1]->setCard(Card::fastId2str(j));
    }
    //LOG_INFO("rollout" << player << bias[0] << bias[1]);
    for (auto i = 0; i < rolloutNum; i++) {
        if (true) {
            auto results = payout2(state, player, bias, randList);
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j) * mNumPlayers;
                    auto skip = false;
                    for (int p = 0; p < mNumPlayers; p++) {
                        if (UNLEAGLE == results[index + p]) {
                            skip = true;
                            break;
                        }
                    }
                    //
                    if (skip) {
                        continue;
                    }
                    //
                    if (UNLEAGLE == avg[index + 0]) {
                        for (int p = 0; p < mNumPlayers; p++) {
                            avg[index + p] = results[index + p];
                        }
                    } else {
                        for (int p = 0; p < mNumPlayers; p++) {
                            avg[index + p] += results[index + p];
                        }
                    }
                    //
                    auto combo = Card::fastId2str(j) + Card::fastId2str(i);
                    auto found = holeCards.find(combo);
                    if (found == holeCards.end()) {
                        holeCards.emplace(combo, 1);
                    } else {
                        found->second++;
                    }
                }
            }
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j) * mNumPlayers;
            auto combo = Card::fastId2str(j) + Card::fastId2str(i);
            auto found = holeCards.find(combo);
            if (found == holeCards.end()) {
                //if (found == holeCards.end() || found->second < rolloutNum) {
                // Restore initial data
                for (int p = 0; p < mNumPlayers; p++) {
                    avg[index + p] = UNLEAGLE;
                }
                //
                continue;
            }
            //
            bool skip = false;
            for (int p = 0; p < mNumPlayers; p++) {
                if (UNLEAGLE == avg[index + p]) {
                    skip = true;
                    break;
                }
            }
            //
            if (skip) {
                // invalid data
                continue;
            }
            //
            for (int p = 0; p < mNumPlayers; p++) {
                //avg[index + p] /= rolloutNum;
                avg[index + p] /= found->second;
            }
        }
    }
    //
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    LOG_DEBUG("@rollout: Player" << player << ", duration=" << duration / pow(10, 6));
    //
    return avg;
}

std::valarray<float> Pluribus::payout(State &state, int player, std::vector<int> bias, int depth) {
    auto t1 = std::chrono::high_resolution_clock::now();
    //
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    int stage = state.getRound() ;
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                returned = payout(st, player, bias, depth + 1);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0 ;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (!legal) {
                        continue;
                    }
                    //
                    target->getHand()[0]->setCard(Card::fastId2str(i));
                    target->getHand()[1]->setCard(Card::fastId2str(j));
                    //
                    auto infoSet = state.infoSet();
                    /*
                    auto agent = getAgent(state.getRound(), infoSet);
                    if (nullptr == agent) {
                        LOG_ERROR("invalid agent: infoSet=" << infoSet);
                        exit(-1);
                    }
                    //
                    auto node = agent->GetNode(infoSet, validActions);
                    if (nullptr == node) {
                        LOG_ERROR("invalid node: infoSet=" << infoSet);
                        exit(-1);
                    }*/
                    //
                    //utilities[index][action] = returned[index * 6 + current];
                    auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                    std::unordered_map<std::string, double>  validStrategy ;
                    if (node != nullptr) {
                        validStrategy = node->getAverageStrategy();
                    } else {
                        // LOG_INFO("null" << infoSet << " " << stage);
                    }
                    //
                    if (validStrategy.empty()) {
                        //validStrategy = state.initialRegret();
                        validStrategy = state.rawStrategy();
                    }
                    //
                    validStrategy = biasStrategy(bias[current], validStrategy);
                    auto validValue = validStrategy.at(action);
                    for (int k = 0; k < 6; k++) {
                        if (firstAction == 1) {
                            nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                        } else {
                            nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand()[0]->setCard(&card1);
            target->getHand()[1]->setCard(&card2);
        }
        //
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        LOG_DEBUG("@payout: Player" << player  << ", depth=" << depth << ", duration=" << duration / pow(10, 6));
        //
        return nodeUtil;
    } else {
        std::unordered_map<std::string, double> validStrategy;
        auto infoSet = state.infoSet();
        auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
        if (node != nullptr) {
            validStrategy = node->getStrategySum();
        } else {
            // LOG_INFO("null " << infoSet << " " << stage);
        }
        //
        if (validStrategy.empty()) {
            //validStrategy = state.initialRegret();
            validStrategy = state.rawStrategy();
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        validStrategy = biasStrategy(bias[current], validStrategy);
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = payout(st, player, bias);
        }
        //
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        LOG_DEBUG("@payout: Player" << player  << ", depth=" << depth << ", duration=" << duration / pow(10, 6));
        //
        return results;
    }
}

std::valarray<float> Pluribus::payout2(State &state, int player, vector<int> bias, vector<int> randlist) {
    if (state.isTerminal()) {
        //LOG_INFO("imm return");
        return state.payoffMatrix3(player, randlist);
    }
    //
    int stage = state.getRound() ;
    auto validActions = state.validActions();
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    if (current == player) {
        //LOG_INFO("RANDLISTSIZE"<< randlist.size());
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        std::unordered_map<std::string, vector<int>> newRandList;
        for (auto action : validActions) {
            vector<int> label;
            newRandList[action] =  label;

        }
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        auto nrand = state.getleagle(player, randlist);
        for (int s = 0; s < nrand.size(); s++) {
            auto index = nrand[s];
            size_t i = 0 , j = 0;
            pair_conv(index , i , j);
            //
            target->getHand()[0]->setCard(Card::fastId2str(i));
            target->getHand()[1]->setCard(Card::fastId2str(j));
            //
            auto infoSet = state.infoSet();
            auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
            std::unordered_map<std::string, double> validStrategy;
            std::unordered_map<std::string, double> validStrategyraw;
            if (node != nullptr) {
                validStrategy = node->getAverageStrategy();
                validStrategyraw = validStrategy;
                if (validStrategy.empty()) {
                    //validStrategy = state.initialRegret();
                    validStrategy = state.rawStrategy();
                    LOG_INFO("null" << infoSet << " " << stage);
                }
            } else {
                LOG_INFO("null" << infoSet << " " << stage);
                //validStrategy = state.initialRegret();
                validStrategy = state.rawStrategy();
            }
            /*
            int haveFold = 0;
            for (auto map : validStrategy) {
                //actions.push_back(map.first);
                //probabilities.push_back(map.second);
                if (map.first == "FOLD") {
                    haveFold = 1;
                }
            }
            //
            if ((Card::fastId2str(j)[0] == 'A') && (Card::fastId2str(i)[0] == 'A')
                    && (state.getEngine()->getRiverCard()->strCluster()[0] != 'K')) {
                if (haveFold == 1) {
                    validStrategy["FOLD"] = 0 ;
                }
            }
            //
            if (state.getEngine()->getRiverCard()->strCluster()[0] == 'K') {
                //LOG_INFO("meet K");
            }*/
            //
            validStrategy = biasStrategy(bias[current], validStrategy);
            std::vector<std::string> actions;
            std::vector<double> probabilities;
            for (auto map : validStrategy) {
                actions.push_back(map.first);
                probabilities.push_back(map.second);
            }
            //
            std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
            auto action = random_choice(mActionEng);
            /*
            if ((Card::fastId2str(j)[0] == 'A') && (Card::fastId2str(i)[0] == 'A')
                    && (actions.at(action) == "FOLD")) {
                //LOG_INFO("infoerror" << validStrategyraw["FOLD"]<< " " << validStrategy["FOLD"]<<" " <<bias[current]);
                //LOG_INFO("reward" << " player" <<player <<"bias"<<bias[0] << bias[1] <<" " <<" " << state.getEngine()->getTurnCard()->strCluster()<<" " <<state.getEngine()->getRiverCard()->strCluster()<<" " << Card::fastId2str(j) << Card::fastId2str(i) << infoSet << target->getHand()[0]->strCluster() << target->getHand()[1]->strCluster());
            }*/
            //
            newRandList[actions[action]].push_back(index);
        }
        //
        firstAction = 0;
        actionIndex += 1;
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        actionIndex = 0;
        for (auto action : validActions) {
            if (newRandList[action].size() != 0) {
                State st(state, action);
                //LOG_INFO("LOGP" << newRandList[actionIndex].size());
                returned = payout2(st, player, bias, newRandList[action]);
                for (int s = 0; s < newRandList[action].size(); s++) {
                    auto index = newRandList[action][s];
                    size_t i = 0 , j = 0;
                    pair_conv(index , i , j);
                    /*
                    if(Card::fastId2str(j)[0] == 'A' && Card::fastId2str(i)[0] == 'A'&& returned[index *6 + player] < 0 and state.getEngine()->getRiverCard()->strCluster()[0] != 'K' ){
                        LOG_INFO("error");
                    }
                    */
                    //nodeUtil[s] = returned[s];
                    for (int k = 0; k < 6; k++) {
                        nodeUtil[index * 6 + k] = returned[index * 6 + k];
                    }
                }
            }
        }
        //
        auto results = nodeUtil;
        for (int s = 0 ; s < 1326; s ++) {
            if (results[s * 6 + player] != UNLEAGLE) {
                size_t v = 0, x = 0;
                pair_conv(s , v , x);
                auto combo1 = Card::fastId2str(v) + '-' + Card::fastId2str(x);
                //if (results[s*6 + player] <0 && Card::fastId2str(v)[0] == 'A' &&  Card::fastId2str(x)[0] == 'A')
                if (Card::fastId2str(v)[0] == 'A' &&  Card::fastId2str(x)[0] == 'A') {
                    //LOG_INFO("reward 1 " << results[s*6 + player] <<" player" <<player <<"bias"<<bias[0] << bias[1] <<" " <<combo1<<" " << state.getEngine()->getTurnCard()->strCluster()<<" " <<state.getEngine()->getRiverCard()->strCluster() << " " <<player );
                }
                //LOG_INFO("oppohand" << state.getEngine()-> getPlayers(!player) ->getHand()[0]->strCluster() << state.getEngine()-> getPlayers(!player) ->getHand()[1]->strCluster());
                //state.getEngine()->printBoard();
            }
        }
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
        std::unordered_map<std::string, double>  validStrategy ;
        if (node != nullptr) {
            validStrategy = node->getStrategySum();
            if (validStrategy.empty()) {
                //validStrategy = state.initialRegret();
                validStrategy = state.rawStrategy();
            }
        } else {
            LOG_INFO("null" << infoSet << " " << stage);
            //validStrategy = state.initialRegret();
            validStrategy = state.rawStrategy();
        }
        //
        if (validStrategy.empty()) {
            //validStrategy = state.initialRegret();
            validStrategy = state.rawStrategy();
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        validStrategy = biasStrategy(bias[current], validStrategy);
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            results = payout2(st, player, bias, randlist);
        }
        //
        for (int s = 0 ; s < 1326; s ++) {
            if (results[s * 6 + player] != UNLEAGLE) {
                size_t v = 0, x = 0;
                pair_conv(s , v , x);
                auto combo1 = Card::fastId2str(v) + '-' + Card::fastId2str(x);
                //if (results[s*6 + player] <0 && Card::fastId2str(v)[0] == 'A' &&  Card::fastId2str(x)[0] == 'A')
                //if (Card::fastId2str(v)[0] == 'A' &&  Card::fastId2str(x)[0] == 'A')
                if (s == 1321 or s == 1322 or s == 1323 or s == 1324) {
                    //LOG_INFO("reward 2 " << results[s*6 + player] <<" player" <<player <<"bias"<<bias[0] << bias[1] <<" " <<combo1<<" " << state.getEngine()->getTurnCard()->strCluster()<<" " <<state.getEngine()->getRiverCard()->strCluster() << " " <<player );
                }
                //LOG_INFO("oppohand" << state.getEngine()-> getPlayers(!player) ->getHand()[0]->strCluster() << state.getEngine()-> getPlayers(!player) ->getHand()[1]->strCluster());
                //state.getEngine()->printBoard();
            }
        }
        //
        return results;
    }
}

std::unordered_map<std::string, double> Pluribus::biasStrategy(int blueprint, std::unordered_map<std::string, double> strategySum) {
    for (auto kv : strategySum) {
        auto key = kv.first;
        //
        if (blueprint == 0)  {
            strategySum[key] = strategySum[key] * 1.0;
        } else if (blueprint == 1) {
            if (("CHECK" == key) || ("CALL" == key))
                strategySum[key] = strategySum[key] * 10.0;
        } else if (blueprint == 2) {
            if  ("FOLD" == key)
                strategySum[key] = strategySum[key] * 10.0;
        } else if (blueprint == 3) {
            if (("ALLIN" != key) && ("FOLD" != key) && ("CHECK" != key) && ("CALL" != key))
                strategySum[key] = strategySum[key] * 10.0;
        }
    }
    //
    float sumProb = 0.0;
    for (auto kv : strategySum) {
        auto key = kv.first;
        sumProb += strategySum[key];
    }
    //
    for (auto kv : strategySum) {
        auto key = kv.first;
        strategySum[key] /= sumProb;
    }
    //
    return strategySum;
}

std::valarray<float> Pluribus::subgameSolve(State &state, int player, int iter, bool prune) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    auto validActions = state.validActions();
    int stage = state.getRound();
    auto current = state.getTurnIndex();
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        auto target = state.getEngine()->getPlayers(player);
        //LOG_INFO("player" << player << "current" << current  << "infoset" << state.infoSet());
        //if(leaf == false){
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        for (auto action : validActions) {
            State st(state, action);
            if (true) {
                if (state.getRound() == st.getRound()) {
                    returned = subgameSolve(st, player, iter, prune);
                } else {
                    vector<int> biasOther(6, 0);
                    std::set<std::string> mLeafActions;
                    mLeafActions.emplace("NULL");
                    mLeafActions.emplace("FOLD");
                    mLeafActions.emplace("CALL");
                    mLeafActions.emplace("RAISE");
                    if (!st.getEngine()->checkGameOver()) {
                        for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                            if (!st.getEngine() -> getPlayers(s)->isFold() && current != s) {
                                auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                                //auto node = agent->GetNode(infoSet, validActions);
                                //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                                //validActions = mLeafActions;
                                auto agent = getAgent(st.getRound(), infoSetLeaf);
                                if (nullptr == agent) {
                                    LOG_ERROR("invalid agent: infoSet=" << infoSetLeaf);
                                    exit(-1);
                                }
                                //
                                auto node = agent->GetNode(infoSetLeaf, mLeafActions);
                                if (nullptr == node ) {
                                    LOG_ERROR("invalid node: infoSet=" << infoSetLeaf);
                                    exit(-1);
                                }
                                //
                                std::vector<std::string> actions;
                                std::vector<double> probabilities;
                                auto validStrategy = node->getStrategy();
                                for (auto map : validStrategy) {
                                    actions.push_back(map.first);
                                    probabilities.push_back(map.second);
                                }
                                //
                                std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                                auto action = random_choice(mActionEng);
                                biasOther[s] = action;
                            }
                        }
                    }
                    //
                    std::valarray<float> results;
                    if (true) {
                        State stt(st);
                        returned = rollout(stt, player, biasOther);
                    }
                    //
                    if (!st.getEngine()->checkGameOver()) {
                        //LOG_INFO("st" << st.getRound() << st.getEngine()->getActivtedNum()
                        //<< " " << st.getEngine()->getPot() );
                        trainChoseBlueprint(st, player);
                    }
                }
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0 ;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (legal) {
                        target->getHand()[0]->setCard(Card::fastId2str(i));
                        target->getHand()[1]->setCard(Card::fastId2str(j));
                        //
                        auto infoSet = state.infoSet();
                        auto agent = getAgent(state.getRound(), infoSet);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSet, validActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        utilities[index][action] = returned[index * 6 + current];
                        //
                        auto validStrategy = node->getStrategy();
                        //LOG_INFO("validStrategy" << validStrategy.size()
                        //<< " " << validActions.size()
                        //<< " " << action << infoSet);
                        auto validValue = validStrategy.at(action);
                        for (int k = 0; k < 6; k++) {
                            if (firstAction == 1) {
                                nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                            } else {
                                nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                            }
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand()[0]->setCard(&card1);
            target->getHand()[1]->setCard(&card2);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto invalid = false;
                for (auto action : validActions) {
                    auto u = &utilities[index];
                    if (u->find(action) == u->end() || u->at(action) == UNLEAGLE) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                target->getHand()[0]->setCard(Card::fastId2str(i));
                target->getHand()[1]->setCard(Card::fastId2str(j));
                //
                string comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                string comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto ranges = &state.handRanges[player];
                for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find = ranges->find(comb1);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                find = ranges->find(comb2);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        //LOG_INFO("player" << player << "current" << current <<  state.infoSet() );
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        if (true) {
            State st(state, actions[action]);
            if (state.getRound() == st.getRound()) {
                results = subgameSolve(st, player, iter, prune);
            } else {
                vector<int> biasOther(6, 0);
                if (!st.getEngine()->checkGameOver()) {
                    for (int s = 0 ; s < state.getEngine() -> getPlayerNum(); s++ ) {
                        if (!st.getEngine() -> getPlayers(s)->isFold() && current != s) {
                            auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                            //auto node = agent->GetNode(infoSet, validActions);
                            //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                            std::set<std::string> mLeafActions;
                            mLeafActions.emplace("NULL");
                            mLeafActions.emplace("FOLD");
                            mLeafActions.emplace("CALL");
                            mLeafActions.emplace("RAISE");
                            validActions = mLeafActions;
                            auto agent = getAgent(st.getRound(), infoSetLeaf);
                            if (nullptr == agent) {
                                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                                exit(-1);
                            }
                            //
                            auto node = agent->GetNode(infoSetLeaf, validActions);
                            if (nullptr == node ) {
                                LOG_ERROR("invalid node: infoSet=" << infoSet);
                                exit(-1);
                            }
                            //
                            std::vector<std::string> actions;
                            std::vector<double> probabilities;
                            auto validStrategy = node->getStrategy();
                            for (auto map : validStrategy) {
                                actions.push_back(map.first);
                                probabilities.push_back(map.second);
                            }
                            //
                            std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                            auto action = random_choice(mActionEng);
                            biasOther[s] = action;
                        }
                    }
                }
                //
                if (true) {
                    State stt(st);
                    results = rollout(stt, player, biasOther);
                }
            }
        }
        //
        return results;
    }
}

void Pluribus::trainChoseBlueprint2(State &state, int player) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::unordered_map<std::string, std::valarray<float>> histogram;
    //
    std::set<std::string> mLeafActions;
    mLeafActions.emplace("NULL");
    mLeafActions.emplace("FOLD");
    mLeafActions.emplace("CALL");
    mLeafActions.emplace("RAISE");
    auto validActions = mLeafActions;
    //
    auto firstAction = true;
    auto actionIndex = 0;
    //
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    //
    auto card1 = *(target->getHand(0));
    auto card2 = *(target->getHand(1));
    //
    for (auto action : validActions) {
        actionIndex++;
        //
        if (true) {
            State st(state);
            //
            std::vector<int> biasOther(6, 0);
            biasOther[player] = actionIndex;
            //
            auto returned = rollout(st, player, biasOther);
            //
            histogram.emplace(action, returned);
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j);
            //
            std::unordered_map<std::string, float> currentRewards;
            std::unordered_map<std::string, std::valarray<float>> actionReward;
            //
            auto invalid = false;
            for (auto action : validActions) {
                //
                auto found = histogram.find(action);
                if (found == histogram.end()) {
                    invalid = true;
                    break;
                }
                //
                auto returned = found->second;
                if (UNLEAGLE == returned[index * 6]) {
                    invalid = true;
                    break;
                }
                //
                currentRewards[action] = returned[index * 6 + current];
                //
                std::valarray<float> reward(0.0, 6);
                for (int k = 0; k < 6; k++) {
                    reward[k] = returned[index * 6 + k];
                }
                //
                actionReward[action] = reward;
            }
            //
            if (invalid) {
                continue;
            }
            //
            auto strCard1 = Card::fastId2str(i);
            auto strCard2 = Card::fastId2str(j);
            //
            target->getHand(0)->setCard(strCard1);
            target->getHand(1)->setCard(strCard2);
            //
            auto comb1 = strCard1 + "-" + strCard2;
            auto comb2 = strCard2 + "-" + strCard1;
            //
            float maxprob = 0.000000000000001;
            float allprob = 0;
            auto ranges = &state.handRanges[player];
            for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                if (iter->second > maxprob) {
                    maxprob = iter->second;
                    allprob += iter->second;
                }
            }
            //
            float nowprob = 0;
            auto find = ranges->find(comb1);
            if (find != ranges->end()) {
                nowprob = find->second;
            }
            //
            find = ranges->find(comb2);
            if (find != ranges->end()) {
                nowprob = find->second;
            }
            //
            auto infoSet = state.infoSetdepthLimit(player) + "leaf";
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            std::valarray<float> values(0.0, 6);
            auto strategy = node->getStrategy();
            for (auto action : validActions) {
                auto facotr = strategy.at(action);
                auto found = actionReward.at(action);
                for (auto p = 0; p < 6; p++) {
                    values[p] += found[p] * facotr;
                }
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = currentRewards.at(action) - values[current];
                temp.emplace(action, regret * nowprob / maxprob);
            }
            //
            node->incRegretSum(temp);
        }
    }
    //
    target->getHand(0)->setCard(&card1);
    target->getHand(1)->setCard(&card2);
    //
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    LOG_DEBUG("@trainChoseBluePrint2: Player" << player << ", duration=" << duration / pow(10, 6));
}

std::valarray<float> Pluribus::subgameSolve2(State &state, int player, int iter, bool prune) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    auto validActions = state.validActions();
    auto stage = state.getRound();
    auto current = state.getTurnIndex();
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<int> validIndexs(1, 1326);
        std::valarray<float> returned;
        auto target = state.getEngine()->getPlayers(player);
        //LOG_INFO("player" << player << "current" << current  << "infoset" << state.infoSet());
        //
        int firstAction = 1;
        int actionIndex = 0;
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        for (auto action : validActions) {
            State st(state, action);
            if (state.getRound() == st.getRound()) {
                returned = subgameSolve2(st, player, iter, prune);
            } else {
                vector<int> biasOther(6, 0);
                std::set<std::string> mLeafActions;
                mLeafActions.emplace("NULL");
                mLeafActions.emplace("FOLD");
                mLeafActions.emplace("CALL");
                mLeafActions.emplace("RAISE");
                for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                    if (current == s || st.getEngine()->getPlayers(s)->isFold())
                        continue;
                    //
                    auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                    auto agent = getAgent(st.getRound(), infoSetLeaf);
                    if (nullptr == agent) {
                        LOG_ERROR("invalid agent: infoSet=" << infoSetLeaf);
                        exit(-1);
                    }
                    //
                    auto node = agent->GetNode(infoSetLeaf, mLeafActions);
                    if (nullptr == node) {
                        LOG_ERROR("invalid node: infoSet=" << infoSetLeaf);
                        exit(-1);
                    }
                    //
                    std::vector<std::string> actions;
                    std::vector<double> probabilities;
                    auto validStrategy = node->getStrategy();
                    for (auto map : validStrategy) {
                        actions.push_back(map.first);
                        probabilities.push_back(map.second);
                    }
                    //
                    std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                    auto action = random_choice(mActionEng);
                    biasOther[s] = action;
                }
                //
                std::valarray<float> results;
                if (true) {
                    State stt(st);
                    returned = rollout(stt, player, biasOther);
                }
                //
                trainChoseBlueprint2(st, player);
            }
            //
            for (int i = 0; i < 52; i++) {
                for (int j = i + 1; j < 52; j++) {
                    auto index = pair_index(i, j);
                    auto legal = true;
                    if (returned[index * 6] == UNLEAGLE) {
                        if (validIndexs[index] == 1) {
                            if (firstAction != 1) {
                                for (int z = 0; z < 6; z++) {
                                    nodeUtil[index * 6 + z] = UNLEAGLE;
                                }
                            }
                            //
                            validIndexs[index] = 0;
                        }
                        //
                        legal = false;
                    } else {
                        if (validIndexs[index] == 0) {
                            legal = false;
                        }
                    }
                    //
                    if (!legal) {
                        continue;
                    }
                    //
                    target->getHand()[0]->setCard(Card::fastId2str(i));
                    target->getHand()[1]->setCard(Card::fastId2str(j));
                    //
                    auto infoSet = state.infoSet();
                    auto agent = getAgent(state.getRound(), infoSet);
                    if (nullptr == agent) {
                        LOG_ERROR("invalid agent: infoSet=" << infoSet);
                        exit(-1);
                    }
                    //
                    auto node = agent->GetNode(infoSet, validActions);
                    if (nullptr == node) {
                        LOG_ERROR("invalid node: infoSet=" << infoSet);
                        exit(-1);
                    }
                    //
                    utilities[index][action] = returned[index * 6 + current];
                    //
                    auto validStrategy = node->getStrategy();
                    //LOG_INFO("validStrategy " << validStrategy.size() << " " << validActions.size() << " " << action << infoSet);
                    auto validValue = validStrategy.at(action);
                    for (int k = 0; k < 6; k++) {
                        if (firstAction == 1) {
                            nodeUtil[index * 6 + k] = returned[index * 6 + k] * validValue;
                        } else {
                            nodeUtil[index * 6 + k] += returned[index * 6 + k] * validValue;
                        }
                    }
                }
            }
            //
            firstAction = 0;
            actionIndex += 1;
            //
            target->getHand()[0]->setCard(&card1);
            target->getHand()[1]->setCard(&card2);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                auto invalid = false;
                for (auto action : validActions) {
                    auto um = &utilities[index];
                    if (um->find(action) == um->end() || um->at(action) == UNLEAGLE) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                auto strCard1 = Card::fastId2str(i);
                auto strCard2 = Card::fastId2str(j);
                //
                target->getHand(0)->setCard(strCard1);
                target->getHand(1)->setCard(strCard2);
                //
                auto comb1 = strCard1 + "-" + strCard2;
                auto comb2 = strCard2 + "-" + strCard1;
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto ranges = &state.handRanges[player];
                for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find = ranges->find(comb1);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                find = ranges->find(comb2);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        State st(state, actions[action]);
        if (state.getRound() == st.getRound()) {
            results = subgameSolve2(st, player, iter, prune);
        } else {
            std::vector<int> biasOther(6, 0);
            for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                if (current == s || st.getEngine()->getPlayers(s)->isFold())
                    continue;
                //
                auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                //auto node = agent->GetNode(infoSet, validActions);
                //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                std::set<std::string> mLeafActions;
                mLeafActions.emplace("NULL");
                mLeafActions.emplace("FOLD");
                mLeafActions.emplace("CALL");
                mLeafActions.emplace("RAISE");
                validActions = mLeafActions;
                auto agent = getAgent(st.getRound(), infoSetLeaf);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSetLeaf, validActions);
                if (nullptr == node ) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::vector<std::string> actions;
                std::vector<double> probabilities;
                auto validStrategy = node->getStrategy();
                for (auto map : validStrategy) {
                    actions.push_back(map.first);
                    probabilities.push_back(map.second);
                }
                //
                std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                auto action = random_choice(mActionEng);
                biasOther[s] = action;
            }
            //
            if (true) {
                State stt(st);
                results = rollout(stt, player, biasOther);
            }
        }
        //
        return results;
    }
}

void Pluribus::trainChoseBlueprint3(State &state, int player, int depth) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::unordered_map<std::string, std::valarray<float>> histogram;
    //
    std::set<std::string> mLeafActions;
    mLeafActions.emplace("NULL");
    mLeafActions.emplace("FOLD");
    mLeafActions.emplace("CALL");
    mLeafActions.emplace("RAISE");
    auto validActions = mLeafActions;
    //
    auto firstAction = true;
    auto actionIndex = 0;
    //
    auto current = state.getTurnIndex();
    auto target = state.getEngine()->getPlayers(player);
    //
    auto card1 = *(target->getHand(0));
    auto card2 = *(target->getHand(1));
    //
    for (auto action : validActions) {
        actionIndex++;
        //
        if (true) {
            State st(state);
            //
            std::vector<int> biasOther(6, 0);
            biasOther[player] = actionIndex;
            //
            auto tt1 = std::chrono::high_resolution_clock::now();
            auto returned = rollout(st, player, biasOther);
            auto tt2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(tt2 - tt1).count();
            LOG_DEBUG("@rollout: Player" << player
                      << ", action=" << action
                      << ", depth=" << depth
                      << ", duration=" << duration / pow(10, 6));
            //
            histogram.emplace(action, returned);
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j);
            //
            std::unordered_map<std::string, float> currentRewards;
            std::unordered_map<std::string, std::valarray<float>> actionReward;
            //
            auto invalid = false;
            for (auto action : validActions) {
                //
                auto found = histogram.find(action);
                if (found == histogram.end()) {
                    invalid = true;
                    break;
                }
                //
                auto returned = found->second;
                if (UNLEAGLE == returned[index * 6]) {
                    invalid = true;
                    break;
                }
                //
                currentRewards[action] = returned[index * 6 + current];
                //
                std::valarray<float> reward(0.0, 6);
                for (int k = 0; k < 6; k++) {
                    reward[k] = returned[index * 6 + k];
                }
                //
                actionReward[action] = reward;
            }
            //
            if (invalid) {
                continue;
            }
            //
            auto strCard1 = Card::fastId2str(i);
            auto strCard2 = Card::fastId2str(j);
            //
            target->getHand(0)->setCard(strCard1);
            target->getHand(1)->setCard(strCard2);
            //
            auto comb1 = strCard1 + "-" + strCard2;
            auto comb2 = strCard2 + "-" + strCard1;
            //
            float maxprob = 0.000000000000001;
            float allprob = 0;
            auto ranges = &state.handRanges[player];
            for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                if (iter->second > maxprob) {
                    maxprob = iter->second;
                    allprob += iter->second;
                }
            }
            //
            float nowprob = 0;
            auto find = ranges->find(comb1);
            if (find != ranges->end()) {
                nowprob = find->second;
            }
            //
            find = ranges->find(comb2);
            if (find != ranges->end()) {
                nowprob = find->second;
            }
            //
            auto infoSet = state.infoSetdepthLimit(player) + "leaf";
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            std::valarray<float> values(0.0, 6);
            auto strategy = node->getStrategy();
            for (auto action : validActions) {
                auto facotr = strategy.at(action);
                auto found = actionReward.at(action);
                for (auto p = 0; p < 6; p++) {
                    values[p] += found[p] * facotr;
                }
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = currentRewards.at(action) - values[current];
                temp.emplace(action, regret * nowprob / maxprob);
            }
            //
            node->incRegretSum(temp);
        }
    }
    //
    target->getHand(0)->setCard(&card1);
    target->getHand(1)->setCard(&card2);
    //
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    LOG_DEBUG("@trainChoseBluePrint3: Player" << player << ", depth=" << depth << ", duration=" << duration / pow(10, 6));
}

std::valarray<float> Pluribus::subgameSolve3(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    auto validActions = state.validActions();
    auto stage = state.getRound();
    auto current = state.getTurnIndex();
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<float> returned;
        auto target = state.getEngine()->getPlayers(player);
        //LOG_INFO("player" << player << "current" << current  << "infoset" << state.infoSet());
        //
        auto card1 = *(target->getHand(0));
        auto card2 = *(target->getHand(1));
        //
        std::unordered_map<std::string, std::valarray<float>> histogram;
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                if (state.getRound() == st.getRound()) {
                    returned = subgameSolve3(st, player, iter, prune, depth + 1);
                } else {
                    vector<int> biasOther(6, 0);
                    std::set<std::string> mLeafActions;
                    mLeafActions.emplace("NULL");
                    mLeafActions.emplace("FOLD");
                    mLeafActions.emplace("CALL");
                    mLeafActions.emplace("RAISE");
                    for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                        if (current == s || st.getEngine()->getPlayers(s)->isFold())
                            continue;
                        //
                        auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                        auto agent = getAgent(st.getRound(), infoSetLeaf);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSetLeaf);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSetLeaf, mLeafActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSetLeaf);
                            exit(-1);
                        }
                        //
                        std::vector<std::string> actions;
                        std::vector<double> probabilities;
                        auto validStrategy = node->getStrategy();
                        for (auto map : validStrategy) {
                            actions.push_back(map.first);
                            probabilities.push_back(map.second);
                        }
                        //
                        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                        auto action = random_choice(mActionEng);
                        biasOther[s] = action;
                    }
                    //
                    if (true) {
                        State stt(st);
                        returned = rollout(stt, player, biasOther);
                    }
                    //
                    // LOG_INFO("---> ChoseBlueprint2: Player" << player << ", action=" << action << ", depth=" << depth);
                    trainChoseBlueprint3(st, player, depth);
                }
            }
            //
            histogram.emplace(action, returned);
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                //
                std::unordered_map<std::string, float> currentRewards;
                std::unordered_map<std::string, std::valarray<float>> actionReward;
                //
                auto invalid = false;
                for (auto action : validActions) {
                    //
                    auto found = histogram.find(action);
                    if (found == histogram.end()) {
                        invalid = true;
                        break;
                    }
                    //
                    auto returned = found->second;
                    if (UNLEAGLE == returned[index * 6]) {
                        invalid = true;
                        break;
                    }
                    //
                    currentRewards[action] = returned[index * 6 + current];
                    //
                    std::valarray<float> reward(0.0, 6);
                    for (auto p = 0; p < 6; p++) {
                        reward[p] = returned[index * 6 + p];
                    }
                    //
                    actionReward[action] = reward;
                }
                //
                if (invalid) {
                    continue;
                }
                //
                auto strCard1 = Card::fastId2str(i);
                auto strCard2 = Card::fastId2str(j);
                //
                target->getHand(0)->setCard(strCard1);
                target->getHand(1)->setCard(strCard2);
                //
                auto comb1 = strCard1 + "-" + strCard2;
                auto comb2 = strCard2 + "-" + strCard1;
                //
                float maxprob = 0.000000000000001;
                float allprob = 0;
                auto ranges = &state.handRanges[player];
                for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                    if (iter->second > maxprob) {
                        maxprob = iter->second;
                        allprob += iter->second;
                    }
                }
                //
                float nowprob = 0;
                auto find = ranges->find(comb1);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                find = ranges->find(comb2);
                if (find != ranges->end()) {
                    nowprob = find->second;
                }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                std::valarray<float> values(0.0, 6);
                auto strategy = node->getStrategy();
                for (auto action : validActions) {
                    auto facotr = strategy.at(action);
                    auto found = actionReward.at(action);
                    for (auto p = 0; p < 6; p++) {
                        values[p] += found[p] * facotr;
                    }
                }
                //
                for (int p = 0; p < 6; p++) {
                    nodeUtil[index * 6 + p] = values[p];
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = currentRewards.at(action) - values[current];
                    temp.emplace(action, regret * nowprob / maxprob);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        State st(state, actions[action]);
        if (state.getRound() == st.getRound()) {
            results = subgameSolve3(st, player, iter, prune, depth + 1);
        } else {
            std::vector<int> biasOther(6, 0);
            if (!st.getEngine()->checkGameOver()) {
                for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                    if (current == s || st.getEngine()->getPlayers(s)->isFold())
                        continue;
                    //
                    auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                    //auto node = agent->GetNode(infoSet, validActions);
                    //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                    std::set<std::string> mLeafActions;
                    mLeafActions.emplace("NULL");
                    mLeafActions.emplace("FOLD");
                    mLeafActions.emplace("CALL");
                    mLeafActions.emplace("RAISE");
                    validActions = mLeafActions;
                    auto agent = getAgent(st.getRound(), infoSetLeaf);
                    if (nullptr == agent) {
                        LOG_ERROR("invalid agent: infoSet=" << infoSet);
                        exit(-1);
                    }
                    //
                    auto node = agent->GetNode(infoSetLeaf, validActions);
                    if (nullptr == node) {
                        LOG_ERROR("invalid node: infoSet=" << infoSet);
                        exit(-1);
                    }
                    //
                    std::vector<std::string> actions;
                    std::vector<double> probabilities;
                    auto validStrategy = node->getStrategy();
                    for (auto map : validStrategy) {
                        actions.push_back(map.first);
                        probabilities.push_back(map.second);
                    }
                    //
                    std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                    auto action = random_choice(mActionEng);
                    biasOther[s] = action;
                }
            }
            //
            if (true) {
                State stt(st);
                results = rollout(stt, player, biasOther);
            }
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::subgameSolve4(State &state, int player, int iter, bool prune, int depth) {
    if (state.isTerminal()) {
        return state.payoffMatrix(player);
    }
    //
    auto validActions = state.validActions();
    auto stage = state.getRound();
    auto current = state.getTurnIndex();
    if (current == player) {
        std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
        std::valarray<std::unordered_map<std::string, double>> utilities(1326);
        std::valarray<float> returned;
        auto target = state.getEngine()->getPlayers(player);
        //LOG_INFO("player" << player << "current" << current  << "infoset" << state.infoSet());
        //
        auto card1 = *(target->getHand()[0]);
        auto card2 = *(target->getHand()[1]);
        //
        std::unordered_map<std::string, std::valarray<float>> matrix;
        for (auto action : validActions) {
            if (true) {
                State st(state, action);
                if (state.getRound() == st.getRound()) {
                    returned = subgameSolve4(st, player, iter, prune, depth + 1);
                } else {
                    std::vector<int> biasOther(6, 0);
                    if (!st.getEngine()->checkGameOver()) {
                        for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                            if (current == s || st.getEngine()->getPlayers(s)->isFold())
                                continue;
                            //
                            auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                            //auto node = agent->GetNode(infoSet, validActions);
                            //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                            //validActions = mLeafActions;
                            auto agent = getAgent(st.getRound(), infoSetLeaf);
                            if (nullptr == agent) {
                                LOG_ERROR("invalid agent: infoSet=" << infoSetLeaf);
                                exit(-1);
                            }
                            //
                            std::set<std::string> mLeafActions;
                            mLeafActions.emplace("NULL");
                            mLeafActions.emplace("FOLD");
                            mLeafActions.emplace("CALL");
                            mLeafActions.emplace("RAISE");
                            auto node = agent->GetNode(infoSetLeaf, mLeafActions);
                            if (nullptr == node ) {
                                LOG_ERROR("invalid node: infoSet=" << infoSetLeaf);
                                exit(-1);
                            }
                            //
                            std::vector<std::string> actions;
                            std::vector<double> probabilities;
                            auto validStrategy = node->getStrategy();
                            for (auto map : validStrategy) {
                                actions.push_back(map.first);
                                probabilities.push_back(map.second);
                            }
                            //
                            std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                            auto action = random_choice(mActionEng);
                            biasOther[s] = action;
                        }
                    }
                    //
                    if (true) {
                        State stt(st);
                        returned = rollout(stt, player, biasOther);
                    }
                    //
                    if (!st.getEngine()->checkGameOver()) {
                        trainChoseBlueprint(st, player);
                    }
                }
                //
                matrix.emplace(action, returned);
            }
        }
        //
        for (int i = 0; i < 52; i++) {
            for (int j = i + 1; j < 52; j++) {
                auto index = pair_index(i, j);
                //
                bool invalid = false;
                for (auto action : validActions) {
                    if (UNLEAGLE == matrix[action][index * 6]) {
                        invalid = true;
                        break;
                    }
                }
                //
                if (invalid) {
                    continue;
                }
                //
                target->getHand()[0]->setCard(Card::fastId2str(i));
                target->getHand()[1]->setCard(Card::fastId2str(j));
                //
                auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
                auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
                //
                // float maxprob = 0.000000000000001;
                // float allprob = 0;
                // auto ranges = &state.handRanges[player];
                // for (auto iter = ranges->begin(); iter != ranges->end(); iter++) {
                //     if (iter->second > maxprob) {
                //         maxprob = iter->second;
                //         allprob += iter->second;
                //     }
                // }
                // //
                // float nowprob = 0;
                // auto find = ranges->find(comb1);
                // if (find != ranges->end()) {
                //     nowprob = find->second;
                // }
                // //
                // find = ranges->find(comb2);
                // if (find != ranges->end()) {
                //     nowprob = find->second;
                // }
                //
                auto infoSet = state.infoSet();
                auto agent = getAgent(state.getRound(), infoSet);
                if (nullptr == agent) {
                    LOG_ERROR("invalid agent: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto node = agent->GetNode(infoSet, validActions);
                if (nullptr == node) {
                    LOG_ERROR("invalid node: infoSet=" << infoSet);
                    exit(-1);
                }
                //
                auto strategy = node->getStrategy();
                for (auto action : validActions) {
                    //
                    utilities[index][action] = returned[index * 6 + current];
                    //
                    auto prob = strategy.at(action);
                    for (int p = 0; p < 6; p++) {
                        if (p == 0) {
                            nodeUtil[index * 6 + p] = 0.0;
                        }
                        //
                        nodeUtil[index * 6 + p] += returned[index * 6 + p] * prob;
                    }
                }
                //
                std::unordered_map<std::string, double> temp;
                for (auto action : validActions) {
                    auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                    // temp.emplace(action, regret * nowprob / maxprob);
                    temp.emplace(action, regret);
                }
                //
                node->incRegretSum(temp);
            }
        }
        //
        target->getHand()[0]->setCard(&card1);
        target->getHand()[1]->setCard(&card2);
        //
        return nodeUtil;
    } else {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        std::vector<std::string> actions;
        std::vector<double> probabilities;
        auto validStrategy = node->getStrategy();
        for (auto map : validStrategy) {
            actions.push_back(map.first);
            probabilities.push_back(map.second);
        }
        //
        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
        auto action = random_choice(mActionEng);
        //
        std::valarray<float> results;
        State st(state, actions[action]);
        if (state.getRound() == st.getRound()) {
            results = subgameSolve4(st, player, iter, prune, depth + 1);
        } else {
            std::vector<int> biasOther(6, 0);
            if (!st.getEngine()->checkGameOver()) {
                for (int s = 0; s < state.getEngine()->getPlayerNum(); s++) {
                    if (!st.getEngine() -> getPlayers(s)->isFold() && current != s) {
                        auto infoSetLeaf = st.infoSetdepthLimit(s) + "leaf";
                        //auto node = agent->GetNode(infoSet, validActions);
                        //auto node = BluePrint::GetInstance().tree()->findAgentNode(stage, infoSet);
                        std::set<std::string> mLeafActions;
                        mLeafActions.emplace("NULL");
                        mLeafActions.emplace("FOLD");
                        mLeafActions.emplace("CALL");
                        mLeafActions.emplace("RAISE");
                        validActions = mLeafActions;
                        auto agent = getAgent(st.getRound(), infoSetLeaf);
                        if (nullptr == agent) {
                            LOG_ERROR("invalid agent: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        auto node = agent->GetNode(infoSetLeaf, validActions);
                        if (nullptr == node) {
                            LOG_ERROR("invalid node: infoSet=" << infoSet);
                            exit(-1);
                        }
                        //
                        std::vector<std::string> actions;
                        std::vector<double> probabilities;
                        auto validStrategy = node->getStrategy();
                        for (auto map : validStrategy) {
                            actions.push_back(map.first);
                            probabilities.push_back(map.second);
                        }
                        //
                        std::discrete_distribution<int> random_choice(probabilities.begin(), probabilities.end());
                        auto action = random_choice(mActionEng);
                        biasOther[s] = action;
                    }
                }
            }
            //
            if (true) {
                State stt(st);
                results = rollout(stt, player, biasOther);
            }
        }
        //
        return results;
    }
}

std::valarray<float> Pluribus::lcfr(State &state, int player, int iter, int depth, std::vector<float> &reachProbs) {
    if (state.isTerminal()) {
        TEST_D("isTerminal: iter=" << iter << " depth=" << depth);
        return state.payoffMatrix(player);
    }
    //
    auto validActions = state.validActions();
    auto stage = state.getRound();
    auto current = state.getTurnIndex();
    std::valarray<float> nodeUtil(UNLEAGLE, 1326 * state.mNumPlayers);
    std::valarray<std::unordered_map<std::string, double>> utilities(1326);
    std::valarray<float> returned;
    auto target = state.getEngine()->getPlayers(player);
    auto card1 = *(target->getHand(0));
    auto card2 = *(target->getHand(1));
    //
    std::unordered_map<std::string, double> prevStrategy;
    if (true) {
        auto infoSet = state.infoSet();
        auto agent = getAgent(state.getRound(), infoSet);
        if (nullptr == agent) {
            LOG_ERROR("invalid agent: infoSet=" << infoSet);
            exit(-1);
        }
        //
        auto node = agent->GetNode(infoSet, validActions);
        if (nullptr == node) {
            LOG_ERROR("invalid node: infoSet=" << infoSet);
            exit(-1);
        }
        //
        prevStrategy = node->getStrategy();
    }
    //
    std::unordered_map<std::string, std::valarray<float>> matrix;
    for (auto action : validActions) {
        if (true) {
            auto playerReachProb = reachProbs[current];
            //
            reachProbs[current] *= prevStrategy[action];
            //
            State st(state, action);
            returned = lcfr(st, player, iter, depth + 1, reachProbs);
            //
            reachProbs[current] = playerReachProb;
            //
            matrix.emplace(action, returned);
        }
    }
    //
    for (int i = 0; i < 52; i++) {
        for (int j = i + 1; j < 52; j++) {
            auto index = pair_index(i, j);
            //
            bool invalid = false;
            for (auto action : validActions) {
                if (UNLEAGLE == matrix[action][index * 6]) {
                    invalid = true;
                    break;
                }
            }
            //
            if (invalid) {
                continue;
            }
            //
            target->getHand(0)->setCard(Card::fastId2str(i));
            target->getHand(1)->setCard(Card::fastId2str(j));
            //
            auto comb1 = Card::fastId2str(i) + "-" + Card::fastId2str(j);
            auto comb2 = Card::fastId2str(j) + "-" + Card::fastId2str(i);
            //
            auto infoSet = state.infoSet();
            auto agent = getAgent(state.getRound(), infoSet);
            if (nullptr == agent) {
                LOG_ERROR("invalid agent: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto node = agent->GetNode(infoSet, validActions);
            if (nullptr == node) {
                LOG_ERROR("invalid node: infoSet=" << infoSet);
                exit(-1);
            }
            //
            auto strategy = node->getStrategy();
            for (auto action : validActions) {
                //
                utilities[index][action] = returned[index * 6 + current];
                //
                auto prob = strategy.at(action);
                for (int p = 0; p < 6; p++) {
                    if (p == 0) {
                        nodeUtil[index * 6 + p] = 0.0;
                    }
                    //
                    nodeUtil[index * 6 + p] += returned[index * 6 + p] * prob;
                }
            }
            //
            std::unordered_map<std::string, double> temp;
            for (auto action : validActions) {
                auto regret = utilities[index].at(action) - nodeUtil[index * 6 + current];
                temp.emplace(action, regret);
            }
            //
            node->incRegretSum(temp);
            //
            if (current == player) {
                float reach_prob = 1.0f;
                for (int x = 0; x < state.mNumPlayers; ++x) {
                    if (x == player)
                        continue;
                    //
                    reach_prob *= reachProbs[x];
                }
                //
                auto pos_pow = 1.5;
                auto pos_param = std::pow((double)(iter + 1), pos_pow);
                auto pos_disc = pos_param / (pos_param + 1.0);
                //
                auto neg_pow = 0.5;
                auto neg_param = std::pow((double)(iter + 1), neg_pow);
                auto neg_disc = neg_param / (neg_param + 1.0);
                //
                auto strat_pow = 3.0;
                auto strat_disc = std::pow((double)(iter + 1) / (double)(iter + 2), strat_pow);
                // for (int x = 0; x < actions.size(); ++x) {
                //     regret[infoset][actions[x]] += reach_prob * (lcfr_value[x] - expected_value);
                //     regret[infoset][actions[x]] *= regret[infoset][actions[x]] > 0.0f ? pos_disc : neg_disc;
                //     //
                //     cumulative_strategy[infoset][actions[x]] += player_reach_prob[player] * infoset_strategy[x];
                //     cumulative_strategy[infoset][actions[x]] *= strat_disc;
                // }
            }
        }
    }
    //
    target->getHand(0)->setCard(&card1);
    target->getHand(1)->setCard(&card2);
    //
    return nodeUtil;
}

std::map<std::size_t, std::vector<std::string>> Pluribus::getClusPairs(std::vector<std::size_t> &boardCards) {
    std::map<std::size_t, std::vector<std::string>> clusPairs;
    //
    std::string boardCardStr;
    for (auto iter = boardCards.begin(); iter != boardCards.end(); iter++) {
        auto boardCard = Card::fastId2str(*iter);
        boardCardStr += boardCard;
    }
    TEST_I("boardCards=" << boardCardStr);
    //
    for (auto i = 0; i < 52; i++) {
        auto found1 = std::find(boardCards.begin(), boardCards.end(), i);
        if (found1 != boardCards.end())
            continue;
        //
        for (auto j = i + 1; j < 52; j++) {
            auto found2 = std::find(boardCards.begin(), boardCards.end(), j);
            if (found2 != boardCards.end())
                continue;
            //
            auto card1 = Card::fastId2str(i);
            auto card2 = Card::fastId2str(j);
            auto cardStr = card1 + card2 + boardCardStr;
            //
            auto hands = card1 + card2;
            auto clus = NumPy::GetInstance().ShmPointer()->TestCard(cardStr);
            auto found3 = clusPairs.find(clus);
            if (found3 == clusPairs.end()) {
                std::vector<std::string> v;
                v.push_back(hands);
                clusPairs.insert(std::make_pair(clus, v));
            } else {
                found3->second.push_back(hands);
            }
        }
    }
    //
    TEST_I("------- clus(" << clusPairs.size() << ") -------");
    for (auto iter = clusPairs.begin(); iter != clusPairs.end(); iter++) {
        std::ostringstream os;
        os << std::right << std::setw(4) << iter->first << " "
           << std::right << std::fixed << std::setprecision(3) << iter->second.size() / 1326.0;
        //
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); iter2++) {
            os << " " << *iter2;
        }
        //
        os << std::endl;
        TEST_I(os.str());
    }
    //
    return clusPairs;
}
