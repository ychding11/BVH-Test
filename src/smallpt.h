#ifndef SMALL_PT_H 
#define SMALL_PT_H 

#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "interface.h"
#include "Vector3.h"
#include "Primitives.h"
#include "scene.h"
#include "camera.h"
#include "randoms.h"

namespace mei
{
	class smallptTest : public Observer
	{
	public:
		static void render(void *data);
        static std::mutex _sMutex;

	private:
		int w, h;
		int spp;
		int iterates;
		Vector3 *c;
		float *data;
		int runTest;
		std::ostringstream ss;
		std::string progress;
		Scene scene;

        std::unique_ptr<Camera> _camera; 

		std::mutex _mutex;
		std::condition_variable _condVar;
		std::thread *_renderThread;
        bool _exitRendering;
		bool _isRendering; //< rendering thread responsible for this
		bool _pauseRender;
		float _ior;

	public:
		smallptTest(int width = 720, int height = 720, int sample = 1);

		~smallptTest()
		{
            _exitRendering = true;
            if (_renderThread) _renderThread->join(); //< wait render thread exit, then delete resource.
			delete _renderThread;
			delete c;
			delete data;
		}

		virtual void* getRenderResult() const override
		{
            std::lock_guard<std::mutex> lock(_sMutex);
            return data;
		}

		std::string renderLog() const { return ss.str(); }
		std::string renderProgress() const { return progress; }

		virtual std::string getRenderProgress() const override
		{
			return progress;
		}

		void clearRenderLog() { ss.clear(); }

		void run()
		{
			if (runTest)
			{
				_condVar.notify_one();
			}
		}
	private:
		void newsmallpt();

	public:
		virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override
		{
            std::lock_guard<std::mutex> lock(_sMutex);
			_camera->setImageSize(newScreenSize.x, newScreenSize.y);
            w = newScreenSize.x; h = newScreenSize.y;
			delete c; delete data; //< delete nullptr is OK
		    c = new Vector3[w * h]; data = new float[w * h * 3];
			assert((c && data));
			memset(data, 0, sizeof(data[0])*w*h);
			memset(c, 0, sizeof(c[0])*w*h);
			iterates = 0;
		}
		virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handlePositionOffsetChange(Float newPositionOffset)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleObjectNumChange(int newObjectNum)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleTestIndexChange(int newTestIndex)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleSampleCountChange(int sample) override
		{
			if (this->spp != sample / 4)
			{
				// signal rendering thread stop
				// wait for rendering thread's signal
				this->spp = sample / 4;
				memset(data, 0, sizeof(data[0])*w*h);
				memset(c, 0, sizeof(c[0])*w*h);
				iterates = 0;
				runTest = true;
			}
		}

		virtual void handleIORChange(float newIOR) override
		{
            _exitRendering = true;
            if (_renderThread) _renderThread->join(); //< wait render thread exit, then delete resource.
			delete _renderThread;
			scene._ior = newIOR;
			memset(data, 0, sizeof(data[0])*w*h);
			memset(c, 0, sizeof(c[0])*w*h);
			iterates = 0;
            _exitRendering = false;
		    _renderThread = new std::thread(smallptTest::render, this);
		}
		virtual void handleSceneMaskChange(uint32_t newMask) override
		{
            _exitRendering = true;
            if (_renderThread) _renderThread->join(); //< wait render thread exit, then delete resource.
			delete _renderThread;
			scene._sphereScene = newMask & 0x1 ? true : false;
			scene._triangleScene = newMask & 0x2 ? true : false;
			memset(data, 0, sizeof(data[0])*w*h);
			memset(c, 0, sizeof(c[0])*w*h);
			iterates = 0;
            _exitRendering = false;
		    _renderThread = new std::thread(smallptTest::render, this);
		}
	};

}

#endif
