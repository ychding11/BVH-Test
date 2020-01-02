//#include <math.h>  
//#include <stdlib.h>
//#include <stdio.h>  
//#include <omp.h>

#include "scene.h"

namespace mei
{

	static int64_t nIntersectionTests; 
	static int64_t nShadowTests;

	// Scene Method Definitions
	bool Scene::Intersect(const Ray &ray, SurfaceInteraction *isect) const
	{
		++nIntersectionTests;
		DCHECK_NE(ray.d, Vector3f(0, 0, 0));
		return aggregate->Intersect(ray, isect);
	}

	bool Scene::IntersectP(const Ray &ray) const
	{
		++nShadowTests;
		DCHECK_NE(ray.d, Vector3f(0, 0, 0));
		return aggregate->IntersectP(ray);
    }


#if 0
	static Sphere spheres[] =
	{
		//Scene: radius, position, emission, color, material
		 Sphere(1e5, Vector3(1e5 + 1,40.8,81.6), Vector3(),Vector3(.75,.25,.25),DIFF),//Left
		 Sphere(1e5, Vector3(-1e5 + 99,40.8,81.6),Vector3(),Vector3(.25,.25,.75),DIFF),//Rght
		 Sphere(1e5, Vector3(50,40.8, 1e5),     Vector3(),Vector3(.75,.75,.75),DIFF),//Back
		 Sphere(1e5, Vector3(50,40.8,-1e5 + 170), Vector3(),Vector3(),           DIFF),//Frnt
		 Sphere(1e5, Vector3(50, 1e5, 81.6),    Vector3(),Vector3(.75,.75,.75),DIFF),//Botm
		 Sphere(1e5, Vector3(50,-1e5 + 81.6,81.6),Vector3(),Vector3(.75,.75,.75),DIFF),//Top
		 Sphere(16.5,Vector3(27,16.5,47),       Vector3(),Vector3(1,1,1)*.999, SPEC),//Mirr
		 //Sphere(16.5,Vector3(40,16.5,58),       Vector3(),Vector3(1,1,1)*.999, SPEC),//Place holder
		 Sphere(16.5,Vector3(73,16.5,78),       Vector3(),Vector3(1,1,1)*.999, REFR),//Glas
		 Sphere(600, Vector3(50,681.6 - .27,81.6),Vector3(12,12,12),  Vector3(), DIFF) //Lite
	};



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
        
