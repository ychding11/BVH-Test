#ifndef SHAPES_H_ 
#define SHAPES_H_ 

#include "2d.h"
#include "3d.h"
#include "interactions.h"

namespace mei
{
	// Shape Declarations
	class Shape {
	public:
		// Shape Interface
		Shape()
		{ }
		virtual ~Shape()
		{ }

		virtual Bounds3f WorldBound() const
		{
			return ObjectBound();
		}

		virtual Bounds3f ObjectBound() const = 0;
		virtual bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture = true) const = 0;

		virtual bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const
		{
			return Intersect(ray, nullptr, nullptr, testAlphaTexture);
		}
		virtual Float Area() const = 0;

#if 0
		// Sample a point on the surface of the shape and return the PDF with
		// respect to area on the surface.
		virtual Interaction Sample(const Point2f &u, Float *pdf) const = 0;
		virtual Float Pdf(const Interaction &) const { return 1 / Area(); }

		// Sample a point on the shape given a reference point |ref| and
		// return the PDF with respect to solid angle from |ref|.
		virtual Interaction Sample(const Interaction &ref, const Point2f &u, Float *pdf) const;
		virtual Float Pdf(const Interaction &ref, const Vector3f &wi) const;

		// Returns the solid angle subtended by the shape w.r.t. the reference
		// point p, given in world space. Some shapes compute this value in
		// closed-form, while the default implementation uses Monte Carlo
		// integration; the nSamples parameter determines how many samples are
		// used in this case.
		virtual Float SolidAngle(const Point3f &p, int nSamples = 512) const;
#endif
	};



	// Triangle Declarations
	struct TriangleMesh {
		// TriangleMesh Public Methods
		TriangleMesh(int nTriangles, const int *vertexIndices,
			int nVertices, const Point3f *P, const Normal3f *N );

		// TriangleMesh Data
		const int nTriangles, nVertices;
		std::vector<int> vertexIndices;
		std::unique_ptr<Point3f[]> p;
		std::unique_ptr<Normal3f[]> n;
#if 0
		std::unique_ptr<Vector3f[]> s;
		std::unique_ptr<Point2f[]> uv;
		std::shared_ptr<Texture<Float>> alphaMask, shadowAlphaMask;
		std::vector<int> faceIndices;
#endif
	}; 

	class Triangle :public Shape 
	{
	private:
		Vector3 _v0, _v1, _v2, _e1, _e2;

		// Triangle Private Data
		std::shared_ptr<TriangleMesh> mesh;
		const int *v;

	public:
		// Triangle Public Methods
		Triangle(/*const Transform *ObjectToWorld, const Transform *WorldToObject, bool reverseOrientation,*/
			const std::shared_ptr<TriangleMesh> &mesh, int triNumber);
		
		virtual Bounds3f ObjectBound() const  override
		{
			const Point3f &p0 = mesh->p[v[0]];
			const Point3f &p1 = mesh->p[v[1]];
			const Point3f &p2 = mesh->p[v[2]];
			return Union(Bounds3f(p0, p1), p2);
		}
		virtual bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect, bool testAlphaTexture = true) const override;

		virtual bool IntersectP(const Ray &ray, bool testAlphaTexture = true) const override;
		
		virtual Float Area() const override
		{
			// Get triangle vertices in _p0_, _p1_, and _p2_
			const Point3f &p0 = mesh->p[v[0]];
			const Point3f &p1 = mesh->p[v[1]];
			const Point3f &p2 = mesh->p[v[2]];
			return 0.5 * Cross(p1 - p0, p2 - p0).Length();
		}
	};
}


#endif
