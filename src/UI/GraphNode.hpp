//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_HPP
#define GRAPH_NODE_HPP

namespace ui::nodes
{

namespace ed = ax::NodeEditor;
enum node_type
{
	n_root_node                = 1 << 0,
	n_render_node              = 1 << 1,
	n_vert_shader              = 1 << 2,
	n_frag_shader              = 1 << 3,
	n_comp_shader              = 1 << 4,
	n_col_attachment_structure = 1 << 5,
	n_depth_structure          = 1 << 6,
	n_push_constant            = 1 << 7,
	n_image_sampler            = 1 << 8,
	n_image_storage            = 1 << 9,
	n_depth_image_sampler      = 1 << 10,
	n_vertex_input             = 1 << 11,
	n_buffer                   = 1 << 12,
	n_none                     = 1 << 13,
};
static std::map<node_type, std::string> node_type_strings{
    {n_root_node, "N_ROOT_NODE"},
    {n_render_node, "N_RENDER_NODE"},
    {n_vert_shader, "N_VERT_SHADER"},
    {n_frag_shader, "N_FRAG_SHADER"},
    {n_comp_shader, "N_COMP_SHADER"},
    {n_col_attachment_structure, "N_COL_ATTACHMENT_STRUCTURE"},
    {n_depth_structure, "N_DEPTH_STRUCTURE"},
    {n_push_constant, "N_PUSH_CONSTANT"},
    {n_image_sampler, "N_IMAGE_SAMPLER"},
    {n_image_storage, "N_IMAGE_STORAGE"},
    {n_depth_image_sampler, "N_DEPTH_IMAGE_SAMPLER"},
    {n_vertex_input, "N_VERTEX_INPUT"},
    {n_buffer, "N_BUFFER"},
    {n_none, "N_NONE"},
};

enum primitive_node_type
{
	int_value,
	uint,
	vec2,
	vec3,
	vec4,
	size_value,
	string_value
};

struct link_info
{
	ed::LinkId id;
	ed::PinId  input_id;
	ed::PinId  output_id;
};

static bool is_valid_node_combination(node_type node_kind, node_type dst_node)
{
	if (node_kind & dst_node)
	{
		return true;
	}
	return false;
}
static std::set<node_type> get_node_type(node_type node_kind)
{
	std::set<node_type> nodes_find = {};
	if (node_kind & n_root_node)
	{
		nodes_find.insert(n_root_node);
	}
	if (node_kind & n_render_node)
	{
		nodes_find.insert(n_render_node);
	}
	if (node_kind & n_vert_shader)
	{
		nodes_find.insert(n_vert_shader);
	}
	if (node_kind & n_frag_shader)
	{
		nodes_find.insert(n_frag_shader);
	}
	if (node_kind & n_comp_shader)
	{
		nodes_find.insert(n_comp_shader);
	}
	if (node_kind & n_col_attachment_structure)
	{
		nodes_find.insert(n_col_attachment_structure);
	}
	if (node_kind & n_depth_structure)
	{
		nodes_find.insert(n_depth_structure);
	}
	if (node_kind & n_push_constant)
	{
		nodes_find.insert(n_push_constant);
	}
	if (node_kind & n_image_sampler)
	{
		nodes_find.insert(n_image_sampler);
	}
	if (node_kind & n_image_storage)
	{
		nodes_find.insert(n_image_storage);
	}
	if (node_kind & n_depth_image_sampler)
	{
		nodes_find.insert(n_depth_image_sampler);
	}
	if (node_kind & n_vertex_input)
	{
		nodes_find.insert(n_vertex_input);
	}
	if (node_kind & n_buffer)
	{
		nodes_find.insert(n_buffer);
	}
	if (nodes_find.empty())
	{
		nodes_find.insert(n_none);
		systems::logger::get_instance()->log_message("There was no nodes finded in nodeType structure");
	}
	return nodes_find;
}
struct pin_info
{
	std::string name{};
	node_type    node_kind = n_none;
	std::any    data     = std::any();
	ed::PinKind pin_kind{};
	int         id        = -1;
	pin_info    *linked_pin = nullptr;
	pin_info(std::string name, node_type node_kind)
	{
		this->name     = name;
		this->node_kind = node_kind;
	}

