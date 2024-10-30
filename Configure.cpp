#include "Configure.hpp"
#include "IniFile.hpp"
#include "Log.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>

using namespace std;

Configure::Configure() {
    mFileName = "./Train.ini";
    mDaemon = 0;
    mNumPlayers = 3;
    mStrategyInterval = 1000;
    mPruneThreshold = 100000;
    mDiscountInterval = 5000;
    mCFRThreshold = 100000;
    mMaxRaiseNum = 5;
    mMinRegret = -30000000;
    mMinIterations = 0;
    mMaxIterations = 100000;
    mSavePath = "./";
    mLoadPath = "./";
    mSerializeInterval = 10000;
    mQueueMaxTaskNum = 250;
    mResetPoolInterval = 1000;
    mSimpleVersion = 0;
    mRealRaise = 0;
    mUnknown = 9999;
    mSingleThread = 0;
    mRaiseRule = 0;
    //
    mRedisUser = "";
    mRedisPass = "";
    //
    mResearchOpening = 0;
    mResearchAddr = "0.0.0.0";
    mResearchPort = 9999;
    mResearchIteration = 100;
    mResearchCondition = 100;
    mResearchMultiThreading = 0;
    mResearchStage = 3;
    mResearchDepth = 5;
    mResearchStrategyInterval = 100;
    mResearchDiscountInterval = 200;
    mResearchCfrThreshold = 2000;
    mResearchPreflopTimes = 100;
    mResearchTimeout = 8;
    mResearchUseCFR = -1;
    mResearchFlopTimes = 100;
    mResearchTurnTimes = 100;
    mResearchRiverTimes = 100;
    mResearchTestInfoset = "";
    mResearchTestDump = 0;
    mResearchTestStrategy = "";
}

Configure::~Configure() {

}

