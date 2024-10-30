#include "State.hpp"
#include "GamePool.hpp"
#include "Configure.hpp"
#include "simulator.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <bitset>
#include <tuple>
#include <string>
#include <set>

State::State(int numPlayers): mEngine(nullptr), mNumPlayers(numPlayers), mRound(-1) {
    mHistory.clear();
    numSeats = 0;
    boardCards.clear();
    for (int i = 0; i < 6; i++) {
        handRanges[i].clear();
    }
    LOG_DEBUG("@State new game begin.");
    mEngine = GamePool::GetInstance().allocator();
    mEngine->ctor(mNumPlayers);
    LOG_DEBUG("@State new game over.");
}

State::State(const State &state, bool shuffle): mEngine(nullptr),
    mNumPlayers(state.mNumPlayers), mHistory(state.mHistory) {
    //
    mRound = -1;
    numSeats = 0;
    boardCards.clear();
    for (int i = 0; i < 6; i++) {
        auto hr = &(state.handRanges[i]);
        handRanges[i].clear();
        //
        for (auto iter = hr->begin(); iter != hr->end(); iter++) {
            handRanges[i].emplace(iter->first, iter->second);
        }
    }
    //
    if (this == &state) {
        LOG_FATAL("@Fuck you: same pointer: this=" << this);
        exit(0);
    }
    //
    mEngine = GamePool::GetInstance().allocator();
    if (mEngine == state.mEngine) {
        LOG_FATAL("@Fuck you: same pointer, oldPtr=" << state.mEngine << "newPtr=" << mEngine);
        exit(0);
    }
    //
    mEngine->clone(state.mEngine);
}

State::State(const State *state, bool shuffle): mEngine(nullptr),
    mNumPlayers(state->mNumPlayers), mHistory(state->mHistory) {
    //
    mRound = -1;
    numSeats = 0;
    boardCards.clear();
    for (int i = 0; i < 6; i++) {
        auto hr = &(state->handRanges[i]);
        handRanges[i].clear();
        //
        for (auto iter = hr->begin(); iter != hr->end(); iter++) {
            handRanges[i].emplace(iter->first, iter->second);
        }
    }
    //
    if (this == state) {
        LOG_FATAL("@Fuck you: same pointer: this=" << this);
        exit(0);
    }
    //
    mEngine = GamePool::GetInstance().allocator();
    if (mEngine == state->mEngine) {
        LOG_FATAL("@Fuck you: same pointer, oldPtr=" << state->mEngine << "newPtr=" << mEngine);
        exit(0);
    }
    //
    mEngine->clone(state->mEngine);
}

State::State(const State &state, const std::string &action, bool search): mEngine(nullptr),
    mNumPlayers(state.mNumPlayers), mHistory(state.mHistory) {
    mRound = -1;
    numSeats = 0;
    boardCards.clear();
    for (int i = 0; i < 6; i++) {
        auto hr = &(state.handRanges[i]);
        handRanges[i].clear();
        //
        for (auto iter = hr->begin(); iter != hr->end(); iter++) {
            handRanges[i].emplace(iter->first, iter->second);
        }
    }
    //
    if (this == &state) {
        LOG_FATAL("@Fuck you: same pointer: this=" << this);
        exit(0);
    }
    //
    mEngine = GamePool::GetInstance().allocator();
    if (mEngine == state.mEngine) {
        LOG_FATAL("@Fuck you: same pointer, oldPtr=" << state.mEngine << "newPtr=" << mEngine);
        exit(0);
    }
    //
    mEngine->clone(state.mEngine);
    //
    Command(action, search);
}

State::~State() {
    if (nullptr != mEngine) {
        GamePool::GetInstance().recoverer(mEngine);
    }
}

void State::Command(const std::string &action, bool search) {
    if (!isTerminal()) {
        doTurnProcess(action, search);
        doGameSchedule();
    }
}

static std::vector<std::vector<int>> preflop_class = {
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
    {1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 1, 2},
    {1, 2, 2, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2},
    {1, 2, 2, 2, 2, 1, 1, 0, 0, 0, 1, 1, 2},
    {0, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 2},
    {0, 0, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2},
    {0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    {1, 1, 1, 1, 2, 2, 3, 5, 4, 3, 3, 3, 5},
    {1, 1, 1, 1, 1, 2, 3, 3, 4, 6, 4, 4, 4},
    {1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 7, 4, 6},
    {2, 2, 2, 3, 3, 3, 4, 3, 4, 4, 4, 8, 7},
    {3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 8, 8},
};

std::unordered_map<std::string, double> State::returnsum(std::vector<float> weight) {
    auto vActions = this->validActions();
    vector<float> probList;
    std::unordered_map<std::string, double> result;
    for (const auto &aa : vActions) {
        if (aa == "FOLD") {
            probList.push_back(weight[0]);
        } else if (aa.find("RAISE1") != std::string::npos) {
            probList.push_back(weight[2]);
        } else if (aa.find("RAISE2") != std::string::npos) {
            probList.push_back(weight[3]);
        } else if (aa.find("RAISE3") != std::string::npos) {
            probList.push_back(weight[4]);
        } else if (aa.find("RAISE4") != std::string::npos) {
            probList.push_back(weight[5]);
        } else if (aa.find("RAISE5") != std::string::npos) {
            probList.push_back(weight[6]);
        } else if (aa.find("RAISE6") != std::string::npos) {
            probList.push_back(weight[7]);
        } else if (aa == "ALLIN") {
            probList.push_back(weight[8]);
        } else if (aa == "CALL" || aa == "CHECK") {
            probList.push_back(weight[1]);
        }
    }
    //
    int i_index = 0 ;
    for (const auto &aa : vActions) {
        result[aa] = double(probList[i_index]);
        i_index += 1;
    }
    //
    return result;
}

//
static Simulator gSimulator;
//
static std::unordered_map<std::string, std::vector<float>> default_strategy;
//
bool State::initDefault() {
    default_strategy.clear();
    //
    std::string filename = Configure::GetInstance().mResearchTestStrategy;
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR("can't open the file: " << filename);
        return false;
    }
    //
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string key;
        float value;
        if (std::getline(lineStream, key, ',')) {
            std::vector<float> values;
            while (lineStream >> value) {
                values.push_back(value);
                if (lineStream.peek() == ',') {
                    lineStream.ignore();
                }
            }
            //
            default_strategy.emplace(key, values);
        }
    }
    //
    LOG_DEBUG("default_strategy_size=" << default_strategy.size());
    return true;
}

struct EvaluateCard {
    int _rank;
    char _suit;
};

EvaluateCard make_card(const string &card_str) {
    char rank_char = card_str[0];
    int rank;
    if (isdigit(rank_char)) {
        rank = rank_char - '0';
    } else {
        switch (rank_char) {
        case 'T': rank = 10; break;
        case 'J': rank = 11; break;
        case 'Q': rank = 12; break;
        case 'K': rank = 13; break;
        case 'A': rank = 14; break;
        }
    }
    //
    char suit = card_str[1];
    return EvaluateCard{rank, suit};
}

vector< EvaluateCard > make_cards(const string &card_strs) {
    vector< EvaluateCard > cards;
    for (size_t i = 0; i < card_strs.length(); i += 2) {
        cards.push_back(make_card(card_strs.substr(i, 2)));
    }
    return cards;
}

int eval_high_cards(const vector<EvaluateCard> &cards) {
    vector<int> ranks;
    for (const auto &card : cards) {
        ranks.push_back(card._rank);
    }
    sort(ranks.begin(), ranks.end(), greater<int>());
    return ranks[0] * 16 + ranks[1];
}

int one_pair(const vector<EvaluateCard> &cards) {
    int rank = 0;
    bitset<15> mem = 0;
    for (const auto &card : cards) {
        if (mem.test(card._rank)) {
            rank = max(rank, card._rank);
        }
        mem.set(card._rank);
    }
    return rank;
}

vector<int> two_pair(const vector<EvaluateCard> &cards) {
    vector<int> ranks;
    bitset<15> mem = 0;
    for (const auto &card : cards) {
        if (mem.test(card._rank)) {
            ranks.push_back(card._rank);
        }
        mem.set(card._rank);
    }
    sort(ranks.begin(), ranks.end(), greater<int>());
    if (ranks.size() >= 2) {
        return vector<int> {ranks[0], ranks[1]};
    }
    return vector<int> {0};
}

int three_card(const vector<EvaluateCard> &cards) {
    map<int, int> counter;
    for (const auto &card : cards) {
        counter[card._rank]++;
    }
    for (const auto &item : counter) {
        if (item.second >= 3) {
            return item.first;
        }
    }
    return 0;
}

int four_card(const vector<EvaluateCard> &cards) {
    map<int, int> counter;
    for (const auto &card : cards) {
        counter[card._rank]++;
    }
    for (const auto &item : counter) {
        if (item.second >= 4) {
            return item.first;
        }
    }
    return 0;
}

tuple<int, int> full_house(const vector<EvaluateCard> &cards) {
    map<int, int> counter;
    int three_card_rank = 0;
    int two_card_rank = 0;
    for (const auto &card : cards) {
        counter[card._rank]++;
    }
    for (const auto &item : counter) {
        if (item.second >= 3) {
            three_card_rank = max(three_card_rank, item.first);
        } else if (item.second >= 2) {
            two_card_rank = max(two_card_rank, item.first);
        }
    }
    return make_tuple(three_card_rank, two_card_rank);
}

tuple<bool, vector<int>> is_flush(const vector<EvaluateCard> &cards) {
    map<char, vector<int>> suits;
    for (const auto &card : cards) {
        suits[card._suit].push_back(card._rank);
    }
    for (auto &[suit, ranks] : suits) {
        if (ranks.size() >= 5) {
            sort(ranks.begin(), ranks.end(), greater<int>());
            return make_tuple(true, ranks);
        }
    }
    return make_tuple(false, vector<int> {});
}

