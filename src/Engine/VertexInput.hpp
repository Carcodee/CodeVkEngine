//
// Created by carlo on 2024-09-26.
//

#ifndef VERTEX_INPUT_HPP
#define VERTEX_INPUT_HPP

namespace engine
{
class vertex_input
{
  public:
	enum attribs
	{
		int_value,
		float_value,
		vec2,
		vec3,
		vec4,
		u8_vec3,
		u8_vec4,
		color_32
	};
	size_t get_size_from(attribs attribs)
	{
		switch (attribs)
		{
			case attribs::int_value:
				return sizeof(int);
				break;
			case attribs::float_value:
				return sizeof(float);
				break;
			case attribs::vec2:
				return sizeof(glm::vec2);
				break;
			case attribs::vec3:
				return sizeof(glm::vec3);
				break;
			case attribs::vec4:
				return sizeof(glm::vec4);
				break;
			case attribs::u8_vec3:
				return sizeof(glm::uvec3);
				break;
			case attribs::u8_vec4:
				return sizeof(glm::uvec4);
				break;
			case attribs::color_32:
				return sizeof(int);
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return 0;
	}

	vk::Format get_format_from_attrib(attribs attribs)
	{
		switch (attribs)
		{
			case attribs::int_value:
				return vk::Format::eR32Sint;
				break;
			case attribs::float_value:
				return vk::Format::eR32Sfloat;
				break;
			case attribs::vec2:
				return vk::Format::eR32G32Sfloat;
				break;
			case attribs::vec3:
				return vk::Format::eR32G32B32Sfloat;
				break;
			case attribs::vec4:
				return vk::Format::eR32G32B32A32Sfloat;
				break;
			case attribs::u8_vec3:
				return vk::Format::eR8G8B8Unorm;
				break;
			case attribs::u8_vec4:
				return vk::Format::eR8G8B8A8Unorm;
				break;
			case attribs::color_32:
				return vk::Format::eR8G8B8A8Snorm;
				break;
			default:
				assert(false && "Invalid attrib type");
				break;
		}
		return vk::Format::eUndefined;
	}

	engine::vertex_input *add_vertex_input_binding(uint32_t buffer_binding, uint32_t stride)
	{
		auto bind_desc = vk::VertexInputBindingDescription()
		                    .setBinding(buffer_binding)
		                    .setStride(stride)
		                    .setInputRate(vk::VertexInputRate::eVertex);
		binding_description.push_back(bind_desc);
		return this;
	}

	engine::vertex_input *add_vertex_attrib(attribs attribs, uint32_t binding, uint32_t offset, uint32_t location)
	{
		auto vertex_attrib = vk::VertexInputAttributeDescription()
		                        .setBinding(binding)
		                        .setLocation(location)
		                        .setFormat(get_format_from_attrib(attribs))
		                        .setOffset(offset);
		input_description.push_back(vertex_attrib);
		return this;
	}

	std::vector<size_t>                              vertex_offsets;
	int                                              attrib_size = 0;
	std::vector<vk::VertexInputAttributeDescription> input_description;
	std::vector<vk::VertexInputBindingDescription>   binding_description;
};

struct vertex_2d
{
	float pos[2];
	float uv[2];

	static engine::vertex_input get_vertex_input()
	{
		engine::vertex_input vertex_layout;
		vertex_layout.add_vertex_attrib(engine::vertex_input::vec2, 0, offsetof(vertex_2d, pos), 0);
		vertex_layout.add_vertex_input_binding(0, sizeof(vertex_2d));

		vertex_layout.add_vertex_attrib(engine::vertex_input::vec2, 0, offsetof(vertex_2d, uv), 1);
		vertex_layout.add_vertex_input_binding(0, sizeof(vertex_2d));
		return vertex_layout;
	}