		std::ostringstream ss;
		ss << "load obj:" << _filepath;
		ss << "\n\tfaces:" << _mesh.faces.size() << " vertices:" << _mesh.vertex.size() << std::endl;
    }

	void TriangleScene::initTriangleScene()
	{
#if 0
        ObjParser objparser;
#else
        _sceneName = "../data/bunny.obj";
        ObjParser objparser(_sceneName);
#endif
        TriangleMesh& mesh = objparser.getTriangleMesh();
		for (unsigned int i = 0; i < mesh.faces.size(); i++)
		{
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

		sceneSize   = _aabbMax - _aabbMin;
		sceneCenter = (_aabbMin + _aabbMax) * 0.5;

		Vector3 extra = sceneSize * 0.7f;
		{
		    Vector3 v0 = Vector3(_aabbMin.x - extra.x, _aabbMin.y, _aabbMin.z - extra.z);
		    Vector3 v1 = Vector3(_aabbMin.x - extra.x, _aabbMin.y, _aabbMax.z + extra.z);
		    Vector3 v2 = Vector3(_aabbMax.x + extra.x, _aabbMin.y, _aabbMin.z - extra.z);
            _triangles.push_back(Triangle(v0, v1, v2));
		}

		{
		    Vector3 v0 = Vector3(_aabbMin.x - extra.x, _aabbMin.y, _aabbMax.z + extra.z);
		    Vector3 v1 = Vector3(_aabbMax.x + extra.x, _aabbMin.y, _aabbMax.z + extra.z);
		    Vector3 v2 = Vector3(_aabbMax.x + extra.x, _aabbMin.y, _aabbMin.z - extra.z);
            _triangles.push_back(Triangle(v0, v1, v2));
		}

		lookfrom = sceneCenter + 0.5 * (sceneSize.y)*(Vector3(0., 0., 6.));
		lookat   = sceneCenter;
        _numTriangles = _triangles.size();

		std::ostringstream ss;
		ss << "scene Name:" << _sceneName << std::endl;
		ss << "\n\tnumber of triangles:" << _numTriangles << std::endl;
		ss << "\n\tmin:    " << _aabbMin.str() << "\t max: " << _aabbMax.str() << std::endl;
		ss << "\n\tlookat: " << lookat.str() << "\t lookfrom: " << lookfrom.str() << std::endl;
	}

	bool TriangleScene::intersecTri(const Ray& r, IntersectionInfo &hit) const
	{
        int id = -1;
        Float t = inf;
		for (int i = 0; i < _numTriangles; i++)
		{
			const Triangle& triangle = _triangles[i];
            Float ct = triangle.intersect(r);
            {
			    if (ct < t && ct > eps && ct < inf)
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

#endif
	

#if 0
	Vector3 Scene::myradiance(const Ray &r, int depth, unsigned short *Xi)
	{
		IntersectionInfo hitInfo;
		if (!intersec(r, hitInfo)) return Vector3(); // if miss, return black
#if 1
		if (dynamic_cast<const Triangle*>(hitInfo.object))
		{
			Vector3 c = hitInfo.object->c;
		    Vector3 n = hitInfo.object->getNormal(hitInfo);
			Float alph = std::fabs(n.dot(Vector3(1,1,0)));
			return c * alph;
		}
#endif
		const Object &obj = *hitInfo.object;        // the hit object
		Vector3 x = hitInfo.hit, n = hitInfo.object->getNormal(hitInfo);
		Vector3 nl = n.dot(r.d) < 0 ? n : n * -1;
		Vector3 f = obj.c;
		const Float p = .1; //< RR stop Pr

		if (++depth > 4 && depth <= 6)
		{
            if (erand48(Xi) < p) return obj.e;
			else f = f * (1 /(1.-p));
		}
		else if (depth > 6)
		{
			return obj.e;
		}

		if (obj.refl == DIFF) // Ideal DIFFUSE reflection
		{
            // Why converge slow with spp goes up ?
            // So how to let diffuse converges fast ?
			return obj.e + f.cmult(myradiance(Ray(x, cosWeightedSample(nl, Xi)), depth, Xi));
		}
		else if (obj.refl == SPEC) // Ideal SPECULAR reflection
		{
			return obj.e + f.cmult(myradiance(Ray(x, reflect(r.d, n)), depth, Xi));
		}
		else // Ideal dielectric REFRACTION
		{
            //! Maybe write a single function to do this things.
            //! Input: IOR1, IOR2, incomming direction(source->shading point), normal
			Ray reflRay(x, reflect(r.d, n));
			bool into = n.dot(nl) > 0;     // Ray from outside going in
			Float nc = 1.;   // Air
			Float nt = _ior;  // IOR  Glass
			Float nnt = into ? nc / nt : nt / nc,
				  ddn = r.d.dot(nl),
				  cos2t;
			if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) < 0)    // Total internal reflection
			{
				return obj.e + f.cmult(myradiance(reflRay, depth, Xi));
			}
			Vector3 tdir = (r.d*nnt - n * ((into ? 1 : -1) * (ddn*nnt + sqrt(cos2t)))).norm();

            Float Re = SchlickApproxim(nt, nc, into ? -ddn : tdir.dot(n)); //! Schlick's approximation
            Float Tr = 1 - Re;

            // Russian roulette weight
            Float P = .25 + .5 * Re,
				  RP = Re / P,
				  TP = Tr / (1 - P);
			return obj.e + f.cmult(depth > 2 ? (erand48(Xi) < P ?   // Russian roulette
				myradiance(reflRay, depth, Xi)*RP : myradiance(Ray(x, tdir), depth, Xi)*TP) :
				myradiance(reflRay, depth, Xi)*Re + myradiance(Ray(x, tdir), depth, Xi)*Tr);
		}
	}
#endif

}//namespace