int is_straight(const vector<EvaluateCard> &cards) {
    //set<EvaluateCard> ranks(cards.begin(), cards.end());
    set<int> ranks;
    for (const auto &card : cards)
        ranks.insert(card._rank);
    for (const auto &rank : ranks) {
        if (ranks.count(rank + 1) && ranks.count(rank + 2) && ranks.count(rank + 3) && ranks.count(rank + 4)) {
            return rank + 4;
        }
    }
    if (ranks.count(14) && ranks.count(2) && ranks.count(3) && ranks.count(4) && ranks.count(5)) {
        return 5;
    }
    return 0;
}

int is_straight_flush(const vector<EvaluateCard> &cards) {
    bool flush;
    vector<int> flush_ranks;
    tie(flush, flush_ranks) = is_flush(cards);
    if (flush) {
        return is_straight(cards);
    }
    return 0;
}

tuple<int, vector<int>> get_hand_rank(const vector<EvaluateCard> &cards) {
    int straight_flush_rank = is_straight_flush(cards);
    if (straight_flush_rank > 0) {
        return make_tuple(1, vector<int> {straight_flush_rank});
    }

    int four_card_rank = four_card(cards);
    if (four_card_rank > 0) {
        return make_tuple(2, vector<int> {four_card_rank});
    }

    int three_card_rank, two_card_rank;
    tie(three_card_rank, two_card_rank) = full_house(cards);
    if (three_card_rank > 0 && two_card_rank > 0) {
        return make_tuple(3, vector<int> {three_card_rank, two_card_rank});
    }

    bool flush;
    vector<int> flush_ranks;
    tie(flush, flush_ranks) = is_flush(cards);
    if (flush) {
        return make_tuple(4, vector<int> {flush_ranks[0], flush_ranks[1], flush_ranks[2], flush_ranks[3], flush_ranks[4]});
    }

    int straight_rank = is_straight(cards);
    if (straight_rank > 0) {
        return make_tuple(5, vector<int> {straight_rank});
    }

    if (three_card_rank > 0) {
        return make_tuple(6, vector<int> {three_card_rank});
    }

    vector<int> two_pair_ranks = two_pair(cards);
    if (two_pair_ranks[0] != 0) {
        return make_tuple(7, vector<int> { two_pair_ranks[0], two_pair_ranks[1]});
    }

    int one_pair_rank = one_pair(cards);
    if (one_pair_rank > 0) {
        return make_tuple(8, vector<int> {one_pair_rank});
    }

    auto high_rank = eval_high_cards(cards);
    return make_tuple(9, vector<int> {high_rank});
}

static int evaluate_hand_strength(const string &input) {
    string community_card_str = input.substr(4);
    string hole_card_str = input.substr(0, 4);
    //
    vector<EvaluateCard> community_cards = make_cards(community_card_str);
    vector<EvaluateCard> hole_cards = make_cards(hole_card_str);
    //
    vector<EvaluateCard> combined_cards(hole_cards);
    combined_cards.insert(combined_cards.end(), community_cards.begin(), community_cards.end());
    //
    auto [community_cls, community_rank] = get_hand_rank(community_cards);
    auto [combined_cls, combined_rank] = get_hand_rank(combined_cards);
    //cout << "Community hand: " << community_hand << " with rank: " << community_rank << endl;
    //cout << "Combined hand: " << combined_hand << " with rank: " << combined_rank << endl;
    LOG_DEBUG( "Community hand: " << community_cls << " with rank: " << community_rank[0] << endl);
    LOG_DEBUG( "Combined hand: " << combined_cls << " with rank: " << combined_rank[0] << endl);
    if (combined_cls == 1) { //straight_flush
        if (community_cls == 1 and combined_rank[0] == community_rank[0]) {
            return 7;
        }
        //
        return 0 ;
    }
    //
    if (combined_cls == 2) { //four
        if (community_cls == 2) {
            return 7;
        }
        //
        return 0 ;
    }
    //
    if (combined_cls == 3) { //fullhouse
        if (community_cls == 3 and community_rank[0] == combined_rank[0] and community_rank[1] == combined_rank[1]) {
            return 6;
        }
        //
        if (community_cls == 3 and community_rank[0] == combined_rank[0] and community_rank[1] < combined_rank[1] and combined_rank[1] == 14) {
            return 3;
        }
        //
        if (community_cls == 6 and community_rank[0] == combined_rank[0] ) {
            return 6;
        }
        //
        if (community_cls == 6 and community_rank[0] == combined_rank[0] and combined_rank[1] == 14) {
            return 3;
        }
        //
        return 0 ;
    }
    //
    if (combined_cls == 4) { //flush
        map<char, vector<int>> community_suits;
        map<char, vector<int>> combined_suits;
        for (const auto &card : community_cards) {
            community_suits[card._suit].push_back(card._rank);
        }
        //
        for (const auto &card : combined_cards) {
            combined_suits[card._suit].push_back(card._rank);
        }
        //
        int involved_hole_card = 0;
        int max_hole_card_rank = 0;
        for (const auto &suit : combined_suits) {
            if (combined_suits[suit.first].size() >= 5) {
                sort(combined_suits[suit.first].begin(), combined_suits[suit.first].end(), greater<int>());
                int hole_card_1_rank = combined_cards[0]._suit == suit.first ? combined_cards[0]._rank : 0;
                int hole_card_2_rank = combined_cards[1]._suit == suit.first ? combined_cards[1]._rank : 0;
                int current_involved_hole_cards = (hole_card_1_rank > 0) + (hole_card_2_rank > 0);
                if (current_involved_hole_cards > involved_hole_card) {
                    involved_hole_card = current_involved_hole_cards;
                    max_hole_card_rank = max(hole_card_1_rank, hole_card_2_rank);
                } else if (current_involved_hole_cards == involved_hole_card) {
                    max_hole_card_rank = max(max_hole_card_rank, max(hole_card_1_rank, hole_card_2_rank));
                }
            }
        }
        //
        if (involved_hole_card == 0) {
            return 8;
        }
        //
        if (involved_hole_card == 1) {
            if (max_hole_card_rank == 14)
                return 0;
            else if (max_hole_card_rank == 13 || max_hole_card_rank == 12 || max_hole_card_rank == 11)
                return 4;
            else
                return 5;
        }
        //
        if (involved_hole_card == 2 ) {
            if (max_hole_card_rank > 12)
                return 0;
            else if (max_hole_card_rank > 7)
                return 2;
            else
                return 4;
        }
        //
        return 0 ;
    }
    //
    if (combined_cls == 5) { //straight
        int dangerous = 0 ;
        map<char, int> suit_count;
        for (const auto &card : community_cards) {
            suit_count[card._suit]++;
            if (suit_count[card._suit] >= 4) {
                dangerous = 1;
            }
        }
        //
        vector<int> community_ranks;
        for (const auto &card : community_cards) {
            community_ranks.push_back(card._rank);
            if (card._rank == 14) {
                community_ranks.push_back(1);
            }
        }
        //
        std::sort(community_ranks.begin(), community_ranks.end());
        //
        int max_hole_card_rank = max(combined_cards[0]._rank, combined_cards[1]._rank);
        int straight_count = 0;
        for (int start_rank = 1; start_rank <= 10; ++start_rank) {
            int consecutive_count = 0;
            for (int rank = start_rank; rank < start_rank + 5; ++rank) {
                if (std::find(community_ranks.begin(), community_ranks.end(), rank) != community_ranks.end()) {
                    consecutive_count++;
                }

            }
            //
            if (consecutive_count >= 4 and start_rank + 4 > combined_rank[0] ) {
                dangerous = 1;
            }
        }
        //
        if (dangerous == 1 or community_cls == 6) {
            return 5;
        }
        //
        if (community_cls == 5 and combined_rank[0] == community_rank[0]) {
            return 7;
        }
        //
        return 1 ;
    }
    //
    int isdangerous = 0;
    map<char, int> suit_count;
    map<char, int> suit_count_for_two;
    for (const auto &card : community_cards) {
        suit_count[card._suit]++;
        suit_count_for_two[card._suit]++;
        if (suit_count[card._suit] >= 4) {
            isdangerous = 1;
        } else if (suit_count_for_two[card._suit] >= 3 && isdangerous == 0) {
            isdangerous = 2;
        }
    }
    //
    vector<int> community_ranks;
    for (const auto &card : community_cards) {
        community_ranks.push_back(card._rank);
        if (card._rank == 14) {
            community_ranks.push_back(1);
        }
    }
    //
    sort(community_ranks.begin(), community_ranks.end());
    //
    int straight_count = 0;
    for (int start_rank = 1; start_rank <= 10; ++start_rank) {
        int straight_count  = 0;
        for (int rank = start_rank; rank < start_rank + 5; ++rank) {
            if (std::find(community_ranks.begin(), community_ranks.end(), rank) != community_ranks.end()) {
                straight_count++;
            }
        }
        //
        if (straight_count >= 4  ) {
            isdangerous = 1;
        }
    }
    //
    if (combined_cls == 6) { //three
        if (community_cls == 6)
            return 8;
        //
        if (isdangerous == 1)
            return 5;
        //if(isdangerous == 2)
        //    return 4;
        return 0 ;
    }
    //
    int max_hand_rank = std::max(combined_cards[0]._rank, combined_cards[1]._rank);
    int max_community_rank = -1;
    for (const auto &card : community_cards) {
        max_community_rank = std::max(max_community_rank, card._rank);
    }
    //
    if (combined_cls == 7) { //twopair
        vector<EvaluateCard> hand;
        hand.push_back( combined_cards[0]);
        hand.push_back( combined_cards[1]);
        if (hand[0]._rank != hand[1]._rank) {
            bool match_first = false;
            bool match_second = false;
            for (const auto &card : hand) {
                if (card._rank == combined_rank[0]) {
                    match_first = true;
                } else if (card._rank == combined_rank[1]) {
                    match_second = true;
                }
            }
            //
            if (match_first && match_second) {
                return 1;
            } else if (match_first || match_second) {
                // ¿¿¿¿¿¿¿¿¿rank
                std::vector<int> community_ranks;
                for (const auto &card : community_cards) {
                    community_ranks.push_back(card._rank);
                }
                //
                std::sort(community_ranks.begin(), community_ranks.end(), std::greater<int>());
                int matched_rank = match_first ? combined_rank[0] : combined_rank[1];
                if (matched_rank == community_ranks.front()) {
                    if (hand[0]._rank < 7 || hand[1]._rank < 7 )
                        return 5;
                    //
                    return 4;
                } else if (matched_rank == community_ranks.back()) {
                    return 6;
                } else {
                    return 5;
                }
            } else {
                return 8;
            }
        } else {
            int max_community_rank = -1;
            int min_community_rank = 15;
            for (const auto &card : community_cards) {
                max_community_rank = std::max(max_community_rank, card._rank);
                min_community_rank = std::min(min_community_rank, card._rank);
            }
            //
            if (hand[0]._rank > max_community_rank) {
                if (isdangerous == 1)
                    return 5;
                if (isdangerous == 2)
                    return 3;
                return 3;
            } else if (hand[0]._rank > min_community_rank) {
                return 5;
            } else {
                return 6;
            }
        }
    }
    //
    if (combined_cls == 8) { //pair
        vector<EvaluateCard> hand;
        hand.push_back( combined_cards[0]);
        hand.push_back( combined_cards[1]);
        if (hand[0]._rank != hand[1]._rank) {
            bool has_matching_card = false;
            for (const auto &card : hand) {
                if (card._rank == combined_rank[0] ) {
                    has_matching_card = true;
                    break;
                }
            }
            //
            if (has_matching_card) {
                std::vector<int> community_ranks;
                for (const auto &card : community_cards) {
                    community_ranks.push_back(card._rank);
                }
                //
                std::sort(community_ranks.begin(), community_ranks.end(), std::greater<int>());
                if (combined_rank[0] == community_ranks.front()) {
                    if (hand[0]._rank < 7 || hand[1]._rank < 7 ) {
                        return 5;
                    }
                    return 4;
                } else if ( combined_rank[0] == community_ranks.back()) {
                    return 6;
                } else {
                    return 5;
                }
            } else {
                return 8;
            }
        } else {
            int max_community_rank = -1;
            int min_community_rank = 15;
            for (const auto &card : community_cards) {
                max_community_rank = std::max(max_community_rank, card._rank);
                min_community_rank = std::min(min_community_rank, card._rank);
            }
            //
            if (hand[0]._rank > max_community_rank) {
                /*
                if (std::max(hand[0]._rank, hand[1]._rank) == 14 ||
                    (community_cards.size() == 3 || community_cards.size() == 4) && combined_cards[0]._rank > max_community_rank && combined_cards[1]._rank > max_community_rank)  {
                    return 7;
                }*/
                if (isdangerous == 1)
                    return 5;
                if (isdangerous == 2)
                    return 4;
                return 3;
            } else if (hand[0]._rank > min_community_rank) {
                return 5;
            } else {
                return 6;
            }
        }
        //
        return 1 ;
    }
    //
    //LOG_INFO("cardpower:" << cardpower << " " << cardStr << endl );
    LOG_DEBUG(" ");
    if (max_hand_rank == 14) {
        return 7;
    }
    //
    if (community_cards.size() == 3 || community_cards.size() == 4) {
        if (combined_cards[0]._rank > max_community_rank
                && combined_cards[1]._rank > max_community_rank) {
            return 7;
        }
    }
    //
    return 8;
    ////////////////return 8 - evaluate(input);
}
//
static int get_rank(char card) {
    string ranks = "23456789TJQKA";
    return ranks.find(card);
}
//
static int get_suit(char card) {
    string suits = "SHDC";
    return suits.find(card);
}
//
static bool open_ended_straight_draw(const vector<int> &ranks) {
    for (size_t i = 0; i < ranks.size() - 3; i++) {
        if (ranks[i + 3] - ranks[i] == 3) {
            return true;
        }
    }
    return false;
}
//
static bool gutshot_straight_draw(const vector<int> &ranks) {
    for (size_t i = 0; i < ranks.size() - 2; i++) {
        if (ranks[i + 2] - ranks[i] == 2) {
            return true;
        }
    }
    return false;
}
//
static bool flush_draw(const vector<string> &cards) {
    int suit_counts[4] = {0};
    for (const auto &card : cards) {
        suit_counts[get_suit(card[1])]++;
    }
    for (int count : suit_counts) {
        if (count == 4) return true;
    }
    return false;
}
//
static int evaluate_hand(string &input) {
    vector<string> hand_cards = {input.substr(0, 2), input.substr(2, 2)};
    vector<string> public_cards;
    for (size_t i = 4; i < input.size(); i += 2) {
        public_cards.push_back(input.substr(i, 2));
    }
    vector<string> all_cards = hand_cards;
    all_cards.insert(all_cards.end(), public_cards.begin(), public_cards.end());

    vector<int> ranks;
    for (const auto &card : all_cards) {
        ranks.push_back(get_rank(card[0]));
    }
    //
    std::sort(ranks.begin(), ranks.end());
    //
    bool oesd = open_ended_straight_draw(ranks);
    bool gsd = gutshot_straight_draw(ranks);
    bool fd = flush_draw(all_cards);
    if ((oesd && fd) || (fd && gsd)) {
        return 3;
    } else if (oesd || fd) {
        return 2;
    } else if (gsd || (flush_draw(hand_cards) && flush_draw(public_cards))) {
        return 1;
    }
    //
    return 0;
}

