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
		    {{1.0f, 1.0f}, {1.0f, 1.0f}}};
		return quadVertices;
	}

	static std::vector<uint32_t> GetQuadIndices()
	{
		std::vector<uint32_t> indices = {
		    0, 1, 2,        // First triangle (top-left, bottom-left, bottom-right)
		    2, 3, 0         // Second triangle (bottom-right, top-right, top-left)
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
	int       id;

	bool operator==(const M_Vertex3D &other) const
	{
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

	bool operator==(const M_Vertex3D &other) const
	{
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

struct GS_Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
	int       id;

	static ENGINE::VertexInput GetVertexInput()
	{
		ENGINE::VertexInput vertexInput;
		vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC2, 0, offsetof(GS_Vertex, pos), 0);
		vertexInput.AddVertexInputBinding(0, sizeof(GS_Vertex));

		vertexInput.AddVertexAttrib(ENGINE::VertexInput::VEC2, 0, offsetof(GS_Vertex, uv), 0);
		vertexInput.AddVertexInputBinding(0, sizeof(GS_Vertex));

		vertexInput.AddVertexAttrib(ENGINE::VertexInput::INT, 0, offsetof(GS_Vertex, id), 1);
		vertexInput.AddVertexInputBinding(0, sizeof(GS_Vertex));
		return vertexInput;
	}
};

struct MvpPc
{
	glm::mat4 model    = glm::mat4(1.0);
	glm::mat4 projView = glm::mat4(1.0);
};

struct SplitMVP
{
	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 view  = glm::mat4(1.0);
	glm::mat4 proj  = glm::mat4(1.0);
};
enum PintMode
{
	P_OCCLUDER     = 0,
	P_LIGHT_SOURCE = 1
};
struct RcPc
{
	int cascadesCount      = 0;
	int probeSizePx        = 8;
	int intervalCount      = 2;
	int fWidth             = 0;
	int fHeight            = 0;
	int baseIntervalLength = 12;
	int cascadeIndex       = -1;
};

struct ProbesGenPc
{
	int cascadeIndex = 0;
	int intervalSize = 16;
	int probeSizePx  = 16;
};

struct PaintingPc
{
	glm::vec4 color         = glm::vec4(1.0);
	int       layerSelected = 0;
	int       painting      = 0;
	int       xMousePos     = 0;
	int       yMousePos     = 0;
	int       radius        = 20;
};
struct CPropsUbo
{
	glm::mat4 invProj = glm::mat4(1.0);
	glm::mat4 invView = glm::mat4(1.0);
	glm::vec3 pos     = glm::vec3(0.0);
	float     zNear   = 0.0f;
	float     zFar    = 0.0f;
};
struct ArrayIndexer
{
	uint32_t offset = 0;
	uint32_t size   = 0;
};
struct ScreenDataPc
{
	int      sWidth           = 0;
	int      sHeight          = 0;
	int      pointLightsCount = 0;
	uint32_t xTileCount       = 0;
	uint32_t yTileCount       = 0;
	uint32_t xTileSizePx      = 0;
	uint32_t yTileSizePx      = 0;
};
struct LightPc
{
	uint32_t xTileCount  = 0;
	uint32_t yTileCount  = 0;
	uint32_t xTileSizePx = 0;
	uint32_t yTileSizePx = 0;
	uint32_t zSlices     = 0;
};
struct MaterialPackedData
{
	glm::vec4 diff            = glm::vec4(1.0);
	float     albedoFactor    = 1.0;
	float     normalFactor    = 0.0;
	float     roughnessFactor = 0.0;
	float     metallicFactor  = 0.0;
	float     alphaCutoff     = 0.0;

	int albedoOffset       = -1;
	int normalOffset       = -1;
	int emissionOffset     = -1;
	int transOffset        = -1;
	int roughnessOffset    = -1;
	int metallicOffset     = -1;
	int metRoughnessOffset = -1;
};
struct Sphere
{
	glm::vec3 center = glm::vec3(0);
	float     radius = 0;
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
	uint32_t cascadeCount       = 4;
	int      probeSizePx        = 16;
	int      intervalCount      = 2;
	int      baseIntervalLength = 12;
};

struct MergeCascadesInfo
{
	glm::uvec2 probeCentersPositionsPx;
	glm::uvec2 probeSizesPx;
	int        dirIndices;
};

struct RadianceCascadesConfigs
{
	int radiancePow  = 2;
	int normalMapPow = 24;
	int specularPow  = 2;
	int roughnessPow = 2;
};

struct GaussianSplat
{
	float x, y, z;
	float nx, ny, nz;
	float f_dc_0, f_dc_1, f_dc_2;
	float opacity;
	float scale_0, scale_1, scale_2;
	float rot_0, rot_1, rot_2, rot_3;

	float f_rest[45];
};
struct ArraysOfGaussians
{
	std::vector<glm::vec3> pos;
	std::vector<glm::vec3> scales;
	std::vector<glm::vec4> rots;
	std::vector<glm::vec3> cols;
	std::vector<float>     alphas;
	std::vector<glm::vec3> shCoefs;
	std::vector<int>       ids;
	glm::vec3              hFovFocal;

	void Init(std::vector<GaussianSplat> splats, float fov, float sWidth, float sHeight)
	{
		size_t size = splats.size();
		assert(size > 0);

		std::random_device rd;
		std::mt19937       gen(rd());

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
			scales.emplace_back(UTIL::M_Exp(glm::vec3(splats[i].scale_0, splats[i].scale_1, splats[i].scale_2)));
			rots.emplace_back(-UTIL::M_NormalizeRotation(glm::vec4(splats[i].rot_0, splats[i].rot_1, splats[i].rot_2, splats[i].rot_3)));
			glm::vec3 sh = glm::vec3(splats[i].f_dc_0, splats[i].f_dc_1, splats[i].f_dc_2);
			cols.emplace_back(sh);
			alphas.emplace_back(UTIL::M_Sigmoid(splats[i].opacity));
			for (int j = 0; j < 15; j += 3)
			{
				glm::vec3 coeff = glm::vec3(splats[i].f_rest[j * 3 + 0], splats[i].f_rest[j * 3 + 1], splats[i].f_rest[j * 3 + 2]);
				shCoefs.emplace_back(coeff);
			}
		}

		float hTanY  = tan(glm::radians(fov) / 2.0);
		float hTanX  = hTanY / sWidth * sHeight;
		float focalZ = sHeight / (2 * hTanY);
		hFovFocal    = glm::vec3(hTanX, hTanY, focalZ);
	}
	std::vector<uint32_t> SortByDepth(glm::mat4 view)
	{
		std::vector<uint32_t> indices(pos.size());
		std::iota(indices.begin(), indices.end(), 0);

		std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b) {
			float za = (view * glm::vec4(pos[a], 1.0f)).z;
			float zb = (view * glm::vec4(pos[b], 1.0f)).z;
			return za < zb;
		});

		return indices;
	}
};
struct GSConfigsPc
{
	float scaleMod = 1.0;
};

