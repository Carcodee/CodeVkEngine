//
// Created by carlo on 2024-10-08.
//

#ifndef RENDERER_STRUCTS_HPP
#define RENDERER_STRUCTS_HPP

#include <cmath>
#include <cstddef>

namespace rendering
{

    struct debug_voxels_mvp
    {
        glm::mat4 proj_view = glm::mat4(1.0);
        glm::vec3 origin = glm::vec3{0.0f, 0.0f, 0.0f};
        alignas(16) glm::uvec3 dimensions = glm::uvec3{100, 100, 100};
        float cell_size = 0.5f;
    };
    struct mvp_pc
    {
        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 proj_view = glm::mat4(1.0);
    };

    struct split_mvp
    {
        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 view = glm::mat4(1.0);
        glm::mat4 proj = glm::mat4(1.0);
    };

    enum pint_mode
    {
        p_occluder = 0,
        p_light_source = 1
    };

    struct rc_pc
    {
        int cascades_count = 0;
        int probe_size_px = 8;
        int interval_count = 2;
        int f_width = 0;
        int f_height = 0;
        int base_interval_length = 12;
        int cascade_index = -1;
    };

    struct probes_gen_pc
    {
        int cascade_index = 0;
        int interval_size = 16;
        int probe_size_px = 16;
    };

    struct painting_pc
    {
        glm::vec4 color = glm::vec4(1.0);
        int layer_selected = 0;
        int painting = 0;
        int x_mouse_pos = 0;
        int y_mouse_pos = 0;
        int radius = 20;
    };

    struct c_props_ubo
    {
        glm::mat4 inv_proj = glm::mat4(1.0);
        glm::mat4 inv_view = glm::mat4(1.0);
        glm::vec3 pos = glm::vec3(0.0);
        float z_near = 0.0f;
        float z_far = 0.0f;
    };

    struct array_indexer
    {
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    struct screen_data_pc
    {
        int s_width = 0;
        int s_height = 0;
        int point_lights_count = 0;
        uint32_t x_tile_count = 0;
        uint32_t y_tile_count = 0;
        uint32_t x_tile_size_px = 0;
        uint32_t y_tile_size_px = 0;
    };

    struct light_pc
    {
        uint32_t x_tile_count = 0;
        uint32_t y_tile_count = 0;
        uint32_t x_tile_size_px = 0;
        uint32_t y_tile_size_px = 0;
        uint32_t z_slices = 0;
    };

    struct material_packed_data
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

    struct sphere
    {
        glm::vec3 center = glm::vec3(0);
        float radius = 0;
    };

    struct model_load_configs
    {
        bool load_meshes_spheres;
        bool compact_mesh;
    };

    struct frustum
    {
        glm::vec4 planes[6];
        glm::vec3 points[8];
    };

    struct cascades_info
    {
        uint32_t cascade_count = 4;
        int probe_size_px = 16;
        int interval_count = 2;
        int base_interval_length = 12;
    };

    struct merge_cascades_info
    {
        glm::uvec2 probe_centers_positions_px;
        glm::uvec2 probe_sizes_px;
        int dir_indices;
    };

    struct radiance_cascades_configs
    {
        int radiance_pow = 2;
        int normal_map_pow = 24;
        int specular_pow = 2;
        int roughness_pow = 2;
    };

    struct gaussian_splat
    {
        float x, y, z;
        float nx, ny, nz;
        float f_dc_0, f_dc_1, f_dc_2;
        float opacity;
        float scale_0, scale_1, scale_2;
        float rot_0, rot_1, rot_2, rot_3;

        float f_rest[45];
    };

    struct arrays_of_gaussians
    {
        std::vector<glm::vec3> pos;
        std::vector<glm::vec3> scales;
        std::vector<glm::vec4> rots;
        std::vector<glm::vec3> cols;
        std::vector<float> alphas;
        std::vector<glm::vec3> sh_coefs;
        std::vector<int> ids;
        glm::vec3 h_fov_focal;

        void init(std::vector<gaussian_splat> splats, float fov, float s_width, float s_height)
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
                scales.emplace_back(util::m_exp(glm::vec3(splats[i].scale_0, splats[i].scale_1, splats[i].scale_2)));
                rots.emplace_back(
                    -util::m_normalize_rotation(glm::vec4(splats[i].rot_0, splats[i].rot_1, splats[i].rot_2,
                                                         splats[i].rot_3)));
                glm::vec3 sh = glm::vec3(splats[i].f_dc_0, splats[i].f_dc_1, splats[i].f_dc_2);
                cols.emplace_back(sh);
                alphas.emplace_back(util::m_sigmoid(splats[i].opacity));
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

        std::vector<uint32_t> sort_by_depth(glm::mat4 view)
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

    struct gs_configs_pc
    {
        float scale_mod = 1.0;
    };

    struct pc_histogram
    {
        uint32_t g_num_elements; // == NUM_ELEMENTS
        uint32_t g_shift; // (*)
        uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
        uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP
    };

    struct pc_radix_sort
    {
        uint32_t g_num_elements; // == NUM_ELEMENTS
        uint32_t g_shift; // (*)
        uint32_t g_num_workgroups; // == NUMBER_OF_WORKGROUPS as defined in the section above
        uint32_t g_num_blocks_per_workgroup; // == NUM_BLOCKS_PER_WORKGROUP
    };

    struct pc_voxel_grid
    {
        glm::vec3 origin = glm::vec3{0.0f, 0.0f, 0.0f};
        alignas(16) glm::uvec3 dimensions = glm::uvec3{10, 10, 10};
        float cell_size = 0.5f;
        uint64_t get_size() const
        {
            return uint64_t(dimensions.x) * dimensions.y * dimensions.z;
        }

        bool is_valid() const
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
    static_assert(offsetof(pc_voxel_grid, dimensions) == 16);
    static_assert(offsetof(pc_voxel_grid, cell_size) == 28);
    static_assert(sizeof(pc_voxel_grid) == 32);
    struct d_voxel_grid
    {
        engine::buffer* d_cells = nullptr;
    };
    struct voxel_grid
    {
        pc_voxel_grid pc_voxel_grid  {};
        d_voxel_grid d_voxel_grid = {};
        std::string name;
        void init(const std::string& voxel_name, engine::render_graph* render_graph)
        {
            name = "CellsBuffer_" + voxel_name;
            d_voxel_grid.d_cells = render_graph->resources_manager->get_buffer(engine::resources_manager::buffer_params{
                name, vk::BufferUsageFlagBits::eStorageBuffer,
                {},
                sizeof(uint32_t) * pc_voxel_grid.get_size(),
                sizeof(uint32_t), nullptr
            });
        }

    };
} // namespace rendering


#endif        // RENDERER_STRUCTS_HPP