	bool has_data()
	{
		return data.has_value();
	}

	template <typename value_type>
	value_type get_data()
	{
		assert(has_data() && "Pin does not have data");
		return std::any_cast<value_type>(data);
	}

	void draw()
	{
		assert(node_kind != n_none);
		if (id > -1)
			ImGui::PushID(id);

		ed::BeginPin(id, pin_kind);
		if (pin_kind == ed::PinKind::Input)
		{
			ImGui::Text(name.c_str());
		}
		else
		{
			std::string name_text = name + "        ->";
			ImGui::Text(name_text.c_str());
		}
		ed::EndPin();

		if (id > -1)
			ImGui::PopID();
	}
};

// enums
struct enum_selectable
{
	std::string              name;
	std::vector<std::string> options;
	int                      selected_idx;

	void draw(const int &id = -1)
	{
		ImGui::PushItemWidth(100.0);
		if (id > -1)
			ImGui::PushID(id);

		ImGui::Text(name.c_str());
		int         index = selected_idx;
		std::string label = options[index].c_str();
		// temp solution
		ImGui::SliderInt(label.c_str(), &selected_idx, 0, options.size() - 1);

		if (id > -1)
			ImGui::PopID();
		ImGui::PopItemWidth();
	}

	template <typename value_type>
	value_type get_enum_from_index(std::map<int, value_type> &map)
	{
		return map.at(selected_idx);
	}
};

struct text_input
{
	std::string name;
	std::string content;

	void draw(const int &id = -1)
	{
		ImGui::PushItemWidth(100.0);
		if (id > -1)
			ImGui::PushID(id);
		constexpr size_t size = 128;
		char             input_t[size];
		std::strncpy(input_t, content.c_str(), size);
		if (ImGui::InputText(name.c_str(), input_t, size))
		{
			content = std::string(input_t);
		}
		if (id > -1)
			ImGui::PopID();
		ImGui::PopItemWidth();
	}
};

struct primitive_input
{
	std::string       name;
	primitive_node_type primitive_type;
	std::any          content;

	bool has_data()
	{
		return content.has_value();
	}

	template <typename value_type>
	value_type get_data()
	{
		assert(has_data() && "Primitive has no data");
		value_type data = std::any_cast<value_type>(content);
		return data;
	}

	void draw(const int &id = -1)
	{
		ImGui::PushItemWidth(100.0);
		if (id > -1)
			ImGui::PushID(id);
		int         int_data;
		int         u_intdata;
		glm::vec2   vec2_data;
		glm::vec3   vec3_data;
		glm::vec4   vec4_data;
		std::string string_data;
		size_t      size_data;
		int         size_temp;
		std::any    result;
		switch (primitive_type)
		{
			case int_value:
				int_data = get_data<int>();
				ImGui::InputInt(name.c_str(), &int_data, 1, 100);
				result = int_data;
				break;
			case uint:
				u_intdata = get_data<int>();
				ImGui::InputInt(name.c_str(), &u_intdata, 1, 100);
				result = u_intdata;
				break;
			case vec2:
				vec2_data = get_data<glm::vec2>();
				ImGui::InputFloat2(name.c_str(), glm::value_ptr(vec2_data));
				result = vec2_data;
				break;
			case vec3:
				vec3_data = get_data<glm::vec3>();
				ImGui::InputFloat3(name.c_str(), glm::value_ptr(vec3_data));
				result = vec3_data;
				break;
			case vec4:
				vec4_data = get_data<glm::vec4>();
				ImGui::InputFloat4(name.c_str(), glm::value_ptr(vec4_data));
				result = vec4_data;
				break;
			case size_value:
				size_data = get_data<size_t>();
				size_temp = static_cast<int>(size_data);
				ImGui::InputInt(name.c_str(), &size_temp);
				size_data = static_cast<size_t>(size_temp);
				result   = size_data;
				break;
			case string_value:
				assert(false && "Invalid case");
				break;
			default:
				assert(false && "Invalid case");
				break;
		}
		content = result;
		if (id > -1)
			ImGui::PopID();
		ImGui::PopItemWidth();
	}
};

// any
// fix this
struct scrollable
{
	std::string           name;
	primitive_node_type     primitive_type;
	std::vector<std::any> content;
	int                   selected_idx    = 0;
	int                   item_hightlight = 0;

