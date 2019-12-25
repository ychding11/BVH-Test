#ifndef INTERACTIONS_H_ 
#define INTERACTIONS_H_ 

#include "2d.h"
#include "3d.h"
#include "primitives.h"
#include "shapes.h"

namespace mei
{
	// Interaction Declarations
	struct Interaction
	{
		// Interaction Public Methods
		Interaction() : time(0) {}
		Interaction(const Point3f &p, const Normal3f &n, const Vector3f &pError, const Vector3f &wo, Float time)
			: p(p),
			time(time),
			pError(pError),
			wo(Normalize(wo)),
			n(n)
			{}

		bool IsSurfaceInteraction() const { return n != Normal3f(); }

		Ray SpawnRay(const Vector3f &d) const
		{
			Point3f o = p; // OffsetRayOrigin(p, pError, n, d);
			return Ray(o, d);
		}

		Ray SpawnRayTo(const Point3f &p2) const
		{
			Point3f origin = p;// OffsetRayOrigin(p, pError, n, p2 - p);
			Vector3f d = p2 - p;
			return Ray(origin, d);
		}

		Ray SpawnRayTo(const Interaction &it) const
		{
			Point3f origin = p;// OffsetRayOrigin(p, pError, n, it.p - p);
			Point3f target = it.p;// OffsetRayOrigin(it.p, it.pError, it.n, origin - it.p);
			Vector3f d = target - origin;
			return Ray(origin, d);
		}

		Interaction(const Point3f &p, const Vector3f &wo, Float time)
			: p(p), time(time), wo(wo){}

		Interaction(const Point3f &p, Float time)
			: p(p), time(time){}

		bool IsMediumInteraction() const { return !IsSurfaceInteraction(); }

		// Interaction Public Data
		Point3f p;
		Float time;
		Vector3f pError;
		Vector3f wo;
		Normal3f n;
	};

	// SurfaceInteraction Declarations
	class SurfaceInteraction : public Interaction
	{
	public:
		// SurfaceInteraction Public Methods
		SurfaceInteraction() {}

		// SurfaceInteraction Public Data
		const Shape *shape = nullptr;
		struct {
			Normal3f n;
			Vector3f dpdu, dpdv;
			Normal3f dndu, dndv;
		} shading;
		const Primitive *primitive = nullptr;
	};

}

#endif
