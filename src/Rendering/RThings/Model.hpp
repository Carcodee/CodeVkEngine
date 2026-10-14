//
// Created by carlo on 2024-10-09.
//

#ifndef MODEL_HPP
#define MODEL_HPP

namespace rendering
{
struct node_mat
{
	rendering::node_mat  *parent_node = nullptr;
	glm::mat4 matrix     = glm::mat4(1.0f);
	glm::mat4 get_worl_mat()
	{
		rendering::node_mat  *parent = parent_node;
		glm::mat4 mat    = matrix;
		while (parent != nullptr)
		{
			mat    = parent->matrix * mat;
			parent = parent->parent_node;
		}
		return mat;
	}
	~node_mat() = default;
};
struct model
{
	int                     id;
	std::vector<engine::m_vertex_3d> vertices;
	std::vector<uint32_t>   indices;

	// map with mesh number
	int                    mesh_count;
	std::vector<uint32_t>  first_vertices;
	std::vector<uint32_t>  first_indices;
	std::vector<uint32_t>  vertices_count;
	std::vector<uint32_t>  indices_count;
	std::vector<rendering::node_mat *> node_mats;
	std::vector<glm::mat4> models_mat;
	std::vector<int>       materials;
	std::vector<sphere>    meshes_spheres;

	engine::staged_buffer *vert_buffer  = nullptr;
	engine::staged_buffer *index_buffer = nullptr;

	void set_world_matrices()
	{
		models_mat.reserve(mesh_count);
		for (auto &node : node_mats)
		{
			models_mat.emplace_back(node->get_worl_mat());
		}
	}
	void set_meshes_spheres()
	{
		for (int i = 0; i < meshes_spheres.size(); ++i)
		{
			meshes_spheres[i].center = glm::vec3(models_mat.at(i) * glm::vec4(meshes_spheres[i].center.x, meshes_spheres[i].center.y, meshes_spheres[i].center.z, 1.0));
			float     scale_factor   = glm::length(glm::vec3(models_mat.at(i) * glm::vec4(0.0, 1.0, 0.0, 0.0)));
			glm::vec4 radius_ws      = models_mat.at(i) * (glm::vec4(0.0, meshes_spheres[i].radius, 0.0, 0.0));
			glm::vec3 radius_pos_ws   = meshes_spheres[i].center + glm::vec3(radius_ws);
			meshes_spheres[i].radius = glm::distance(meshes_spheres[i].center, radius_pos_ws);
		}
	}
	~model()
	{
		for (auto node : node_mats)
		{
			delete node;
		}
	}
};
}        // namespace rendering
#endif        // MODEL_HPP
