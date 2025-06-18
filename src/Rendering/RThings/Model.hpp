//
// Created by carlo on 2024-10-09.
//

#ifndef MODEL_HPP
#define MODEL_HPP

namespace Rendering
{
struct NodeMat
{
	NodeMat  *parentNode = nullptr;
	glm::mat4 matrix     = glm::mat4(1.0f);
	glm::mat4 GetWorlMat()
	{
		NodeMat  *parent = parentNode;
		glm::mat4 mat    = matrix;
		while (parent != nullptr)
		{
			mat    = parent->matrix * mat;
			parent = parent->parentNode;
		}
		return mat;
	}
	~NodeMat() = default;
};
struct Model
{
	int                     id;
	std::vector<M_Vertex3D> vertices;
	std::vector<uint32_t>   indices;

	// map with mesh number
	int                    meshCount;
	std::vector<uint32_t>  firstVertices;
	std::vector<uint32_t>  firstIndices;
	std::vector<uint32_t>  verticesCount;
	std::vector<uint32_t>  indicesCount;
	std::vector<NodeMat *> nodeMats;
	std::vector<glm::mat4> modelsMat;
	std::vector<int>       materials;
	std::vector<Sphere>    meshesSpheres;

	ENGINE::StagedBuffer *vertBuffer  = nullptr;
	ENGINE::StagedBuffer *indexBuffer = nullptr;

	void SetWorldMatrices()
	{
		modelsMat.reserve(meshCount);
		for (auto &node : nodeMats)
		{
			modelsMat.emplace_back(node->GetWorlMat());
		}
	}
	void SetMeshesSpheres()
	{
		for (int i = 0; i < meshesSpheres.size(); ++i)
		{
			meshesSpheres[i].center = glm::vec3(modelsMat.at(i) * glm::vec4(meshesSpheres[i].center.x, meshesSpheres[i].center.y, meshesSpheres[i].center.z, 1.0));
			float     scaleFactor   = glm::length(glm::vec3(modelsMat.at(i) * glm::vec4(0.0, 1.0, 0.0, 0.0)));
			glm::vec4 radiusWs      = modelsMat.at(i) * (glm::vec4(0.0, meshesSpheres[i].radius, 0.0, 0.0));
			glm::vec3 radiusPosWs   = meshesSpheres[i].center + glm::vec3(radiusWs);
			meshesSpheres[i].radius = glm::distance(meshesSpheres[i].center, radiusPosWs);
		}
	}
	~Model()
	{
		for (auto node : nodeMats)
		{
			delete node;
		}
	}
};
}        // namespace Rendering
#endif        // MODEL_HPP
