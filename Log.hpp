#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <cstdarg>
#include <valarray>
//
#include "log4cplus/logger.h"
#include "log4cplus/tstring.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/configurator.h"
#include "log4cplus/loglevel.h"
#include "log4cplus/initializer.h"
#include "log4cplus/fileappender.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

//
extern log4cplus::Logger logger0;
extern log4cplus::Logger logger1;
//
extern void logger_initialize();
extern void logger_shutdown();
//
#define LOG_TRACE(logEvent)   LOG4CPLUS_TRACE(logger0, logEvent)
#define LOG_DEBUG(logEvent)   LOG4CPLUS_DEBUG(logger0, logEvent)
#define LOG_INFO(logEvent)    LOG4CPLUS_INFO(logger0,  logEvent)
#define LOG_WARN(logEvent)    LOG4CPLUS_WARN(logger0,  logEvent)
#define LOG_ERROR(logEvent)   LOG4CPLUS_ERROR(logger0, logEvent)
#define LOG_FATAL(logEvent)   LOG4CPLUS_FATAL(logger0, logEvent)
//
#define LOGF_TRACE(...)       LOG4CPLUS_TRACE_FMT(logger0, __VA_ARGS__)
#define LOGF_DEBUG(...)       LOG4CPLUS_DEBUG_FMT(logger0, __VA_ARGS__)
#define LOGF_INFO(...)        LOG4CPLUS_INFO_FMT(logger0,  __VA_ARGS__)
#define LOGF_WARN(...)        LOG4CPLUS_WARN_FMT(logger0,  __VA_ARGS__)
#define LOGF_ERROR(...)       LOG4CPLUS_ERROR_FMT(logger0, __VA_ARGS__)
#define LOGF_FATAL(...)       LOG4CPLUS_FATAL_FMT(logger0, __VA_ARGS__)
//
#define TEST_T(logEvent)   LOG4CPLUS_TRACE(logger1, logEvent)
#define TEST_D(logEvent)   LOG4CPLUS_DEBUG(logger1, logEvent)
#define TEST_I(logEvent)   LOG4CPLUS_INFO(logger1,  logEvent)
#define TEST_W(logEvent)   LOG4CPLUS_WARN(logger1,  logEvent)
#define TEST_E(logEvent)   LOG4CPLUS_ERROR(logger1, logEvent)
#define TEST_F(logEvent)   LOG4CPLUS_FATAL(logger1, logEvent)
//
#define TESTF_T(...)       LOG4CPLUS_TRACE_FMT(logger1, __VA_ARGS__)
#define TESTF_D(...)       LOG4CPLUS_DEBUG_FMT(logger1, __VA_ARGS__)
#define TESTF_I(...)       LOG4CPLUS_INFO_FMT(logger1,  __VA_ARGS__)
#define TESTF_W(...)       LOG4CPLUS_WARN_FMT(logger1,  __VA_ARGS__)
#define TESTF_E(...)       LOG4CPLUS_ERROR_FMT(logger1, __VA_ARGS__)
#define TESTF_F(...)       LOG4CPLUS_FATAL_FMT(logger1, __VA_ARGS__)
//
extern std::size_t pair_index(std::size_t i, std::size_t j);
extern std::size_t pair_init();
extern std::size_t pair_conv(std::size_t idx, std::size_t &i, std::size_t &j);
//
#endif //

