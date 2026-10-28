//
// Created by carlo on 2024-10-08.
//

#ifndef RENDERER_STRUCTS_HPP
#define RENDERER_STRUCTS_HPP

#include <cmath>
#include <cstddef>

namespace rendering
{

    struct Debug_Voxels_Mvp
    {
        glm::mat4 proj_view = glm::mat4(1.0);
        glm::vec3 origin = glm::vec3{0.0f, 0.0f, 0.0f};
        alignas(16) glm::uvec3 dimensions = glm::uvec3{100, 100, 100};
        float cell_size = 0.5f;
    };
    struct Mvp_Pc
    {
        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 proj_view = glm::mat4(1.0);
    };

    struct Split_Mvp
    {
        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 view = glm::mat4(1.0);
        glm::mat4 proj = glm::mat4(1.0);
    };

    enum Pint_Mode
    {
        P_Occluder = 0,
        P_Light_Source = 1
    };

    struct Rc_Pc
    {
        int cascades_count = 0;
        int probe_size_px = 8;
        int interval_count = 2;
        int f_width = 0;
        int f_height = 0;
        int base_interval_length = 12;
        int cascade_index = -1;
    };

    struct Probes_Gen_Pc
    {
        int cascade_index = 0;
        int interval_size = 16;
        int probe_size_px = 16;
    };

    struct Painting_Pc
    {
        glm::vec4 color = glm::vec4(1.0);
        int layer_selected = 0;
        int painting = 0;
        int x_mouse_pos = 0;
        int y_mouse_pos = 0;
        int radius = 20;
    };

    struct C_Props_Ubo
    {
        glm::mat4 inv_proj = glm::mat4(1.0);
        glm::mat4 inv_view = glm::mat4(1.0);
        glm::vec3 pos = glm::vec3(0.0);
        float z_near = 0.0f;
        float z_far = 0.0f;
    };

    struct Array_Indexer
    {
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    struct Screen_Data_Pc
    {
        int s_width = 0;
        int s_height = 0;
        int point_lights_count = 0;
        uint32_t x_tile_count = 0;
        uint32_t y_tile_count = 0;
        uint32_t x_tile_size_px = 0;
        uint32_t y_tile_size_px = 0;
    };

    struct Light_Pc
    {
        uint32_t x_tile_count = 0;
        uint32_t y_tile_count = 0;
        uint32_t x_tile_size_px = 0;
        uint32_t y_tile_size_px = 0;
        uint32_t z_slices = 0;
    };

    struct Material_Packed_Data
    {
        glm::vec4 diff = glm::vec4(1.0);
        float albedo_factor = 1.0;
        float normal_factor = 0.0;
        float roughness_factor = 0.0;
        float metallic_factor = 0.0;
        float alpha_cutoff = 0.0;

        int albedo_offset = -1;
        int normal_offset = -1;
        int emission_offset = -1;
        int trans_offset = -1;
        int roughness_offset = -1;
        int metallic_offset = -1;
        int met_roughness_offset = -1;
    };

    struct Sphere
    {
        glm::vec3 center = glm::vec3(0);
        float radius = 0;
    };

    struct Model_Load_Configs
    {
        bool load_meshes_spheres;
        bool compact_mesh;
    };

    struct Frustum
    {
        glm::vec4 planes[6];
        glm::vec3 points[8];
    };

    struct Cascades_Info
    {
        uint32_t cascade_count = 4;
        int probe_size_px = 16;
        int interval_count = 2;
        int base_interval_length = 12;
    };

    struct Merge_Cascades_Info
    {
        glm::uvec2 probe_centers_positions_px;
        glm::uvec2 probe_sizes_px;
        int dir_indices;
    };

    struct Radiance_Cascades_Configs
    {
        int radiance_pow = 2;
        int normal_map_pow = 24;
        int specular_pow = 2;
        int roughness_pow = 2;
    };

    struct Gaussian_Splat
    {
        float x, y, z;
        float nx, ny, nz;
        float f_dc_0, f_dc_1, f_dc_2;
        float opacity;
        float scale_0, scale_1, scale_2;
        float rot_0, rot_1, rot_2, rot_3;

        float f_rest[45];
    };

    struct Arrays_Of_Gaussians
    {
        std::vector<glm::vec3> pos;
        std::vector<glm::vec3> scales;
        std::vector<glm::vec4> rots;
        std::vector<glm::vec3> cols;
        std::vector<float> alphas;
        std::vector<glm::vec3> sh_coefs;
        std::vector<int> ids;
        glm::vec3 h_fov_focal;

