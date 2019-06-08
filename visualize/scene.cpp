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

        float f1, f2, f3;
        assert(_objModel.v_size % 3 == 0);
        _mesh.v = _objModel.v;

        int* f = _objModel.f;
        assert(_objModel.f_size % 9 == 0);
		int nTriangles = _objModel.f_size / 9;
        for (int i = 0; i < nTriangles ; i++)
        {
			int idx0 = f[i * 9 + 0] * 3;
			int idx1 = f[i * 9 + 3] * 3;
			int idx2 = f[i * 9 + 6] * 3;
			Vector3 v0 = Vector3(_mesh.v[idx0 + 0], _mesh.v[idx0 + 1], _mesh.v[idx0 + 2]);
			Vector3 v1 = Vector3(_mesh.v[idx1 + 0], _mesh.v[idx1 + 1], _mesh.v[idx1 + 2]);
			Vector3 v2 = Vector3(_mesh.v[idx2 + 0], _mesh.v[idx2 + 1], _mesh.v[idx2 + 2]);
            _mesh.verts.push_back(v0);
            _mesh.verts.push_back(v1);
            _mesh.verts.push_back(v2);
            _mesh.faces.push_back(TriangleFace(3*i, 3*i+1, 3*i+2));
        }
        
        // calculate the bounding box of the _mesh
        _mesh.bounding_box[0] = Vector3(1000000, 1000000, 1000000);
        _mesh.bounding_box[1] = Vector3(-1000000, -1000000, -1000000);
        for (unsigned int i = 0; i < _mesh.verts.size(); i++)
        {
            _mesh.bounding_box[0] = min(_mesh.verts[i], _mesh.bounding_box[0]);
            _mesh.bounding_box[1] = max(_mesh.verts[i], _mesh.bounding_box[1]);
        }

        std::cout << "obj loaded: faces:" << _mesh.faces.size() << " vertices:" << _mesh.verts.size() << std::endl;
        std::cout << "obj aabb: min:(" << _mesh.bounding_box[0].x << "," << _mesh.bounding_box[0].y << "," << _mesh.bounding_box[0].z << ") max:("
            << _mesh.bounding_box[1].x << "," << _mesh.bounding_box[1].y << "," << _mesh.bounding_box[1].z << ")" << std::endl;
    }

	void TriangleScene::initTriangleScene()
	{
#if 1
        ObjParser objparser;
#else
        ObjParser objparser("../data/bunny.obj");
#endif
        TriangleMesh& mesh1 = objparser.getTriangleMesh();

		_aabb_min = mesh1.bounding_box[0] * _scale;  _aabb_min = _aabb_min + _translate;
		_aabb_max = mesh1.bounding_box[1] * _scale;  _aabb_max = _aabb_max + _translate;

        std::cout << "obj aabb: min:(" << mesh1.bounding_box[0].x << "," << mesh1.bounding_box[0].y << "," << mesh1.bounding_box[0].z << ") max:("
            << mesh1.bounding_box[1].x << "," << mesh1.bounding_box[1].y << "," << mesh1.bounding_box[1].z << ")" << std::endl;

		for (unsigned int i = 0; i < mesh1.faces.size(); i++)
		{
			// make a local copy of the triangle vertices
			Vector3 v0 = mesh1.verts[ mesh1.faces[i].v[0] ];
			Vector3 v1 = mesh1.verts[ mesh1.faces[i].v[1] ];
			Vector3 v2 = mesh1.verts[ mesh1.faces[i].v[2] ];

			// scale
			v0 *= _scale;
			v1 *= _scale;
			v2 *= _scale;

			// translate
			v0 += _translate;
			v1 += _translate;
			v2 += _translate;

            _triangles.push_back(Triangle(v0, v1, v2));
		}
        _numTriangles = _triangles.size();

		std::cout << "Total number of triangles in Scene :" << _numTriangles << std::endl;
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
        }
        return id == -1 && t == hit.t;
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