std::unordered_map<std::string, double> State::rawStrategy() {
    std::vector<float> aaa = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    //
    auto aa =  returnsum(aaa);
    //auto aa = initialRegret();
    float sum = 0 ;
    for (auto iter = aa.begin(); iter != aa.end(); iter++) {
        sum += (*iter).second;
    }
    //
    for (auto iter = aa.begin(); iter != aa.end(); iter++) {
        aa[(*iter).first] /= sum;
    }
    //
    return aa;
}

std::unordered_map<std::string, double> State::initialRegret() {
    int n_chips_to_call_id = 0;
    int biggest_bet_id = 0 ;
    auto player = getEngine()-> getPlayers(getEngine()->getTurnIndex());
    int allpot = PLAYER_CHIPS - getEngine()-> getPlayers(0)->getChips();
    allpot += PLAYER_CHIPS - getEngine()-> getPlayers(1)->getChips();
    allpot += PLAYER_CHIPS - getEngine()-> getPlayers(2)->getChips();
    allpot += PLAYER_CHIPS - getEngine()-> getPlayers(3)->getChips();
    allpot += PLAYER_CHIPS - getEngine()-> getPlayers(4)->getChips();
    allpot += PLAYER_CHIPS - getEngine()-> getPlayers(5)->getChips();
    //
    int cur = player->getChips();
    int all_pot = getEngine()->getCurrentPot();
    int dif = getEngine()->getRoundMaxBet() - player->getState().bet;
    int n_chips_to_call_i = dif;
    int n_chips_to_call = dif;
    int biggest_bet_i = PLAYER_CHIPS - player->getChips() + dif; // warning!!!!!!!!!
    float pot_size = (float)n_chips_to_call_i / all_pot;
    //float n_chips_to_call = (float)n_chips_to_call_i / biggest_bet_i;
    float biggest_bet = biggest_bet_i / 200.0;
    int pot_afterflop_id = 0 ;
    std::vector<float> initregret ;
    std::unordered_map<std::string, double> result;
    //double n_chips_to_call = 10; // Example value
    //double all_pot = 100; // Example value
    int curRound = getEngine()->getRound();
    int pot_size_id = 0;
    if (pot_size <= 0.13) {
        pot_size_id = 0;
    } else if (pot_size <= 0.30) {
        pot_size_id = 1;
    } else if (pot_size <= 0.38) {
        pot_size_id = 2;
    } else if (pot_size <= 0.46) {
        pot_size_id = 3;
    } else if (pot_size <= 0.55) {
        pot_size_id = 4;
    } else if (pot_size <= 0.63) {
        pot_size_id = 5;
    } else if (pot_size <= 0.7) {
        pot_size_id = 6;
    } else if (pot_size <= 0.775) {
        pot_size_id = 7;
    } else if (pot_size <= 0.815) {
        pot_size_id = 8;
    } else if (pot_size <= 0.85) {
        pot_size_id = 9;
    } else if (pot_size <= 0.9) {
        pot_size_id = 10;
    } else if (pot_size <= 0.925) {
        pot_size_id = 11;
    } else if (pot_size <= 0.944) {
        pot_size_id = 12;
    } else if (pot_size <= 0.96) {
        pot_size_id = 13;
    } else if (pot_size <= 0.972) {
        pot_size_id = 14;
    } else if (pot_size <= 0.9775) {
        pot_size_id = 15;
    } else if (pot_size <= 0.9825) {
        pot_size_id = 16;
    } else if (pot_size <= 0.9875) {
        pot_size_id = 17;
    } else {
        pot_size_id = 18;
    }
    //
    if (pot_size_id == 0) {
        n_chips_to_call = 0;
    }
    //
    if (n_chips_to_call != 0) {
        result = returnsum(default_strategy["default_fold"]);
    } else {
        result = returnsum(default_strategy["default_call"]);
    }
    //
    int beforactivepeople = getEngine()->getBeforeActivePlayerNum(getEngine()->getTurnIndex());
    int afteractivepeople =  getEngine()->getBehindActivePlayerNum(getEngine()->getTurnIndex());
    std::vector<std::string> legal_actions =  validActionsV();
    int legal_actions_size = legal_actions.size();
    int len_legal_actions = legal_actions.size();
    if (getEngine()->getRound() == GAME_ROUND_PREFLOP) {
        std::vector<int> ranks;
        std::vector<char> suits;
        //for (const auto& card : current_player.cards) {
        //    ranks.push_back(card.rank_int);
        //    suits.push_back(card.suit);
        std::sort(ranks.rbegin(), ranks.rend());
        bool suited = (suits.size() == std::set<char>(suits.begin(), suits.end()).size()) ? false : true;
        int clusterid = getEngine()->getCardCluster(getEngine()-> getTurnIndex());
        int row = clusterid % 13;
        int height = clusterid / 13;
        int classid = preflop_class[height][row];
        int callid = 0;
        if (biggest_bet < 2) {
            callid = 1; // limp
        } else if (biggest_bet <= 6) {
            callid = 2; // open raise or call open raise
        } else if (biggest_bet <= 24) {
            callid = 4; // call 3bet or 3bet
        } else if (biggest_bet <= 48) {
            callid = 5; // call 4bet or 4bet
        } else if (biggest_bet < 230) {
            callid = 6; // call 5bet or 5bet
        } else {
            callid = 6; // call 6bet or 6bet
        }
        //
        int positionfix = 0;
        int limpindex = 0;
        int player_i_index =  getEngine()->getBeforeAllPlayerNum( getEngine()->getTurnIndex());
        int _player_i_index = player_i_index;
        classid = classid - 1;
        this->callid = callid ;
        this->cardpower = classid ;
        if (height == row) {
            this->pair = 1;
        } else {
            this->pair = 0;
        }
        //if (classid > 2)
        //    classid = classid + 2;
        if (player_i_index == 0) { // utg
            if (classid < 4) {
                positionfix = -2;
            }
        } else if (player_i_index == 1) { // MP
            if (classid < 3) {
                positionfix = -1;
            }
        } else if (player_i_index == 2) { // co
            positionfix = 0;
        } else if (player_i_index == 3) { // bt
            if (classid == 0) {
                positionfix = 1;
            }
        } else if (player_i_index == 4) { // sb
            if (classid == 1 || classid == 0) {
                positionfix = 1;
            }
        } else if (player_i_index == 5) { // bb
            if (beforactivepeople + afteractivepeople > 1) {
                if (classid < 1) {
                    positionfix = 0;
                } else if (classid == 1) {
                    positionfix = 0;
                }
            } else {
                if (classid == 0) {
                    positionfix = 2;
                } else if (classid == 1) {
                    positionfix = 1;
                }
            }
        }
        //print("prefloppower %d  %d/n",callid,classid + positionfix);
        //LOG_INFO("preflop cardpower:" << callid << " "  << classid + positionfix << endl );

        if (classid + positionfix < callid) { // fold
            // Fold
            if (n_chips_to_call == 0) {
                if (legal_actions.size() != 3) {
                    result = returnsum(default_strategy["0f_100c_0r"]);
                } else {
                    result = returnsum(default_strategy["0f_100c_0r"]);
                }
            } else {
                if (callid < 4) {
                    result = returnsum(default_strategy["pre_f_cidm5"]);
                } else {
                    result = returnsum(default_strategy["pre_f_cidb5"]);
                }
            }
        } else if (classid + positionfix == callid) {  // call
            if (n_chips_to_call == 0) {
                if (callid < 4) {
                    if (legal_actions.size() != 3) {
                        result = returnsum(default_strategy["pre_c_cidm5"]);
                    } else {
                        result = returnsum(default_strategy["pre_c_cidm5_3"]);
                    }
                } else {
                    if (legal_actions.size() != 3) {
                        result = returnsum(default_strategy["pre_c_cidb5"]);
                    } else {
                        result = returnsum(default_strategy["pre_c_cidb5_3"]);
                    }
                }
            } else {
                if (callid == 1) {
                    if (beforactivepeople + afteractivepeople == 1 && (_player_i_index == 4 || _player_i_index == 5)) {
                        result = returnsum(default_strategy["pre_cr_cid1"]);
                    } else {
                        result = returnsum(default_strategy["pre_fr_cid1"]);
                    }
                } else if (callid != 5 && callid != 6 && callid != 7) {
                    result = returnsum(default_strategy["pre_cr_cidm5"]);
                } else {
                    result = returnsum(default_strategy["pre_cr_cidb5"]);
                }
            }
        } else if (classid + positionfix > callid && n_chips_to_call != 0) { // ·´¼Ó×¢raise
            int callcha = classid + positionfix - callid;
            if (callid == 1) {
                result = returnsum(default_strategy["pre_rr_cid1"]);
            } else if (callid == 2) { // 3bet 50 ¸ÅÂÊ
                if (classid + positionfix - callid == 1) {
                    result = returnsum(default_strategy["pre_rr_cid2"]);
                } else if (classid + positionfix - callid == 2) {
                    if (beforactivepeople == 1) {
                        result = returnsum(default_strategy["pre_rr2a_cid2"]);
                    } else {
                        std::vector<float> initregret = default_strategy["pre_rr2_cid2"];
                        initregret[5] += beforactivepeople;
                        initregret[6] += beforactivepeople;
                        result = returnsum(initregret);
                    }
                } else {
                    result = returnsum(default_strategy["pre_rrb2_cid2"]);
                }
            } else if (callid == 4) { // 4bet 30¸ÅÂÊ
                if (classid + positionfix - callid == 1) {
                    result = returnsum(default_strategy["pre_rr_cid4"]);
                } else if (classid + positionfix - callid == 2) {
                    if (beforactivepeople == 1) {
                        result = returnsum(default_strategy["pre_rr2a_cid4"]);
                    } else {
                        result = returnsum(default_strategy["pre_rr2_cid4"]);
                    }
                } else {
                    result = returnsum(default_strategy["pre_rrb2_cid4"]);
                }
            } else if (callid == 5) { // 5bet 20 ¸ÅÂÊ
                if (classid + positionfix - callid == 1) {
                    result = returnsum(default_strategy["pre_rr_cid5"]);
                } else if (classid + positionfix - callid == 2) {
                    if (beforactivepeople == 1) {
                        result = returnsum(default_strategy["pre_rr2a_cid5"]);
                    } else {
                        result = returnsum(default_strategy["pre_rr2_cid5"]);
                    }
                } else {
                    result = returnsum(default_strategy["pre_rrb2_cid5"]);
                }
            } else if (callid == 6) { // 5bet 20 ¸ÅÂÊ
                result = returnsum(default_strategy["pre_rr_cid6"]);
            }
        }
    } else {
        int callid = 0;
        double pot_afterflop_id = 0;
        float pot_preflop = mEngine->getPotPreflop();
        double pot_afterflop = pot_preflop / (all_pot * (1 - pot_size));
        double  pot_aafterflop_id = 0;
        double pot_aafterflop = pot_preflop / (all_pot * (1 + pot_size));
        int multi_enter = mEngine->multi_enter;
        // The rest of the code from the first block
        if (pot_afterflop < 0.033) {
            pot_afterflop_id = 7;
        } else if (pot_afterflop < 0.047) {
            pot_afterflop_id = 6;
        } else if (pot_afterflop < 0.076) {
            pot_afterflop_id = 5;
        } else if (pot_afterflop < 0.12) {
            pot_afterflop_id = 4;
        } else if (pot_afterflop < 0.3) {
            pot_afterflop_id = 3;
        } else if (pot_afterflop < 0.45) {
            pot_afterflop_id = 2;
        } else if (pot_afterflop < 0.625) {
            pot_afterflop_id = 1;
        } else {
            pot_afterflop_id = 0;
        }
        //
        if (pot_aafterflop < 0.033) {
            pot_aafterflop_id = 7;
        } else if (pot_aafterflop < 0.047) {
            pot_aafterflop_id = 6;
        } else if (pot_aafterflop < 0.076) {
            pot_aafterflop_id = 5;
        } else if (pot_aafterflop < 0.109) {
            pot_aafterflop_id = 4;
        } else if (pot_aafterflop < 0.3) {
            pot_aafterflop_id = 3;
        } else if (pot_aafterflop < 0.45) {
            pot_aafterflop_id = 2;
        } else if (pot_aafterflop < 0.612) {
            pot_aafterflop_id = 1;
        } else {
            pot_aafterflop_id = 0;
        }
        //
        std::string cardStr = "";
        cardStr += player->getHand(0)->strCluster();
        cardStr += player->getHand(1)->strCluster();
        if (curRound > 1) {
            cardStr += mEngine -> getFlopCard()[0]->strCluster();
            cardStr += mEngine -> getFlopCard()[1]->strCluster();
            cardStr += mEngine -> getFlopCard()[2]->strCluster();
        }
        if (curRound > 2) {
            cardStr += mEngine ->  getTurnCard()->strCluster();
        }
        if (curRound > 3) {
            cardStr += mEngine -> getRiverCard()->strCluster();
        }
        //int clusterid = getCluster();
        int cardpower = evaluate_hand_strength(cardStr);
        this->cardpower = cardpower;

        int listernpower = evaluate_hand(cardStr);
        LOG_DEBUG("cardpower:" << cardpower << " " << cardStr << listernpower << "multi_enter" << multi_enter << endl );
        LOG_DEBUG(" ");
        int highindex = 0;
        int publicmaxlen = 1;
        int cardpowerold = cardpower;
        cardpower = 7 - cardpower;
        int remain_round = 0;
        int limit_value = 0;
        if (getEngine()->getRound()  == GAME_ROUND_FLOP) {
            remain_round = 2;
            limit_value = 3;
            if (multi_enter == 1) {
                limit_value = 3;
            } else if (multi_enter == 0) {
                limit_value = 2;
            } else if (multi_enter > 1) {
                limit_value = 4;
            }

        } else if (getEngine()->getRound()  == GAME_ROUND_TURN) {
            remain_round = 1;
            limit_value = 3;
            if (multi_enter == 1) {
                limit_value = 3;
            } else if (multi_enter == 0) {
                limit_value = 1;
            } else if (multi_enter > 1) {
                limit_value = 4;
            }
        } else if (getEngine()->getRound()  == GAME_ROUND_RIVER) {
            remain_round = 0;
            if (multi_enter == 1) {
                limit_value = 2;
            } else if (multi_enter == 0) {
                limit_value = 1;
            } else if (multi_enter > 1) {
                limit_value = 4;
            }
        }
        //
        double positionfix = 2.5;
        if (afteractivepeople == 0) {
            positionfix = 1;
        }
        //
        double after_flop_fix = 3;
        if (pot_afterflop_id == 2 || pot_afterflop_id == 1) {
            after_flop_fix = 1.5;
        } else if (pot_afterflop_id == 0) {
            after_flop_fix = 1;
        } else if (pot_afterflop_id > 2)  {
            after_flop_fix = 2;
        }
        //printf("prefloppower %d %d %d", pot_afterflop_id,pot_aafterflop_id,cardpower);
        if (n_chips_to_call == 0) {
            if (cardpower > limit_value) {  // ¼ÛÖµÏÂ×¢
                if (cardpower > pot_afterflop_id) {  // Èç¹ûÏÂ×¢Ã»ÓÐÂú
                    double powercha = cardpower - multi_enter - pot_afterflop_id;
                    if ((powercha == 1) && (remain_round == 2)) {  // µÍÆµÐ¡×¢
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["f_v_s_9"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["f_v_s_3"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        }
                        return result;
                    } else if ((powercha == 1) && (remain_round == 1)) {  // ÖÐÆµÐ¡×¢
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["t_v_s_9"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["t_v_s_3"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        }
                        return result;
                    } else if ((powercha == 1) && (remain_round == 0)) {  // ¸ßÆµÐ¡×¢
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["r_v_s_9"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["r_v_s_3"];
                            initregret[1] *= positionfix;
                            result = returnsum(initregret);
                        }
                        return result;
                    } else if (powercha > 1 && remain_round == 2) {  // µÍÆµ´ó×¢
                        if (cardpower - pot_afterflop_id > 2) {
                            initregret = default_strategy["f_v_bb2_9"];
                            initregret[1] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["f_v_bb2_3"];
                                initregret[1] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (powercha == 2) {
                            initregret = default_strategy["f_v_b2_9"];
                            initregret[1] *= positionfix;
                            initregret[3] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["f_v_b2_3"];
                                initregret[1] *= positionfix;
                                initregret[3] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        }
                    } else if (powercha > 1 && remain_round == 1) {  // ÖÐÆµ´ó×¢
                        if (powercha > 2) {
                            initregret = default_strategy["t_v_bb2_9"];
                            initregret[1] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["t_v_b2_3"];
                                initregret[1] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (powercha == 2) {
                            initregret = default_strategy["t_v_b2_9"];
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["t_v_b2_3"];
                                result = returnsum(initregret);
                            }
                            return result;
                        }
                    } else if (powercha > 1 && remain_round == 0) {  // ¸ßÆµ´ó×¢
                        if (powercha > 3) {
                            initregret = default_strategy["r_v_bb3_9"];
                            initregret[3] *= positionfix;
                            initregret[4] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_v_bb3_3"];
                                initregret[3] *= positionfix;
                                initregret[4] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (powercha > 2) {
                            initregret = default_strategy["r_v_bb2_9"];
                            initregret[3] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_v_bb2_3"];
                                initregret[3] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (powercha == 2) {
                            initregret = default_strategy["r_v_b2_9"];
                            initregret[1] *= positionfix;
                            if (legal_actions.size() != 3) {
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_v_b2_3"];
                                initregret[1] *= positionfix;
                                result = returnsum(initregret);
                            }
                            return result;
                        }
                    }
                } else {
                    initregret = default_strategy["g_v_c_9"];
                    initregret[1] *= positionfix;
                    if (legal_actions.size() != 3) {
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_v_c_3"];
                        initregret[1] *= positionfix;
                        result = returnsum(initregret);
                    }
                }
            } else if (remain_round == 2) { //bluff
                if (cardpower == -1 || cardpower == 0) {
                    if (multi_enter != 0) {
                        if (listernpower == 1) {
                            listernpower = 0;
                        }
                    }
                    if (listernpower == 3) {
                        if (len_legal_actions != 3) {
                            initregret = default_strategy["f_b_l3_9"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["f_b_l3_3"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        }
                        return result;
                    } else if (listernpower == 2) {
                        if (len_legal_actions != 3) {
                            initregret = default_strategy["f_b_l2_9"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["f_b_l2_3"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        }
                        return result;
                    } else if (listernpower == 1) {
                        if (len_legal_actions != 3) {
                            initregret = default_strategy["f_b_l1_9"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["f_b_l1_3"];
                            initregret[1] *= positionfix * after_flop_fix;
                            result = returnsum(initregret);
                        }
                        return result;
                    }
                }
            } else if (remain_round == 1) {
                if (multi_enter != 0) {
                    if (listernpower == 1) {
                        listernpower = 0;
                    }
                }
                if (listernpower == 3) {

                    if (len_legal_actions != 3) {
                        initregret = default_strategy["t_b_l3_9"];
                        initregret[1] *= after_flop_fix;
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["t_b_l3_3"];
                        initregret[1] *= after_flop_fix;
                        result = returnsum(initregret);
                    }
                    return result;
                } else if (listernpower == 2) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["t_b_l2_9"];
                        initregret[1] *= after_flop_fix;
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["t_b_l2_3"];
                        result = returnsum(initregret);
                    }
                    return result;
                } else if (listernpower == 1 && pot_afterflop_id > 2) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["t_b_l1b2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["t_b_l1b2_3"];
                        result = returnsum(initregret);
                    }
                    return result;
                } else if (listernpower == 1 && pot_afterflop_id < 2 && cardpower == 0) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["t_b_l1m2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["t_b_l1m2_3"];
                        result = returnsum(initregret);
                    }
                    return result;
                } else if (listernpower == 1 && pot_afterflop_id == 2 && cardpower == 0) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["t_b_l12_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["t_b_l12_3"];
                        result = returnsum(initregret);
                    }
                    return result;
                }
            } else if (remain_round == 0) {
                if (true) {
                    if (publicmaxlen > 2 && highindex == 1) {
                        if (listernpower == 3) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l3mb2h1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l3mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (listernpower == 2) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l2mb2h1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l2mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (listernpower == 1) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l1mb2h1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l1mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                            return result;
                        }
                    } else if (publicmaxlen > 2 || highindex == 1) {
                        if (listernpower == 3) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l3mb2oh1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l3mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                        } else if (listernpower == 2) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l2mb2oh1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l2mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                            return result;
                        } else if (listernpower == 1) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["r_b_l1mb2oh1_9"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["r_b_l1mb2h1_3"];
                                initregret.resize(9);
                                result = returnsum(initregret);
                            }
                            return result;
                        }
                    }
                }
            }
            // Continue with the rest of the code
            if (listernpower != 1) {
                if (remain_round == 0) {
                    if (legal_actions.size() != 3) {
                        initregret = default_strategy["gr_b_ln1_9"];
                        initregret.resize(9);
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["gr_b_ln1_3"];
                        initregret.resize(9);
                        result = returnsum(initregret);
                    }
                } else {
                    if (legal_actions.size() != 3) {
                        initregret = default_strategy["gnr_b_ln1_9"];
                        initregret.resize(9);
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["gnr_b_ln1_3"];
                        initregret.resize(9);
                        result = returnsum(initregret);
                    }
                }
            } else {
                if (legal_actions.size() != 3) {
                    initregret = default_strategy["gr_b_l1_9"];
                    initregret.resize(9);
                    result = returnsum(initregret);
                } else {
                    initregret = default_strategy["gr_b_l1_3"];
                    initregret.resize(9);
                    result = returnsum(initregret);
                }
            }
        } else {
            float drawplus = 0;
            if (multi_enter == 0) {
                if (cardpower == 1) {
                    drawplus += 0.7;
                } else if (cardpower == 2) {
                    drawplus += 0.6;
                } else if (cardpower == 0) {
                    drawplus += 1;
                } else if (cardpower == -1) {
                    drawplus += 1;
                }
                //
                if (remain_round == 2) {
                    if (listernpower == 3) {
                        drawplus += 1.6;
                    } else if (listernpower == 2) {
                        if (cardpower != -1 && cardpower != 0) {
                            drawplus += 1;
                        } else if (cardpower == -1 || cardpower == 0) {
                            drawplus += 1.7;
                        }
                    }
                    if (listernpower == 1) {
                        if (cardpower == -1) {
                            drawplus += 0.6;
                        } else if (cardpower == 0) {
                            drawplus += 0.7;
                        }
                    }
                } else if (remain_round == 1) {
                    if (listernpower == 3) {
                        drawplus += 1;
                    } else if (listernpower == 2) {
                        drawplus += 0.8;
                    }
                }
            } else {
                if (cardpower == 1) {
                    drawplus += 0;
                } else if (cardpower == 2) {
                    drawplus += 0;
                } else if (cardpower == 0) {
                    drawplus += 1;
                } else if (cardpower == -1) {
                    drawplus += 0;
                }
                //
                if (listernpower == 3 && remain_round != 0) {
                    drawplus += remain_round;
                } else if (listernpower == 2 && remain_round != 0) {
                    if (cardpower != -1 && cardpower != 0) {
                        drawplus += 1 + 0.6 * remain_round;
                    } else if (cardpower == -1) {
                        drawplus += 2 + 0.6 * remain_round;
                    } else if (cardpower == 0) {
                        drawplus += 1 + 0.6 * remain_round;
                    }
                }
                if (multi_enter == 1) {
                    if (cardpower < 3) {
                        drawplus -= multi_enter * 0.8;
                    }
                } else if (multi_enter > 1) {
                    if (cardpower < 3) {
                        drawplus -= multi_enter * 0.8;
                    } else if (cardpower == 3) {
                        drawplus -= multi_enter * 0.4;
                    }
                }
            }
            //
            float sumpower = cardpower + drawplus;
            int n_raises = mEngine ->getRaiseCount();
            int _n_raises = mEngine ->getRaiseCount();
            LOG_DEBUG("after cardpower:" << pot_afterflop_id << "  " << pot_aafterflop_id << "  " << cardpower << "sumpower" << sumpower << endl );
            if (pot_aafterflop_id + 1.95 < sumpower && pot_size_id < 5) { // raise
                if (remain_round == 0) {
                    if (_n_raises == 1) {
                        if (legal_actions.size() != 3) {
                            std::vector<float> initregret = (default_strategy["r_c_b2r1_9"]);
                            result = returnsum(initregret);
                        } else {
                            std::vector<float> initregret = (default_strategy["r_c_b2r1_3"]);
                            result = returnsum(initregret);
                        }
                    } else if (_n_raises >= 2) {
                        if (legal_actions.size() != 3) {
                            std::vector<float> initregret = (default_strategy["r_c_b2r2_9"]);
                            result = returnsum(initregret);
                        } else {
                            std::vector<float> initregret = (default_strategy["r_c_b2r2_3"]);
                            result = returnsum(initregret);
                        }
                    }
                } else {
                    if (cardpower != 3) {
                        if (_n_raises == 1) {
                            if (legal_actions.size() != 3) {
                                std::vector<float> initregret = (default_strategy["ft_c_b2r1pn3_9"]);
                                result = returnsum(initregret);
                            } else {
                                std::vector<float> initregret = (default_strategy["ft_c_b2r1pn3_3"]);
                                result = returnsum(initregret);
                            }
                        } else if (_n_raises >= 2) {
                            if (legal_actions.size() != 3) {
                                std::vector<float> initregret = (default_strategy["ft_c_b2r2pn3_9"]);
                                result = returnsum(initregret);
                            } else {
                                std::vector<float> initregret = (default_strategy["ft_c_b2r2pn3_3"]);
                                result = returnsum(initregret);
                            }
                        }
                    } else {
                        if (_n_raises == 1) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["ft_c_b2r1p3_9"];
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["ft_c_b2r1p3_3"];
                                result = returnsum(initregret);
                            }
                        } else if (_n_raises >= 2) {
                            if (legal_actions.size() != 3) {
                                initregret = default_strategy["ft_c_b2r2p3_9"];
                                result = returnsum(initregret);
                            } else {
                                initregret = default_strategy["ft_c_b2r2p3_3"];
                                result = returnsum(initregret);
                            }
                        }
                    }
                }
            }
            //
            if (pot_aafterflop_id - 2 > sumpower && sumpower < 3 && pot_size_id < 5) {
                if (n_raises == 1) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s2r1_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s2r1_3"];
                        result = returnsum(initregret);
                    }
                } else if (n_raises >= 2) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s2r2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s2r2_3"];
                        result = returnsum(initregret);
                    }
                }
            } else if (pot_aafterflop_id > sumpower && pot_aafterflop_id - sumpower < 0.15 && sumpower < 3 && pot_size_id < 5) {
                if (n_raises == 1) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s15r1_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s15r1_3"];
                        result = returnsum(initregret);
                    }
                } else if (n_raises >= 2) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s15r2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s15r2_3"];
                        result = returnsum(initregret);
                    }
                }
            } else if (pot_aafterflop_id > sumpower && pot_aafterflop_id - sumpower < 0.3 && sumpower < 3 && pot_size_id < 5) {
                if (n_raises == 1) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s30r1_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s30r1_3"];
                        result = returnsum(initregret);
                    }
                } else if (n_raises >= 2) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s30r2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s30r2_3"];
                        result = returnsum(initregret);
                    }
                }
            } else if (pot_aafterflop_id > sumpower && pot_aafterflop_id - sumpower < 0.5 && sumpower < 3 && pot_size_id < 5) {
                if (n_raises == 1) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s50r1_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s50r1_3"];
                        result = returnsum(initregret);
                    }
                } else if (n_raises >= 2) {
                    if (legal_actions_size != 3) {
                        initregret = default_strategy["g_f_s50r2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_f_s50r2_3"];
                        result = returnsum(initregret);
                    }
                }
            }
            //
            if (pot_aafterflop_id > sumpower && pot_aafterflop_id - sumpower >= 0.5 && sumpower < 3 && pot_size_id < 5) {
                if (_n_raises == 1) {
                    if (remain_round == 0 && cardpower >= 3) {
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["r_f_b50r1pb3_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["r_f_b50r1pb3_3"];
                            result = returnsum(initregret);
                        }
                    } else {
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["g_f_b50r1_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_b50r1_3"];
                            result = returnsum(initregret);
                        }
                    }
                } else if (_n_raises >= 2) {
                    if (remain_round == 0 && cardpower >= 3) {
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["r_f_b50r2pb3_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["r_f_b50r2pb3_3"];
                            result = returnsum(initregret);
                        }
                    } else {
                        if (legal_actions.size() != 3) {
                            initregret = default_strategy["g_f_b50r2_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_b50r2_3"];
                            result = returnsum(initregret);
                        }
                    }
                }
            } else if (pot_aafterflop_id > sumpower && sumpower >= 3 && pot_size_id < 5) {
                if (_n_raises == 1) {
                    if (pot_aafterflop_id - sumpower < 1.25) {
                        if (legal_actions.size() != 3) {
                            // Ò»°ëÒ»°ë×¥
                            initregret = default_strategy["g_f_r1pb125_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r1pb125_3"];
                            result = returnsum(initregret);
                        }
                    } else if (pot_aafterflop_id - sumpower < 2.25) {
                        if (legal_actions.size() != 3) {
                            // Èý·ÖÖ®¶þ×¥
                            initregret = default_strategy["g_f_r1pb225_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r1pb225_3"];
                            result = returnsum(initregret);
                        }
                    } else if (pot_aafterflop_id - sumpower < 3.25) {
                        if (legal_actions.size() != 3) {
                            // Èý·ÖÖ®Ò»×¥
                            initregret = default_strategy["g_f_r1pb325_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r1pb325_3"];
                            result = returnsum(initregret);
                        }
                    }
                } else if (_n_raises >= 2) {
                    if (pot_aafterflop_id - cardpower == -1) {
                        if (legal_actions.size() != 3) {
                            // Ò»°ëÒ»°ë×¥
                            initregret = default_strategy["g_f_r2pb100_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r2pb100_3"];
                            result = returnsum(initregret);
                        }
                    } else if (pot_aafterflop_id - cardpower == -2) {
                        if (legal_actions.size() != 3) {
                            // Èý·ÖÖ®¶þ×¥
                            initregret = default_strategy["g_f_r2pb200_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r2pb200_3"];
                            result = returnsum(initregret);
                        }
                    } else if (pot_aafterflop_id - cardpower == -3) {
                        if (legal_actions.size() != 3) {
                            // Èý·ÖÖ®Ò»×¥
                            initregret = default_strategy["g_f_r2pb300_9"];
                            result = returnsum(initregret);
                        } else {
                            initregret = default_strategy["g_f_r2pb300_3"];
                            result = returnsum(initregret);
                        }
                    }
                }
            } else if (pot_aafterflop_id > sumpower && pot_size_id > 4) {
                return  returnsum({4, 0, -800, -800, -800, -800, -800, -800, -8000});

                std::string infostr;
                std::vector<std::string> tablelist;
                std::vector<int> rangelist;
                if (remain_round == 0) {
                    infostr = std::to_string(cardpowerold);
                    tablelist = {"8", "7", "6", "5", "4", "3", "2", "1", "0"};
                    rangelist = {10, 15, 20, 20, 15, 7, 3, 5, 5};
                } else if (remain_round == 1) {
                    if (cardpowerold > 3) {
                        infostr = std::to_string(cardpowerold) + std::to_string(listernpower);
                    } else {
                        infostr = std::to_string(cardpowerold);
                    }

                    tablelist = {"80", "81", "70", "71", "60", "61", "82", "72", "50", "51", "62", "52", "83", "73",
                                 "40", "41", "63", "53", "42", "43", "3", "2", "1", "0"
                                };
                    rangelist = {4, 4, 6, 13, 8, 8, 4, 5, 8, 8, 2, 2, 1, 1, 6, 4, 1, 1, 2, 1, 3, 3, 3, 2};
                } else if (remain_round == 2) {
                    if (cardpowerold > 3) {
                        infostr = std::to_string(cardpowerold) + std::to_string(listernpower);
                    } else {
                        infostr = std::to_string(cardpowerold);
                    }
                    tablelist = {"80", "81", "70", "71", "60", "61", "82", "72", "50", "62", "51", "52", "83", "73", "40", "63", "53", "41", "42", "43", "3", "2", "1", "0"};
                    rangelist = {5, 5, 10, 20, 7, 5, 4, 5, 6, 2, 6, 2, 1, 1, 6, 1, 1, 4, 2, 1, 2, 2, 1, 1};
                }
                double stay = 1 - (1 / static_cast<double>(pot_aafterflop_id) - 1) / 2 / (1 + (1 / static_cast<double>(pot_aafterflop_id) - 1) / 2);
                stay = stay * 100;
                double nstay;
                if (stay < 7) {
                    nstay = 7;
                } else if (stay < 10) {
                    nstay = 10;
                } else {
                    nstay = stay * 1.2;
                }
                int sumprob = 0;
                int befor_addprob, after_addprob;
                for (size_t index = 0; index < rangelist.size(); ++index) {
                    sumprob += rangelist[index];
                    if (tablelist[index] == infostr) {
                        befor_addprob = 100 - (sumprob - rangelist[index]); // ´ó 90
                        after_addprob = 100 - sumprob; // Ð¡ 75
                        break;
                    }
                }
                //std::vector<double> result;
                if (befor_addprob <= nstay) {
                    if (len_legal_actions != 3) {
                        result = returnsum({0, 4, -800, -800, -800, -800, -800, -800, -8000});
                    } else {
                        result = returnsum({0, 4, -800, -800, -800, -800, -800, -800, -3000});
                    }
                } else if (after_addprob >= nstay) {
                    if (len_legal_actions != 3) {
                        result = returnsum({4, 0, -800, -800, -800, -800, -800, -800, -8000});
                    } else {
                        result = returnsum({4, 0, -800, -800, -800, -800, -800, -800, -3000});
                    }
                } else if (after_addprob < nstay && befor_addprob > nstay) {
                    float stay_amount = (nstay - after_addprob) / (befor_addprob - after_addprob);

                    if (len_legal_actions != 3) {
                        result = returnsum({5 * (1 - (stay_amount)), 5 * stay_amount, -800, -800, -800, -800, -800, -800, -8000});
                    } else {
                        result = returnsum({5 * (1 - (stay_amount)), 5 * stay_amount, -800, -800, -800, -800, -800, -800, -3000});
                    }
                }
            } else if (pot_aafterflop_id <= sumpower) {
                if (_n_raises == 1) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["g_c_r1_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_c_r1_3"];
                        result = returnsum(initregret);
                    }
                } else if (_n_raises >= 2) {
                    if (len_legal_actions != 3) {
                        initregret = default_strategy["g_c_r2_9"];
                        result = returnsum(initregret);
                    } else {
                        initregret = default_strategy["g_c_r2_3"];
                        result = returnsum(initregret);
                    }
                }
            }
        }
    }
    return result;
}

