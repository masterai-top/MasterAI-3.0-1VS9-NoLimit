#include "GamePool.hpp"

GamePool::GamePool() {
    mAllocateNum = 0;
    mRecycleNum = 0;
}

GamePool::~GamePool() {

}

bool GamePool::init() {
    LOG_INFO("@Pool: init.");
    return true;
}

bool GamePool::final() {
    status();
    //
    LOG_INFO("@Pool: final over.");
    return true;
}

void GamePool::status() {
    LOG_INFO("@Pool: Number of allocated objects: " << mAllocateNum);
    LOG_INFO("@Pool: Number of free objects: "  << mRecycleNum);
    LOG_INFO("@Pool: Number of unreleased objects: " << (mAllocateNum - mRecycleNum));
}

Game *GamePool::allocator() {
    Game *ptr  = new Game();
    mAllocateNum++;
    return ptr;
}

void GamePool::recoverer(Game *ptr) {
    if (ptr != nullptr) {
        delete ptr;
        mRecycleNum++;
    }
}

uint64_t GamePool::getAllocateNum() {
    return mAllocateNum;
}

uint64_t GamePool::getRecycleNum() {
    return mRecycleNum;
}
