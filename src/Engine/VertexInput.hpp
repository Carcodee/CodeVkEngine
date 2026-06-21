//
// Created by carlo on 2024-09-26.
//

#ifndef VERTEXINPUT_HPP
#define VERTEXINPUT_HPP

namespace ENGINE
{
class VertexInput
{
  public:
	enum Attribs
	{
		INT,
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		U8VEC3,
		U8VEC4,
		COLOR_32
	};
	size_t GetSizeFrom(Attribs attribs)
	{
		switch (attribs)
		{
			case Attribs::INT:
				return sizeof(int);
				break;
			case Attribs::FLOAT:
				return sizeof(float);
				break;
			case Attribs::VEC2:
				return sizeof(glm::vec2);
				break;
			case Attribs::VEC3:
				return sizeof(glm::vec3);
				break;
			case Attribs::VEC4:
				return sizeof(glm::vec4);
				break;
			case Attribs::U8VEC3:
				return sizeof(glm::uvec3);
				break;
			case Attribs::U8VEC4:
				return sizeof(glm::uvec4);
				break;
			case Attribs::COLOR_32:
				return sizeof(int);
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return 0;
	}

	vk::Format GetFormatFromAttrib(Attribs attribs)
	{
		switch (attribs)
		{
			case Attribs::INT:
				return vk::Format::eR32Sint;
				break;
			case Attribs::FLOAT:
				return vk::Format::eR32Sfloat;
				break;
			case Attribs::VEC2:
				return vk::Format::eR32G32Sfloat;
				break;
			case Attribs::VEC3:
				return vk::Format::eR32G32B32Sfloat;
				break;
			case Attribs::VEC4:
				return vk::Format::eR32G32B32A32Sfloat;
				break;
			case Attribs::U8VEC3:
				return vk::Format::eR8G8B8Unorm;
				break;
			case Attribs::U8VEC4:
				return vk::Format::eR8G8B8A8Unorm;
				break;
			case Attribs::COLOR_32:
				return vk::Format::eR8G8B8A8Snorm;
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return vk::Format::eUndefined;
	}

	VertexInput *AddVertexInputBinding(uint32_t bufferBinding, uint32_t stride)
	{
		auto bindDesc = vk::VertexInputBindingDescription()
		                    .setBinding(bufferBinding)
		                    .setStride(stride)
		                    .setInputRate(vk::VertexInputRate::eVertex);
		bindingDescription.push_back(bindDesc);
		return this;
	}

	VertexInput *AddVertexAttrib(Attribs attribs, uint32_t binding, uint32_t offset, uint32_t location)
	{
		auto vertexAttrib = vk::VertexInputAttributeDescription()
		                        .setBinding(binding)
		                        .setLocation(location)
		                        .setFormat(GetFormatFromAttrib(attribs))
		                        .setOffset(offset);
		inputDescription.push_back(vertexAttrib);
		return this;
	}

	std::vector<size_t>                              vertexOffsets;
	int                                              attribSize = 0;
	std::vector<vk::VertexInputAttributeDescription> inputDescription;
	std::vector<vk::VertexInputBindingDescription>   bindingDescription;
};

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
}        // namespace ENGINE

namespace std
{
template <>
struct hash<ENGINE::M_Vertex3D>
{
	size_t operator()(ENGINE::M_Vertex3D const &vertex) const
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

#endif        // VERTEXINPUT_HPP
