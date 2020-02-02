
#include "modelimporter.h"

	void ModelImporter::processNode(const aiNode * node, const aiScene * scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(this->processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}

	std::vector<std::shared_ptr<Shape>>	ModelImporter::processMesh(const aiMesh * mesh, const aiScene * scene)
	{
		std::vector<Point3f> positions;
		std::vector<int> indices;
		int nTriangles = mesh->mNumFaces;
		int nVertices = mesh->mNumVertices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			if (mesh->HasPositions())
			{
				Point3f pos{
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
				};
				positions.push_back(pos);
			}
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return CreateTriangleMeshShape(
			 nTriangles, indices.data(),
			 nVertices, positions.data());
	}
