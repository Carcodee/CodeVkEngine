//

// Created by carlo on 2024-11-25.
//

#ifndef RENDERING_RES_MANAGER_HPP
#define RENDERING_RES_MANAGER_HPP

namespace rendering
{
    class rendering_res_manager
    {
    public:
        void load_ply(std::string path, std::vector<glm::vec3>& positions)
        {
            // Construct the data object by reading from file
            assert(std::filesystem::path(path).extension() == ".ply" && "Invalid PLY file");
            assert(std::filesystem::exists(path));

            happly::PLYData ply_in(path);

            // Get mesh-style data from the object
            positions = ply_in.getVertexPositions_glm();
            // std::vector<std::vector<size_t>> fInd = ply_in.getFaceIndices<size_t>();
        }

        void load_gs(std::string path, std::vector<gaussian_splat>& splats)
        {
            // Construct the data object by reading from file
            assert(std::filesystem::path(path).extension() == ".ply" && "Invalid PLY file");
            assert(std::filesystem::exists(path));

            happly::PLYData ply_in(path);

            // Get mesh-style data from the object
            std::vector<float> x_pos = ply_in.getElement("vertex").getProperty<float>("x");
            std::vector<float> y_pos = ply_in.getElement("vertex").getProperty<float>("y");
            std::vector<float> z_pos = ply_in.getElement("vertex").getProperty<float>("z");

            std::vector<float> nx = ply_in.getElement("vertex").getProperty<float>("nx");
            std::vector<float> ny = ply_in.getElement("vertex").getProperty<float>("ny");
            std::vector<float> nz = ply_in.getElement("vertex").getProperty<float>("nz");

            std::vector<float> fdc0 = ply_in.getElement("vertex").getProperty<float>("f_dc_0");
            std::vector<float> fdc1 = ply_in.getElement("vertex").getProperty<float>("f_dc_1");
            std::vector<float> fdc2 = ply_in.getElement("vertex").getProperty<float>("f_dc_2");

            std::vector<float> opacity = ply_in.getElement("vertex").getProperty<float>("opacity");

            std::vector<float> scale0 = ply_in.getElement("vertex").getProperty<float>("scale_0");
            std::vector<float> scale1 = ply_in.getElement("vertex").getProperty<float>("scale_1");
            std::vector<float> scale2 = ply_in.getElement("vertex").getProperty<float>("scale_2");

            std::vector<float> rot0 = ply_in.getElement("vertex").getProperty<float>("rot_0");
            std::vector<float> rot1 = ply_in.getElement("vertex").getProperty<float>("rot_1");
            std::vector<float> rot2 = ply_in.getElement("vertex").getProperty<float>("rot_2");
            std::vector<float> rot3 = ply_in.getElement("vertex").getProperty<float>("rot_3");

            int f_rest_idx = 0;
            std::string curr_coef;
            std::vector<std::vector<float>> f_rests;

            for (int i = 0; i < 45; ++i)
            {
                curr_coef = "f_rest_" + std::to_string(f_rest_idx);
                std::vector<float> coef_array = ply_in.getElement("vertex").getProperty<float>(curr_coef.c_str());
                f_rests.emplace_back(coef_array);
                f_rest_idx++;
            }

            for (int i = 0; i < x_pos.size(); ++i)
            {
                splats.emplace_back(gaussian_splat{
                    x_pos[i], y_pos[i], z_pos[i],
                    nx[i], ny[i], nz[i],
                    fdc0[i], fdc1[i], fdc2[i],
                    opacity[i],
                    scale0[i], scale1[i], scale2[i],
                    rot0[i], rot1[i], rot2[i], rot3[i]
                });
                for (int j = 0; j < 15; ++j)
                {
                    splats.back().f_rest[j * 3 + 0] = f_rests.at(j).at(i);
                    splats.back().f_rest[j * 3 + 1] = f_rests.at(j + 15).at(i);
                    splats.back().f_rest[j * 3 + 2] = f_rests.at(j + 30).at(i);
                }
            }
        }

