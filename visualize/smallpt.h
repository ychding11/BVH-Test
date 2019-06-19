#ifndef SMALL_PT_H 
#define SMALL_PT_H 

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "interface.h"
#include "Stopwatch.h"
#include "objparser.h"
#include "Vector3.h"
#include "Primitives.h"
#include "BVHTree.h"

extern double erand48(unsigned short xseed[3]);

namespace smallpt
{

#ifndef M_PI
#define M_PI  3.1415926
#endif

#define Log printf


    //! Generate a random float in [0, 1)
    Float randomFloat();

	inline Float minFloat(Float a, Float b) { return a < b ? a : b; }
	inline Float maxFloat(Float a, Float b) { return a > b ? a : b; }

	//! Generate rays easly
	class Camera
	{
	private:
		Float _vfov;
		uint32_t _w, _h;
		Vector3 _p, _u, _v, _d; //< coordiate & position

		void constructCoordinate()
		{
			Float halfVfov = _vfov * 0.5 * (M_PI / 180.);
            Float h = std::tanf(halfVfov);
			Float aspect = double(_w) / double(_h);
			_u = Vector3(aspect * h, 0, 0);
			_v = (_u%_d).norm()*h;
		}

        //! get a sample in [0,0), support subpixel;
        Float getSample(unsigned short *X = nullptr)
        {
            static uint32_t i = 0;
            uint32_t N = 2; //< NxN sub-pixel
            Float w = 1. / double(N) * 0.5; //< width of sub-pixel
            Float c = w; //< first center of sub-pixel
            Float rd; //< random uniform distribute [0,1)
            if (X)
            {
                rd = erand48(X);
            }
            else
            {
                rd = randomFloat();
            }
            rd = (2. * rd - 1.) * w;
            c =  (i++) % N * w * 2.;
            return c + rd;
        }

	public:
		Camera() = delete; //< No default constructor allowed
		//Camera(uint32_t w, uint32_t _h, Float vfov);

		//! \param position is in world space
		//! \param dir is in world space and is normalized
		Camera(Vector3 position, Vector3 dir, uint32_t w = 1024, uint32_t h = 1024, Float vfov = 55.)
			: _p(position), _d(dir)
			, _w(w), _h(h)
			, _vfov(vfov)
		{
			std::cout << "camera dir: " << _d.str() << "\ncamera pos:" << _p.str() << std::endl;
			constructCoordinate();
		}

		void setImageSize(uint32_t w, uint32_t h)
		{
			if (w != _w || h != _h)
			{
				_w = w; _h = h;
				constructCoordinate();
			}
		}

		Float aspect() const { return double(_w) / double(_h); };

