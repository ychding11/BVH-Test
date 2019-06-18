#include "BVHTree.h"

namespace smallpt
{

	int BVHTree::CreateBVH(int triStart, int triCount)
	{
		// sort input triangles by a randomly chosen axis
		int axis = XorShift32(_randSeed) % 3;
		if (axis == 0)
			std::sort(s_TriIndices + triStart, s_TriIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < s_TriangleCount);
				assert(b >= 0 && b < s_TriangleCount);
				AABB boxa = s_Triangles[a].getBBox();
				AABB boxb = s_Triangles[b].getBBox();
				return boxa.bmin.x < boxb.bmin.x;
			});
		else if (axis == 1)
			std::sort(s_TriIndices + triStart, s_TriIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < s_TriangleCount);
				assert(b >= 0 && b < s_TriangleCount);
				AABB boxa = s_Triangles[a].getBBox();
				AABB boxb = s_Triangles[b].getBBox();
				return boxa.bmin.y < boxb.bmin.y;
			});
		else if (axis == 2)
			std::sort(s_TriIndices + triStart, s_TriIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < s_TriangleCount);
				assert(b >= 0 && b < s_TriangleCount);
				AABB boxa = s_Triangles[a].getBBox();
				AABB boxb = s_Triangles[b].getBBox();
				return boxa.bmin.z < boxb.bmin.z;
			});

		// create the node
		BVHNode node;
		int nodeIndex = (int)s_BVH.size();
		s_BVH.push_back(node);

		// if we have less than N triangles, make this node a leaf that just has all of them
		if (triCount <= 4)
		{
			node.data1 = triStart;
			node.data2 = triCount;
			node.leaf = true;
			node.box = s_Triangles[s_TriIndices[triStart]].getBBox();
			for (int i = 1; i < triCount; ++i)
			{
				auto tribox = s_Triangles[s_TriIndices[triStart + i]].getBBox();
				node.box = node.box.Union(tribox);
			}
		}
		else
		{
			node.data1 = CreateBVH(triStart, triCount / 2);
			node.data2 = CreateBVH(triStart + triCount / 2, triCount - triCount / 2);
			node.leaf = false;
			assert(node.data1 >= 0 && node.data1 < s_BVH.size());
			assert(node.data2 >= 0 && node.data2 < s_BVH.size());
			node.box = s_BVH[node.data1].box.Union(s_BVH[node.data2].box);
		}
		s_BVH[nodeIndex] = node;
		return nodeIndex;
	}

	int BVHTree::HitBVH(int index, const Ray& r, float tMax, IntersectionInfo* outHit)
	{
		const BVHNode& node = s_BVH[index];
		if (!node.box.hit(r))
			return -1;

		// if leaf node, check against triangles
		if (node.leaf)
		{
			int hitID = -1;
			for (int i = 0; i < node.data2; ++i)
			{
				int triIndex = s_TriIndices[node.data1 + i];
				assert(triIndex >= 0 && triIndex < s_TriangleCount);
				if (s_Triangles[triIndex].getIntersection(r, outHit) && outHit->t < tMax)
				{
					hitID = triIndex;
					tMax = outHit->t;
				}
			}
			return hitID;
		}

		int leftId = HitBVH(node.data1, r, tMax, outHit);
		if (leftId != -1)
		{
			// left was hit: only check right hit up until left hit distance
			int rightId = HitBVH(node.data2, r, outHit->t, outHit);
			if (rightId != -1) return rightId;
			return leftId;
		}
		// left was not hit: check right
		int rightId = HitBVH(node.data2, r, tMax, outHit);
		return rightId;
	}
}
