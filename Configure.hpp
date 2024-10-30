#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <iostream>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <queue>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <ctime>
#include <cstdarg>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Single.hpp"

using namespace std;

//
class Configure : public Singleton<Configure> {
  public:
    //
    Configure();
    //
    ~Configure();

  public:
    //
    bool Init();
    //
    bool Final();

  private:
    //
    void StringSplit(string str, const char split, std::set<string> &ss);

  public:
    //
    std::string mFileName;
    //
    int mDaemon;
    //
    int mNumPlayers;
    //
    int mStrategyInterval;
    //
    int mPruneThreshold;
    //
    int mDiscountInterval;
    //
    int mCFRThreshold;
    //
    int mMaxRaiseNum;
    //
    int mMinRegret;
    //
    int mMinIterations;
    //
    int mMaxIterations;
    //
    std::string mSavePath;
    //
    std::string mLoadPath;
    //
    int mSerializeInterval;
    //
    int mQueueMaxTaskNum;
    //
    int mResetPoolInterval;
    //
    int mSimpleVersion;
    //
    int mRealRaise;
    //
    int mUnknown;
    //
    int mSingleThread;
    //
    int mRaiseRule;
    //
    int mOppoPosition;
    //
    std::string mRedisUser;
    //
    std::string mRedisPass;
    //
    std::set<std::string> mRedisNodes;
    //
    int mResearchOpening;
    //
    std::string mResearchAddr;
    //
    int mResearchPort;
    //
    int mResearchIteration;
    //
    int mResearchCondition;
    //
    int mResearchMultiThreading;
    //
    int mResearchTimeout;
    //
    int mResearchUseCFR;
    //
    int mResearchStage;
    //
    int mResearchDepth;
    //
    int mResearchStrategyInterval;
    //
    int mResearchDiscountInterval;
    //
    int mResearchCfrThreshold;
    //
    int mResearchPreflopTimes;
    //
    int mResearchFlopTimes;
    //
    int mResearchTurnTimes;
    //
    int mResearchRiverTimes;
    //
    std::string mResearchTestInfoset;
    //
    int mResearchTestDump;
    //
    std::string mResearchTestStrategy;
};

#endif