        void load_gltf(std::string path, rendering::model& model, bool compact_indices)
        {
            if (!std::filesystem::exists(path))
            {
                systems::logger::get_instance()->log_message("Path do not exist: " + path);
                return;
            }
            tinygltf::Model gltf_model;
            std::string err;
            std::string warn;
            tinygltf::TinyGLTF gltf_context;
            gltf_context.LoadASCIIFromFile(&gltf_model, &err, &warn, path);
            rendering::node_mat* root_node = new rendering::node_mat();

            model.mesh_count = (int)gltf_model.meshes.size();
            for (auto& scene : gltf_model.scenes)
            {
                for (auto& node : scene.nodes)
                {
                    if (compact_indices)
                    {
                        load_gltf_node_contiguous_mesh(gltf_model, gltf_model.nodes[node], root_node, model);
                    }
                    else
                    {
                        load_gltf_node(gltf_model, gltf_model.nodes[node], root_node, model);
                    }
                }
            }
            model.set_world_matrices();
            model.set_meshes_spheres();
            load_gltf_materials(gltf_model, path);
            systems::logger::get_instance()->log_message("Model loader Errors: " + err);
            systems::logger::get_instance()->log_message("Model loader Warns: " + warn);
        }

        void load_gltf_node(tinygltf::Model& gltf_model, tinygltf::Node& node, rendering::node_mat* parent_node_mat, rendering::model& model)
        {
            glm::mat4 node_matrix = glm::mat4(1.0f);
            if (node.scale.size() == 3)
            {
                node_matrix = glm::scale(node_matrix, glm::vec3(glm::make_vec3(node.scale.data())));
            }
            if (node.rotation.size() == 4)
            {
                glm::quat rot = glm::make_quat(node.rotation.data());
                node_matrix *= glm::mat4(rot);
            }
            if (node.translation.size() == 3)
            {
                node_matrix = glm::translate(node_matrix, glm::vec3(glm::make_vec3(node.translation.data())));
            }
            if (node.matrix.size() == 16)
            {
                node_matrix = glm::make_mat4(node.matrix.data());
            }

            rendering::node_mat* node_material = new rendering::node_mat();
            node_material->parent_node = parent_node_mat;
            node_material->matrix = node_matrix;

            for (auto& child : node.children)
            {
                load_gltf_node(gltf_model, gltf_model.nodes[child], node_material, model);
            }

            if (node.mesh > -1)
            {
                model.node_mats.push_back(node_material);
                model.first_vertices.push_back(model.vertices.size());
                model.first_indices.push_back(model.indices.size());
                tinygltf::Mesh& curr_mesh = gltf_model.meshes[node.mesh];

                if (curr_mesh.primitives[0].material > -1)
                {
                    model.materials.push_back(
                        get_instance()->materials.size() + curr_mesh.primitives[0].material);
                }
                else
                {
                    model.materials.push_back(0);
                }
                for (auto& primitive : curr_mesh.primitives)
                {
                    int vertex_count = 0;
                    int index_count = 0;
                    {
                        const float* pos_buff = nullptr;
                        const float* normals_buff = nullptr;
                        const float* tangents_buff = nullptr;
                        const float* text_coords_buff = nullptr;
                        if (primitive.attributes.find("POSITION") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("POSITION")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            pos_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                            vertex_count = accessor.count;
                        }
                        if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            normals_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }
                        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            text_coords_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }
                        if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("TANGENT")
                                ->second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            tangents_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }

                        model.vertices.reserve(vertex_count);
                        float max_distance = 0.0;
                        glm::vec3 min = glm::vec3();
                        glm::vec3 max = glm::vec3();
                        for (int i = 0; i < vertex_count; ++i)
                        {
                            engine::m_vertex_3d vertex{};
                            glm::vec3 pos = glm::make_vec3(&pos_buff[i * 3]);
                            vertex.pos = pos;
                            vertex.normal = normals_buff ? glm::make_vec3(&normals_buff[i * 3]) : glm::vec3(0.0f);
                            // not passing vec4 tangents at the moment
                            glm::vec4 tangent = tangents_buff ? glm::make_vec4(&tangents_buff[i * 4]) : glm::vec4(0.0f);
                            vertex.tangent = tangents_buff
                                                 ? glm::vec3(tangent.x, tangent.y, tangent.z) * tangent.w
                                                 : glm::vec3(0.0f);
                            vertex.uv = text_coords_buff ? glm::make_vec2(&text_coords_buff[i * 2]) : glm::vec2(0.0f);
                            vertex.id = node.mesh;
                            float distance = glm::distance(glm::vec3(0.0), vertex.pos);
                            if (distance > max_distance)
                            {
                                max_distance = distance;
                            }
                            min = glm::min(min, pos);
                            max = glm::max(max, pos);
                            model.vertices.push_back(vertex);
                        }
                        glm::vec3 center = glm::vec3((min + max) * 0.5f);
                        float radius = glm::distance(center, max);
                        model.meshes_spheres.emplace_back(sphere{center, radius});
                    }
                    // indices
                    {
                        tinygltf::Accessor& accessor = gltf_model.accessors[primitive.indices];
                        tinygltf::BufferView& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                        index_count += static_cast<uint32_t>(accessor.count);
                        // meshIndexCount.push_back(indexCount);
                        switch (accessor.componentType)
                        {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                            {
                                const uint32_t* buff = reinterpret_cast<const uint32_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(buff[j]);
                                }
                                break;
                            }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                            {
                                const uint16_t* buff = reinterpret_cast<const uint16_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(buff[j]);
                                }
                                break;
                            }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                            {
                                const uint8_t* buff = reinterpret_cast<const uint8_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(buff[j]);
                                }
                                break;
                            }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!" <<
                                std::endl;
                            return;
                        }
                    }

                    model.indices_count.push_back(index_count);
                    model.vertices_count.push_back(vertex_count);
                }
            }
        }

        void load_gltf_node_contiguous_mesh(tinygltf::Model& gltf_model, tinygltf::Node& node, rendering::node_mat* parent_node_mat,
                                        rendering::model& model)
        {
            glm::mat4 node_matrix = glm::mat4(1.0f);
            if (node.scale.size() == 3)
            {
                node_matrix = glm::scale(node_matrix, glm::vec3(glm::make_vec3(node.scale.data())));
            }
            if (node.rotation.size() == 4)
            {
                glm::quat rot = glm::make_quat(node.rotation.data());
                node_matrix *= glm::mat4(rot);
            }
            if (node.translation.size() == 3)
            {
                node_matrix = glm::translate(node_matrix, glm::vec3(glm::make_vec3(node.translation.data())));
            }
            if (node.matrix.size() == 16)
            {
                node_matrix = glm::make_mat4(node.matrix.data());
            }

            rendering::node_mat* node_material = new rendering::node_mat();
            node_material->parent_node = parent_node_mat;
            node_material->matrix = node_matrix;

            for (auto& child : node.children)
            {
                load_gltf_node_contiguous_mesh(gltf_model, gltf_model.nodes[child], node_material, model);
            }

            if (node.mesh > -1)
            {
                model.node_mats.push_back(node_material);
                model.first_vertices.push_back(model.vertices.size());
                model.first_indices.push_back(model.indices.size());
                tinygltf::Mesh& curr_mesh = gltf_model.meshes[node.mesh];

                if (curr_mesh.primitives[0].material > -1)
                {
                    model.materials.push_back(get_instance()->materials.size() + curr_mesh.primitives[0].material);
                }
                else
                {
                    model.materials.push_back(0);
                }
                for (auto& primitive : curr_mesh.primitives)
                {
                    int vertices_size = model.vertices.size();
                    int vertex_count = 0;
                    int index_count = 0;
                    {
                        const float* pos_buff = nullptr;
                        const float* normals_buff = nullptr;
                        const float* tangents_buff = nullptr;
                        const float* text_coords_buff = nullptr;
                        if (primitive.attributes.find("POSITION") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("POSITION")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            pos_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                            vertex_count = accessor.count;
                        }
                        if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("NORMAL")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            normals_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }
                        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("TEXCOORD_0")->
                                second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            text_coords_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }
                        if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                        {
                            tinygltf::Accessor& accessor = gltf_model.accessors[primitive.attributes.find("TANGENT")
                                ->second];
                            tinygltf::BufferView& view = gltf_model.bufferViews[accessor.bufferView];
                            tangents_buff = reinterpret_cast<const float*>(&gltf_model.buffers[view.buffer].data[view.
                                byteOffset + accessor.byteOffset]);
                        }

                        model.vertices.reserve(vertex_count);
                        for (int i = 0; i < vertex_count; ++i)
                        {
                            engine::m_vertex_3d vertex{};
                            glm::vec3 pos = glm::make_vec3(&pos_buff[i * 3]);
                            vertex.pos = pos;
                            vertex.normal = normals_buff ? glm::make_vec3(&normals_buff[i * 3]) : glm::vec3(0.0f);
                            // not passing vec4 tangents at the moment
                            glm::vec4 tangent = tangents_buff ? glm::make_vec4(&tangents_buff[i * 4]) : glm::vec4(0.0f);
                            vertex.tangent = tangents_buff
                                                 ? glm::vec3(tangent.x, tangent.y, tangent.z) * tangent.w
                                                 : glm::vec3(0.0f);
                            vertex.uv = text_coords_buff ? glm::make_vec2(&text_coords_buff[i * 2]) : glm::vec2(0.0f);
                            vertex.id = node.mesh;
                            model.vertices.push_back(vertex);
                        }
                    }
                    // indices
                    {
                        tinygltf::Accessor& accessor = gltf_model.accessors[primitive.indices];
                        tinygltf::BufferView& buffer_view = gltf_model.bufferViews[accessor.bufferView];
                        index_count += static_cast<uint32_t>(accessor.count);
                        // meshIndexCount.push_back(indexCount);
                        switch (accessor.componentType)
                        {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                            {
                                const uint32_t* buff = reinterpret_cast<const uint32_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(vertices_size + buff[j]);
                                }
                                break;
                            }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                            {
                                const uint16_t* buff = reinterpret_cast<const uint16_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(vertices_size + buff[j]);
                                }
                                break;
                            }
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                            {
                                const uint8_t* buff = reinterpret_cast<const uint8_t*>(&(gltf_model.buffers[buffer_view.
                                    buffer].data[buffer_view.byteOffset + accessor.byteOffset]));
                                for (size_t j = 0; j < accessor.count; ++j)
                                {
                                    model.indices.push_back(vertices_size + buff[j]);
                                }
                                break;
                            }
                        default:
                            std::cerr << "Index component type " << accessor.componentType << " not supported!" <<
                                std::endl;
                            return;
                        }
                    }
                    model.indices_count.push_back(index_count);
                    model.vertices_count.push_back(vertex_count);
                }
            }
        }

        void load_gltf_materials(tinygltf::Model& model, std::string model_path, std::string model_path_abs = "")
        {
            std::string model_folder;
            std::filesystem::path path(model_path);
            model_path = path.parent_path().string();
            model_folder = model_path;
            if (!model_path_abs.empty())
            {
                model_folder = model_path_abs;
            }
            if (!std::filesystem::exists(model_path))
            {
                systems::logger::get_instance()->set_log_preferences(systems::log_level::l_warn);
                systems::logger::get_instance()->log_message("Texture path does not exist at: " + model_path);
                return;
            };

            for (int i = 0; i < model.materials.size(); ++i)
            {
                tinygltf::Material& gltf_mat = model.materials[i];
                std::string material_name = "Material_" + get_instance()->materials.size();
                rendering::material* material = get_instance()->push_material(material_name);
                material->material_packed_data.diff = glm::vec4(gltf_mat.pbrMetallicRoughness.baseColorFactor[0],
                                                              gltf_mat.pbrMetallicRoughness.baseColorFactor[1],
                                                              gltf_mat.pbrMetallicRoughness.baseColorFactor[2],
                                                              gltf_mat.pbrMetallicRoughness.baseColorFactor[3]);
                material->material_packed_data.roughness_factor = static_cast<float>(gltf_mat.pbrMetallicRoughness.
                    roughnessFactor);
                material->material_packed_data.metallic_factor = static_cast<float>(gltf_mat.pbrMetallicRoughness.
                    metallicFactor);
                if (gltf_mat.alphaMode == "OPAQUE")
                {
                    material->material_packed_data.alpha_cutoff = 1.0f;
                }
                if (gltf_mat.alphaMode == "BLEND")
                {
                    material->material_packed_data.alpha_cutoff = gltf_mat.alphaCutoff;
                }
                if (gltf_mat.pbrMetallicRoughness.baseColorTexture.index > -1)
                {
                    std::string texture_path = get_gltf_texture_path(
                        model_folder,
                        model.images[model.textures[gltf_mat.pbrMetallicRoughness.baseColorTexture.index].source].uri);
                    if (!texture_path.empty())
                    {
                        engine::image_shipper* texture = engine::resources_manager::get_instance()->batch_shipper(
                            texture_path, texture_path, 1, 1, engine::g_shipper_format, engine::graphics_read);
                        material->material_packed_data.diff = glm::vec4(1.0f);
                        material->material_packed_data.albedo_factor = 1.0f;
                        int id = engine::resources_manager::get_instance()->get_shipper_id(texture_path);
                        material->set_texture(albedo, id);
                    }
                }
                if (gltf_mat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
                {
                    std::string texture_path = get_gltf_texture_path(
                        model_folder,
                        model.images[model.textures[gltf_mat.pbrMetallicRoughness.metallicRoughnessTexture.index].source]
                        .uri);
                    if (!texture_path.empty())
                    {
                        engine::image_shipper* texture = engine::resources_manager::get_instance()->batch_shipper(
                            texture_path, texture_path, 1, 1, engine::g_shipper_format, engine::graphics_read);
                        material->material_packed_data.diff = glm::vec4(1.0f);
                        material->material_packed_data.albedo_factor = 1.0f;
                        int id = engine::resources_manager::get_instance()->get_shipper_id(texture_path);
                        material->set_texture(metallic_roughness, id);
                    }
                }
                if (gltf_mat.emissiveTexture.index > -1)
                {
                    std::string texture_path = get_gltf_texture_path(
                        model_folder,
                        model.images[model.textures[gltf_mat.emissiveTexture.index].source].uri);
                    if (!texture_path.empty())
                    {
                        engine::image_shipper* texture = engine::resources_manager::get_instance()->batch_shipper(
                            texture_path, texture_path, 1, 1, engine::g_shipper_format, engine::graphics_read);
                        material->material_packed_data.diff = glm::vec4(1.0f);
                        material->material_packed_data.albedo_factor = 1.0f;
                        int id = engine::resources_manager::get_instance()->get_shipper_id(texture_path);
                        material->set_texture(emission, id);
                    }
                }
                if (gltf_mat.normalTexture.index > -1)
                {
                    std::string texture_path = get_gltf_texture_path(
                        model_folder,
                        model.images[model.textures[gltf_mat.normalTexture.index].source].uri);
                    if (!texture_path.empty())
                    {
                        engine::image_shipper* texture = engine::resources_manager::get_instance()->batch_shipper(
                            texture_path, texture_path, 1, 1, engine::g_shipper_format, engine::graphics_read);
                        material->material_packed_data.diff = glm::vec4(1.0f);
                        material->material_packed_data.albedo_factor = 1.0f;
                        int id = engine::resources_manager::get_instance()->get_shipper_id(texture_path);
                        material->set_texture(normal, id);
                    }
                }
            }
        }

        std::string get_gltf_texture_path(std::string model_path, std::string uri_path)
        {
            if (std::filesystem::exists(uri_path))
            {
                return uri_path;
            }
            std::string path = model_path + "\\" + uri_path;
            if (std::filesystem::exists(path))
            {
                return path;
            }
            std::cout << "there is no valid texture path on the gltf path: " << model_path << "\n";
            return "";
        }

        rendering::material* push_material(std::string name)
        {
            rendering::material* material;
            if (!materials_names.contains(name))
            {
                materials.emplace_back(std::make_unique<rendering::material>());
                material = materials.back().get();
                material_packed_data.emplace_back(&material->material_packed_data);
            }
            else
            {
                systems::logger::get_instance()->set_log_preferences(systems::log_level::l_warn);
                systems::logger::get_instance()->log_message(
                    "Material with name: \"" + name + "\" already exist returning that material");
                material = get_material_from_name(name);
            }
            return material;
        }

        static rendering_res_manager* get_instance()
        {
            if (instance == nullptr)
            {
                instance = new rendering_res_manager();
            }
            return instance;
        }

        rendering::material* get_material_from_name(std::string name)
        {
            if (!materials_names.contains(name))
            {
                systems::logger::get_instance()->set_log_preferences(systems::log_level::l_error);
                systems::logger::get_instance()->log_message("Material with name: \"" + name + "\" does not exist");
                return nullptr;
            }
            return materials.at(materials_names.at(name)).get();
        }

        int get_material_id(std::string name)
        {
            if (!materials_names.contains(name))
            {
                systems::logger::get_instance()->set_log_preferences(systems::log_level::l_error);
                systems::logger::get_instance()->log_message("Material with name: \"" + name + "\" does not exist");
                return -1;
            }
            return materials_names.at(name);
        }

        std::vector<rendering::material_packed_data> get_material_data()
        {
            std::vector<rendering::material_packed_data> material_packed_datas;
            material_packed_datas.reserve(material_packed_data.size());
            for (auto& material_packed_data : material_packed_data)
            {
                material_packed_datas.emplace_back(*material_packed_data);
            }
            return material_packed_datas;
        }

        rendering::model* get_model(std::string path)
        {
            if (models_names.contains(path))
            {
                return models.at(models_names.at(path)).get();
            }
            models_names.try_emplace(path, models.size());
            rendering::model* model = models.emplace_back(std::make_unique<rendering::model>()).get();
            model->id = models.size() - 1;
            load_gltf(path, *model, false);

            std::string vert_buff_name = "VertBuff_" + std::to_string(model->id);
            std::string index_buff_name = "IndexBuff_" + std::to_string(model->id);
            model->vert_buffer = engine::resources_manager::get_instance()->get_stage_buffer(
                vert_buff_name, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
                sizeof(engine::m_vertex_3d) * model->vertices.size(), sizeof(engine::m_vertex_3d),
                model->vertices.data());
            model->index_buffer = engine::resources_manager::get_instance()->get_stage_buffer(
                index_buff_name, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
                sizeof(uint32_t) * model->indices.size(), sizeof(uint32_t),
                model->indices.data());

            return model;
        }

        animator_2d* get_animator(std::string name, std::string path, int frame_spacing, rendering::animator_info animation_info)
        {
            if (animators_names.contains(name))
            {
                systems::logger::get_instance()->log("Invalid name for animator");
                return nullptr;
            }
            animators_names.try_emplace(name, animators.size());
            animators.emplace_back(std::make_unique<animator_2d>());
            if (animation_info.is_atlas)
            {
                animators.back()->load_atlas(path, frame_spacing, animation_info);
            }
            else
            {
                animators.back()->load_frames(path, frame_spacing);
            }
            return animators.back().get();
        }

        animator_2d* get_animator_by_name(std::string name)
        {
            if (!animators_names.contains(name))
            {
                systems::logger::get_instance()->log("Invalid name for animator");
                return nullptr;
            }
            return animators.at(animators_names.at(name)).get();
        }

        void build_indirect_buffers()
        {
            indirect_draws_cmd_infos.clear();
            for (auto& pair : indirect_models_to_draw)
            {
                rendering::model* model = pair.second;

                for (int j = 0; j < model->mesh_count; ++j)
                {
                    engine::draw_indirect_indexed_cmd indirect_cmd{};
                    indirect_cmd.first_index = model->first_indices[j];
                    indirect_cmd.first_instance = 0;
                    indirect_cmd.index_count = model->indices_count[j];
                    indirect_cmd.instance_count = 0;
                    indirect_cmd.vertex_offset = model->first_vertices[j];
                    indirect_draws_cmd_infos.emplace_back(indirect_cmd);
                }
            }
            indirect_draw_buffer = engine::resources_manager::get_instance()->get_buffer(
                engine::resources_manager::buffer_params{
                    "indirect_cmds_buffer", vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
                    sizeof(engine::draw_indirect_indexed_cmd) * indirect_draws_cmd_infos.size(),
                    sizeof(engine::draw_indirect_indexed_cmd), indirect_draws_cmd_infos.data()
                });
        }

        void update_resources()
        {
            flush_indirect_buffers();
        }

        void flush_indirect_buffers()
        {
            if (indirect_draws_cmd_infos.empty())
            {
                return;
            }
            cull_count = 0;
            if (indirect_draw_buffer->mapped_mem != nullptr)
            {
                indirect_draw_buffer->unmap();
                engine::draw_indirect_indexed_cmd* infos = nullptr;
                infos = reinterpret_cast<engine::draw_indirect_indexed_cmd*>(indirect_draw_buffer->map());
                for (int i = 0; i < rendering_res_manager::get_instance()->indirect_draws_cmd_infos.size(); ++i)
                {
                    if (infos->instance_count > 0)
                    {
                        cull_count++;
                    }
                    ++infos;
                }
                indirect_draw_buffer->unmap();
            }
            for (int i = 0; i < indirect_draws_cmd_infos.size(); ++i)
            {
                indirect_draws_cmd_infos[i].instance_count = 0;
            }
            indirect_draw_buffer = engine::resources_manager::get_instance()->set_buffer(
                "indirect_cmds_buffer", sizeof(engine::draw_indirect_indexed_cmd) * indirect_draws_cmd_infos.size(),
                sizeof(engine::draw_indirect_indexed_cmd), indirect_draws_cmd_infos.data());
        }

        rendering::model* push_model_to_indirect_batch(std::string name)
        {
            if (indirect_models_to_draw.contains(name))
            {
                return indirect_models_to_draw.at(name);
            }
            rendering::model* model = get_model(name);
            indirect_models_to_draw.try_emplace(name, model);
            return model;
        }

        rendering::model* push_model_to_individual_draw(std::string name)
        {
            if (individual_draw_models.contains(name))
            {
                return individual_draw_models.at(name);
            }
            rendering::model* model = get_model(name);
            individual_draw_models.try_emplace(name, model);
            return model;
        }

        std::map<std::string, int> materials_names;
        std::map<std::string, int> models_names;
        std::map<std::string, int> textures_names;
        std::map<std::string, int> animators_names;

        std::vector<std::unique_ptr<rendering::material>> materials;
        std::vector<rendering::material_packed_data*> material_packed_data;
        std::vector<std::unique_ptr<rendering::model>> models;
        std::vector<std::unique_ptr<animator_2d>> animators;
        engine::buffer* indirect_draw_buffer;
        std::map<std::string, rendering::model*> individual_draw_models;
        std::map<std::string, rendering::model*> indirect_models_to_draw;
        std::vector<engine::draw_indirect_indexed_cmd> indirect_draws_cmd_infos;
        int cull_count = 0;

        rendering_res_manager()
        {
        };
        ~rendering_res_manager() = default;

    private:
        static rendering_res_manager* instance;
    };

    rendering_res_manager* rendering_res_manager::instance = nullptr;
} // namespace rendering
#endif        // RENDERING_RES_MANAGER_HPP