        void Init(std::vector<Gaussian_Splat> splats, float fov, float s_width, float s_height)
        {
            size_t size = splats.size();
            assert(size > 0);

            std::random_device rd;
            std::mt19937 gen(rd());

            for (int i = 0; i < splats.size(); ++i)
            {
                // std::uniform_real_distribution<> distributionCov(-0.2f, 0.2f);
                // rots[i].x = distributionCov(gen);
                // rots[i].y = distributionCov(gen);
                // rots[i].z = distributionCov(gen);
                // rots[i].w = distributionCov(gen);
                //
                // scales[i].x = distributionCov(gen);
                // scales[i].y = distributionCov(gen);
                // scales[i].z = distributionCov(gen);
                //
                pos.emplace_back(-glm::vec3(splats[i].x, splats[i].y, splats[i].z));
                scales.emplace_back(util::M_Exp(glm::vec3(splats[i].scale_0, splats[i].scale_1, splats[i].scale_2)));
                rots.emplace_back(
                    -util::M_Normalize_Rotation(glm::vec4(splats[i].rot_0, splats[i].rot_1, splats[i].rot_2,
                                                         splats[i].rot_3)));
                glm::vec3 sh = glm::vec3(splats[i].f_dc_0, splats[i].f_dc_1, splats[i].f_dc_2);
                cols.emplace_back(sh);
                alphas.emplace_back(util::M_Sigmoid(splats[i].opacity));
                for (int j = 0; j < 15; j += 3)
                {
                    glm::vec3 coeff = glm::vec3(splats[i].f_rest[j * 3 + 0], splats[i].f_rest[j * 3 + 1],
                                                splats[i].f_rest[j * 3 + 2]);
                    sh_coefs.emplace_back(coeff);
                }
            }

            float h_tan_y = tan(glm::radians(fov) / 2.0);
            float h_tan_x = h_tan_y / s_width * s_height;
            float focal_z = s_height / (2 * h_tan_y);
            h_fov_focal = glm::vec3(h_tan_x, h_tan_y, focal_z);
        }

        std::vector<uint32_t> Sort_By_Depth(glm::mat4 view)
        {
            std::vector<uint32_t> indices(pos.size());
            std::iota(indices.begin(), indices.end(), 0);

            std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b)
            {
                float za = (view * glm::vec4(pos[a], 1.0f)).z;
                float zb = (view * glm::vec4(pos[b], 1.0f)).z;
                return za < zb;
            });

            return indices;
        }
    };

    struct Gs_Configs_Pc
    {
        float scale_mod = 1.0;
    };

    struct Pc_Histogram
    {
        uint32_t g_num_elements; // == NUM_ELEMENTS
        uint32_t g_shift; // (*)
        uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
        uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP
    };

    struct Pc_Radix_Sort
    {
        uint32_t g_num_elements; // == NUM_ELEMENTS
        uint32_t g_shift; // (*)
        uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
        uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP
    };

    struct Pc_Voxel_Grid
    {
        glm::vec3 origin = glm::vec3{0.0f, 0.0f, 0.0f};
        alignas(16) glm::uvec3 dimensions = glm::uvec3{10, 10, 10};
        float cell_size = 0.5f;
        uint64_t Get_Size() const
        {
            return uint64_t(dimensions.x) * dimensions.y * dimensions.z;
        }

        bool Is_Valid() const
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                if (dimensions[axis] == 0 || dimensions[axis] > 4096 ||
                    !std::isfinite(origin[axis]) ||
                    !std::isfinite(origin[axis] + float(dimensions[axis]) * cell_size))
                    return false;
            }
            return std::isfinite(cell_size) && cell_size > 0.0f;
        }
    };
    static_assert(offsetof(Pc_Voxel_Grid, dimensions) == 16);
    static_assert(offsetof(Pc_Voxel_Grid, cell_size) == 28);
    static_assert(sizeof(Pc_Voxel_Grid) == 32);

    struct Voxel_Grid
    {
        Pc_Voxel_Grid pc_voxel_grid  {};
        engine::Render_Graph* render_graph = nullptr;
        std::string name;
        void Init(const std::string& voxel_name, engine::Render_Graph* render_graph_in)
        {
            name = "CellsBuffer_" + voxel_name;
            this->render_graph = render_graph_in;
            render_graph->resources_manager->Get_Buffer(engine::Resources_Manager::Buffer_Params{
                name, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                {vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible},
                sizeof(uint32_t) * pc_voxel_grid.Get_Size(),
                sizeof(uint32_t),nullptr
            });
        }
        void Reset()
        {
            render_graph->resources_manager->Set_Buffer(name, sizeof(uint32_t) * pc_voxel_grid.Get_Size(), sizeof(uint32_t), nullptr);
        }

    };
} // namespace rendering


#endif        // RENDERER_STRUCTS_HPP
