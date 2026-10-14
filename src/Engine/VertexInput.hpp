//
// Created by carlo on 2024-09-26.
//

#ifndef VERTEX_INPUT_HPP
#define VERTEX_INPUT_HPP

namespace engine
{
class Vertex_Input
{
  public:
	enum Attribs
	{
		Int_Value,
		Float_Value,
		Vec2,
		Vec3,
		Vec4,
		U8_Vec3,
		U8_Vec4,
		Color_32
	};
	size_t Get_Size_From(Attribs attribs)
	{
		switch (attribs)
		{
			case Attribs::Int_Value:
				return sizeof(int);
				break;
			case Attribs::Float_Value:
				return sizeof(float);
				break;
			case Attribs::Vec2:
				return sizeof(glm::vec2);
				break;
			case Attribs::Vec3:
				return sizeof(glm::vec3);
				break;
			case Attribs::Vec4:
				return sizeof(glm::vec4);
				break;
			case Attribs::U8_Vec3:
				return sizeof(glm::uvec3);
				break;
			case Attribs::U8_Vec4:
				return sizeof(glm::uvec4);
				break;
			case Attribs::Color_32:
				return sizeof(int);
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return 0;
	}

	vk::Format Get_Format_From_Attrib(Attribs attribs)
	{
		switch (attribs)
		{
			case Attribs::Int_Value:
				return vk::Format::eR32Sint;
				break;
			case Attribs::Float_Value:
				return vk::Format::eR32Sfloat;
				break;
			case Attribs::Vec2:
				return vk::Format::eR32G32Sfloat;
				break;
			case Attribs::Vec3:
				return vk::Format::eR32G32B32Sfloat;
				break;
			case Attribs::Vec4:
				return vk::Format::eR32G32B32A32Sfloat;
				break;
			case Attribs::U8_Vec3:
				return vk::Format::eR8G8B8Unorm;
				break;
			case Attribs::U8_Vec4:
				return vk::Format::eR8G8B8A8Unorm;
				break;
			case Attribs::Color_32:
				return vk::Format::eR8G8B8A8Snorm;
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return vk::Format::eUndefined;
	}

	engine::Vertex_Input *Add_Vertex_Input_Binding(uint32_t buffer_binding, uint32_t stride)
	{
		auto bind_desc = vk::VertexInputBindingDescription()
		                    .setBinding(buffer_binding)
		                    .setStride(stride)
		                    .setInputRate(vk::VertexInputRate::eVertex);
		binding_description.push_back(bind_desc);
		return this;
	}

	engine::Vertex_Input *Add_Vertex_Attrib(Attribs attribs, uint32_t binding, uint32_t offset, uint32_t location)
	{
		auto vertex_attrib = vk::VertexInputAttributeDescription()
		                        .setBinding(binding)
		                        .setLocation(location)
		                        .setFormat(Get_Format_From_Attrib(attribs))
		                        .setOffset(offset);
		input_description.push_back(vertex_attrib);
		return this;
	}

	std::vector<size_t>                              vertex_offsets;
	int                                              attrib_size = 0;
	std::vector<vk::VertexInputAttributeDescription> input_description;
	std::vector<vk::VertexInputBindingDescription>   binding_description;
};

struct Vertex_2d
{
	float pos[2];
	float uv[2];

	static engine::Vertex_Input Get_Vertex_Input()
	{
		engine::Vertex_Input vertex_layout;
		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec2, 0, offsetof(Vertex_2d, pos), 0);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(Vertex_2d));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec2, 0, offsetof(Vertex_2d, uv), 1);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(Vertex_2d));
		return vertex_layout;
	}

	static std::vector<Vertex_2d> Get_Quad_Vertices()
	{
		std::vector<Vertex_2d> quad_vertices = {
		    {{-1.0f, 1.0f}, {0.0f, 1.0f}},
		    {{-1.0f, -1.0f}, {0.0f, 0.0f}},
		    {{1.0f, -1.0f}, {1.0f, 0.0f}},
		    {{1.0f, 1.0f}, {1.0f, 1.0f}}};
		return quad_vertices;
	}

	static std::vector<uint32_t> Get_Quad_Indices()
	{
		std::vector<uint32_t> indices = {
		    0, 1, 2,        // First triangle (top-left, bottom-left, bottom-right)
		    2, 3, 0         // Second triangle (bottom-right, top-right, top-left)
		};
		return indices;
	};
};

