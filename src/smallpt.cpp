﻿#include <math.h> 
#include <stdlib.h>
#include <stdio.h>  
//#include <omp.h>

#include "parallel.h"
#include "progressreporter.h"
#include "smallpt.h"
#include "profiler.h"

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
		test.newsmallpt();
        //test.renderTile();
	}

	smallptTest::smallptTest(int width, int height, int sample)
		: _imageWidth(width), _imageHeight(height), _spp(sample) 
		, _renderThread(nullptr)
        , _exitRendering(false)
		, _pauseRender(false)
		, _ior(1.5f)
	{
		_imageSizeInPixel = _imageHeight * _imageWidth;
        _scene = std::shared_ptr<Scene>(new Scene);
		_camera = std::shared_ptr<Camera>(new Camera(_scene->_triangles.lookfrom, (_scene->_triangles.lookat - _scene->_triangles.lookfrom).norm(), _imageWidth, _imageHeight, 90.));
		//_camera = std::shared_ptr<Camera>(new Camera(Vector3(50,52,295.6), Vector3(0, -0.042612, -1).norm(), _imageWidth, _imageHeight, 90.));
		this->handleScreenSizeChange(glm::ivec2(width, height));
		this->handleSampleCountChange(sample);

		this->handleSceneMaskChange(0x2);
	}

    std::mutex smallptTest::_sMutex;

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
			ss << "[width: " << _imageWidth << ",height: " << _imageHeight <<  "]" << std::endl;
			ss << "[current: " << _curSPP << "] spp:" << _spp << std::endl;
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

	void Render(std::shared_ptr<Camera> camera, std::shared_ptr<Scene> scene)
	{
		// Compute number of tiles, _nTiles_, to use for parallel rendering
		Bounds2i sampleBounds = camera->sampleBound();
		Vector2i sampleExtent = camera->filmSize();
		const int tileSize = 16;
		Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
			           (sampleExtent.y + tileSize - 1) / tileSize);


		//ProgressReporter reporter(nTiles.x * nTiles.y, "Rendering");

		{
			ParallelFor2D([&](Point2i tile) {
				// Render section of image corresponding to _tile_

				// Get sampler instance for tile
				int seed = tile.y * nTiles.x + tile.x;
                unsigned short Xi[] = { seed*seed*seed, 0, 0 };

				// Compute sample bounds for tile
				int x0 = sampleBounds.pMin.x + tile.x * tileSize;
				int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
				int y0 = sampleBounds.pMin.y + tile.y * tileSize;
				int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
				Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

                std::unique_ptr<FilmTile> filmTile = camera->pFilm->GetFilmTile(tileBounds);

				// Loop over pixels in tile to render them
				for (Point2i pixel : tileBounds) {
                    Ray ray = camera->getRay(pixel.x, pixel.y, Xi);
                    Vector3 L = scene->myradiance(ray, 0, Xi);

                    // Add camera ray's contribution to image
                    filmTile->AddSample(pixel, L);
				}

                // Merge image tile into _Film_
                camera->pFilm->MergeFilmTile(std::move(filmTile));

			//	reporter.update();
			}, nTiles);
		//	reporter.done();
		}
	}
    
}