bool Configure::Init() {
    IniFile ini;
    if (0 != ini.Load(mFileName)) {
        LOG_ERROR("Load ini-file failed: file=" << mFileName);
        return false;
    }
    //
    cout << "***************************************" << endl;
    //
    if (0 != ini.GetIntValue("train", "daemon", &mDaemon)) {
        return false;
    } else {
        cout << "tarin.daemon=" << mDaemon << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "player_num", &mNumPlayers)) {
        cerr << "tarin.player_num failed" << endl;
        return false;
    } else {
        cout << "tarin.player_num=" << mNumPlayers << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "min_iterations", &mMinIterations)) {
        cerr << "tarin.min_iterations failed" << endl;
        return false;
    } else {
        cout << "tarin.min_iterations=" << mMinIterations << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "max_iterations", &mMaxIterations)) {
        cerr << "tarin.max_iterations failed" << endl;
        return false;
    } else {
        cout << "tarin.max_iterations=" << mMaxIterations << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "regret_min", &mMinRegret)) {
        cerr << "tarin.regret_min failed" << endl;
        return false;
    } else {
        cout << "tarin.regret_min=" << mMinRegret << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "strategy_interval", &mStrategyInterval)) {
        cerr << "tarin.strategy_interval failed" << endl;
        return false;
    } else {
        cout << "tarin.strategy_interval=" << mStrategyInterval << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "prune_threshold", &mPruneThreshold)) {
        cerr << "tarin.prune_threshold failed" << endl;
        return false;
    } else {
        cout << "tarin.prune_threshold=" << mPruneThreshold << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "discount_interval", &mDiscountInterval)) {
        cerr << "tarin.discount_interval failed" << endl;
        return false;
    } else {
        cout << "tarin.discount_interval=" << mDiscountInterval << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "lcfr_threshold", &mCFRThreshold)) {
        cerr << "tarin.lcfr_threshold failed" << endl;
        return false;
    } else {
        cout << "tarin.lcfr_threshold=" << mCFRThreshold << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "max_raise_num", &mMaxRaiseNum)) {
        cerr << "tarin.max_raise_num failed" << endl;
        return false;
    } else {
        cout << "tarin.max_raise_num=" << mMaxRaiseNum << endl;
    }
    //
    if (0 != ini.GetStringValue("train", "save_path", &mSavePath)) {
        cerr << "tarin.save_path failed" << endl;
        return false;
    } else {
        cout << "tarin.save_path=" << mSavePath << endl;
    }
    //
    if (0 != ini.GetStringValue("train", "load_path", &mLoadPath)) {
        cerr << "tarin.load_path failed" << endl;
        return false;
    } else {
        cout << "tarin.load_path=" << mLoadPath << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "serialize_interval", &mSerializeInterval)) {
        cerr << "tarin.serialize_interval failed" << endl;
        return false;
    } else {
        cout << "tarin.serialize_interval=" << mSerializeInterval << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "queue_max_task_num", &mQueueMaxTaskNum)) {
        cerr << "tarin.queue_max_task_num failed" << endl;
        return false;
    } else {
        cout << "tarin.queue_max_task_num=" << mQueueMaxTaskNum << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "reset_pool_interval", &mResetPoolInterval)) {
        cerr << "tarin.reset_pool_interval failed" << endl;
        return false;
    } else {
        cout << "tarin.reset_pool_interval=" << mResetPoolInterval << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "simple_version", &mSimpleVersion)) {
        cerr << "tarin.simple_version failed" << endl;
        return false;
    } else {
        cout << "tarin.simple_version=" << mSimpleVersion << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "real_raise", &mRealRaise)) {
        cerr << "tarin.real_raise failed" << endl;
        return false;
    } else {
        cout << "tarin.real_raise=" << mRealRaise << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "unknown", &mUnknown)) {
        cerr << "tarin.unknown failed" << endl;
        return false;
    } else {
        cout << "tarin.unknown=" << mUnknown << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "single_thread", &mSingleThread)) {
        cerr << "tarin.single_thread failed" << endl;
        return false;
    } else {
        cout << "tarin.single_thread=" << mSingleThread << endl;
    }
    //
    if (0 != ini.GetIntValue("train", "raise_rule", &mRaiseRule)) {
        cerr << "tarin.raise_rule failed" << endl;
        return false;
    } else {
        cout << "tarin.raise_rule" << mRaiseRule << endl;
    }
    if (0 != ini.GetIntValue("train", "oppo_position", &mOppoPosition)) {
        cerr << "tarin.oppo_position failed" << endl;
        return false;
    } else {
        cout << "tarin.oppo_position" << mOppoPosition << endl;
    }


    ////////////////////////////////////////////////////////////////////
    if (0 != ini.GetStringValue("redis", "redis_user", &mRedisUser)) {
        cerr << "redis.redis_user failed" << endl;
        return false;
    } else {
        cout << "redis.redis_user=" << mRedisUser << endl;
    }
    //
    if (0 != ini.GetStringValue("redis", "redis_pass", &mRedisPass)) {
        cerr << "redis.redis_pass failed" << endl;
        return false;
    } else {
        cout << "redis.redis_pass=" << mRedisPass << endl;
    }
    //
    std::string redisNodes;
    if (0 != ini.GetStringValue("redis", "redis_nodes", &redisNodes)) {
        cerr << "redis.redis_nodes failed" << endl;
        return false;
    } else {
        cout << "redis.redis_nodes=" << redisNodes << endl;
        StringSplit(redisNodes, '/', mRedisNodes);
    }
    ////////////////////////////////////////////////////////////////////
    if (0 != ini.GetIntValue("deploy", "research_opening", &mResearchOpening)) {
        cerr << "deploy.research_opening failed" << endl;
        return false;
    } else {
        cout << "deploy.research_opening=" << mResearchOpening << endl;
    }
    if (0 != ini.GetStringValue("deploy", "research_addr", &mResearchAddr)) {
        cerr << "deploy.research_addr failed" << endl;
        return false;
    } else {
        cout << "deploy.research_addr=" << mResearchAddr << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_port", &mResearchPort)) {
        cerr << "deploy.research_port failed" << endl;
        return false;
    } else {
        cout << "deploy.research_port=" << mResearchPort << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_iteration", &mResearchIteration)) {
        cerr << "deploy.research_iteration failed" << endl;
        return false;
    } else {
        cout << "deploy.research_iteration=" << mResearchIteration << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_condition", &mResearchCondition)) {
        cerr << "deploy.research_condition failed" << endl;
        return false;
    } else {
        cout << "deploy.research_condition=" << mResearchCondition << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_multithreading", &mResearchMultiThreading)) {
        cerr << "deploy.research_multithreading failed" << endl;
        return false;
    } else {
        cout << "deploy.research_multithreading=" << mResearchMultiThreading << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_timeout", &mResearchTimeout)) {
        cerr << "deploy.research_timeout failed" << endl;
        return false;
    } else {
        cout << "deploy.research_timeout=" << mResearchTimeout << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_usecfr", &mResearchUseCFR)) {
        cerr << "deploy.research_usecfr failed" << endl;
        return false;
    } else {
        cout << "deploy.research_usecfr=" << mResearchUseCFR << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_stage", &mResearchStage)) {
        cerr << "deploy.research_stage failed" << endl;
        return false;
    } else {
        cout << "deploy.research_stage=" << mResearchStage << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_depth", &mResearchDepth)) {
        cerr << "deploy.research_depth failed" << endl;
        return false;
    } else {
        cout << "deploy.research_depth=" << mResearchDepth << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_strategy_interval", &mResearchStrategyInterval)) {
        cerr << "deploy.research_strategy_interval failed" << endl;
        return false;
    } else {
        cout << "deploy.research_strategy_interval=" << mResearchStrategyInterval << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_discount_interval", &mResearchDiscountInterval)) {
        cerr << "deploy.research_discount_interval failed" << endl;
        return false;
    } else {
        cout << "deploy.research_discount_interval=" << mResearchDiscountInterval << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_cfr_threshold", &mResearchCfrThreshold)) {
        cerr << "deploy.research_cfr_threshold failed" << endl;
        return false;
    } else {
        cout << "deploy.research_cfr_threshold=" << mResearchCfrThreshold << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_preflop_times", &mResearchPreflopTimes)) {
        cerr << "deploy.research_preflop_times failed" << endl;
        return false;
    } else {
        cout << "deploy.research_preflop_times=" << mResearchPreflopTimes << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_flop_times", &mResearchFlopTimes)) {
        cerr << "deploy.research_flop_times failed" << endl;
        return false;
    } else {
        cout << "deploy.research_flop_times=" << mResearchFlopTimes << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_turn_times", &mResearchTurnTimes)) {
        cerr << "deploy.research_turn_times failed" << endl;
        return false;
    } else {
        cout << "deploy.research_turn_times=" << mResearchTurnTimes << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_river_times", &mResearchRiverTimes)) {
        cerr << "deploy.research_river_times failed" << endl;
        return false;
    } else {
        cout << "deploy.research_river_times=" << mResearchRiverTimes << endl;
    }
    if (0 != ini.GetStringValue("deploy", "research_test_infoset", &mResearchTestInfoset)) {
        cerr << "deploy.research_test_infoset failed" << endl;
        return false;
    } else {
        cout << "deploy.research_test_infoset=" << mResearchTestInfoset << endl;
    }
    if (0 != ini.GetIntValue("deploy", "research_test_dump", &mResearchTestDump)) {
        cerr << "deploy.mResearchTestDump failed" << endl;
        return false;
    } else {
        cout << "deploy.mResearchTestDump=" << mResearchTestDump << endl;
    }
    if (0 != ini.GetStringValue("deploy", "research_test_strategy", &mResearchTestStrategy)) {
        cerr << "deploy.research_test_strategy failed" << endl;
        return false;
    } else {
        cout << "deploy.research_test_strategy=" << mResearchTestStrategy << endl;
    }
    ////////////////////////////////////////////////////////////////////
    if (true) {
        //
        if (0 != access(mSavePath.c_str(), 0)) {
            if (mkdir(mSavePath.c_str(), 0771) < 0 ) {
                cerr << "mkdir(" << mSavePath << ") failed, errno=" << errno
                     << ", errmsg = " << strerror(errno) << endl;
                return false;
            }
        }
        //
        std::string mLogPath = "./log/";
        if (0 != access(mLogPath.c_str(), 0)) {
            if (mkdir(mLogPath.c_str(), 0771) < 0 ) {
                cerr << "mkdir(" << mLogPath << ") failed, errno=" << errno
                     << ", errmsg = " << strerror(errno) << endl;
                return false;
            }
        }
    }
    //
    cout << "***************************************" << endl;
    return true;
}

bool Configure::Final() {
    LOG_INFO("Configure Final!");
    return true;
}

void Configure::StringSplit(string str, const char split, std::set<string> &ss) {
    ss.clear();
    //
    std::istringstream iss(str);
    std::string line;
    while (getline(iss, line, split)) {
        ss.insert(line);
    }
}
