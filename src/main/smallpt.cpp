#include <math.h> 
#include <stdlib.h>
#include <stdio.h>  
//#include <omp.h>

#include "smallpt.h"
#include "profiler.h"

#include "scene.h"
#include "camera.h"
#include "integrators.h"
//#include "randoms.h"

namespace mei
{

#ifndef M_PI
#define M_PI  3.1415926
#endif

	std::vector<ProfilerEntry> CPUProfiler::ProfilerData(16);
	std::vector<ProfilerEntry> CPUProfiler::ProfilerDataA;

	inline Float clamp(Float x) { return x < 0 ? 0 : x>1 ? 1 : x; }
	inline int toInt(Float x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

	void smallptTest::render(void *data)
	{
		smallptTest &test = *(smallptTest*)data;
        test._integrator->Render(*test._scene);
	}

	smallptTest::smallptTest(int width, int height, int sample)
		: _imageWidth(width), _imageHeight(height), _spp(sample) 
		, _renderThread(nullptr)
        , _exitRendering(false)
		, _pauseRender(false)
	{
		_imageSizeInPixel = _imageHeight * _imageWidth;
        _scene  = std::unique_ptr<Scene>(new Scene);
		_camera = std::unique_ptr<Camera>(new Camera(_scene->_triangles.lookfrom, (_scene->_triangles.lookat - _scene->_triangles.lookfrom).norm(), _imageWidth, _imageHeight, 90.));

		this->handleScreenSizeChange(glm::ivec2(width, height));
		this->handleSampleCountChange(sample);
	}

#if 0
	//< 1. Start Rendering Thread When Test Created.
	//< 2. When ssp reached, Exit Rendering Thread.
	//< 3. When in Rendering, Only shadow data can be accessed.
	void smallptTest::newsmallpt(void)
	{
		Vector3 r;

		CHECK(_isRendering == false);

		while (!_exitRendering)
		{
			CPUProfiler profiler("Render Time",true);
			
			_isRendering = true;

			++_curSPP;
			Float invSPP = 1. / double(_spp);

            {
				//std::lock_guard<std::mutex> lock(_sMutex);
			    #pragma omp parallel for schedule(static, 1) private(r)       // OpenMP
			    for (int y = 0; y < _imageHeight; y++) // Loop over image rows
			    {
				    #if 0
				    #pragma omp critical
				    {
					    ss << " Thread Number: " << omp_get_num_threads() << "\t Thread ID: " << omp_get_thread_num() << "\n";
				    }
				    #endif
				    unsigned short Xi[] = { y*y*y, 0, _curSPP*_curSPP*_curSPP}; // random seed is important
                    for (uint32_t x = 0; x < _imageWidth; x++)   // Loop cols
                    {
                        int i = y * _imageWidth + x;
                        Ray ray = _camera->getRay(x, y, Xi);
                        r = _scene->myradiance(ray, 0, Xi);
						{
							_cumulativePixels[i] = _cumulativePixels[i] + r*invSPP;
							// Convert to float
							_pixels[i*3 + 0] = clamp(_cumulativePixels[i].x );
							_pixels[i*3 + 1] = clamp(_cumulativePixels[i].y );
							_pixels[i*3 + 2] = clamp(_cumulativePixels[i].z );
						}
                    }
			    }
            }
			
			//< get rendering process
			ss.str(""); ss.clear();
			ss << "[width: " << _imageWidth << ",height: " << _imageHeight <<  "]\t";
			ss << "[spp: " << _curSPP << "/" << _spp << "]" << std::endl;
			progress = ss.str();

			//< spp reached, exit rendering thread
			if (_curSPP == _spp)
			{
				_isRendering = false;
				break;
			}
		}// while end
		_isRendering = false;
	}
#endif

}