//


// Created by carlo on 2024-11-25.
//







#ifndef ENGINERESMANAGER_HPP
#define ENGINERESMANAGER_HPP

namespace Rendering
{

    class RenderingResManager
    {
    public:
    	void LoadPLY(std::string path, std::vector<glm::vec3>& positions)
    	{
    		// Construct the data object by reading from file
    		assert(std::filesystem::path(path).extension() == ".ply" && "Invalid PLY file");
    		assert(std::filesystem::exists(path));
    		
    		happly::PLYData plyIn(path);

    		// Get mesh-style data from the object
    		positions = plyIn.getVertexPositions_glm();
    		std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();
    		
    	}
    	void LoadGLTF(std::string path,Model& model, bool compactIndices)
    	{
    		if (!std::filesystem::exists(path))
    		{
    			SYSTEMS::Logger::GetInstance()->LogMessage("Path do not exist: "+ path);
    			return;
    		}
    		tinygltf::Model gltfModel;
		    std::string err;
		    std::string warn;
		    tinygltf::TinyGLTF gltfContext;
		    gltfContext.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
    		NodeMat* rootNode = new NodeMat();

    		model.meshCount = (int)gltfModel.meshes.size();
		    for (auto& scene : gltfModel.scenes)
		    {
			    for (auto& node : scene.nodes)
			    {
			    	if (compactIndices)
			    	{
					    LoadGLTFNodeContiguousMesh(gltfModel, gltfModel.nodes[node], rootNode, model);
			    	}else
			    	{
			    		LoadGLTFNode(gltfModel, gltfModel.nodes[node], rootNode, model);
			    	}
			    }
			    
		    }
    		model.SetWorldMatrices();
    		model.SetMeshesSpheres();
    		LoadGLTFMaterials(gltfModel, path);
		    SYSTEMS::Logger::GetInstance()->LogMessage("Model loader Errors: " + err);
		    SYSTEMS::Logger::GetInstance()->LogMessage("Model loader Warns: " + warn);
    	}
    	void LoadGLTFNode(tinygltf::Model& gltfModel, tinygltf::Node& node, NodeMat* parentNodeMat, Model& model)
    	{

    		glm::mat4 nodeMatrix = glm::mat4(1.0f);
    		if(node.scale.size()==3){
    			nodeMatrix = glm::scale(nodeMatrix,glm::vec3 (glm::make_vec3(node.scale.data())));
    		}
    		if(node.rotation.size()==4){
    			glm::quat rot = glm::make_quat(node.rotation.data());
    			nodeMatrix *= glm::mat4(rot);
    		}
    		if(node.translation.size()==3){
    			nodeMatrix = glm::translate(nodeMatrix,glm::vec3 (glm::make_vec3(node.translation.data())));
    		}
    		if(node.matrix.size()==16){
    			nodeMatrix = glm::make_mat4(node.matrix.data());
    		}
    		
    		NodeMat* nodeMat = new NodeMat();
		    nodeMat->parentNode = parentNodeMat;
    		nodeMat->matrix = nodeMatrix;

		    for (auto& child : node.children)
		    {
		    	LoadGLTFNode(gltfModel, gltfModel.nodes[child], nodeMat, model);
		    }
    		
    		if (node.mesh > -1)
    		{
    			model.nodeMats.push_back(nodeMat);
    			model.firstVertices.push_back(model.vertices.size());
    			model.firstIndices.push_back(model.indices.size());
    			tinygltf::Mesh& currMesh = gltfModel.meshes[node.mesh];
    			
    			if (currMesh.primitives[0].material > -1)
    			{
				    model.materials.push_back(
					    GetInstance()->materials.size() + currMesh.primitives[0].material);
    			}else
    			{
    				model.materials.push_back(0);
    			}
			    for (auto& primitive : currMesh.primitives)
			    {
				    int vertexCount = 0;
				    int indexCount = 0;
			    	{
			    		const float* posBuff = nullptr;
			    		const float* normalsBuff = nullptr;
			    		const float* tangentsBuff = nullptr;
			    		const float* textCoordsBuff = nullptr;
			    		if (primitive.attributes.find("POSITION") != primitive.attributes.end())
			    		{
			    			tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("POSITION")->second];
			    			tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
			    			posBuff = reinterpret_cast<const float*> (&gltfModel.buffers[view.buffer].data[view.byteOffset + accessor.byteOffset]);
			    			vertexCount = accessor.count;
			    			
			    		}
					    if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("NORMAL")->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    normalsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }
					    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("TEXCOORD_0")->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    textCoordsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }
					    if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("TANGENT")
							    ->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    tangentsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }

