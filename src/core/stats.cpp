#include <algorithm>
#include <array>
#include <atomic>
#include <cinttypes>
#include <functional>
#include <mutex>
#include <type_traits>

#include "stats.h"
#include "stringprint.h"


namespace mei
{

// Statistics Local Variables
std::vector<std::function<void(StatsAccumulator &)>> *StatRegisterer::funcs = new std::vector<std::function<void(StatsAccumulator &)>>;

static StatsAccumulator statsAccumulator;

void PrintStats(FILE *dest) { statsAccumulator.Print(dest); }

void ClearStats() { statsAccumulator.Clear(); }

// Statistics Definitions
void ReportThreadStats()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    StatRegisterer::CallCallbacks(statsAccumulator);
}

void StatRegisterer::CallCallbacks(StatsAccumulator &accum)
{
    for (auto func : *funcs) func(accum);
}


static void getCategoryAndTitle(const std::string &str, std::string *category,
                                std::string *title)
{
    const char *s = str.c_str();
    const char *slash = strchr(s, '/');
    if (!slash)
        *title = str;
    else
    {
        *category = std::string(s, slash - s);
        *title = std::string(slash + 1);
    }
}

void StatsAccumulator::Print(FILE *dest)
{
    fprintf(dest, "Statistics:\n");
    std::map<std::string, std::vector<std::string>> toPrint;

    for (auto &counter : counters) {
        if (counter.second == 0) continue;
        std::string category, title;
        getCategoryAndTitle(counter.first, &category, &title);
        toPrint[category].push_back(StringPrintf( "%-42s               %12" PRIu64, title.c_str(), counter.second));
    }
    for (auto &counter : memoryCounters)
    {
        if (counter.second == 0) continue;
        std::string category, title;
        getCategoryAndTitle(counter.first, &category, &title);
        double kb = (double)counter.second / 1024.;
        if (kb < 1024.)
            toPrint[category].push_back(StringPrintf( "%-42s                  %9.2f kB", title.c_str(), kb));
        else
		{
            float mib = kb / 1024.;
            if (mib < 1024.)
                toPrint[category].push_back(StringPrintf( "%-42s                  %9.2f MiB", title.c_str(), mib));
            else
			{
                float gib = mib / 1024.;
                toPrint[category].push_back(StringPrintf( "%-42s                  %9.2f GiB", title.c_str(), gib));
            }
        }
    }

	for (auto &categories : toPrint)
	{
		fprintf(dest, "  %s\n", categories.first.c_str());
		for (auto &item : categories.second)
			fprintf(dest, "    %s\n", item.c_str());
	}
}

void StatsAccumulator::Clear()
{
    counters.clear();
    memoryCounters.clear();
}

}