	static std::vector<vertex_2d> get_quad_vertices()
	{
		std::vector<vertex_2d> quad_vertices = {
		    {{-1.0f, 1.0f}, {0.0f, 1.0f}},
		    {{-1.0f, -1.0f}, {0.0f, 0.0f}},
		    {{1.0f, -1.0f}, {1.0f, 0.0f}},
		    {{1.0f, 1.0f}, {1.0f, 1.0f}}};
		return quad_vertices;
	}

	static std::vector<uint32_t> get_quad_indices()
	{
		std::vector<uint32_t> indices = {
		    0, 1, 2,        // First triangle (top-left, bottom-left, bottom-right)
		    2, 3, 0         // Second triangle (bottom-right, top-right, top-left)
		};
		return indices;
	};
};

struct m_vertex_3d
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 uv;
	int       id;

	bool operator==(const m_vertex_3d &other) const
	{
		return pos == other.pos && uv == other.uv && normal == other.normal && tangent == other.tangent && id == other.id;
	}

	static engine::vertex_input get_vertex_input()
	{
		engine::vertex_input vertex_layout;
		vertex_layout.add_vertex_attrib(engine::vertex_input::vec3, 0, offsetof(m_vertex_3d, pos), 0);
		vertex_layout.add_vertex_input_binding(0, sizeof(m_vertex_3d));

		vertex_layout.add_vertex_attrib(engine::vertex_input::vec3, 0, offsetof(m_vertex_3d, normal), 1);
		vertex_layout.add_vertex_input_binding(0, sizeof(m_vertex_3d));

		vertex_layout.add_vertex_attrib(engine::vertex_input::vec3, 0, offsetof(m_vertex_3d, tangent), 2);
		vertex_layout.add_vertex_input_binding(0, sizeof(m_vertex_3d));

		vertex_layout.add_vertex_attrib(engine::vertex_input::vec2, 0, offsetof(m_vertex_3d, uv), 3);
		vertex_layout.add_vertex_input_binding(0, sizeof(m_vertex_3d));

		vertex_layout.add_vertex_attrib(engine::vertex_input::int_value, 0, offsetof(m_vertex_3d, id), 4);
		vertex_layout.add_vertex_input_binding(0, sizeof(m_vertex_3d));

		return vertex_layout;
	}
};
struct d_vertex_3d
{
	glm::vec3 pos;

	// Unit cube centered at the origin, for scaling by the voxel cell size.
	static std::vector<d_vertex_3d> get_cube_vertices()
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

	static std::vector<uint32_t> get_cube_indices()
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

	bool operator==(const m_vertex_3d &other) const
	{
		return pos == other.pos;
	}

	static engine::vertex_input get_vertex_input()
	{
		engine::vertex_input vertex_layout;
		vertex_layout.add_vertex_attrib(engine::vertex_input::vec3, 0, offsetof(d_vertex_3d, pos), 0);
		vertex_layout.add_vertex_input_binding(0, sizeof(d_vertex_3d));
		return vertex_layout;
	}
};

struct gs_vertex
{
	glm::vec2 pos;
	glm::vec2 uv;
	int       id;

	static engine::vertex_input get_vertex_input()
	{
		engine::vertex_input vertex_layout;
		vertex_layout.add_vertex_attrib(engine::vertex_input::vec2, 0, offsetof(gs_vertex, pos), 0);
		vertex_layout.add_vertex_input_binding(0, sizeof(gs_vertex));

		vertex_layout.add_vertex_attrib(engine::vertex_input::vec2, 0, offsetof(gs_vertex, uv), 0);
		vertex_layout.add_vertex_input_binding(0, sizeof(gs_vertex));

		vertex_layout.add_vertex_attrib(engine::vertex_input::int_value, 0, offsetof(gs_vertex, id), 1);
		vertex_layout.add_vertex_input_binding(0, sizeof(gs_vertex));
		return vertex_layout;
	}
};
}        // namespace engine

namespace std
{
template <>
struct hash<engine::m_vertex_3d>
{
	size_t operator()(engine::m_vertex_3d const &vertex) const
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
