//
// Created by carlo on 2024-10-09.
//

#ifndef MODEL_HPP
#define MODEL_HPP

namespace rendering
{
    struct Node_Mat
    {
        rendering::Node_Mat* parent_node = nullptr;
        glm::mat4 matrix = glm::mat4(1.0f);

        glm::mat4 Get_Worl_Mat()
        {
            rendering::Node_Mat* parent = parent_node;
            glm::mat4 mat = matrix;
            while (parent != nullptr)
            {
                mat = parent->matrix * mat;
                parent = parent->parent_node;
            }
            return mat;
        }

        ~Node_Mat() = default;
    };

    struct Model
    {
        struct Model_Properties
        {
            uint32_t index_count = 0;
            uint32_t vertex_count = 0;
            uint32_t triangle_count = 0;
        };

        int id;
        std::vector<engine::M_Vertex_3d> vertices;
        std::vector<uint32_t> indices;

        // map with mesh number
        int mesh_count;
        std::vector<uint32_t> first_vertices;
        std::vector<uint32_t> first_indices;
        std::vector<uint32_t> vertices_count;
        std::vector<uint32_t> indices_count;
        std::vector<rendering::Node_Mat*> node_mats;
        std::vector<glm::mat4> models_mat;
        std::vector<int> materials;
        std::vector<Sphere> meshes_spheres;

        engine::Staged_Buffer* vert_buffer = nullptr;
        engine::Staged_Buffer* index_buffer = nullptr;
        Model_Properties model_properties;

        void Set_World_Matrices()
        {
            models_mat.reserve(mesh_count);
            for (auto& node : node_mats)
            {
                models_mat.emplace_back(node->Get_Worl_Mat());
            }
        }

        void Set_Meshes_Spheres()
        {
            for (int i = 0; i < meshes_spheres.size(); ++i)
            {
                meshes_spheres[i].center = glm::vec3(models_mat.at(i) * glm::vec4(
                    meshes_spheres[i].center.x, meshes_spheres[i].center.y, meshes_spheres[i].center.z, 1.0));
                float scale_factor = glm::length(glm::vec3(models_mat.at(i) * glm::vec4(0.0, 1.0, 0.0, 0.0)));
                glm::vec4 radius_ws = models_mat.at(i) * (glm::vec4(0.0, meshes_spheres[i].radius, 0.0, 0.0));
                glm::vec3 radius_pos_ws = meshes_spheres[i].center + glm::vec3(radius_ws);
                meshes_spheres[i].radius = glm::distance(meshes_spheres[i].center, radius_pos_ws);
            }
        }

        void Calculate_Vertex_Index_Data()
        {
            model_properties.index_count = 0;
            model_properties.vertex_count = 0;
            model_properties.triangle_count = 0;
            for (int i = 0; i < indices_count.size(); ++i)
            {
                model_properties.index_count += indices_count[i];
            }
            for (int i = 0; i < vertices_count.size(); ++i)
            {
                model_properties.vertex_count += vertices_count[i];
            }
            model_properties.triangle_count = model_properties.index_count / 3;
        }

        ~Model()
        {
            for (auto node : node_mats)
            {
                delete node;
            }
        }
    };
} // namespace rendering
#endif        // MODEL_HPP