	void draw(const int &id = -1)
	{
		int                      int_data = -1;
		std::vector<std::string> items;
		items.reserve(content.size());
		std::string string_data = "";
		int         idx        = 0;
		for (auto &item : content)
		{
			switch (primitive_type)
			{
				case int_value:
					int_data    = std::any_cast<int>(item);
					string_data = std::to_string(int_data).c_str();
				case uint:
					assert(false && "Invalid case");
					break;
				case vec3:
					assert(false && "Invalid case");
					break;
				case vec2:
					assert(false && "Invalid case");
					break;
				case string_value:
					string_data = std::any_cast<std::string>(item).c_str();
				default:
					break;
			}
			items.push_back(string_data.c_str());
			idx++;
		}

		ImGui::PushItemWidth(100.0);
		if (id > -1)
			ImGui::PushID(id);
		ImGui::Text(name.c_str());
		int         index    = selected_idx;
		std::string filename = std::filesystem::path(items[index]).filename().string();
		const char *label    = filename.c_str();
		// temp solution

		ImGui::SliderInt(label, &selected_idx, 0, content.size() - 1);

		if (id > -1)
			ImGui::PopID();
		ImGui::PopItemWidth();
	}

	template <typename value_type>
	value_type get_current()
	{
		return std::any_cast<value_type>(content.at(selected_idx));
	}
};
enum widget_type
{
	w_selectable,
	w_text_input,
	w_primitive
};
struct dynamic_structure
{
	struct node_widgets_infos
	{
		union
		{
			enum_selectable selectable_info;
			text_input      text_input_info;
			primitive_input primitive_info;
		};

		widget_type type;

		node_widgets_infos()
		{
		}

		node_widgets_infos(const node_widgets_infos &other) :
		    type(other.type)
		{
			switch (type)
			{
				case w_selectable:
					new (&selectable_info) enum_selectable(other.selectable_info);
					break;
				case w_text_input:
					new (&text_input_info) text_input(other.text_input_info);
					break;
				case w_primitive:
					new (&primitive_info) primitive_input(other.primitive_info);
					break;
			}
		}

		node_widgets_infos(const node_widgets_infos &&other) noexcept :
		    type(other.type)
		{
			switch (type)
			{
				case w_selectable:
					new (&selectable_info) enum_selectable(std::move(other.selectable_info));
					break;
				case w_text_input:
					new (&text_input_info) text_input(std::move(other.text_input_info));
					break;
				case w_primitive:
					new (&primitive_info) primitive_input(std::move(other.primitive_info));
					break;
			}
		}

		~node_widgets_infos()
		{
			destroy();
		}

		void destroy()
		{
			switch (type)
			{
				case w_selectable:
					selectable_info.~enum_selectable();
					break;
				case w_text_input:
					text_input_info.~text_input();
					break;
				case w_primitive:
					primitive_info.~primitive_input();
					break;
			}
		}

		node_widgets_infos &operator=(const node_widgets_infos &other)
		{
			if (this != &other)
			{
				destroy();
				type = other.type;
				switch (type)
				{
					case w_selectable:
						new (&selectable_info) enum_selectable(other.selectable_info);
						break;
					case w_text_input:
						new (&text_input_info) text_input(other.text_input_info);
						break;
					case w_primitive:
						new (&primitive_info) primitive_input(other.primitive_info);
						break;
				}
			}
			return *this;
		}

