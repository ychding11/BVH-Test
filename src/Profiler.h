#ifndef Stopwatch_h
#define Stopwatch_h

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/time.h>

class Stopwatch {
private:
	double start;
	double _stopwatch() const
	{
		struct timeval time;
		gettimeofday(&time, 0 );
		return 1.0 * time.tv_sec + time.tv_usec / (double)1e6;
	}
public:
	Stopwatch() { reset(); }
	void reset() { start = _stopwatch(); }
	double read() const {	return _stopwatch() - start; }
};
#else

#include <numeric>
#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>

// https://www.modernescpp.com/index.php/the-three-clocks
template <typename T>
void printRatio()
{
    std::cout << "  precision: " << T::num << "/" << T::den << " second " << std::endl;
    typedef typename std::ratio_multiply<T, std::kilo>::type MillSec;
    typedef typename std::ratio_multiply<T, std::mega>::type MicroSec;
    std::cout << std::fixed;
    std::cout << "             " << static_cast<double>(MillSec::num) / MillSec::den << " milliseconds " << std::endl;
    std::cout << "             " << static_cast<double>(MicroSec::num) / MicroSec::den << " microseconds " << std::endl;
}

class Stopwatch {
private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock>  _stopwatch() const
    {
        return std::chrono::system_clock::now();
    }

public:
    Stopwatch() { reset(); }
    void reset() { start = _stopwatch(); }

    // return second
    double read() const
    {
        std::chrono::duration<double> diff = _stopwatch() - start;
        return diff.count();
    }
};

struct ProfilerEntry
{
    std::string name;
    double detaTime;
	int count;

    std::string str()
    {
        std::ostringstream ss;
        ss << std::setw(10) << name << "\t" << std::setprecision(3) << std::fixed << (detaTime / count) * 1000.f << " ms";
        return ss.str();
    }
};

class CPUProfiler
{
private:
    static std::vector<ProfilerEntry> ProfilerData;
    static std::vector<ProfilerEntry> ProfilerDataA;
public:
    static void begin() { ProfilerData.clear(); }

    static std::string end()
    {
        std::ostringstream ss;
        for (int i = 0; i < ProfilerDataA.size(); ++i)
        {
            ss << ProfilerDataA[i].str() << std::endl;
			if (ProfilerDataA[i].count >= 30)
			{
				ProfilerDataA[i].count = 0;
				ProfilerDataA[i].detaTime = 0.0;
			}
        }
		ss << "\n----------------------\n";
        for (int i = 0; i < ProfilerData.size(); ++i)
        {
            ss << ProfilerData[i].str() << std::endl;
        }
        return ss.str();
    }

private:
    std::string _name;
    bool _isPersist;
    Stopwatch _stopWatch;

public:
    CPUProfiler(std::string name)
        : _name(name)
		, _isPersist(false)
        , _stopWatch()
    {
    }

    CPUProfiler(std::string name, bool persist)
        : _name(name)
		, _isPersist(persist)
        , _stopWatch()
    {
    }

    ~CPUProfiler()
    {
        double deta = _stopWatch.read();
		
		if (_isPersist == false)
		{
			ProfilerEntry entry = {_name, deta, 1};
			ProfilerData.push_back(entry);
		}
		else
		{
			int found = -1;
			for (int i = 0; i < ProfilerDataA.size(); ++i)
			{
				if (ProfilerDataA[i].name == _name)
				{
					found = i;
					break;
				}
			}
			if (found == -1)
			{
				ProfilerEntry entry = {_name, deta, 1};
				ProfilerDataA.push_back(entry);

			}
			else
			{
				ProfilerDataA[found].count++;
				ProfilerDataA[found].detaTime += deta;
			}
		}
    }

};

#endif
#endif