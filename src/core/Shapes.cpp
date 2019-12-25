#include "shapes.h"

namespace mei
{
	static int64_t nMeshes, nTris;
	static int64_t triMeshBytes;

	TriangleMesh::TriangleMesh(int nTriangles, const int *vertexIndices,
		int nVertices, const Point3f *P, const Normal3f *N)
		: nTriangles(nTriangles),
		nVertices(nVertices),
		vertexIndices(vertexIndices, vertexIndices + 3 * nTriangles)
	{
		++nMeshes;
		nTris += nTriangles;
		triMeshBytes += sizeof(*this) + this->vertexIndices.size() * sizeof(int) +
			nVertices * (sizeof(*P) + (N ? sizeof(*N) : 0));

		// Transform mesh vertices to world space
		p.reset(new Point3f[nVertices]);
		for (int i = 0; i < nVertices; ++i) p[i] = P[i];

		// Copy _UV_, _N_, and _S_ vertex data, if present
		if (N)
		{
			n.reset(new Normal3f[nVertices]);
			for (int i = 0; i < nVertices; ++i) n[i] = N[i];
		}
	}

	/******************************************************************************
	 * Triangle Definition
	******************************************************************************/
	Triangle::Triangle(/*const Transform *ObjectToWorld, const Transform *WorldToObject, bool reverseOrientation,*/
		const std::shared_ptr<TriangleMesh> &mesh, int triNumber)
		: Shape(), mesh(mesh)
	{
		v = &mesh->vertexIndices[3 * triNumber];
		triMeshBytes += sizeof(*this);
	}

	/******************************************************************************
	 * http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
	******************************************************************************/
	bool Triangle::Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture = true) const
	{
		const Point3f &v0 = mesh->p[v[0]];
		const Point3f &v1 = mesh->p[v[1]];
		const Point3f &v2 = mesh->p[v[2]];
		const Vector3f e1 = (v1 - v0);
		const Vector3f e2 = (v2 - v0);

		Vector3f tvec = ray.o - v0;
		Vector3f pvec = Cross(ray.d, e2); // ray.d % _e2;
		Float     det = Dot(e1, pvec); // _e1.dot(pvec);
		if (fabs(det) < 1e-5) { return false; }//< parallel
		//if (det < 1e-5) { return false;  }//< parallel or backface 
		Float invdet = 1.0 / det;

		Float u = Dot(tvec, pvec)* invdet; //tvec.dot(pvec)* invdet;
		if (u < 0.0f || u > 1.0f) { return false; }//< outside triangle

		Vector3f qvec = Cross(tvec, e1);//tvec % _e1;
		Float v = Dot(ray.d,qvec)* invdet;//r.d.dot(qvec)* invdet;
		if (v < 0.0f || (u + v) > 1.0f) { return false; } //< outside triangle

		*tHit = Dot(e2,qvec)* invdet;// _e2.dot(qvec)* invdet;
		return true;
	}

	bool Triangle::IntersectP(const Ray &ray, bool testAlphaTexture = true) const 
	{
		// Get triangle vertices in _p0_, _p1_, and _p2_
		const Point3f &v0 = mesh->p[v[0]];
		const Point3f &v1 = mesh->p[v[1]];
		const Point3f &v2 = mesh->p[v[2]];
		const Vector3f e1 = (v1 - v0);
		const Vector3f e2 = (v2 - v0);

		Vector3f tvec = ray.o - v0;
		Vector3f pvec = Cross(ray.d, e2); // ray.d % _e2;
		Float     det = Dot(e1, pvec); // _e1.dot(pvec);
		if (fabs(det) < 1e-5) { return false; }//< parallel
		//if (det < 1e-5) { return false;  }//< parallel or backface 
		Float invdet = 1.0 / det;

		Float u = Dot(tvec, pvec)* invdet; //tvec.dot(pvec)* invdet;
		if (u < 0.0f || u > 1.0f) { return false; }//< outside triangle

		Vector3f qvec = Cross(tvec, e1);//tvec % _e1;
		Float v = Dot(ray.d,qvec)* invdet;//r.d.dot(qvec)* invdet;
		if (v < 0.0f || (u + v) > 1.0f) { return false; } //< outside triangle

		//return _e2.dot(qvec)* invdet;
		return true;
	}
}