		node_widgets_infos &operator=(const node_widgets_infos &&other) noexcept
		{
			if (this != &other)
			{
				destroy();
				type = other.type;
				switch (type)
				{
					case w_selectable:
						new (&selectable_info) enum_selectable(std::move(other.selectable_info));
						break;
					case w_text_input:
						new (&text_input_info) text_input(std::move(other.text_input_info));
						break;
					case w_primitive:
						new (&primitive_info) primitive_input(std::move(other.primitive_info));
						break;
				}
			}
			return *this;
		}
	};
	std::string                     name;
	std::map<int, node_widgets_infos> widgets_infos;

	dynamic_structure(const dynamic_structure &other)
	{
		this->name         = other.name;
		this->widgets_infos = other.widgets_infos;
	}

	dynamic_structure(const std::string &name, enum_selectable &selectable)
	{
		this->name                      = name;
		node_widgets_infos base_widget_info = {};
		base_widget_info.type             = w_selectable;
		new (&base_widget_info.selectable_info) enum_selectable(selectable);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	dynamic_structure(const std::string &name, text_input &text_input_info)
	{
		this->name                      = name;
		node_widgets_infos base_widget_info = {};
		base_widget_info.type             = w_text_input;
		new (&base_widget_info.text_input_info) text_input(text_input_info);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	dynamic_structure(const std::string &name, primitive_input &primitive_info)
	{
		this->name                      = name;
		node_widgets_infos base_widget_info = {};
		base_widget_info.type             = w_primitive;
		new (&base_widget_info.primitive_info) primitive_input(primitive_info);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	~dynamic_structure()
	{
		for (auto &widget : widgets_infos)
		{
			widget.second.destroy();
		}
	}

	void add_option()
	{
		widgets_infos.try_emplace(widgets_infos.size(), widgets_infos.at(0));
	}

	void remove_last()
	{
		if (widgets_infos.size() == 1)
		{
			return;
		}

		auto it = widgets_infos.find(widgets_infos.size() - 1);
		if (it != widgets_infos.end())
		{
			widgets_infos.erase(it);
		}
		else
		{
			systems::logger::get_instance()->log("Invalid Key");
		}
	}

	void draw(const int &id = -1)
	{
		if (id > -1)
			ImGui::PushID(id);

		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::PushItemWidth(20);
		if (ImGui::Button("+"))
		{
			add_option();
		}
		ImGui::SameLine();
		if (ImGui::Button("-"))
		{
			remove_last();
		}
		ImGui::PopItemWidth();
		int w_id = 0;
		for (auto &widget_info : widgets_infos)
		{
			switch (widget_info.second.type)
			{
				case w_selectable:
					widget_info.second.selectable_info.draw(w_id++);
					break;
				case w_text_input:
					widget_info.second.text_input_info.draw(w_id++);
					break;
				case w_primitive:
					widget_info.second.primitive_info.draw(w_id++);
					break;
			}
		}
		if (id > -1)
			ImGui::PopID();
	}
};

struct multi_option
{
	std::string                     name;
	std::map<int, std::string>      options;
	std::map<int, text_input>        input_texts;
	std::map<int, enum_selectable>   selectables;
	std::map<int, scrollable>       scrollables;
	std::map<int, dynamic_structure> dynamic_structures;
	int                             selected_idx = 0;

	multi_option(const std::string &name, std::vector<std::string> options, std::map<int, text_input> input_texts,
	            std::map<int, enum_selectable> selectables, std::map<int, scrollable> scrollables, std::map<int, dynamic_structure> dynamic_structures)
	{
		this->name = name;
		for (int i = 0; i < options.size(); ++i)
		{
			this->options.try_emplace(i, options[i]);
		}
		this->input_texts        = input_texts;
		this->selectables       = selectables;
		this->scrollables       = scrollables;
		this->dynamic_structures = dynamic_structures;
	}

	void draw(const int &id = -1)
	{
		if (id > -1)
			ImGui::PushID(id);
		for (auto &option : options)
		{
			if (ImGui::RadioButton(option.second.c_str(), selected_idx == option.first))
			{
				selected_idx = option.first;
			}
		}
		if (input_texts.contains(selected_idx))
			input_texts.at(selected_idx).draw();

		if (selectables.contains(selected_idx))
			selectables.at(selected_idx).draw();

		if (scrollables.contains(selected_idx))
			scrollables.at(selected_idx).draw();

		if (dynamic_structures.contains(selected_idx))
			dynamic_structures.at(selected_idx).draw();

		if (id > -1)
			ImGui::PopID();
	}
};
struct button
{
	std::string name;

	std::function<void()> action;

	void draw(const int &id = -1)
	{
		if (id > -1)
			ImGui::PushID(id);

		if (ImGui::Button(name.c_str(), ImVec2{50, 50}))
		{
			action();
		}
		if (id > -1)
			ImGui::PopID();
	}
};

struct graph_node
{
	engine::render_graph                      *render_graph;
	::window_provider                           *window_provider;
	std::unordered_map<int, pin_info>          input_nodes;
	std::unordered_map<int, pin_info>          output_nodes;
	std::unordered_map<int, enum_selectable>   selectables;
	std::unordered_map<int, text_input>        text_inputs;
	std::unordered_map<int, primitive_input>   primitives;
	std::unordered_map<int, scrollable>       scrollables;
	std::unordered_map<int, ui::nodes::multi_option>      multi_options;
	std::unordered_map<int, dynamic_structure> dynamic_structures;
	// since all ids are not repeated this will represent properly all the widgets

	std::unordered_map<int, bool> drawable_widgets;
	std::unordered_map<int, bool> add_more_widgets;

	std::unordered_map<int, node_type>     graph_nodes_links;
	std::unordered_map<int, ui::nodes::graph_node *> *graph_nodes_ref       = nullptr;
	graph_node_res_manager                  *graph_node_res_manager = nullptr;

	std::string name;
	glm::vec2   pos;
	ed::NodeId  node_id;
	int        *widgets_id_gen = nullptr;
	int         global_id     = -1;
	bool        update_data   = false;
	bool        valid        = false;
	bool        first_frame   = true;

	std::unordered_map<std::string, std::function<void(ui::nodes::graph_node &)> *> callbacks;

	void update_info();

	template <typename f, typename s>
	std::vector<std::pair<f, s>> get_vector_from_map(std::unordered_map<f, s> &map)
	{
		std::vector<std::pair<f, s>> copied_map = {};
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			copied_map.push_back(*it);
		}
		return copied_map;
	}

	void sort();

	int run_callback(std::string callback);

	void draw();

	void toggle_draw(int id, bool draw_condition);

	void recompile_node();

	int add_link(int id, node_type node_kind = n_none);

	std::map<node_type, ui::nodes::graph_node *> get_graph_node_ref(node_type node_kind);

	pin_info *get_input_data_by_id(int id);

	pin_info *get_input_data_by_name(const std::string &name);

	pin_info *get_output_data_by_id(int id);

	pin_info *get_output_data_by_name(const std::string &name);

	bool has_output(node_type type);

	bool has_input(node_type type);

	void set_ouput_data(const std::string &name, std::any data);

	void set_ouput_data(int id, std::any data);

	scrollable *get_scrollable(const std::string &name);

	ui::nodes::multi_option *get_multi_option(const std::string name);

	text_input *get_text_input(const std::string &name);

	dynamic_structure *get_dynamic_structure(const std::string &name);
};

}        // namespace ui::nodes

#endif        // GRAPH_NODE_HPP
