//
// Created by carlo on 2024-10-08.
//


#ifndef RENDERINGSTRUCTS_HPP
#define RENDERINGSTRUCTS_HPP

namespace Rendering
{
    struct Vertex2D
    {
        float pos[2];
        float uv[2];
    	
    	static ENGINE::VertexInput GetVertexInput()
		{
            ENGINE::VertexInput vertexInput;
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC2, 0, offsetof(Vertex2D, pos), 0);
            vertexInput.AddVertexInputBinding(0, sizeof(Vertex2D));

            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC2, 0, offsetof(Vertex2D, uv), 1);
            vertexInput.AddVertexInputBinding(0, sizeof(Vertex2D));
			return vertexInput;
 		}

        static std::vector<Vertex2D> GetQuadVertices()
        {
	        std::vector<Vertex2D> quadVertices = {
		        {{-1.0f, 1.0f}, {0.0f, 1.0f}},
		        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
		        {{1.0f, -1.0f}, {1.0f, 0.0f}},
		        {{1.0f, 1.0f}, {1.0f, 1.0f}}
	        };
	        return quadVertices;
        }

        static std::vector<uint32_t> GetQuadIndices()
        {
	        std::vector<uint32_t> indices = {
		        0, 1, 2, // First triangle (top-left, bottom-left, bottom-right)
		        2, 3, 0 // Second triangle (bottom-right, top-right, top-left)
	        };
	        return indices;
        };


    };

    struct M_Vertex3D
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 uv;
    	int id;
        
		bool operator==(const M_Vertex3D& other) const {
			return pos == other.pos && uv == other.uv && normal == other.normal && tangent == other.tangent && id == other.id;
		}

    	static ENGINE::VertexInput GetVertexInput()
		{
			
            ENGINE::VertexInput vertexInput;
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC3, 0, offsetof(M_Vertex3D, pos), 0);
            vertexInput.AddVertexInputBinding(0, sizeof(M_Vertex3D));

            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC3, 0, offsetof(M_Vertex3D, normal), 1);
            vertexInput.AddVertexInputBinding(0, sizeof(M_Vertex3D));
			
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC3, 0, offsetof(M_Vertex3D, tangent), 2);
            vertexInput.AddVertexInputBinding(0, sizeof(M_Vertex3D));
			
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC2, 0, offsetof(M_Vertex3D, uv), 3);
            vertexInput.AddVertexInputBinding(0, sizeof(M_Vertex3D));
            			
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::INT, 0, offsetof(M_Vertex3D, id), 4);
            vertexInput.AddVertexInputBinding(0, sizeof(M_Vertex3D));

			return vertexInput;
 		}

    };
	struct D_Vertex3D
	{
	    glm::vec3 pos;
        
		bool operator==(const M_Vertex3D& other) const {
			return pos == other.pos;
		}

    	static ENGINE::VertexInput GetVertexInput()
		{
			
            ENGINE::VertexInput vertexInput;
            vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC3, 0, offsetof(D_Vertex3D, pos), 0);
            vertexInput.AddVertexInputBinding(0, sizeof(D_Vertex3D));
			return vertexInput;
 		}
	
	};
	

	struct MvpPc 
	{
		glm::mat4 model = glm::mat4(1.0);
		glm::mat4 projView = glm::mat4(1.0);
	};
	enum PintMode
	{
		P_OCCLUDER = 0,
		P_LIGHT_SOURCE = 1
	};
	struct RcPc
	{
		int cascadesCount = 0;
		int probeSizePx = 8;
		int intervalCount = 2;
		int fWidth = 0;
		int fHeight = 0;
		int baseIntervalLength = 12;
		int cascadeIndex = -1;
	};

	struct ProbesGenPc
	{
		int cascadeIndex = 0;
		int intervalSize = 16;
		int probeSizePx = 16;
	};

	struct PaintingPc 
	{
		glm::vec4 color = glm::vec4(1.0);
		int layerSelected = 0;
		int painting = 0;
		int xMousePos = 0;
		int yMousePos = 0;
		int radius = 20;
	};
	struct CPropsUbo
	{
		glm::mat4 invProj= glm::mat4(1.0);
		glm::mat4 invView= glm::mat4(1.0);
		glm::vec3 pos = glm::vec3(0.0);
		float zNear = 0.0f;
		float zFar = 0.0f;
	};
	struct ArrayIndexer 
	{
		uint32_t offset = 0;
		uint32_t size = 0;
	};
	struct ScreenDataPc
	{
		int sWidth = 0; 
		int sHeight= 0; 
		int pointLightsCount= 0; 
		uint32_t xTileCount = 0; 
		uint32_t yTileCount = 0; 
		uint32_t xTileSizePx = 0; 
		uint32_t yTileSizePx = 0; 
	};
	struct LightPc 
	{
		uint32_t xTileCount = 0;
		uint32_t yTileCount = 0;
		uint32_t xTileSizePx = 0;
		uint32_t yTileSizePx = 0;
		uint32_t zSlices = 0;
		
	};
    struct MaterialPackedData
    {
	    glm::vec4 diff = glm::vec4(1.0);
	    float albedoFactor = 1.0;
	    float normalFactor = 0.0;
	    float roughnessFactor = 0.0;
	    float metallicFactor = 0.0;
	    float alphaCutoff = 0.0;
    	
    	int albedoOffset = -1;
    	int normalOffset = -1;
    	int emissionOffset = -1;
    	int transOffset = -1;
    	int roughnessOffset = -1;
    	int metallicOffset = -1;
    	int metRoughnessOffset = -1;
    };
	struct Sphere
	{
		glm::vec3 center = glm::vec3(0);
		float radius = 0;
	};

	struct ModelLoadConfigs
	{
		bool loadMeshesSpheres;
		bool compactMesh;
	};

	struct Frustum
	{
		glm::vec4 planes[6];
		glm::vec3 points[8];
	};
	struct CascadesInfo
	{
		uint32_t cascadeCount = 4;
		int probeSizePx = 16;
		int intervalCount = 2;
		int baseIntervalLength = 12;
	};

	struct MergeCascadesInfo 
	{
		glm::uvec2 probeCentersPositionsPx;
		glm::uvec2 probeSizesPx;
		int dirIndices;
	};

	struct RadianceCascadesConfigs
	{
		int radiancePow = 2;
		int normalMapPow = 24;
		int specularPow = 2;
		int roughnessPow = 2;
	};

	struct GaussianSplat
	{
		glm::mat3 covarianceMatrix = glm::identity<glm::mat3>();
		glm::vec3 pos = glm::vec3(0.0);
		glm::vec3 col = glm::vec3(1.0);
		float alpha = 1.0;
	};
 	struct ArraysOfGaussians
	{
		std::vector<glm::mat3> covarianceMats;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> cols;
		std::vector<float> alphas;

 		GaussianSplat GetGaussianAt(int idx)
 		{
 			GaussianSplat gaussianSplat = {covarianceMats[idx], positions[idx], cols[idx], alphas[idx]};
 			return gaussianSplat;
 		}
 		
	};   
}

namespace std
{
	template <>
	struct hash<Rendering::M_Vertex3D>
	{
		size_t operator()(Rendering::M_Vertex3D const& vertex) const
		{
			// Hash the position, color, normal, and texCoord
			size_t hash1 = hash<glm::vec3>()(vertex.pos);
			size_t hash2 = hash<glm::vec3>()(vertex.normal);
			size_t hash3 = hash<glm::vec3>()(vertex.tangent);
			size_t hash4 = hash<glm::vec2>()(vertex.uv);

			// Combine the hashes using bitwise operations
			size_t result = hash1;
			result = (result * 31) + hash2;
			result = (result * 31) + hash3;
			result = (result * 31) + hash4;

			return result;
		}
	};
}

#endif //RENDERINGSTRUCTS_HPP
