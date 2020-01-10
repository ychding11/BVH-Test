
#include "primitives.h"
#include "interactions.h"
#include "shapes.h"

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

    ListAggregate::ListAggregate(std::vector<std::shared_ptr<Primitive>> p)
        : primitives(std::move(p))
    {
    }

    ListAggregate::~ListAggregate(){}

    Bounds3f ListAggregate::WorldBound() const
    {

    }

    bool ListAggregate::Intersect(const Ray &ray, SurfaceInteraction *isect) const
    {

    }
    bool ListAggregate::IntersectP(const Ray &ray) const
    {

    }

    std::shared_ptr<ListAggregate> CreateListAggregate(std::vector<std::shared_ptr<Primitive>> prims)
    {
        return std::shared_ptr<ListAggregate>{new ListAggregate(prims)};
    }
} //namespace

