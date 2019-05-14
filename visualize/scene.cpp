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

	TriangleMesh mesh1;
	TriangleMesh mesh2;
	int total_number_of_triangles;
	Vector3	scene_aabbox_min;
	Vector3	scene_aabbox_max;
	std::vector<Vector3> triangles;

    // read triangle data from obj file
    void loadObj(const std::string filename, TriangleMesh &mesh)
    {
        std::ifstream in(filename.c_str());

        if (!in.good())
        {
            std::cout << "ERROR: loading obj:(" << filename << ") file not found or not good" << "\n";
            system("PAUSE");
            exit(0);
        }

        char buffer[256], str[255];
        Float f1, f2, f3;

        while (!in.getline(buffer, 255).eof())
        {
            buffer[255] = '\0';
            sscanf_s(buffer, "%s", str, 255);

            // reading a vertex
            if (buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))
            {
                if (sscanf(buffer, "v %f %f %f", &f1, &f2, &f3) == 3)
                {
                    mesh.verts.push_back(Vector3(f1, f2, f3));
                }
                else
                {
                    std::cout << "ERROR: vertex not in wanted format in OBJLoader" << "\n";
                    exit(-1);
                }
            }

            // reading faceMtls 
            else if (buffer[0] == 'f' && (buffer[1] == ' ' || buffer[1] == 32))
            {
                TriangleFace f;
                int nt = sscanf(buffer, "f %d %d %d", &f.v[0], &f.v[1], &f.v[2]);
                if (nt != 3)
                {
                    std::cout << "ERROR: I don't know the format of that FaceMtl" << "\n";
                    exit(-1);
                }
                mesh.faces.push_back(f);
            }
        }

        // calculate the bounding box of the mesh
        mesh.bounding_box[0] = Vector3(1000000, 1000000, 1000000);
        mesh.bounding_box[1] = Vector3(-1000000, -1000000, -1000000);
        for (unsigned int i = 0; i < mesh.verts.size(); i++)
        {
            mesh.bounding_box[0] = min(mesh.verts[i], mesh.bounding_box[0]);
            mesh.bounding_box[1] = max(mesh.verts[i], mesh.bounding_box[1]);
        }

        std::cout << "obj file loaded: number of faces:" << mesh.faces.size() << " number of vertices:" << mesh.verts.size() << std::endl;
        std::cout << "obj bounding box: min:(" << mesh.bounding_box[0].x << "," << mesh.bounding_box[0].y << "," << mesh.bounding_box[0].z << ") max:"
            << mesh.bounding_box[1].x << "," << mesh.bounding_box[1].y << "," << mesh.bounding_box[1].z << ")" << std::endl;
    }

    ///http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
    Float intersectTriangle(const Ray &r,
        const Vector3 &v0,
        const Vector3 &edge1,
        const Vector3 &edge2)
    {
        Vector3 tvec = r.o - v0;
        Vector3 pvec = r.d.cross(edge2);
        Float  det = edge1.dot( pvec);
        if (det < 1e-5)
        {

        }

        Float invdet = 1.0 / det;

        Float u = tvec.dot(pvec) * invdet;
        if (u < 0.0f || u > 1.0f)
        {
            return -1.0f;
        }

        Vector3 qvec = tvec % edge1;
        Float v = r.d.dot(qvec) * invdet;
        if (v < 0.0f || (u + v) > 1.0f)
        {
            return -1.0f;
        }

        return edge2.dot(qvec) * invdet;
    }

	void initTriangleScene()
	{
		loadObj("../data/bunny.obj", mesh1);
		loadObj("../data/bunny.obj", mesh2);

		Float scalefactor1 = 200;
		Float scalefactor2 = 300;
		Vector3 offset1 = Vector3(90, 22, 100);// (30, -2, 80);
		Vector3 offset2 = Vector3(30, -2, 80);

		for (unsigned int i = 0; i < mesh1.faces.size(); i++)
		{
			// make a local copy of the triangle vertices
			Vector3 v0 = mesh1.verts[mesh1.faces[i].v[0] - 1];
			Vector3 v1 = mesh1.verts[mesh1.faces[i].v[1] - 1];
			Vector3 v2 = mesh1.verts[mesh1.faces[i].v[2] - 1];

			// scale
			v0 *= scalefactor1;
			v1 *= scalefactor1;
			v2 *= scalefactor1;

			// translate
			v0 += offset1;
			v1 += offset1;
			v2 += offset1;

			triangles.push_back(Vector3(v0.x, v0.y, v0.z));
			triangles.push_back(Vector3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z));
			triangles.push_back(Vector3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z));
		}

		// compute bounding box of this mesh
		mesh1.bounding_box[0] *= scalefactor1; mesh1.bounding_box[0] += offset1;
		mesh1.bounding_box[1] *= scalefactor1; mesh1.bounding_box[1] += offset1;

		for (unsigned int i = 0; i < mesh2.faces.size(); i++)
		{
			Vector3 v0 = mesh2.verts[mesh2.faces[i].v[0] - 1];
			Vector3 v1 = mesh2.verts[mesh2.faces[i].v[1] - 1];
			Vector3 v2 = mesh2.verts[mesh2.faces[i].v[2] - 1];

			v0 *= scalefactor2;
			v1 *= scalefactor2;
			v2 *= scalefactor2;

			v0 += offset2;
			v1 += offset2;
			v2 += offset2;

			triangles.push_back(Vector3(v0.x, v0.y, v0.z));
			triangles.push_back(Vector3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z));
			triangles.push_back(Vector3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z));
		}

		mesh2.bounding_box[0] *= scalefactor2; mesh2.bounding_box[0] += offset2;
		mesh2.bounding_box[1] *= scalefactor2; mesh2.bounding_box[1] += offset2;

		total_number_of_triangles = triangles.size() / 3;

		std::cout << "total number of triangles check:" << mesh1.faces.size() + mesh2.faces.size() << " == " << total_number_of_triangles << std::endl;

		// compute scene bounding box by merging bounding boxes of individual meshes 
		scene_aabbox_min = mesh2.bounding_box[0];
		scene_aabbox_max = mesh2.bounding_box[1];
		scene_aabbox_min = min(scene_aabbox_min, mesh1.bounding_box[0]);
		scene_aabbox_max = max(scene_aabbox_max, mesh1.bounding_box[1]);

	}

	void intersectAllTriangles(const Ray& r, Float& t_scene, int& triangle_id, const int number_of_triangles, int& geomtype)
	{
		for (int i = 0; i < number_of_triangles; i++)
		{
			Vector3 v0 = triangles[i * 3];
			Vector3 e1 = triangles[i * 3 + 1];
			Vector3 e2 = triangles[i * 3 + 2];

			Float t = intersectTriangle(r, v0, e1, e2);

			if (t < t_scene && t > 1e-5)
			{
				t_scene = t;
				triangle_id = i;
				geomtype = 3;
			}
		}
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

	bool intersect_scene(const Ray &r, Float &t, int &sphere_id, int &box_id, int& triangle_id, const int number_of_triangles, int &geomtype, const Vector3 &bbmin, const Vector3 &bbmax)
	{

		Float tmin = 1e20;
		Float tmax = -1e20;
		Float d = 1e21;
		Float k = 1e21;
		Float q = 1e21;
		Float inf = t = 1e20;

		Float numspheres = sizeof(spheres) / sizeof(Sphere);
		for (int i = int(numspheres); i--;)
			if ((d = spheres[i].intersect(r)) && d < t) { t = d; sphere_id = i; geomtype = 1; }

#if 0
		Float numboxes = sizeof(boxes) / sizeof(Box);
		for (int i = int(numboxes); i--;)
			if ((k = boxes[i].intersect(r)) && k < t) { t = k; box_id = i; geomtype = 2; }

		Box scene_bbox; 
		scene_bbox.min = bbmin;
		scene_bbox.max = bbmax;

		if (scene_bbox.intersect(r))
		{
			intersectAllTriangles(r, t, triangle_id, number_of_triangles, geomtype);
		}
#endif
		return t < inf;
	}

	Vector3 getTriangleNormal(const int triangleIndex)
	{
		Vector3 edge1 = triangles[triangleIndex * 3 + 1];
		Vector3 edge2 = triangles[triangleIndex * 3 + 2];
		return (edge1 % edge2).norm();
	}

	Vector3 radiance(Ray &r, const int totaltris, unsigned short *Xi, const Vector3& scene_aabb_min, const Vector3& scene_aabb_max)
	{
		Vector3 mask(1.0f, 1.0f, 1.0f);
		Vector3 accucolor(0.0f, 0.0f, 0.0f);

		for (int bounces = 0; bounces < 5; bounces++)
		{ 
			Float t = 1e6; // distance to intersection 
			int sphere_id = -1;
			int box_id = -1;   // index of intersected sphere 
			int triangle_id = -1;
			int geomtype = -1;
			Vector3 f;  // primitive colour
			Vector3 emit; // primitive emission colour
			Vector3 x; // intersection point
			Vector3 n; // normal
			Vector3 nl; // oriented normal
			Vector3 d; // ray direction of next path segment
			Refl_t refltype;

			// intersect ray with scene
			// intersect_scene keeps track of closest intersected primitive and distance to closest intersection point
			if (!intersect_scene(r, t, sphere_id, box_id, triangle_id, totaltris, geomtype, scene_aabb_min, scene_aabb_max))
				return Vector3(0.0f, 0.0f, 0.0f);

			// if sphere:
			if (geomtype == 1)
			{
				Sphere &sphere = spheres[sphere_id]; // hit object with closest intersection
				x = r.o+ r.d*t;  // intersection point on object
				n = normalize(x - sphere.p);		// normal
				nl = n.dot(r.d) < 0 ? n : n * -1; // correctly oriented normal
				f = sphere.c;   // object colour
				refltype = sphere.refl;
				emit = sphere.e;  // object emission
				accucolor += (mask * emit);
			}

			// if box:
			if (geomtype == 2)
			{
				Box &box = boxes[box_id];
				x = r.o+ r.d*t;  // intersection point on object
				n = normalize(box.normalAt(x)); // normal
				nl = n.dot(r.d) < 0 ? n : n * -1; // correctly oriented normal
				f = box.col;  // box colour
				refltype = box.refl;
				emit = box.emi; // box emission
				accucolor += (mask * emit);
			}

			// if triangle:
			if (geomtype == 3)
			{
				int tri_index = triangle_id;
				x = r.o+ r.d*t;  // intersection point on object
				n = normalize(getTriangleNormal(tri_index));  // normal 
				nl = n.dot(r.d) < 0 ? n : n * -1; // correctly oriented normal

				// colour, refltype and emit value are hardcoded and apply to all triangles
				// no per triangle material support yet
				f = Vector3(0.9f, 0.4f, 0.1f);  // triangle colour
				emit = Vector3(0.0f, 0.0f, 0.0f);
				refltype = REFR;
				accucolor += (mask * emit);
			}

			// ideal diffuse reflection (see "Realistic Ray Tracing", P. Shirley)
			if (refltype == DIFF)
			{
				Float r1 = 2 * M_PI*erand48(Xi),
					r2 = erand48(Xi),
					r2s = sqrt(r2);
				Vector3 w = nl,
					u = ((fabs(w.x) > .1 ? Vector3(0, 1) : Vector3(1)) % w).norm(),
					v = w % u;
				Vector3 d = (u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2)).norm();

				// multiply mask with colour of object
				mask = mask.mult(f);
			}

			// ideal specular reflection (mirror) 
			if (refltype == SPEC)
			{
				d = r.d- 2.0f * n * n.dot(r.d);
				mask = mask.mult(f);
			}

			// ideal refraction (based on smallpt code by Kevin Beason)
			if (refltype == REFR)
			{

				bool into = n.dot(nl) > 0; // is ray entering or leaving refractive material?
				Float nc = 1.0f;  // Index of Refraction air
				Float nt = 1.5f;  // Index of Refraction glass/water
				Float nnt = into ? nc / nt : nt / nc;  // IOR ratio of refractive materials
				Float ddn = r.d.dot(nl);
				Float cos2t = 1.0f - nnt * nnt * (1.f - ddn * ddn);

				if (cos2t < 0.0f) // total internal reflection 
				{
					d = r.d- 2.0f * n * n.dot(r.d);
				}
				else // cos2t > 0
				{
					// compute direction of transmission ray
					Vector3 tdir = normalize(r.d* nnt - n * ((into ? 1 : -1) * (ddn*nnt + sqrtf(cos2t))));

					Float R0 = (nt - nc)*(nt - nc) / (nt + nc)*(nt + nc);
					Float c = 1.f - (into ? -ddn : tdir.dot(n));
					Float Re = R0 + (1.f - R0) * c * c * c * c * c;
					Float Tr = 1 - Re; // Transmission
					Float P = .25f + .5f * Re;
					Float RP = Re / P;
					Float TP = Tr / (1.f - P);

					// randomly choose reflection or transmission ray
					if (erand48(Xi) < 0.25) // reflection ray
					{
						mask *= RP;
						d = r.d- 2.0f * n * n.dot(r.d);
					}
					else // transmission ray
					{
						mask *= TP;
						d = tdir; //r = Ray(x, tdir); 
					}
				}
			}

			// Next pass
			r.o = x;
			r.d = d;
		}

		// add radiance up to a certain ray depth
		// return accumulated ray colour after all bounces are computed
		return accucolor;
	}

}//namespace