std::string State::infoSet() {
    std::stringstream oss;
    oss << getEngine()->getRound()                               << "-";
    oss << getEngine()->getBeforeAllPlayerNum(getTurnIndex())    << "-";
    oss << getEngine()->getBeforeActivePlayerNum(getTurnIndex()) << "-";
    oss << getEngine()->getBehindAllPlayerNum(getTurnIndex())    << "-";
    oss << getEngine()->getBehindActivePlayerNum(getTurnIndex()) << "-";
    oss << getEngine()->getCardCluster(getTurnIndex())           << "-";
    oss << getEngine()->getRaiseIndex()                          << "-";
    oss << getEngine()->getPotCluster()                          << "-";
    oss << getEngine()->getFirstBet()                            << "-";
    oss << getEngine()->getOppoRelativePosition()                << "-";
    oss << getEngine()->getLastRaiseIndex()                      << "-";
    oss << getEngine()->getTurnPlayer()->getActionSequence();
    return oss.str();
}

std::string State::infoSetdepthLimit( int turnIndex) {
    std::stringstream oss;
    oss << getEngine()->getRound()                          << "-";
    oss << getEngine()->getBeforeAllPlayerNum(turnIndex)    << "-";
    oss << getEngine()->getBeforeActivePlayerNum(turnIndex) << "-";
    oss << getEngine()->getBehindAllPlayerNum(turnIndex)    << "-";
    oss << getEngine()->getBehindActivePlayerNum(turnIndex) << "-";
    oss << getEngine()->getCardCluster(turnIndex,0)           << "-";
    oss << getEngine()->getPotClusterD(turnIndex)            << "-";
    oss << getEngine()->getLastRaiseIndexD(turnIndex);
    return oss.str();
}


