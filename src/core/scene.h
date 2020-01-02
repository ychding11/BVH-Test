#ifndef SCENES_H_
#define SCENES_H_


//#include "objparser.h"
#include "Primitives.h"

extern double erand48(unsigned short xseed[3]);

namespace mei
{
    class Scene
    {
    public:
        // Scene Public Methods
        Scene(std::shared_ptr<Primitive> aggregate /*, const std::vector<std::shared_ptr<Light>> &lights*/)
            : aggregate(aggregate)
        {
            // Scene Constructor Implementation
            worldBound = aggregate->WorldBound();
        }

	public:
		const Bounds3f &WorldBound() const { return worldBound; }
		bool Intersect(const Ray &ray, SurfaceInteraction *isect) const;
		bool IntersectP(const Ray &ray) const;
	private:
		// Scene Private Data
		std::shared_ptr<Primitive> aggregate;
		Bounds3f worldBound;
    };

}//namespace

#endif


