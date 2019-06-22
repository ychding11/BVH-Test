#include "BVHTree.h"

namespace smallpt
{

    const int cTrianglesInLeaf = 12;

	int BVHTree::CreateBVH(int triStart, int triCount)
	{
		// sort input triangles by a randomly chosen axis
		int axis = XorShift32(_randSeed) % 3;
		if (axis == 0)
			std::sort(_triangleIndices + triStart, _triangleIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < _triangleCount);
				assert(b >= 0 && b < _triangleCount);
				AABB boxa = _triangles[a].getBBox();
				AABB boxb = _triangles[b].getBBox();
				return boxa.bmin.x < boxb.bmin.x;
			});
		else if (axis == 1)
			std::sort(_triangleIndices + triStart, _triangleIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < _triangleCount);
				assert(b >= 0 && b < _triangleCount);
				AABB boxa = _triangles[a].getBBox();
				AABB boxb = _triangles[b].getBBox();
				return boxa.bmin.y < boxb.bmin.y;
			});
		else if (axis == 2)
			std::sort(_triangleIndices + triStart, _triangleIndices + triStart + triCount, [this](int a, int b)
			{
				assert(a >= 0 && a < _triangleCount);
				assert(b >= 0 && b < _triangleCount);
				AABB boxa = _triangles[a].getBBox();
				AABB boxb = _triangles[b].getBBox();
				return boxa.bmin.z < boxb.bmin.z;
			});

		// create the node
		BVHNode node;
		int nodeIndex = (int)_nodes.size();
		_nodes.push_back(node);

        //! Leaf
		if (triCount <= cTrianglesInLeaf)
		{
			node.data1 = triStart;
			node.data2 = triCount;
			node.leaf = true;
			node.box = _triangles[_triangleIndices[triStart]].getBBox();
			for (int i = 1; i < triCount; ++i)
			{
				auto tribox = _triangles[_triangleIndices[triStart + i]].getBBox();
				node.box = node.box.Union(tribox);
			}
		}
		else
		{
			node.data1 = CreateBVH(triStart, triCount / 2);
			node.data2 = CreateBVH(triStart + triCount / 2, triCount - triCount / 2);
			node.leaf = false;
			assert(node.data1 >= 0 && node.data1 < _nodes.size());
			assert(node.data2 >= 0 && node.data2 < _nodes.size());
			node.box = _nodes[node.data1].box.Union(_nodes[node.data2].box);
		}
		_nodes[nodeIndex] = node;
		return nodeIndex;
	}

	int BVHTree::HitBVH(int index, const Ray& r, float tMax, IntersectionInfo* outHit) const
	{
		const BVHNode& node = _nodes[index];
		if (!node.box.hit(r))
			return -1;

		// if leaf node, check against triangles
		if (node.leaf)
		{
			int hitID = -1;
			for (int i = 0; i < node.data2; ++i)
			{
				int triIndex = _triangleIndices[node.data1 + i];
				assert(triIndex >= 0 && triIndex < _triangleCount);
				if (_triangles[triIndex].getIntersection(r, outHit) && outHit->t < tMax)
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