bool State::isTerminal() {
    if (getEngine()->checkGameOver()) {
        getEngine()->addBetsToPot();
        getEngine()->quickGotoGameOver();
    }
    //
    return getEngine()->isGameOver();
}

int State::doTurnProcess(const std::string &action, bool search) {
    int pot = getEngine()->getCurrentPot();
    int cur = getEngine()->getTurnPlayer() ->getChips();
    int dif = getEngine()->getRoundMaxBet() - getEngine()->getTurnPlayer()->getState().bet;
    int biggest_bet_i = PLAYER_CHIPS - cur + dif; // warning!!!!!!!!!
    int allBet = PLAYER_CHIPS - cur;
    bool firstBet = false;
    if ((biggest_bet_i == 200) && (getEngine()->getRound() == GAME_ROUND_PREFLOP)) {
        firstBet = true;
    }
    //
    int num = 0;
    if (action == "FOLD") {
        getTurnPlayer()->fold();
    } else if (action == "CHECK") {
        getTurnPlayer()->check();
    } else if (action == "CALL") {
        num = getEngine()->getRoundMaxBet() - getTurnPlayer()->getState().bet;
        getTurnPlayer()->applyAction(num, CALL);
    } else if (action == "RAISE0") {

        num = 200;
        getTurnPlayer()->applyAction(num, RAISE0);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;


    } else if (action == "RAISE1") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise) {
                num = floor((float)(pot + dif) * 0.33 + dif);
            } else {
                num = floor((float)(pot) * 0.33);
            }
        } else {
            num = 400 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE1);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "RAISE2") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise)
                num = floor((float)(pot + dif) * 0.50 + dif);
            else
                num = floor((float)(pot) * 0.50 );
        } else {
            num = 600 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE2);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "RAISE3") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise) {
                num = floor((float)(pot + dif) * 0.75 + dif);
            } else {
                num = floor((float)(pot) * 0.75);
            }
        } else {
            num = 800 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE3);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "RAISE4") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise) {
                num = floor((float)(pot + dif) * 1.0 + dif);
            } else {
                num = floor((float)(pot) * 1);
            }
        } else {
            num = 1000 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE4);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "RAISE5") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise) {
                num = floor((float)(pot + dif) * 1.5 + dif);
            } else {
                num = floor((float)(pot) * 1.5);
            }
        } else {
            num = 1500 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE5);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "RAISE6") {
        if (firstBet == false) {
            if (1 == Configure::GetInstance().mRealRaise) {
                num = floor((float)(pot + dif) * 2 + dif);
            } else {
                num = floor((float)(pot) * 2);
            }
        } else {
            num = 2000 - allBet;
        }
        //
        getTurnPlayer()->applyAction(num, RAISE5);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
        getEngine()->raiseNumber = num - dif;
    } else if (action == "ALLIN") {
        num = getTurnPlayer()->getChips();
        getTurnPlayer()->applyAction(num, ALLIN);
        getEngine()->incRaiseCount();
        getEngine()->setLastPlayerToActor();
        getEngine()->setLastRaiserIndex();
    } else {
        LOG_ERROR("invalid action=" << action);
    }
    //
    LOG_DEBUG("-----------------------------------------------");
    LOG_DEBUG("@State: PlayRound. Player" << getTurnIndex()
              << " round="  << getEngine()->getRound()
              << ", move="   << action
              << ", num="    << num
              << ", search=" << search
              << ", last="   << getLastIndex());
    //
    getEngine()->printHand();
    return 0;
}