struct M_Vertex_3d
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;
	int       id;

	bool operator==(const M_Vertex_3d &other) const
	{
		return pos == other.pos && uv == other.uv && normal == other.normal && tangent == other.tangent && id == other.id;
	}

	static engine::Vertex_Input Get_Vertex_Input()
	{
		engine::Vertex_Input vertex_layout;
		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec3, 0, offsetof(M_Vertex_3d, pos), 0);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(M_Vertex_3d));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec3, 0, offsetof(M_Vertex_3d, normal), 1);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(M_Vertex_3d));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec3, 0, offsetof(M_Vertex_3d, tangent), 2);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(M_Vertex_3d));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec2, 0, offsetof(M_Vertex_3d, uv), 3);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(M_Vertex_3d));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Int_Value, 0, offsetof(M_Vertex_3d, id), 4);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(M_Vertex_3d));

		return vertex_layout;
	}
};
struct D_Vertex_3d
{
	glm::vec3 pos;

	// Unit cube centered at the origin, for scaling by the voxel cell size.
	static std::vector<D_Vertex_3d> Get_Cube_Vertices()
	{
		return {
		    {{-0.5f, -0.5f, -0.5f}},
		    {{ 0.5f, -0.5f, -0.5f}},
		    {{ 0.5f,  0.5f, -0.5f}},
		    {{-0.5f,  0.5f, -0.5f}},
		    {{-0.5f, -0.5f,  0.5f}},
		    {{ 0.5f, -0.5f,  0.5f}},
		    {{ 0.5f,  0.5f,  0.5f}},
		    {{-0.5f,  0.5f,  0.5f}}
		};
	}

	static std::vector<uint32_t> Get_Cube_Indices()
	{
		// Counterclockwise when viewed from outside each face.
		return {
		    4, 5, 6, 6, 7, 4, // +Z
		    1, 0, 3, 3, 2, 1, // -Z
		    5, 1, 2, 2, 6, 5, // +X
		    0, 4, 7, 7, 3, 0, // -X
		    7, 6, 2, 2, 3, 7, // +Y
		    0, 1, 5, 5, 4, 0  // -Y
		};
	}

	bool operator==(const M_Vertex_3d &other) const
	{
		return pos == other.pos;
	}

	static engine::Vertex_Input Get_Vertex_Input()
	{
		engine::Vertex_Input vertex_layout;
		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec3, 0, offsetof(D_Vertex_3d, pos), 0);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(D_Vertex_3d));
		return vertex_layout;
	}
};

struct Gs_Vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
	int       id;

	static engine::Vertex_Input Get_Vertex_Input()
	{
		engine::Vertex_Input vertex_layout;
		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec2, 0, offsetof(Gs_Vertex, pos), 0);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(Gs_Vertex));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Vec2, 0, offsetof(Gs_Vertex, uv), 0);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(Gs_Vertex));

		vertex_layout.Add_Vertex_Attrib(engine::Vertex_Input::Int_Value, 0, offsetof(Gs_Vertex, id), 1);
		vertex_layout.Add_Vertex_Input_Binding(0, sizeof(Gs_Vertex));
		return vertex_layout;
	}
};
}        // namespace engine

namespace std
{
template <>
struct hash<engine::M_Vertex_3d>
{
	size_t operator()(engine::M_Vertex_3d const &vertex) const
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

#endif        // VERTEX_INPUT_HPP
