#ifndef SMALL_PT_H 
#define SMALL_PT_H 

#include <cmath>
#include <sstream>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "interface.h"
#include "3d.h"

namespace mei
{
    //< forward declare
    class Integrator;
    class Scene;
    class Camera;

	// smallptTest as a basic Test
	class smallptTest : public Observer
	{
	public:
		static void render(void *data);

	private:
		uint32_t _imageWidth;
		uint32_t _imageHeight;
		uint32_t _imageSizeInPixel;
		uint32_t _spp;
		uint32_t _curSPP = 0;
		Vector3f *_cumulativePixels = nullptr;
		float *_pixels = nullptr;
		float *_renderResult = nullptr;
		bool _renderSettingIsDirty = true;
		bool _renderTargetIsDirty = true;

        std::unique_ptr<Scene>       _scene; 
        std::unique_ptr<Integrator>  _integrator; 

		std::thread *_renderThread = nullptr;
        bool _exitRendering = false; //< tell rendering thread to exit
		bool _isRendering = false; //< rendering thread is working 
		bool _pauseRender;

		std::ostringstream ss;
		std::string progress;

	private:
		void startRenderingThread(void)
		{
			CHECK(_renderThread == nullptr); //< only one rendering thread is allowed
			_exitRendering = false;
			_isRendering = false;
			_renderThread = new std::thread(smallptTest::render, this);
		}
		void exitRenderingThread(void)
		{
			if (_renderThread == nullptr) return;
			{
				_exitRendering = true;
				if (_renderThread) _renderThread->join(); //< wait render thread exit, then delete resource.
			    _isRendering = false;
			}
			CHECK(_renderThread->joinable()==false);
			delete _renderThread;
			_renderThread = nullptr;
		}

		void clearRenderTargets()
		{
			memset(_pixels, 0, sizeof(_pixels[0]) * _imageSizeInPixel);
			memset(_cumulativePixels, 0, sizeof(_cumulativePixels[0]) * _imageSizeInPixel);
			memset(_renderResult, 0, sizeof(_renderResult[0]) * _imageSizeInPixel);
		}

		void reclaimRenderTargets()
		{
			_cumulativePixels = new Vector3f[_imageSizeInPixel];
			_pixels           = new float[_imageSizeInPixel * 3];
			_renderResult     = new float[_imageSizeInPixel * 3];
		}

		void destroyRenderTargets()
		{
			delete _cumulativePixels;
			delete _pixels;
			delete _renderResult;
		}

	public:
		smallptTest(int width = 720, int height = 720, int sample = 1);

		~smallptTest()
		{
			exitRenderingThread();
			destroyRenderTargets();
		}

		virtual void* getRenderResult() const override
		{
            //std::lock_guard<std::mutex> lock(_sMutex);
            return _pixels;
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
			if (_renderSettingIsDirty)
			{
				startRenderingThread();
				_renderSettingIsDirty = false;
			}
		}

	public:

		//< Render Target change:
		//< 1. Exit current rendering thread.
		//< 2. Set new settings, reclaim memory.
		//< 3. Set flag to tell render setting is dirty
		virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override;

		//< Render ssp change:
		//< 1. Exit current rendering thread.
		//< 2. Set new settings, clear render memory.
		//< 3. Set flag to tell render setting is dirty
		virtual void handleSampleCountChange(int sample) override
		{
			if (this->_spp != sample)
			{
				exitRenderingThread();

				this->_spp = sample;
				_renderSettingIsDirty = true;
				clearRenderTargets();
			}
		}

		//< Render Target change:
		//< 1. Exit current rendering thread.
		//< 2. Set new settings, clear memory.
		//< 3. Set flag to tell render setting is dirty
		virtual void handleIORChange(float newIOR) override
		{
		}

		//< Render Target change:
		//< 1. Exit current rendering thread.
		//< 2. Set new settings, clear render memory.
		//< 3. Set flag to tell render setting is dirty
		virtual void handleSceneMaskChange(uint32_t newMask) override
		{
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
	};

	
}

#endif
