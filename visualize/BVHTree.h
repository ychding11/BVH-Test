#ifndef BVH_h
#define BVH_h

#include "smallpt.h"

namespace smallpt
{

	struct BVHNode
	{
		AABB box;
		int data1; // node: left index; leaf: start triangle index
		int data2; // node: right index; leaf: triangle count
		bool leaf;
	};

	class BVHTree
	{

		// Scene information: a copy of the input triangles
		int s_TriangleCount;
		Triangle* s_Triangles;
		int* s_TriIndices;

		std::vector<BVHNode> s_BVH;

		uint32_t XorShift32(uint32_t& state)
		{
			uint32_t x = state;
			x ^= x << 13;
			x ^= x >> 17;
			x ^= x << 15;
			state = x;
			return x;
		}

	public:

		void cleanup()
		{
			delete[] s_Triangles;
			delete[] s_TriIndices;
			s_BVH.clear();
		}
	private:
		int CreateBVH(int triStart, int triCount, uint32_t& rngState);

		int HitBVH(int index, const Ray& r, float tMin, float tMax, IntersectionInfo& outHit)
		{
			const BVHNode& node = s_BVH[index];
			//if (!node.box.hit(r))
			//if (!node.box. (invR, tMin, tMax))
				return -1;

			// if leaf node, check against triangles
			if (node.leaf)
			{
				int hitID = -1;
				for (int i = 0; i < node.data2; ++i)
				{
					int triIndex = s_TriIndices[node.data1 + i];
					assert(triIndex >= 0 && triIndex < s_TriangleCount);
					//if (s_Triangles[triIndex].HitTriangle(r, tMin, tMax, outHit))
					{
						hitID = triIndex;
						tMax = outHit.t;
					}
				}
				return hitID;
			}

			int leftId = HitBVH(node.data1, r, tMin, tMax, outHit);
			if (leftId != -1)
			{
				// left was hit: only check right hit up until left hit distance
				int rightId = HitBVH(node.data2, r, tMin, outHit.t, outHit);
				if (rightId != -1) return rightId;
				return leftId;
			}
			// left was not hit: check right
			int rightId = HitBVH(node.data2, r, tMin, tMax, outHit);
			return rightId;
		}
		
	};
};

#endif