int State::doTurnSchedule() {
    while (!getEngine()->checkAllin()) {
        auto player = getEngine()->getNextTurnPlayer();
        if (player->isFold()) {
            LOG_DEBUG("@State: Mover(skip fold). turn=" << player->getIndex() << ", round=" << getRound());
            continue;
        } else if (player->isAllIn()) {
            LOG_DEBUG("@State: Mover(skip allin). turn=" << player->getIndex() << ", round=" << getRound());
            continue;
        } else {
            LOG_DEBUG("@State: Mover(next player). turn=" << player->getIndex() << ", round=" << getRound());
            break;
        }
    }
    //
    return 0;
}

int State::doGameSchedule() {
    if (isTerminal()) {
        LOG_DEBUG("isTerminal true");
        return 0;
    }
    //
    if (!getEngine()->isGameOver() && !getEngine()->isRoundOver()) {
        doTurnSchedule();
    }
    //
    while (getEngine()->isRoundOver() && !getEngine()->isGameOver()) {
        if (true) {
            if ( getEngine()->getRound() == GAME_ROUND_PREFLOP ) {
                int numPlayers = getEngine()->getPlayerNum();
                int fold_num = 0 ;
                int multi_enter = 0;
                for (int s = 0 ; s < numPlayers; s++) {
                    if (getEngine()->getPlayer(s)->isFold()) {
                        fold_num += 1;
                    }
                }
                //
                int raise_num = getEngine()->getRaiseCount();
                if (raise_num < 2) {
                    if (fold_num == 3)
                        multi_enter = 1;
                    else if (fold_num < 3)
                        multi_enter = 2;
                } else if ( raise_num == 2) {
                    if (fold_num == 4)
                        multi_enter = 1;
                    else
                        multi_enter = 2;
                } else {
                    if (fold_num == 4)
                        multi_enter = 2;
                    if (fold_num < 4)
                        multi_enter = 3;
                }
                //
                getEngine()->multi_enter = multi_enter;
            }
        }
        //
        getEngine()->addBetsToPot();
        getEngine()->setPlayerStates();
        getEngine()->gotoNextRound();
    }
    //
    return 0;
}