struct PcHistogram
{
	uint32_t g_num_elements;                    // == NUM_ELEMENTS
	uint32_t g_shift;                           // (*)
	uint32_t g_num_workgroups;                  // == NUMBER_OF_WORKGROUPS as defined in the section above
	uint32_t g_num_blocks_per_workgroup;        // == NUM_BLOCKS_PER_WORKGROUP
};

struct PcRadixSort
{
	uint32_t g_num_elements;                    // == NUM_ELEMENTS
	uint32_t g_shift;                           // (*)
	uint32_t g_num_workgroups;                  // == NUMBER_OF_WORKGROUPS as defined in the section above
	uint32_t g_num_blocks_per_workgroup;        // == NUM_BLOCKS_PER_WORKGROUP
};
}        // namespace Rendering

namespace std
{
template <>
struct hash<Rendering::M_Vertex3D>
{
	size_t operator()(Rendering::M_Vertex3D const &vertex) const
	{
		// Hash the position, color, normal, and texCoord
		size_t hash1 = hash<glm::vec3>()(vertex.pos);
		size_t hash2 = hash<glm::vec3>()(vertex.normal);
		size_t hash3 = hash<glm::vec3>()(vertex.tangent);
		size_t hash4 = hash<glm::vec2>()(vertex.uv);

		// Combine the hashes using bitwise operations
		size_t result = hash1;
		result        = (result * 31) + hash2;
		result        = (result * 31) + hash3;
		result        = (result * 31) + hash4;

		return result;
	}
};
}        // namespace std

#endif        // RENDERINGSTRUCTS_HPP
