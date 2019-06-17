#include "BVHTree.h"

namespace smallpt
{

	int BVHTree::CreateBVH(int triStart, int triCount, uint32_t& rngState)
	{
		// sort input triangles by a randomly chosen axis
		int axis = XorShift32(rngState) % 3;
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
			node.data1 = CreateBVH(triStart, triCount / 2, rngState);
			node.data2 = CreateBVH(triStart + triCount / 2, triCount - triCount / 2, rngState);
			node.leaf = false;
			assert(node.data1 >= 0 && node.data1 < s_BVH.size());
			assert(node.data2 >= 0 && node.data2 < s_BVH.size());
			node.box = s_BVH[node.data1].box.Union(s_BVH[node.data2].box);
		}
		s_BVH[nodeIndex] = node;
		return nodeIndex;
	}
};