std::valarray<float> State::payoff() {
    int numPlayers = getEngine()->getPlayerNum();
    //
    std::valarray<float> payoffs(numPlayers);
    auto ptr = getEngine()->getPayOffs();
    for (int i = 0; i < numPlayers; i++) {
        payoffs[i] = (float)(*(ptr + i));
    }
    //
    return payoffs;
}

std::vector<int> State::getleagle(int player, vector<int> randList) {
    std::vector<int> Cardlist;
    //
    for (int s = 0; s < getEngine()->getPlayerNum(); s ++) {
        if ((s != player) && !getEngine()->getPlayers(s)->isFold()) {
            auto target = getEngine()->getPlayers(s);
            Cardlist.push_back(Card::str2fastId(target->getHand()[0]->strCluster()));
            Cardlist.push_back(Card::str2fastId(target->getHand()[1]->strCluster()));
        }
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_PREFLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[0]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[1]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[2]->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_FLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getTurnCard()->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_TURN) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getRiverCard()->strCluster()));
    }
    //
    auto target = getEngine()->getPlayers(player);
    auto card1 = *(target->getHand()[0]);
    auto card2 = *(target->getHand()[1]);
    vector<int> result;
    for (int s = 0; s < randList.size(); s ++) {
        size_t index = randList[s];
        size_t i = 0 , j = 0;
        pair_conv(index, i, j);
        auto search = std::find(Cardlist.begin(), Cardlist.end(), i);
        if (search != Cardlist.end())
            continue;
        //
        search = std::find(Cardlist.begin(), Cardlist.end(), j);
        if (search != Cardlist.end())
            continue;
        //
        auto combo1 = Card::fastId2str(j) + '-' + Card::fastId2str(i);
        auto combo2 = Card::fastId2str(i) + '-' + Card::fastId2str(j);
        auto ranges = &handRanges[player];
        if (ranges->find(combo1) == ranges->end() && ranges->find(combo2) == ranges->end())
            continue;
        //
        result.push_back(randList[s]);
    }
    //
    return result;
}

