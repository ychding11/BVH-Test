#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>

#ifdef RUN_ON_LINUX
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#else
#include <windows.h>
#endif

#include "progressreporter.h"

namespace smallpt
{

	// This function is OS dependt
	static int terminalWidth(void);

	// Create worker thread to report current progress.
	// The way to report is by printing here.
	ProgressReporter::ProgressReporter(int64_t totalWork, const std::string &title)
		: totalWork(totalWork)
		, title(title)
		, startTime(std::chrono::system_clock::now())
	{
		workDone = 0; // atomic variable
		exitThread = false; // atomic variable
		updateThread = std::thread([this]() {    printBar();   });
		std::cout << "Progress reporter thread created, total workd=" << totalWork << "\n";
	}

	// Wait worker thread to complete job
	ProgressReporter::~ProgressReporter()
	{
		workDone = totalWork;
		exitThread = true; // set thread exit flag.
		updateThread.join(); // wait thread complete jobs.
		printf("\n");
	}

	void ProgressReporter::done(void)
	{
		workDone = totalWork;
	}

	void ProgressReporter::printBar(void)
	{
		int barLength = terminalWidth() - 28;
		//int totalPlus = std::max(2, (barLength - (int)title.size()));
		int totalPlus = 8 < (barLength - (int)title.size()) ? (barLength - (int)title.size()) : 8;
		int plusPrinted = 0;

		const int bufLen = title.size() + totalPlus + 64;
		std::unique_ptr<char[]> buf(new char[bufLen]);
		snprintf(buf.get(), bufLen, "\r%s [", title.c_str());
		char *curSpace = buf.get() + strlen(buf.get());
		char *s = curSpace;
		for (int i = 0; i < totalPlus; ++i) *s++ = ' ';
		*s++ = ']';
		*s++ = ' ';
		*s++ = '\0';
		fputs(buf.get(), stdout);
		fflush(stdout);

		std::chrono::milliseconds sleepDuration(250);
		int iterCount = 0;
		while (!exitThread) //wait thread exit flag.
		{
			std::this_thread::sleep_for(sleepDuration);
			++iterCount;
			if (iterCount == 10) sleepDuration *= 2;
			else if (iterCount == 70) sleepDuration *= 2;
			else if (iterCount == 520) sleepDuration *= 5;

			float percentDone = float(workDone) / float(totalWork);
			int plusNeeded = std::round(totalPlus * percentDone);
			while (plusPrinted < plusNeeded)
			{
				*curSpace++ = '+';
				++plusPrinted;
			}
			fputs(buf.get(), stdout);

#if 1
			float seconds = elapsedMs() / 1000.f;
			float remains = seconds / percentDone - seconds;
			if (percentDone == 1.f)
				printf(" (%.1fs)       ", seconds);
			//else if (!std::isinf(remains))
			else if (remains >= 7200.0f) // more than 2 hours
				printf(" (%.1fs|%.1fs) ", seconds);
			else
				printf(" (%.1fs|?s)    ", seconds);
#endif			
			fflush(stdout);
		}
	}

	// This function is OS dependt
	static int terminalWidth(void)
	{
#ifdef RUN_ON_LINUX
		struct winsize w;
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0)
		{
			if (errno != ENOTTY)
			{
				static bool warned = false;
				if (!warned)
				{
					warned = true;
					printf("-- Error in ioctl() in terminalWidth(): %d\n", errno);
				}
			}
		}
		return w.ws_col;
#else
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (h == INVALID_HANDLE_VALUE || !h) {
			fprintf(stderr, "GetStdHandle() call failed");
			return 80;
		}
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
		GetConsoleScreenBufferInfo(h, &bufferInfo);
		return bufferInfo.dwSize.X;
#endif

	}
}
