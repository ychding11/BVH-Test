
#include "primitives.h"

namespace mei
{

    static uint64_t primitiveMemory;

    // Primitive Method Definitions
    Primitive::~Primitive() {}

    // GeometricPrimitive Method Definitions
    GeometricPrimitive::GeometricPrimitive(const std::shared_ptr<Shape> &shape)
        : shape(shape)
    {
        primitiveMemory += sizeof(*this);
    }

    // GeometricPrimitive Public Methods
    Bounds3f GeometricPrimitive::WorldBound() const
    {
        return shape->WorldBound();
    }

    bool GeometricPrimitive::Intersect(const Ray &r, SurfaceInteraction *isect) const
    {
        Float tHit;
        if (!shape->Intersect(r, &tHit, isect)) return false;
        r.tMax = tHit;
        isect->primitive = this;
        CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
        // Initialize _SurfaceInteraction::mediumInterface_ after _Shape_
        // intersection
        return true;
    }
    bool GeometricPrimitive::IntersectP(const Ray &r) const
    {
        return shape->IntersectP(r);
    }
}