					    model.vertices.reserve(vertexCount);
				    	float maxDistance =0.0;
				    	glm::vec3 min = glm::vec3();
				    	glm::vec3 max = glm::vec3();
					    for (int i = 0; i < vertexCount; ++i)
					    {
							M_Vertex3D vertex{};
					    	glm::vec3 pos =glm::make_vec3(&posBuff[i * 3]);
					    	vertex.pos = pos;
							vertex.normal = normalsBuff ? glm::make_vec3(&normalsBuff[i * 3]) : glm::vec3(0.0f);
							//not passing vec4 tangents at the moment
							glm::vec4 tangent = tangentsBuff ? glm::make_vec4(&tangentsBuff[i * 4]) : glm::vec4(0.0f);
							vertex.tangent = tangentsBuff ? glm::vec3(tangent.x, tangent.y, tangent.z) * tangent.w: glm::vec3(0.0f);
							vertex.uv = textCoordsBuff? glm::make_vec2(&textCoordsBuff[i * 2]): glm::vec2(0.0f);
					    	vertex.id =node.mesh;
					    	float distance = glm::distance(glm::vec3(0.0),vertex.pos);
					    	if (distance > maxDistance)
					    	{
					    		maxDistance = distance;
					    	}
					    	min = glm::min(min, pos);
					    	max = glm::max(max, pos);
							model.vertices.push_back(vertex);
					    }
				    	glm::vec3 center = glm::vec3((min + max) * 0.5f);
				    	float radius = glm::distance(center, max);
					    model.meshesSpheres.emplace_back(Sphere{center, radius});
			    	}
				    //indices
					{
						tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
						tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						indexCount+= static_cast<uint32_t>(accessor.count);
						// meshIndexCount.push_back(indexCount);
						switch (accessor.componentType) {
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
								const uint32_t* buff = reinterpret_cast<const uint32_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(buff[j]);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
								const uint16_t* buff = reinterpret_cast<const uint16_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(buff[j]);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
								const uint8_t* buff = reinterpret_cast<const uint8_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(buff[j]);
								}
								break;
							}
							default:
								std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
								return;
						}

					}
			    	
			    	model.indicesCount.push_back(indexCount);
			    	model.verticesCount.push_back(vertexCount);
			    }
    			
    		}
    	}
	    void LoadGLTFNodeContiguousMesh(tinygltf::Model& gltfModel, tinygltf::Node& node, NodeMat* parentNodeMat, Model& model)
    	{
    		glm::mat4 nodeMatrix = glm::mat4(1.0f);
    		if(node.scale.size()==3){
    			nodeMatrix = glm::scale(nodeMatrix,glm::vec3 (glm::make_vec3(node.scale.data())));
    		}
    		if(node.rotation.size()==4){
    			glm::quat rot = glm::make_quat(node.rotation.data());
    			nodeMatrix *= glm::mat4(rot);
    		}
    		if(node.translation.size()==3){
    			nodeMatrix = glm::translate(nodeMatrix,glm::vec3 (glm::make_vec3(node.translation.data())));
    		}
    		if(node.matrix.size()==16){
    			nodeMatrix = glm::make_mat4(node.matrix.data());
    		}
    		
    		NodeMat* nodeMat = new NodeMat();
		    nodeMat->parentNode = parentNodeMat;
    		nodeMat->matrix = nodeMatrix;

		    for (auto& child : node.children)
		    {
		    	LoadGLTFNodeContiguousMesh(gltfModel, gltfModel.nodes[child], nodeMat, model);
		    }
    		
    		if (node.mesh > -1)
    		{
    			model.nodeMats.push_back(nodeMat);
    			model.firstVertices.push_back(model.vertices.size());
    			model.firstIndices.push_back(model.indices.size());
    			tinygltf::Mesh& currMesh = gltfModel.meshes[node.mesh];
    			
    			if (currMesh.primitives[0].material > -1)
    			{
				    model.materials.push_back(GetInstance()->materials.size() + currMesh.primitives[0].material);
    			}else
    			{
    				model.materials.push_back(0);
    			}
			    for (auto& primitive : currMesh.primitives)
			    {
			    	int verticesSize = model.vertices.size();
				    int vertexCount = 0;
				    int indexCount = 0;
			    	{
			    		const float* posBuff = nullptr;
			    		const float* normalsBuff = nullptr;
			    		const float* tangentsBuff = nullptr;
			    		const float* textCoordsBuff = nullptr;
			    		if (primitive.attributes.find("POSITION") != primitive.attributes.end())
			    		{
			    			tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("POSITION")->second];
			    			tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
			    			posBuff = reinterpret_cast<const float*> (&gltfModel.buffers[view.buffer].data[view.byteOffset + accessor.byteOffset]);
			    			vertexCount = accessor.count;
			    			
			    		}
					    if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("NORMAL")->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    normalsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }
					    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("TEXCOORD_0")->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    textCoordsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }
					    if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
					    {
						    tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.find("TANGENT")
							    ->
							    second];
						    tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						    tangentsBuff = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[view.
							    byteOffset + accessor.byteOffset]);
					    }

					    model.vertices.reserve(vertexCount);
					    for (int i = 0; i < vertexCount; ++i)
					    {
							M_Vertex3D vertex{};
					    	glm::vec3 pos =glm::make_vec3(&posBuff[i * 3]);
					    	vertex.pos = pos;
							vertex.normal = normalsBuff ? glm::make_vec3(&normalsBuff[i * 3]) : glm::vec3(0.0f);
							//not passing vec4 tangents at the moment
							glm::vec4 tangent = tangentsBuff ? glm::make_vec4(&tangentsBuff[i * 4]) : glm::vec4(0.0f);
							vertex.tangent = tangentsBuff ? glm::vec3(tangent.x, tangent.y, tangent.z) * tangent.w: glm::vec3(0.0f);
							vertex.uv = textCoordsBuff? glm::make_vec2(&textCoordsBuff[i * 2]): glm::vec2(0.0f);
					    	vertex.id = node.mesh; 
							model.vertices.push_back(vertex);
					    }
			    	}
				    //indices
					{
						tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
						tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						indexCount+= static_cast<uint32_t>(accessor.count);
						// meshIndexCount.push_back(indexCount);
						switch (accessor.componentType) {
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
								const uint32_t* buff = reinterpret_cast<const uint32_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(verticesSize + buff[j]);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
								const uint16_t* buff = reinterpret_cast<const uint16_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(verticesSize + buff[j]);
								}
								break;
							}
							case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
								const uint8_t* buff = reinterpret_cast<const uint8_t*>(&(gltfModel.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]));
								for (size_t j = 0; j <accessor.count; ++j) {
									model.indices.push_back(verticesSize + buff[j]);
								}
								break;
							}
							default:
								std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
								return;
						}
					}
			    	model.indicesCount.push_back(indexCount);
			    	model.verticesCount.push_back(vertexCount);
			    }
    		}
    	}
    	void LoadGLTFMaterials(tinygltf::Model &model, std::string modelPath, std::string modelPathAbs = "")
    	{
    		std::string modelFolder;
    		std::filesystem::path path (modelPath);
    		modelPath = path.parent_path().string();
		    modelFolder = modelPath;
    		if (!modelPathAbs.empty())
    		{
			    modelFolder = modelPathAbs;
    		}
    		if (!std::filesystem::exists(modelPath))
    		{
    			SYSTEMS::Logger::GetInstance()->SetLogPreferences(SYSTEMS::LogLevel::L_WARN);
    			SYSTEMS::Logger::GetInstance()->LogMessage("Texture path does not exist at: " + modelPath);
    			return;
    		};
	     
		    for (int i = 0; i < model.materials.size(); ++i)
		    {
		    	tinygltf::Material& gltfMat= model.materials[i];
		    	std::string materialName = "Material_"+ GetInstance()->materials.size();
		    	Material* material = GetInstance()->PushMaterial(materialName);
			    material->materialPackedData.diff = glm::vec4(gltfMat.pbrMetallicRoughness.baseColorFactor[0],
			                              gltfMat.pbrMetallicRoughness.baseColorFactor[1],
			                              gltfMat.pbrMetallicRoughness.baseColorFactor[2],
			                              gltfMat.pbrMetallicRoughness.baseColorFactor[3]);
		    	material->materialPackedData.roughnessFactor = static_cast<float>(gltfMat.pbrMetallicRoughness.roughnessFactor);
		    	material->materialPackedData.metallicFactor = static_cast<float>(gltfMat.pbrMetallicRoughness.metallicFactor);
		    	if (gltfMat.alphaMode =="OPAQUE"){
		    		material->materialPackedData.alphaCutoff = 1.0f;
		    	}
		    	if (gltfMat.alphaMode =="BLEND"){
		    		material->materialPackedData.alphaCutoff = gltfMat.alphaCutoff;
		    	}
			    if (gltfMat.pbrMetallicRoughness.baseColorTexture.index > -1)
			    {
				    std::string texturePath = GetGltfTexturePath(
				    	modelFolder,
					    model.images[model.textures[gltfMat.pbrMetallicRoughness.baseColorTexture.index].source].uri);
				    if (!texturePath.empty())
				    {
					    ENGINE::ImageShipper* texture = ENGINE::ResourcesManager::GetInstance()->BatchShipper(
						   texturePath, texturePath, 1, 1, ENGINE::g_ShipperFormat, ENGINE::GRAPHICS_READ);
					    material->materialPackedData.diff = glm::vec4(1.0f);
					    material->materialPackedData.albedoFactor = 1.0f;
				    	int id = ENGINE::ResourcesManager::GetInstance()->GetShipperID(texturePath);
				    	material->SetTexture(ALBEDO, id);
				    }
			    }
			    if (gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
			    {
				    std::string texturePath = GetGltfTexturePath(
					    modelFolder,
					    model.images[model.textures[gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index].source].
					    uri);
				    if (!texturePath.empty())
				    {
					    ENGINE::ImageShipper* texture = ENGINE::ResourcesManager::GetInstance()->BatchShipper(
						    texturePath, texturePath, 1, 1, ENGINE::g_ShipperFormat, ENGINE::GRAPHICS_READ);
					    material->materialPackedData.diff = glm::vec4(1.0f);
					    material->materialPackedData.albedoFactor = 1.0f;
					    int id = ENGINE::ResourcesManager::GetInstance()->GetShipperID(texturePath);
					    material->SetTexture(METALLIC_ROUGHNESS, id);
				    }
			    	
			    }
			    if (gltfMat.emissiveTexture.index > -1)
			    {
				    std::string texturePath = GetGltfTexturePath(
						modelFolder,
						model.images[model.textures[gltfMat.emissiveTexture.index].source].
						uri);
			    	if (!texturePath.empty())
			    	{
			    		ENGINE::ImageShipper* texture = ENGINE::ResourcesManager::GetInstance()->BatchShipper(
							texturePath, texturePath, 1, 1, ENGINE::g_ShipperFormat, ENGINE::GRAPHICS_READ);
			    		material->materialPackedData.diff = glm::vec4(1.0f);
			    		material->materialPackedData.albedoFactor = 1.0f;
			    		int id = ENGINE::ResourcesManager::GetInstance()->GetShipperID(texturePath);
			    		material->SetTexture(EMISSION, id);
			    	}
			    }
			    if (gltfMat.normalTexture.index > -1)
			    {
				    std::string texturePath = GetGltfTexturePath(
						modelFolder,
						model.images[model.textures[gltfMat.normalTexture.index].source].
						uri);
			    	if (!texturePath.empty())
			    	{
			    		ENGINE::ImageShipper* texture = ENGINE::ResourcesManager::GetInstance()->BatchShipper(
							texturePath, texturePath, 1, 1, ENGINE::g_ShipperFormat, ENGINE::GRAPHICS_READ);
			    		material->materialPackedData.diff = glm::vec4(1.0f);
			    		material->materialPackedData.albedoFactor = 1.0f;
			    		int id = ENGINE::ResourcesManager::GetInstance()->GetShipperID(texturePath);
			    		material->SetTexture(NORMAL, id);
			    	}
			    }
		    }
    	}
    	std::string GetGltfTexturePath(std::string modelPath, std::string uriPath) {
    		if (std::filesystem::exists(uriPath)){
    			return uriPath;
    		}
    		std::string path =modelPath +"\\"+ uriPath;
    		if (std::filesystem::exists(path)){
    			return path;
    		}
    		std::cout<< "there is no valid texture path on the gltf path: "<<modelPath <<"\n"; 
    		return "";
    	}
    	
        Material* PushMaterial(std::string name)
        {

            Material* material;
            if (!materialsNames.contains(name))
            {
                materials.emplace_back(std::make_unique<Material>());
                material = materials.back().get();
                materialPackedData.emplace_back(&material->materialPackedData);
                
            }else
            {
                SYSTEMS::Logger::GetInstance()->SetLogPreferences(SYSTEMS::LogLevel::L_WARN);
                SYSTEMS::Logger::GetInstance()->LogMessage("Material with name: \"" + name + "\" already exist returning that material");
                material = GetMaterialFromName(name);
            }
            return material;
        }

        static RenderingResManager* GetInstance(ENGINE::RenderGraph* renderGraph = nullptr)
        {
            if (instance == nullptr && renderGraph)
            {
                instance = new RenderingResManager(renderGraph);
            }
            return instance;
        }

        Material* GetMaterialFromName(std::string name)
        {
            if (!materialsNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->SetLogPreferences(SYSTEMS::LogLevel::L_ERROR);
                SYSTEMS::Logger::GetInstance()->LogMessage("Material with name: \"" +name +"\" does not exist");
                return nullptr;
            }
            return materials.at(materialsNames.at(name)).get();
        }
        
        int GetMaterialId(std::string name)
        {
             if (!materialsNames.contains(name))
            {
                SYSTEMS::Logger::GetInstance()->SetLogPreferences(SYSTEMS::LogLevel::L_ERROR);
                SYSTEMS::Logger::GetInstance()->LogMessage("Material with name: \"" +name +"\" does not exist");
                return -1;
            }
            return materialsNames.at(name);    
        }
        std::vector<MaterialPackedData> GetMaterialData(){
            std::vector<MaterialPackedData> materialPackedDatas;
            materialPackedDatas.reserve(materialPackedData.size());
            for (auto& materialPackedData : materialPackedData)
            {
                materialPackedDatas.emplace_back(*materialPackedData);
            }
            return materialPackedDatas;
        }

        Model* GetModel(std::string path)
        {
            if (modelsNames.contains(path))
            {
               return  models.at(modelsNames.at(path)).get();
            }
            modelsNames.try_emplace(path, models.size());
            Model* model = models.emplace_back(std::make_unique<Model>()).get();
    		model->id = models.size()-1;
            LoadGLTF(path, *model, false);
    		
    		std::string vertBuffName = "VertBuff_" + std::to_string(model->id);
    		std::string indexBuffName = "IndexBuff_" + std::to_string(model->id);
            model->vertBuffer = ENGINE::ResourcesManager::GetInstance()->GetStageBuffer(
	            vertBuffName, vk::BufferUsageFlagBits::eVertexBuffer,
	            sizeof(M_Vertex3D) * model->vertices.size(),
	            model->vertices.data());
            model->indexBuffer = ENGINE::ResourcesManager::GetInstance()->GetStageBuffer(
	            indexBuffName, vk::BufferUsageFlagBits::eIndexBuffer,
	            sizeof(uint32_t) * model->indices.size(),
	            model->indices.data());
    		
            return model;
        }

    	Animator2D* GetAnimator(std::string name, std::string path, int frameSpacing, AnimatorInfo animatorInfo)
    	{
    		if (animatorsNames.contains(name))
    		{
    			SYSTEMS::Logger::GetInstance()->Log("Invalid name for animator");
    			return nullptr;
    		}
    		animatorsNames.try_emplace(name, animators.size());
		    animators.emplace_back(std::make_unique<Animator2D>());
    		if (animatorInfo.isAtlas)
    		{
    			animators.back()->LoadAtlas(path, frameSpacing, animatorInfo);
    		}else
    		{
    			animators.back()->LoadFrames(path, frameSpacing);
    		}
    		return animators.back().get();
    	}

	    Animator2D* GetAnimatorByName(std::string name)
    	{
     		if (!animatorsNames.contains(name))
    		{
    			SYSTEMS::Logger::GetInstance()->Log("Invalid name for animator");
    			return nullptr;
    		}
    		return animators.at(animatorsNames.at(name)).get();
    	}
    	
    	void BuildIndirectBuffers()
    	{
    		indirectDrawsCmdInfos.clear();
		    for (auto& pair : indirectModelsToDraw)
		    {
		    	Model* model = pair.second;

			    for (int j = 0; j < model->meshCount; ++j)
			    {
				    ENGINE::DrawIndirectIndexedCmd indirectCmd{};
				    indirectCmd.firstIndex = model->firstIndices[j];
				    indirectCmd.firstInstance = 0;
				    indirectCmd.indexCount = model->indicesCount[j];
				    indirectCmd.instanceCount = 0;
				    indirectCmd.vertexOffset = model->firstVertices[j];
				    indirectDrawsCmdInfos.emplace_back(indirectCmd);
			    }    
		    }
		    indirectDrawBuffer = ENGINE::ResourcesManager::GetInstance()->GetBuffer( "IndirectCmds", vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
			    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
			    sizeof(ENGINE::DrawIndirectIndexedCmd) * indirectDrawsCmdInfos.size(), indirectDrawsCmdInfos.data());
    		
    	}
    	
    	void UpdateResources()
    	{

    		FlushIndirectBuffers();
    	}

    	void FlushIndirectBuffers()
    	{
     		if (indirectDrawsCmdInfos.empty()){return;}
    		cullCount = 0;
    		if (indirectDrawBuffer->mappedMem != nullptr)
    		{
			    indirectDrawBuffer->Unmap();
			    ENGINE::DrawIndirectIndexedCmd* infos = nullptr;
			    infos = reinterpret_cast<ENGINE::DrawIndirectIndexedCmd*>(indirectDrawBuffer->Map());
			    for (int i = 0; i < RenderingResManager::GetInstance()->indirectDrawsCmdInfos.size(); ++i)
			    {
				    if (infos->instanceCount > 0)
				    {
					    cullCount++;
				    }
				    ++infos;
			    }
			    indirectDrawBuffer->Unmap();
    		}
		    for (int i = 0; i < indirectDrawsCmdInfos.size(); ++i)
		    {
		    	indirectDrawsCmdInfos[i].instanceCount = 0;
		    }
		    indirectDrawBuffer = ENGINE::ResourcesManager::GetInstance()->SetBuffer(
			    "IndirectCmds", sizeof(ENGINE::DrawIndirectIndexedCmd) * indirectDrawsCmdInfos.size(),
			    indirectDrawsCmdInfos.data());   		
    	}

    	Model* PushModelToIndirectBatch(std::string name)
    	{
    		if (indirectModelsToDraw.contains(name))
    		{
    			return indirectModelsToDraw.at(name);
    		}
    		Model* model = GetModel(name);
    		indirectModelsToDraw.try_emplace(name ,model);
    		return model;
    	}

	    Model* PushModelToIndividualDraw(std::string name)
    	{
    		if (individualDrawModels.contains(name))
    		{
    			return individualDrawModels.at(name);
    		}
    		Model* model = GetModel(name);
    		individualDrawModels.try_emplace(name ,model);
    		return model;
    	}

    	
    	ENGINE::RenderGraphNode* GetTemplateNode_DF(std::string name, std::string shaderName)
    	{
		    ENGINE::AttachmentInfo colInfo = ENGINE::GetColorAttachmentInfo(
			    glm::vec4(0.0f), ENGINE::g_32bFormat);
		    ENGINE::Shader* vShader = renderGraph->resourcesManager->CreateDefaultShader(shaderName, ENGINE::ShaderStage::S_VERT);
		    ENGINE::Shader* fShader = renderGraph->resourcesManager->CreateDefaultShader(shaderName, ENGINE::ShaderStage::S_FRAG);
		    auto renderNode = renderGraph->AddPass(name);
		    renderNode->SetConfigs({true});
		    renderNode->SetVertShader(vShader);
		    renderNode->SetFragShader(fShader);
		    renderNode->SetVertexInput(Vertex2D::GetVertexInput());
		    //change this
		    renderNode->SetPushConstantSize(4);
    		return renderNode;
    	}

	    ENGINE::RenderGraphNode* GetTemplateComputeNode_DF(std::string name, std::string shaderName)
	    {
		    ENGINE::Shader* shader = renderGraph->resourcesManager->CreateDefaultShader(shaderName, ENGINE::ShaderStage::S_COMP);
		    auto renderNode = renderGraph->AddPass(name);
		    renderNode->SetConfigs({true});
		    renderNode->SetCompShader(shader);
		    //change this
		    renderNode->SetPushConstantSize(4);
		    return renderNode;
	    }
    	ENGINE::RenderGraphNode* GetTemplateNode(std::string name, std::string vPath, std::string fPath)
    	{
		    ENGINE::AttachmentInfo colInfo = ENGINE::GetColorAttachmentInfo(
			    glm::vec4(0.0f), ENGINE::g_32bFormat);
		    ENGINE::Shader* vShader = renderGraph->resourcesManager->GetShader(vPath, ENGINE::ShaderStage::S_VERT);
		    ENGINE::Shader* fShader = renderGraph->resourcesManager->GetShader(fPath, ENGINE::ShaderStage::S_FRAG);
		    auto renderNode = renderGraph->AddPass(name);
		    renderNode->SetConfigs({true});
		    renderNode->SetVertShader(vShader);
		    renderNode->SetFragShader(fShader);
		    renderNode->SetVertexInput(Vertex2D::GetVertexInput());
		    //change this
		    renderNode->SetPushConstantSize(4);
    		return renderNode;
    	}

	    ENGINE::RenderGraphNode* GetTemplateComputeNode(std::string name, std::string path)
	    {
		    ENGINE::Shader* shader = renderGraph->resourcesManager->GetShader(path, ENGINE::ShaderStage::S_COMP);
		    auto renderNode = renderGraph->AddPass(name);
		    renderNode->SetConfigs({true});
		    renderNode->SetCompShader(shader);
		    //change this
		    renderNode->SetPushConstantSize(4);
		    return renderNode;
	    }
    	
        std::map<std::string, int> materialsNames;
        std::map<std::string, int> modelsNames;
        std::map<std::string, int> texturesNames;
        std::map<std::string, int> animatorsNames;
        
        std::vector<std::unique_ptr<Material>> materials;
        std::vector<MaterialPackedData*> materialPackedData;
        std::vector<std::unique_ptr<Model>> models;
    	std::vector<std::unique_ptr<Animator2D>> animators;
    	ENGINE::Buffer* indirectDrawBuffer;
    	std::map<std::string, Model*> individualDrawModels;
    	std::map<std::string, Model*> indirectModelsToDraw;
    	std::vector<ENGINE::DrawIndirectIndexedCmd> indirectDrawsCmdInfos;
    	int cullCount =0;
        
        RenderingResManager(ENGINE::RenderGraph* renderGraph)
        {
	        this->renderGraph = renderGraph;	
            ENGINE::Buffer* quadVertBuffer = this->renderGraph->resourcesManager->GetStageBuffer(
                "quad_default", vk::BufferUsageFlagBits::eVertexBuffer,
                sizeof(Vertex2D) * Vertex2D::GetQuadVertices().size(),
                Vertex2D::GetQuadVertices().data())->deviceBuffer.get();

            ENGINE::Buffer* quadIndexBuffer =this->renderGraph->resourcesManager->GetStageBuffer(
                "quad_index_default", vk::BufferUsageFlagBits::eIndexBuffer,
                sizeof(uint32_t) * Vertex2D::GetQuadIndices().size(),
                Vertex2D::GetQuadIndices().data())->deviceBuffer.get();


        };
        ~RenderingResManager() = default;
    private:
        static RenderingResManager* instance;
    	ENGINE::RenderGraph* renderGraph;
    	
    };
    
    RenderingResManager* RenderingResManager::instance = nullptr;
    
}
#endif //ENGINERESMANAGER_HPP