		//! get a random ray based on (u, v) in image plane
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr)
		{
            Float x = getSample(X);
            Float y = getSample(X);
            Float dW = 1. / double(_w);
            Float dH = 1. / double(_h);
            x = x * dW + double(u) / double(_w) - 0.5;
            y = y * dW + double(v) / double(_w) - 0.5;
            Vector3 d = _u * x + _v * y + _d;
			//Ray r(_p + d * 140, d.norm());
			Ray r(_p, d.norm());
			return r;
		}
	};


    struct hitable
    {
       virtual bool intersec(const Ray&r, IntersectionInfo& hit) const = 0;
    };

    class SphereScene : public hitable
    {
    private:
        std::vector<Object*> _prims;
        bool _initialized;

    public:
		SphereScene() :  _initialized(false) { init(); }
        bool initialized() const { return _initialized; }
		void init();
        void initScene(Sphere* scene, int n)
        {
            _prims.clear();
            for (int i = n; i--; )
            {
                _prims.push_back(&scene[i]);
            }
            _initialized = true;
        }

        bool intersec(const Ray&r, IntersectionInfo& hit) const override;
    };

    //! only position needed 
    struct TriangleFace
    {
        int v[3]; //< vertex indices
        TriangleFace() { v[0] = v[1] = v[2] = 0; }
        TriangleFace(int x, int y, int z) { v[0] = x; v[1] = y; v[2] = z; }
    };

    struct TriangleMesh
    {
        std::vector<Vector3> vertex;
        std::vector<TriangleFace> faces;
    };

    class ObjParser
    {
    private:
        std::string  _filepath;
        TriangleMesh _mesh;
        ObjFile _objModel;

        void loadObj();

        void unitTriangle()
        {
#if 0
            _mesh.vertex.push_back(Vector3(0,1,0));
            _mesh.vertex.push_back(Vector3(-1,0,0));
            _mesh.vertex.push_back(Vector3(0,0,1));
            _mesh.vertex.push_back(Vector3(1,0,0));
            _mesh.vertex.push_back(Vector3(0,0,-1));
            //_mesh.faces.push_back(TriangleFace(0, 1, 2));
            _mesh.faces.push_back({ 0, 1, 2 });
            _mesh.faces.push_back({ 0, 2, 3 });
            _mesh.faces.push_back({ 0, 3, 4 });
            _mesh.faces.push_back({ 0, 4, 1 });
#endif
            _mesh.vertex.push_back(Vector3(-1,1,0));
            _mesh.vertex.push_back(Vector3(1,1,0));
            _mesh.vertex.push_back(Vector3(1,-1,0));
            _mesh.vertex.push_back(Vector3(-1,-1,0));
            //_mesh.faces.push_back(TriangleFace(0, 1, 2));
            _mesh.faces.push_back({ 0, 1, 2 });
            _mesh.faces.push_back({ 0, 2, 3 });

        }

    public :
        ObjParser() :_filepath("")
        {
            unitTriangle();
        }

        ObjParser(std::string file) :_filepath(file)
        {
            loadObj();
        }

        const TriangleMesh& getTriangleMesh() const { return _mesh; }
              TriangleMesh& getTriangleMesh()       { return _mesh; }
    };

    class TriangleScene : public hitable
    {
    private:
	    int     _numTriangles;
	    Vector3	_aabbMin;
	    Vector3	_aabbMax;
        Float   _scale;
        Vector3 _translate;
        std::vector<Triangle> _triangles;
        std::string _sceneName;
        BVHTree _bvh;

	    void initTriangleScene();
	public:
		Vector3 sceneSize;
		Vector3 sceneCenter;
		Vector3 lookfrom;
		Vector3 lookat;

    public:
        TriangleScene()
            :_scale(1.), _translate(0, 0, 0)
			,_aabbMin(inf, inf, inf)
			,_aabbMax(-inf, -inf, -inf)
        {
	        initTriangleScene();
        }

        bool intersec(const Ray& r, IntersectionInfo &hit) const override;
        bool intersecTri(const Ray& r, IntersectionInfo &hit) const;

        bool intersecBVH(const Ray& r, IntersectionInfo &hit) const
        {
            IntersectionInfo info;
            bool ret = _bvh.intersec(r, &info);
            hit = info;
            return ret;
        }

		Vector3 aabbMin() const { return _aabbMin; }
		Vector3 aabbMax() const { return _aabbMax; }
    };

	class smallptTest;

    class Scene
    {
    private:
        bool _initialized;
		SphereScene _spheres;
		Float _ior;
		bool _sphereScene;
		bool _triangleScene;
	public:
		TriangleScene _triangles;

	friend class smallptTest;

        bool intersec(const Ray& r, IntersectionInfo &hit) const;

    public:
		Scene()
            : _initialized(false), _ior(1.5)
			, _sphereScene(true)
			, _triangleScene(false)
		{ init(); }

		bool init();
        bool initialized() const { return _initialized; }

		Vector3 myradiance(const Ray &r, int depth, unsigned short *Xi);
		//Vector3 newradiance(const Ray &r, int depth, unsigned short *Xi);

		//! used for hit algorithms test.
		Vector3 hittest(const Ray &r);

    };

	// resize screen size
	// reset sample number
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

        Camera *_camera; // cam pos, dir

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
			Log("%s()\n", __FUNCTION__);
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
