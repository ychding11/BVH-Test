#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mei.h"
#include "shapes.h"


using namespace mei;

class ModelImporter
{
public:

	ModelImporter()
	{
		flags = aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_PreTransformVertices |
			aiProcess_RemoveRedundantMaterials |
			aiProcess_OptimizeMeshes |
			aiProcess_FlipUVs |
			aiProcess_FlipWindingOrder;

	}

	bool LoadModel(std::string filename)
	{
		const aiScene* scene = importer.ReadFile(filename, flags);

		if (scene == nullptr)
		{
			LOG(ERROR) << std::string("Failed to load model " + filename + ": " + std::string(importer.GetErrorString()));
			return false;
		}

		if (scene->mNumMeshes == 0)
		{
			LOG(ERROR) << std::string("Model " + filename + " has no meshes");
			return false;
		}

		if (scene->mNumMaterials == 0)
		{
			LOG(ERROR) << std::string("Model " + filename + " has no materials");
		}

		processNode(scene->mRootNode, scene);

		return true;
	}

private:
	void processNode(const aiNode * node, const aiScene * scene);
	std::vector<std::shared_ptr<Shape>>	processMesh(const aiMesh * mesh, const aiScene * scene);

    Assimp::Importer importer;
    uint32_t flags;
public:
	std::vector<std::vector<std::shared_ptr<Shape>>> meshes;
};
