#ifndef PROFILER_H_
#define PROFILER_H_

#include <numeric>
#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace utility 
{

	class Framerate
	{
	private:
		float fps_;
		std::chrono::duration<float> delta_;

	public:
		Framerate() : delta_(0.f), fps_(0.f)
        {
        }

        //< update every frame in main loop 
		void Update()
		{
			static auto last = std::chrono::steady_clock::now();
			auto now = std::chrono::steady_clock::now();
			delta_ = now - last;
			last = now;
		}

        //< refresh fps every second
		float GetFps() 
		{
			static auto last = std::chrono::steady_clock::now();
			static float delta_sum = 0.f;
			static unsigned times = 0;

			delta_sum += delta_.count();
			times++;

			auto now = std::chrono::steady_clock::now();
			if(now > last + std::chrono::seconds(1) && times)
			{
				fps_ = (float)times / delta_sum;
				delta_sum = times = 0;
				last = now;
			}

			return fps_;
		}

		float GetDelta() const
        {
            return delta_.count();
        }
	};

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

    class Stopwatch
    {
    private:
        std::chrono::time_point<std::chrono::system_clock> start;
        std::chrono::time_point<std::chrono::system_clock>  _stopwatch() const
        {
            return std::chrono::system_clock::now();
        }

    public:
        Stopwatch()  { reset(); }
        void reset() { start = _stopwatch(); }

        //< return second unit
        double read() const
        {
            std::chrono::duration<double> diff = _stopwatch() - start;
            return diff.count();
        }
    };

    class FrameRateDetector
    {
    private:
        double _frametime; //< in ms unit
        double _framerate; //< 1000/_frametime
        Stopwatch _stopwatch;
    public:
        FrameRateDetector() :_frametime(1e20), _stopwatch() { _framerate = 1000. / _frametime; }
        void start() {}
        void stop()
        {
            _frametime = _stopwatch.read() * 1000.; //< in ms
            _framerate = 1000. / _frametime;
        }
        double framerate() const { return _framerate; }
        double frametime() const { return _frametime; }
    };

    struct ProfilerEntry
    {
        std::string name; //< profiler entry name, for example "Update", "Render"
        std::vector<double> samples; //< run time samples for this entry

        ProfilerEntry()
        { }

        ProfilerEntry(std::string _name, double v, double avg = 0.0)
            : name(_name)
            , samples(1,v)
            , avgTime(avg)
        {
        }

        std::string str()
        {
            auto & v = samples;
            auto lambda_avg = [&v](double a, double b) { return a + b / v.size(); };
            avgTime = std::accumulate(v.begin(), v.end(), 0.0, lambda_avg);
            auto maxTime = *std::max_element(v.begin(), v.end());
            auto minTime = *std::min_element(v.begin(), v.end());

            std::ostringstream ss;
            if (v.size() > 1)
            {
                ss << std::left << std::setw(24)
                    <<  name << "\t avg "
                    << std::setprecision(3) << std::fixed
                    << avgTime * 1000.f << " ms\n";

                ss << std::left << std::setw(24)
                    <<  name << "\t min "
                    << std::setprecision(3) << std::fixed
                    << minTime * 1000.f << " ms\n";

                ss << std::left << std::setw(24)
                    <<  name << "\t max "
                    << std::setprecision(3) << std::fixed
                    << maxTime * 1000.f << " ms\n";
            }
            else
            {
                for (auto i = v.begin(); i != v.end(); ++i)
                {
                    ss << std::left << std::setw(24) <<  name << "\t" << std::setprecision(3) << std::fixed << *i * 1000.f << " ms\n";
                }
            }

            return ss.str();
        }
    private:
        double avgTime = 0.0;
    };

    class CPUProfiler
    {
    private:
        static std::unordered_map<std::string, ProfilerEntry> ProfilerData;

    public:
        static void begin() { ProfilerData.clear(); }

        static std::string result()
        {
            std::ostringstream ss;
            ss << "\n-------------------------- Profiler Summary ----------------------------\n";
            for (auto it = ProfilerData.begin(); it != ProfilerData.end(); ++it)
            {
                ss << it->second.str() << std::endl;
            }
            return ss.str();
        }

        static std::string end()
        {
            std::string str = result();
            std::cout << str;
            return str;
        }

    private:
        std::string _name;
        Stopwatch _stopWatch;

    public:
        CPUProfiler(std::string name)
            : _name(name)
            , _stopWatch()
        { }

        ~CPUProfiler()
        {
            double deta = _stopWatch.read();
            auto ret = ProfilerData.find(_name);
            if (ret == ProfilerData.end())
            {
                ProfilerEntry entry = {_name, deta, 0.0};
                ProfilerData[_name] = entry;
            }
            else
            {
                ProfilerData[_name].samples.push_back(deta);
            }
        }
    };

}

#define PROFILER_MARKER(name) utility::CPUProfiler profiler_##name(#name);

#endif
