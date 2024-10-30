#include "Log.hpp"

//
log4cplus::Initializer initializer;
log4cplus::Logger logger0;
log4cplus::Logger logger1;
//
void logger_initialize() {
    PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT ("Log4cplus.properties"));
    Logger::getRoot();
    //
    logger0 = Logger::getInstance(LOG4CPLUS_TEXT("CFR"));
    logger1 = Logger::getInstance(LOG4CPLUS_TEXT("TEST"));
    LOG4CPLUS_INFO(logger0, "Starting test loop....");
    LOG4CPLUS_INFO(logger1, "Starting test loop....");
}

void logger_shutdown() {
    Logger::shutdown();
}

static std::valarray<std::size_t> gHandPairs(1326);

std::size_t pair_index(std::size_t i, std::size_t j) {
    if ((i == j) || (i < 0) || (i > 51) || (j < 0) || (j > 51)) {
        return -10000;
    }
    //
    int minIdx = 0;
    int maxIdx = 0;
    if (i < j) {
        minIdx = i;
        maxIdx = j;
    } else {
        minIdx = j;
        maxIdx = i;
    }
    //
    int idx = 0;
    for (int z = 0; z <= minIdx; z++) {
        idx += z;
    }
    //
    return (52 * minIdx - idx) + (maxIdx - minIdx - 1);
}

std::size_t pair_init() {
    for (auto i = 0; i < 52; i++) {
        for (auto j = i + 1; j < 52; j++) {
            auto idx = pair_index(i, j);
            gHandPairs[idx] = i * 100 + j;
            TEST_D("i=" << i << ", j=" << j << ", idx=" << idx << ", tag=" << gHandPairs[idx]);
        }
    }
    return 0;
}

std::size_t pair_conv(std::size_t idx, std::size_t &i, std::size_t &j) {
    if (idx < 0 || idx > 1326) {
        i = -1;
        j = -1;
        return 1;
    }
    //
    auto val = gHandPairs[idx];
    //
    i = val / 100;
    j = val % 100;
    return 0;
}