#ifndef BVH_TREE_H
#define BVH_TREE_H 

#include "Primitives.h"

namespace smallpt
{
	struct BVHNode
	{
		int data1; // node: left index; leaf: start triangle index
		int data2; // node: right index; leaf: triangle count
		bool leaf;
		AABB box;
	};

    class BVHTree
    {
    private:
        int _triangleCount;
		//std::unique_ptr<Triangle> _triangles;
        Triangle* _triangles;
        int* _triangleIndices;

        std::vector<BVHNode> _nodes;
        uint32_t _randSeed;

        uint32_t XorShift32(uint32_t& state)
        {
            uint32_t x = state;
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 15;
            state = x;
            return x;
        }

        void cleanup()
        {
            //delete[] _triangles;
            delete[] _triangleIndices;
            _nodes.clear();
        }

    public:

        BVHTree() :_randSeed(0x1234) {}
        ~BVHTree() { cleanup(); }

        void InitTree(const std::vector<Triangle> &triangles)
        {
            _triangleCount = triangles.size();
            _triangles = new Triangle[_triangleCount];
            memcpy(_triangles, &triangles[0], _triangleCount * sizeof(triangles[0]));

            _triangleIndices = new int[_triangleCount];
            for (int i = 0; i < _triangleCount; ++i)
                _triangleIndices[i] = i;
            CreateBVH(0, _triangleCount);
            LOG_INFO("Create BVH done.");
        }

    public:
        bool intersec(const Ray& r, IntersectionInfo *hit) const
        {
            int i = HitBVH(0, r, inf, hit);
            if (i != -1)
            {
                hit->hit = r.o + hit->t * r.d;
                return true;
            }
            return false;
        }

    private:
        int CreateBVH(int triStart, int triCount);

        int HitBVH(int index, const Ray& r, float tMax, IntersectionInfo* outHit) const;
    };


}

#endif
