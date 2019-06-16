#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  
#include <omp.h>
#include <vector>
#include <random>
#include <iostream>
#include <fstream>

#include "smallpt.h"

namespace smallpt
{

#ifndef M_PI
#define M_PI  3.1415926
#endif

    // only read positon triangle data from obj file
    void ObjParser::loadObj()
    {
        {
            CPUProfiler("Load Model", true);

            //! Load full obj Model
            if (!objParseFile(_objModel, _filepath.c_str()))
            {
                std::cout << "ERROR: loading obj:(" << _filepath << ") file not found or not good" << "\n";
                system("PAUSE");
                exit(0);
            }
        }

        assert(_objModel.v_size % 3 == 0);
        assert(_objModel.f_size % 9 == 0);
        float *v = _objModel.v;

        int* f = _objModel.f;
		int nTriangles = _objModel.f_size / 9;
        for (int i = 0; i < nTriangles ; i++)
        {
			int idx0 = f[i * 9 + 0] * 3;
			int idx1 = f[i * 9 + 3] * 3;
			int idx2 = f[i * 9 + 6] * 3;
			Vector3 v0 = Vector3(v[idx0 + 0], v[idx0 + 1], v[idx0 + 2]);
			Vector3 v1 = Vector3(v[idx1 + 0], v[idx1 + 1], v[idx1 + 2]);
			Vector3 v2 = Vector3(v[idx2 + 0], v[idx2 + 1], v[idx2 + 2]);
            _mesh.vertex.push_back(v0);
            _mesh.vertex.push_back(v1);
            _mesh.vertex.push_back(v2);
            _mesh.faces.push_back(TriangleFace(3*i, 3*i+1, 3*i+2));
        }
        
        std::cout << "obj loaded: faces:" << _mesh.faces.size() << " vertices:" << _mesh.vertex.size() << std::endl;
    }

	void TriangleScene::initTriangleScene()
	{
#if 0
        ObjParser objparser;
#else
        ObjParser objparser("../data/bunny.obj");
#endif
        TriangleMesh& mesh = objparser.getTriangleMesh();
		for (unsigned int i = 0; i < mesh.faces.size(); i++)
		{
			// make a local copy of the triangle vertices
			Vector3 v0 = mesh.vertex[ mesh.faces[i].v[0] ];
			Vector3 v1 = mesh.vertex[ mesh.faces[i].v[1] ];
			Vector3 v2 = mesh.vertex[ mesh.faces[i].v[2] ];

			// scale
			v0 *= _scale;
			v1 *= _scale;
			v2 *= _scale;

			// translate
			v0 += _translate;
			v1 += _translate;
			v2 += _translate;

			_aabbMin = cmin(_aabbMin, v0); _aabbMax = cmax(_aabbMax, v0);
			_aabbMin = cmin(_aabbMin, v1); _aabbMax = cmax(_aabbMax, v1);
			_aabbMin = cmin(_aabbMin, v2); _aabbMax = cmax(_aabbMax, v2);

            _triangles.push_back(Triangle(v0, v1, v2));
		}
        _numTriangles = _triangles.size();

		sceneSize   = _aabbMax - _aabbMin;
		sceneCenter = (_aabbMin + _aabbMax) * 0.5;
		//lookfrom = sceneCenter + 0.5 * length(sceneSize)*(Vector3(0., 0., 1.));
		lookfrom = sceneCenter + 0.5 * (sceneSize.y)*(Vector3(0., 0., 6.));
		lookat   = sceneCenter;

		std::cout << "Total number of triangles in Scene :" << _numTriangles << std::endl;
		std::cout << "min: " << _aabbMin.str() << "\nmax: " << _aabbMax.str() << std::endl;
		std::cout << "lookat: " << lookat.str() << "\nlookfrom: " << lookfrom.str() << std::endl;
	}

	bool TriangleScene::intersec(const Ray& r, IntersectionInfo &hit) const
	{
        int id = -1;
        Float inf = 1e20, t = inf;
		for (int i = 0; i < _numTriangles; i++)
		{
			const Triangle& triangle = _triangles[i];
            Float ct = triangle.intersect(r);
            {
			    if (ct < t && ct > 1e-5 && ct < inf)
			    {
			        t = ct; id = i;
			    }
            }
		}
        if (id != -1 && t < hit.t)
        {
            hit.t = t;
			hit.object = &_triangles[id];
			hit.hit = r.o + r.d*t;
			return true;
        }
        return false;
	}

	bool Scene::intersec(const Ray& r, IntersectionInfo &hit) const
	{
		bool a = false;
		if (_sphereScene)
		{
			a = _spheres.intersec(r, hit) || a;
		}
		if (_triangleScene)
		{
			a = _triangles.intersec(r, hit) || a;
		}
		return a;
	}
}//namespace


