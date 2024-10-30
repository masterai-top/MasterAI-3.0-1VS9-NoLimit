#ifndef GAME_POOL_H
#define GAME_POOL_H

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <bitset>
#include <atomic>
#include <algorithm>
#include <shared_mutex>
#include <unordered_set>
#include "Log.hpp"
#include "Game.hpp"
#include "Single.hpp"

using namespace std;

class GamePool : public Singleton<GamePool> {
public:
    //
    GamePool();
    //
    ~GamePool();
    //
    bool init();
    //
    bool final();
    //
    void status();
    //
    Game *allocator();
    //
    void recoverer(Game *ptr);
    //
    uint64_t getAllocateNum();
    //
    uint64_t getRecycleNum();

private:
    //
    std::atomic<uint64_t> mAllocateNum;
    //
    std::atomic<uint64_t> mRecycleNum;
};

#endif
