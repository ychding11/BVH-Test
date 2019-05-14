#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  
#include <omp.h>
#include <vector>
#include <random>
#include <iostream>
#include <fstream>

#include "smallpt.h"
#include "Ray.h"
#include "Object.h"
#include "BVH.h"


namespace smallpt
{

#ifndef M_PI
#define M_PI  3.1415926
#endif

    // only read positon triangle data from obj file
    void ObjParser::loadObj()
    {
        std::ifstream in(_filepath.c_str());

        if (!in.good())
        {
            std::cout << "ERROR: loading obj:(" << _filepath << ") file not found or not good" << "\n";
            system("PAUSE");
            exit(0);
        }

        char buffer[256], str[255];
        Float f1, f2, f3;

        while (!in.getline(buffer, 255).eof())
        {
            buffer[255] = '\0';
            sscanf_s(buffer, "%s", str, 255);
            if (buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))// reading a vertex
            {
                if (sscanf(buffer, "v %e %e %e", &f1, &f2, &f3) == 3)
                {
                    _mesh.verts.push_back(Vector3(f1, f2, f3));
                }
                else
                {
                    std::cout << "ERROR: vertex not in wanted format in OBJLoader" << "\n";
                    exit(-1);
                }
            }
            else if (buffer[0] == 'f' && (buffer[1] == ' ' || buffer[1] == 32))// reading faceMtls 
            {
                TriangleFace f;
                int nt = sscanf(buffer, "f %d %d %d", &f.v[0], &f.v[1], &f.v[2]);
                if (nt != 3)
                {
                    std::cout << "ERROR: I don't know the format of that FaceMtl" << "\n";
                    exit(-1);
                }
                _mesh.faces.push_back(f);
            }
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
        std::cout << "obj asbb: min:(" << _mesh.bounding_box[0].x << "," << _mesh.bounding_box[0].y << "," << _mesh.bounding_box[0].z << ") max:"
            << _mesh.bounding_box[1].x << "," << _mesh.bounding_box[1].y << "," << _mesh.bounding_box[1].z << ")" << std::endl;
    }

