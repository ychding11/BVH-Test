
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef _CORE_STATS_H
#define _CORE_STATS_H

#include <map>
#include <vector>
#include <chrono>
#include <string>
#include <functional>

namespace mei
{

// Called by both Main Thread and Worker Thread
void ReportThreadStats();

// Called by Main Thread
void PrintStats(FILE *dest);

// Called by Main Thread
void ClearStats();


// Statistics Declarations
class StatsAccumulator; //forward declare

class StatRegisterer
{
  public:
    // StatRegisterer Public Methods
    StatRegisterer(std::function<void(StatsAccumulator &)> func)
    {
        funcs->push_back(func);
    }
    static void CallCallbacks(StatsAccumulator &accum);

  private:
    // StatRegisterer Private Data
    static std::vector<std::function<void(StatsAccumulator &)>> *funcs;
};


class StatsAccumulator
{
  public:
    // StatsAccumulator Public Methods
    void ReportCounter(const std::string &name, int64_t val)
    {
        counters[name] += val;
    }
    void ReportMemoryCounter(const std::string &name, int64_t val)
    {
        memoryCounters[name] += val;
    }

    void Print(FILE *file);
    void Clear();

  private:
    // StatsAccumulator Private Data
    std::map<std::string, int64_t> counters;
    std::map<std::string, int64_t> memoryCounters;
};


// Statistics Macros
#define STAT_COUNTER(title, var)                           \
    static thread_local int64_t var;                  \
    static void STATS_FUNC##var(StatsAccumulator &accum) { \
        accum.ReportCounter(title, var);                   \
        var = 0;                                           \
    }                                                      \
    static StatRegisterer STATS_REG##var(STATS_FUNC##var)
#define STAT_MEMORY_COUNTER(title, var)                    \
    static thread_local int64_t var;                  \
    static void STATS_FUNC##var(StatsAccumulator &accum) { \
        accum.ReportMemoryCounter(title, var);             \
        var = 0;                                           \
    }                                                      \
    static StatRegisterer STATS_REG##var(STATS_FUNC##var)

// Work around lack of support for constexpr in VS2013.
#ifdef PBRT_IS_MSVC2013
#define STATS_INT64_T_MIN LLONG_MAX
#define STATS_INT64_T_MAX _I64_MIN
#define STATS_DBL_T_MIN DBL_MAX
#define STATS_DBL_T_MAX -DBL_MAX
#else
#define STATS_INT64_T_MIN std::numeric_limits<int64_t>::max()
#define STATS_INT64_T_MAX std::numeric_limits<int64_t>::lowest()
#define STATS_DBL_T_MIN   std::numeric_limits<double>::max()
#define STATS_DBL_T_MAX   std::numeric_limits<double>::lowest()
#endif

}

#endif  // PBRT_CORE_STATS_H