std::valarray<float> State::payoffMatrix3(int player, vector<int> randList) {
    std::valarray<float> payoffs(UNLEAGLE, 1326 * mNumPlayers);
    //
    std::vector<int> Cardlist;
    for (int s = 0; s < getEngine()->getPlayerNum(); s ++) {
        if ((s != player) && !getEngine()->getPlayers(s)->isFold()) {
            auto target = getEngine()->getPlayers(s);
            Cardlist.push_back(Card::str2fastId(target->getHand()[0]->strCluster()));
            Cardlist.push_back(Card::str2fastId(target->getHand()[1]->strCluster()));
        }
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_PREFLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[0]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[1]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[2]->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_FLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getTurnCard()->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_TURN) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getRiverCard()->strCluster()));
    }
    //
    auto target = getEngine()->getPlayers(player);
    auto card1 = *(target->getHand()[0]);
    auto card2 = *(target->getHand()[1]);
    //
    for (int s = 0; s < randList.size(); s++) {
        size_t index = randList[s];
        size_t i = 0 , j = 0;
        pair_conv(index, i, j);
        auto search = std::find(Cardlist.begin(), Cardlist.end(), i);
        if (search != Cardlist.end())
            continue;
        //
        search = std::find(Cardlist.begin(), Cardlist.end(), j);
        if (search != Cardlist.end())
            continue;
        //
        auto combo1 = Card::fastId2str(j) + '-' + Card::fastId2str(i);
        auto combo2 = Card::fastId2str(i) + '-' + Card::fastId2str(j);
        auto ranges = &handRanges[player];
        if (ranges->find(combo1) == ranges->end() && ranges->find(combo2) == ranges->end())
            continue;
        //
        target->getHand()[0]->setCard(Card::fastId2str(i));
        target->getHand()[1]->setCard(Card::fastId2str(j));
        getEngine()->determineWinner(false, true);
        //
        auto ptr = getEngine()->getPayOffs();
        for (int c = 0; c < getEngine()->getPlayerNum(); c++) {
            payoffs[index * 6 + c] = (float)(*(ptr + c));
        }
        //
        int numOther = 0;
        for (int i = 0; i < 6; ++i) {
            auto player = getEngine()->getPlayers(i);
            if (player->isFold()) {
                numOther++;
            }
        }
        /*
        if(Card::fastId2str(j)[0] == 'A' && Card::fastId2str(i)[0] == 'A' and numOther != 5 and  getEngine()->getRiverCard()->strCluster()[0] != 'K' and payoffs[index*6 + player] < 0 ){
            LOG_INFO("index" << payoffs[index*6 + player] << getEngine()-> getPlayers(player) ->getHand()[0]->strCluster() << getEngine()-> getPlayers(player) ->getHand()[1]->strCluster() <<getEngine()-> getPlayers(!player) ->getHand()[0]->strCluster() << getEngine()-> getPlayers(!player) ->getHand()[1]->strCluster());
            LOG_INFO("player" <<numOther);
        }*/
    }
    //LOG_INFO("randList" << randList.size());
    //
    target->getHand()[0]->setCard(&card1);
    target->getHand()[1]->setCard(&card2);
    return payoffs;
}

std::valarray<float> State::payoffMatrix(int player) {
    std::valarray<float> payoffs(UNLEAGLE, 1326 * mNumPlayers);
    //
    std::vector<int> Cardlist;
    for (int s = 0; s < getEngine()->getPlayerNum(); s ++) {
        if ((s != player) && !getEngine()->getPlayers(s)->isFold()) {
            auto target = getEngine()->getPlayers(s);
            Cardlist.push_back(Card::str2fastId(target->getHand()[0]->strCluster()));
            Cardlist.push_back(Card::str2fastId(target->getHand()[1]->strCluster()));
        }
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_PREFLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[0]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[1]->strCluster()));
        Cardlist.push_back(Card::str2fastId(getEngine()->getFlopCard()[2]->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_FLOP) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getTurnCard()->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_TURN) {
        Cardlist.push_back(Card::str2fastId(getEngine()->getRiverCard()->strCluster()));
    }
    //
    auto target = getEngine()->getPlayers(player);
    auto card1 = *(target->getHand()[0]);
    auto card2 = *(target->getHand()[1]);
    //
    for (int i = 0; i < 52; i++) {
        auto search = std::find(Cardlist.begin(), Cardlist.end(), i);
        if (search != Cardlist.end())
            continue;
        //
        for (int j = i + 1; j < 52; j++) {
            search = std::find(Cardlist.begin(), Cardlist.end(), j);
            if (search != Cardlist.end())
                continue;
            //
            auto index = pair_index(i, j);
            auto combo1 = Card::fastId2str(j) + '-' + Card::fastId2str(i);
            auto combo2 = Card::fastId2str(i) + '-' + Card::fastId2str(j);
            auto ranges = &handRanges[player];
            if (ranges->find(combo1) == ranges->end() && ranges->find(combo2) == ranges->end())
                continue;
            //
            target->getHand()[0]->setCard(Card::fastId2str(i));
            target->getHand()[1]->setCard(Card::fastId2str(j));
            getEngine()->determineWinner(false, true);
            //
            auto ptr = getEngine()->getPayOffs();
            for (int i = 0; i < getEngine()->getPlayerNum(); i++) {
                payoffs[index * 6 + i] = (float)(*(ptr + i));
            }
        }
    }
    //
    target->getHand()[0]->setCard(&card1);
    target->getHand()[1]->setCard(&card2);
    return payoffs;
}

std::unordered_map<std::string, std::valarray<float>> State::payoffMatrix2(int player) {
    //
    std::vector<std::size_t> knownCards;
    //
    if (getEngine()->getRound() > GAME_ROUND_PREFLOP) {
        knownCards.push_back(Card::str2fastId(getEngine()->getFlopCard()[0]->strCluster()));
        knownCards.push_back(Card::str2fastId(getEngine()->getFlopCard()[1]->strCluster()));
        knownCards.push_back(Card::str2fastId(getEngine()->getFlopCard()[2]->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_FLOP) {
        knownCards.push_back(Card::str2fastId(getEngine()->getTurnCard()->strCluster()));
    }
    //
    if (getEngine()->getRound() > GAME_ROUND_TURN) {
        knownCards.push_back(Card::str2fastId(getEngine()->getRiverCard()->strCluster()));
    }
    //
    for (int s = 0; s < getEngine()->getPlayerNum(); s++) {
        auto target = getEngine()->getPlayers(s);
        if (getEngine()->getPlayers(s)->isFold()) {
            continue;
        }
        //
        if (s == player) {
            continue;
        }
        //
        knownCards.push_back(Card::str2fastId(target->getHand()[0]->strCluster()));
        knownCards.push_back(Card::str2fastId(target->getHand()[1]->strCluster()));
    }
    //
    std::unordered_map<std::string, std::valarray<float>> kv;
    //
    if (true) {
        auto target = getEngine()->getPlayers(player);
        auto card1 = *(target->getHand(0));
        auto card2 = *(target->getHand(1));
        for (int i = 0; i < 52; i++) {
            auto search = std::find(knownCards.begin(), knownCards.end(), i);
            if (search != knownCards.end())
                continue;
            //
            for (int j = i + 1; j < 52; j++) {
                search = std::find(knownCards.begin(), knownCards.end(), j);
                if (search != knownCards.end())
                    continue;
                //
                auto card1 = Card::fastId2str(i);
                auto card2 = Card::fastId2str(j);
                target->getHand(0)->setCard(card1);
                target->getHand(1)->setCard(card2);
                //
                getEngine()->determineWinner(false, true);
                //
                std::valarray<float> valPayoffs(0.0, mNumPlayers);
                for (int i = 0; i < mNumPlayers; i++) {
                    valPayoffs[i] = getEngine()->getPayOffs(i);
                }
                //
                auto combo = card2 + '-' + card1;
                kv.emplace(combo, valPayoffs);
                //
                if ((kv[combo].size() != mNumPlayers) || (6 != mNumPlayers)) {
                    TEST_E("@payoffMatrix2: combo=" << combo << ", payoffsSize=" << valPayoffs.size());
                    exit(-1);
                }
            }
        }
        //
        target->getHand(0)->setCard(&card1);
        target->getHand(1)->setCard(&card2);
    }
    //
    return kv;
}

std::vector<int> State::winners() {
    std::vector<int> winners;
    return winners;
}

std::set<std::string> State::validActions() {
    std::set<std::string> mActions;
    int actSequence = getEngine()->getTurnPlayer()->getActionSequence();
    if (actSequence & (1 << FOLD))
        mActions.emplace("FOLD");
    if (actSequence & (1 << CALL))
        mActions.emplace("CALL");
    if (actSequence & (1 << CHECK))
        mActions.emplace("CHECK");
    if (actSequence & (1 << RAISE1))
        mActions.emplace("RAISE1");
    if (actSequence & (1 << RAISE2))
        mActions.emplace("RAISE2");
    if (actSequence & (1 << RAISE3))
        mActions.emplace("RAISE3");
    if (actSequence & (1 << RAISE4))
        mActions.emplace("RAISE4");
    if (actSequence & (1 << RAISE5))
        mActions.emplace("RAISE5");
    if (actSequence & (1 << RAISE6))
        mActions.emplace("RAISE6");
    if (actSequence & (1 << ALLIN))
        mActions.emplace("ALLIN");
    //
    return mActions;
}

std::vector<std::string> State::validActionsV() {
    std::vector<std::string> mActions;
    int actSequence = getEngine()->getTurnPlayer()->getActionSequence();
    if (actSequence & (1 << FOLD))
        mActions.push_back("FOLD");
    if (actSequence & (1 << CALL))
        mActions.push_back("CALL");
    if (actSequence & (1 << CHECK))
        mActions.push_back("CHECK");
    if (actSequence & (1 << RAISE1))
        mActions.push_back("RAISE1");
    if (actSequence & (1 << RAISE2))
        mActions.push_back("RAISE2");
    if (actSequence & (1 << RAISE3))
        mActions.push_back("RAISE3");
    if (actSequence & (1 << RAISE4))
        mActions.push_back("RAISE4");
    if (actSequence & (1 << RAISE5))
        mActions.push_back("RAISE5");
    if (actSequence & (1 << RAISE6))
        mActions.push_back("RAISE6");
    if (actSequence & (1 << ALLIN))
        mActions.push_back("ALLIN");
    //
    return mActions;
}

Player *State::getTurnPlayer() {
    return getEngine()->getPlayer(getTurnIndex());
}

Player *State::getLastPlayer() {
    return getEngine()->getPlayer(getLastIndex());
}

Game *State::getEngine() {
    return mEngine;
}

int State::getTurnIndex() {
    return getEngine()->getTurnIndex();
}

int State::getLastIndex() {
    return getEngine()->getLastIndex();
}

int State::getNextIndex() {
    return getEngine()->getNextIndex();
}

int State::getRound() {
    return (int)getEngine()->getRound();
}