	void TriangleScene::initTriangleScene()
	{
        ObjParser objparser;
        //ObjParser objparser("../data/bunny.obj");
        TriangleMesh& mesh1 = objparser.getTriangleMesh();

		_aabb_min = mesh1.bounding_box[0] * _scale;  _aabb_min = _aabb_min + _translate;
		_aabb_max = mesh1.bounding_box[1] * _scale;  _aabb_max = _aabb_max + _translate;

		for (unsigned int i = 0; i < mesh1.faces.size(); i++)
		{
			// make a local copy of the triangle vertices
			Vector3 v0 = mesh1.verts[mesh1.faces[i].v[0] - 1];
			Vector3 v1 = mesh1.verts[mesh1.faces[i].v[1] - 1];
			Vector3 v2 = mesh1.verts[mesh1.faces[i].v[2] - 1];

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
			if (Float ct = triangle.intersect(r))
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
		bool a = _spheres.intersec(r, hit);
		bool b = _triangles.intersec(r, hit);
		return a || b;
	}

	static Box boxes[] =
	{
		// FORMAT: { minbounds,   maxbounds,         emission,    colour,       Refl_t }
		{ { 5.0f, 0.0f, 70.0f },   { 45.0f, 11.0f, 115.0f },  { .0f, .0f, 0.0f }, { 0.5f, 0.5f, 0.5f }, DIFF },
		{ { 85.0f, 0.0f, 95.0f },  { 95.0f, 20.0f, 105.0f },  { .0f, .0f, 0.0f }, { 0.5f, 0.5f, 0.5f }, DIFF },
		{ { 75.0f, 20.0f, 85.0f }, { 105.0f, 22.0f, 115.0f }, { .0f, .0f, 0.0f }, { 0.5f, 0.5f, 0.5f }, DIFF },
	};

	static Sphere spheres[] =
	{
#if 1
#if 0
		// FORMAT: { Float radius, Vector3 position, Vector3 emission, Vector3 colour, Refl_t material }
		// cornell box
		{ 1e5f, { 1e5f + 1.0f, 40.8f, 81.6f }, { 0.0f, 0.0f, 0.0f }, { 0.75f, 0.25f, 0.25f }, DIFF }, //Left 1e5f
		{ 1e5f, { -1e5f + 99.0f, 40.8f, 81.6f }, { 0.0f, 0.0f, 0.0f }, { .25f, .25f, .75f }, DIFF }, //Right 
		{ 1e5f, { 50.0f, 40.8f, 1e5f }, { 0.0f, 0.0f, 0.0f }, { .75f, .75f, .75f }, DIFF }, //Back 
		{ 1e5f, { 50.0f, 40.8f, -1e5f + 600.0f }, { 0.0f, 0.0f, 0.0f }, { 0.00f, 0.00f, 0.00f }, DIFF }, //Front 
		{ 1e5f, { 50.0f, -1e5f, 81.6f }, { 0.0f, 0.0f, 0.0f }, { .75f, .75f, .75f }, DIFF }, //Bottom 
		{ 1e5f, { 50.0f, -1e5f + 81.6f, 81.6f }, { 0.0f, 0.0f, 0.0f }, { .75f, .75f, .75f }, DIFF }, //Top 
		{ 16.5f, { 27.0f, 16.5f, 47.0f }, { 0.0f, 0.0f, 0.0f }, { 0.99f, 0.99f, 0.99f }, SPEC }, // small sphere 1
		{ 16.5f, { 73.0f, 16.5f, 78.0f }, { 0.0f, 0.f, .0f }, { 0.09f, 0.49f, 0.3f }, REFR }, // small sphere 2
		{ 600.0f, { 50.0f, 681.6f - .5f, 81.6f }, { 3.0f, 2.5f, 2.0f }, { 0.0f, 0.0f, 0.0f }, DIFF }  // Light 12, 10 ,8
#endif

		//Scene: radius, position, emission, color, material
		 Sphere(1e5, Vector3(1e5 + 1,40.8,81.6), Vector3(),Vector3(.75,.25,.25),DIFF),//Left
		 Sphere(1e5, Vector3(-1e5 + 99,40.8,81.6),Vector3(),Vector3(.25,.25,.75),DIFF),//Rght
		 Sphere(1e5, Vector3(50,40.8, 1e5),     Vector3(),Vector3(.75,.75,.75),DIFF),//Back
		 Sphere(1e5, Vector3(50,40.8,-1e5 + 170), Vector3(),Vector3(),           DIFF),//Frnt
		 Sphere(1e5, Vector3(50, 1e5, 81.6),    Vector3(),Vector3(.75,.75,.75),DIFF),//Botm
		 Sphere(1e5, Vector3(50,-1e5 + 81.6,81.6),Vector3(),Vector3(.75,.75,.75),DIFF),//Top
		 Sphere(16.5,Vector3(27,16.5,47),       Vector3(),Vector3(1,1,1)*.999, SPEC),//Mirr
		 Sphere(16.5,Vector3(73,16.5,78),       Vector3(),Vector3(1,1,1)*.999, REFR),//Glas
		 Sphere(600, Vector3(50,681.6 - .27,81.6),Vector3(12,12,12),  Vector3(), DIFF) //Lite

#else
		//outdoor scene: radius, position, emission, color, material

		//{ 1600, { 3000.0f, 10, 6000 }, { 37, 34, 30 }, { 0.f, 0.f, 0.f }, DIFF },  // 37, 34, 30 // sun
		//{ 1560, { 3500.0f, 0, 7000 }, { 50, 25, 2.5 }, { 0.f, 0.f, 0.f }, DIFF },  //  150, 75, 7.5 // sun 2
		{ 10000, { 50.0f, 40.8f, -1060 }, { 0.0003, 0.01, 0.15 }, { 0.175f, 0.175f, 0.25f }, DIFF }, // sky
		{ 100000, { 50.0f, -100000, 0 }, { 0.0, 0.0, 0 }, { 0.8f, 0.2f, 0.f }, DIFF }, // ground
		{ 110000, { 50.0f, -110048.5, 0 }, { 3.6, 2.0, 0.2 }, { 0.f, 0.f, 0.f }, DIFF },  // horizon brightener
		{ 4e4, { 50.0f, -4e4 - 30, -3000 }, { 0, 0, 0 }, { 0.2f, 0.2f, 0.2f }, DIFF }, // mountains
		{ 82.5, { 30.0f, 180.5, 42 }, { 16, 12, 6 }, { .6f, .6f, 0.6f }, DIFF },  // small sphere 1
		{ 12, { 115.0f, 10, 105 }, { 0.0, 0.0, 0.0 }, { 0.9f, 0.9f, 0.9f }, REFR },  // small sphere 2
		{ 22, { 65.0f, 22, 24 }, { 0, 0, 0 }, { 0.9f, 0.9f, 0.9f }, SPEC }, // small sphere 3
#endif
	};

}//namespace


