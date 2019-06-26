#ifndef SCENES_H_
#define SCENES_H_

#include <vector>

#include "objparser.h"
#include "Primitives.h"
#include "BVHTree.h"

extern double erand48(unsigned short xseed[3]);

namespace mei
{
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
            _mesh.vertex.push_back(Vector3(-1,1,0));
            _mesh.vertex.push_back(Vector3(1,1,0));
            _mesh.vertex.push_back(Vector3(1,-1,0));
            _mesh.vertex.push_back(Vector3(-1,-1,0));
            // quad by 2 triangles
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
            _bvh.InitTree(_triangles);
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
		Float _ior;
		bool _sphereScene;
		bool _triangleScene;
		SphereScene _spheres;

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

}//namespace

#endif


