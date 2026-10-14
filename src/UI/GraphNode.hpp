//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_HPP
#define GRAPH_NODE_HPP

namespace ui::nodes
{

namespace ed = ax::NodeEditor;
enum Node_Type
{
	N_Root_Node                = 1 << 0,
	N_Render_Node              = 1 << 1,
	N_Vert_Shader              = 1 << 2,
	N_Frag_Shader              = 1 << 3,
	N_Comp_Shader              = 1 << 4,
	N_Col_Attachment_Structure = 1 << 5,
	N_Depth_Structure          = 1 << 6,
	N_Push_Constant            = 1 << 7,
	N_Image_Sampler            = 1 << 8,
	N_Image_Storage            = 1 << 9,
	N_Depth_Image_Sampler      = 1 << 10,
	N_Vertex_Input             = 1 << 11,
	N_Buffer                   = 1 << 12,
	N_None                     = 1 << 13,
};
static std::map<Node_Type, std::string> node_type_strings{
    {N_Root_Node, "N_ROOT_NODE"},
    {N_Render_Node, "N_RENDER_NODE"},
    {N_Vert_Shader, "N_VERT_SHADER"},
    {N_Frag_Shader, "N_FRAG_SHADER"},
    {N_Comp_Shader, "N_COMP_SHADER"},
    {N_Col_Attachment_Structure, "N_COL_ATTACHMENT_STRUCTURE"},
    {N_Depth_Structure, "N_DEPTH_STRUCTURE"},
    {N_Push_Constant, "N_PUSH_CONSTANT"},
    {N_Image_Sampler, "N_IMAGE_SAMPLER"},
    {N_Image_Storage, "N_IMAGE_STORAGE"},
    {N_Depth_Image_Sampler, "N_DEPTH_IMAGE_SAMPLER"},
    {N_Vertex_Input, "N_VERTEX_INPUT"},
    {N_Buffer, "N_BUFFER"},
    {N_None, "N_NONE"},
};

enum Primitive_Node_Type
{
	Int_Value,
	Uint,
	Vec2,
	Vec3,
	Vec4,
	Size_Value,
	String_Value
};

struct Link_Info
{
	ed::LinkId id;
	ed::PinId  input_id;
	ed::PinId  output_id;
};

static bool Is_Valid_Node_Combination(Node_Type node_kind, Node_Type dst_node)
{
	if (node_kind & dst_node)
	{
		return true;
	}
	return false;
}
static std::set<Node_Type> Get_Node_Type(Node_Type node_kind)
{
	std::set<Node_Type> nodes_find = {};
	if (node_kind & N_Root_Node)
	{
		nodes_find.insert(N_Root_Node);
	}
	if (node_kind & N_Render_Node)
	{
		nodes_find.insert(N_Render_Node);
	}
	if (node_kind & N_Vert_Shader)
	{
		nodes_find.insert(N_Vert_Shader);
	}
	if (node_kind & N_Frag_Shader)
	{
		nodes_find.insert(N_Frag_Shader);
	}
	if (node_kind & N_Comp_Shader)
	{
		nodes_find.insert(N_Comp_Shader);
	}
	if (node_kind & N_Col_Attachment_Structure)
	{
		nodes_find.insert(N_Col_Attachment_Structure);
	}
	if (node_kind & N_Depth_Structure)
	{
		nodes_find.insert(N_Depth_Structure);
	}
	if (node_kind & N_Push_Constant)
	{
		nodes_find.insert(N_Push_Constant);
	}
	if (node_kind & N_Image_Sampler)
	{
		nodes_find.insert(N_Image_Sampler);
	}
	if (node_kind & N_Image_Storage)
	{
		nodes_find.insert(N_Image_Storage);
	}
	if (node_kind & N_Depth_Image_Sampler)
	{
		nodes_find.insert(N_Depth_Image_Sampler);
	}
	if (node_kind & N_Vertex_Input)
	{
		nodes_find.insert(N_Vertex_Input);
	}
	if (node_kind & N_Buffer)
	{
		nodes_find.insert(N_Buffer);
	}
	if (nodes_find.empty())
	{
		nodes_find.insert(N_None);
		systems::Logger::Get_Instance()->Log_Message("There was no nodes finded in nodeType structure");
	}
	return nodes_find;
}
struct Pin_Info
{
	std::string name{};
	Node_Type    node_kind = N_None;
	std::any    data     = std::any();
	ed::PinKind pin_kind{};
	int         id        = -1;
	Pin_Info    *linked_pin = nullptr;
	Pin_Info(std::string name, Node_Type node_kind)
	{
		this->name     = name;
		this->node_kind = node_kind;
	}

	bool Has_Data()
	{
		return data.has_value();
	}

	template <typename Value_Type>
	Value_Type Get_Data()
	{
		assert(Has_Data() && "Pin does not have data");
		return std::any_cast<Value_Type>(data);
	}

	void Draw()
	{
		assert(node_kind != N_None);
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
struct Enum_Selectable
{
	std::string              name;
	std::vector<std::string> options;
	int                      selected_idx;

	void Draw(const int &id = -1)
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

	template <typename Value_Type>
	Value_Type Get_Enum_From_Index(std::map<int, Value_Type> &map)
	{
		return map.at(selected_idx);
	}
};

struct Text_Input
{
	std::string name;
	std::string content;

	void Draw(const int &id = -1)
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

struct Primitive_Input
{
	std::string       name;
	Primitive_Node_Type primitive_type;
	std::any          content;

	bool Has_Data()
	{
		return content.has_value();
	}

	template <typename Value_Type>
	Value_Type Get_Data()
	{
		assert(Has_Data() && "Primitive has no data");
		Value_Type data = std::any_cast<Value_Type>(content);
		return data;
	}

	void Draw(const int &id = -1)
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
			case Int_Value:
				int_data = Get_Data<int>();
				ImGui::InputInt(name.c_str(), &int_data, 1, 100);
				result = int_data;
				break;
			case Uint:
				u_intdata = Get_Data<int>();
				ImGui::InputInt(name.c_str(), &u_intdata, 1, 100);
				result = u_intdata;
				break;
			case Vec2:
				vec2_data = Get_Data<glm::vec2>();
				ImGui::InputFloat2(name.c_str(), glm::value_ptr(vec2_data));
				result = vec2_data;
				break;
			case Vec3:
				vec3_data = Get_Data<glm::vec3>();
				ImGui::InputFloat3(name.c_str(), glm::value_ptr(vec3_data));
				result = vec3_data;
				break;
			case Vec4:
				vec4_data = Get_Data<glm::vec4>();
				ImGui::InputFloat4(name.c_str(), glm::value_ptr(vec4_data));
				result = vec4_data;
				break;
			case Size_Value:
				size_data = Get_Data<size_t>();
				size_temp = static_cast<int>(size_data);
				ImGui::InputInt(name.c_str(), &size_temp);
				size_data = static_cast<size_t>(size_temp);
				result   = size_data;
				break;
			case String_Value:
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
struct Scrollable
{
	std::string           name;
	Primitive_Node_Type     primitive_type;
	std::vector<std::any> content;
	int                   selected_idx    = 0;
	int                   item_hightlight = 0;

	void Draw(const int &id = -1)
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
				case Int_Value:
					int_data    = std::any_cast<int>(item);
					string_data = std::to_string(int_data).c_str();
				case Uint:
					assert(false && "Invalid case");
					break;
				case Vec3:
					assert(false && "Invalid case");
					break;
				case Vec2:
					assert(false && "Invalid case");
					break;
				case String_Value:
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

	template <typename Value_Type>
	Value_Type Get_Current()
	{
		return std::any_cast<Value_Type>(content.at(selected_idx));
	}
};
enum Widget_Type
{
	W_Selectable,
	W_Text_Input,
	W_Primitive
};
struct Dynamic_Structure
{
	struct Node_Widgets_Infos
	{
		union
		{
			Enum_Selectable selectable_info;
			Text_Input      text_input_info;
			Primitive_Input primitive_info;
		};

		Widget_Type type;

		Node_Widgets_Infos()
		{
		}

		Node_Widgets_Infos(const Node_Widgets_Infos &other) :
		    type(other.type)
		{
			switch (type)
			{
				case W_Selectable:
					new (&selectable_info) Enum_Selectable(other.selectable_info);
					break;
				case W_Text_Input:
					new (&text_input_info) Text_Input(other.text_input_info);
					break;
				case W_Primitive:
					new (&primitive_info) Primitive_Input(other.primitive_info);
					break;
			}
		}

		Node_Widgets_Infos(const Node_Widgets_Infos &&other) noexcept :
		    type(other.type)
		{
			switch (type)
			{
				case W_Selectable:
					new (&selectable_info) Enum_Selectable(std::move(other.selectable_info));
					break;
				case W_Text_Input:
					new (&text_input_info) Text_Input(std::move(other.text_input_info));
					break;
				case W_Primitive:
					new (&primitive_info) Primitive_Input(std::move(other.primitive_info));
					break;
			}
		}

		~Node_Widgets_Infos()
		{
			Destroy();
		}

		void Destroy()
		{
			switch (type)
			{
				case W_Selectable:
					selectable_info.~Enum_Selectable();
					break;
				case W_Text_Input:
					text_input_info.~Text_Input();
					break;
				case W_Primitive:
					primitive_info.~Primitive_Input();
					break;
			}
		}

		Node_Widgets_Infos &operator=(const Node_Widgets_Infos &other)
		{
			if (this != &other)
			{
				Destroy();
				type = other.type;
				switch (type)
				{
					case W_Selectable:
						new (&selectable_info) Enum_Selectable(other.selectable_info);
						break;
					case W_Text_Input:
						new (&text_input_info) Text_Input(other.text_input_info);
						break;
					case W_Primitive:
						new (&primitive_info) Primitive_Input(other.primitive_info);
						break;
				}
			}
			return *this;
		}

		Node_Widgets_Infos &operator=(const Node_Widgets_Infos &&other) noexcept
		{
			if (this != &other)
			{
				Destroy();
				type = other.type;
				switch (type)
				{
					case W_Selectable:
						new (&selectable_info) Enum_Selectable(std::move(other.selectable_info));
						break;
					case W_Text_Input:
						new (&text_input_info) Text_Input(std::move(other.text_input_info));
						break;
					case W_Primitive:
						new (&primitive_info) Primitive_Input(std::move(other.primitive_info));
						break;
				}
			}
			return *this;
		}
	};
	std::string                     name;
	std::map<int, Node_Widgets_Infos> widgets_infos;

	Dynamic_Structure(const Dynamic_Structure &other)
	{
		this->name         = other.name;
		this->widgets_infos = other.widgets_infos;
	}

	Dynamic_Structure(const std::string &name, Enum_Selectable &selectable)
	{
		this->name                      = name;
		Node_Widgets_Infos base_widget_info = {};
		base_widget_info.type             = W_Selectable;
		new (&base_widget_info.selectable_info) Enum_Selectable(selectable);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	Dynamic_Structure(const std::string &name, Text_Input &text_input_info)
	{
		this->name                      = name;
		Node_Widgets_Infos base_widget_info = {};
		base_widget_info.type             = W_Text_Input;
		new (&base_widget_info.text_input_info) Text_Input(text_input_info);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	Dynamic_Structure(const std::string &name, Primitive_Input &primitive_info)
	{
		this->name                      = name;
		Node_Widgets_Infos base_widget_info = {};
		base_widget_info.type             = W_Primitive;
		new (&base_widget_info.primitive_info) Primitive_Input(primitive_info);
		widgets_infos.try_emplace(widgets_infos.size(), std::move(base_widget_info));
	}

	~Dynamic_Structure()
	{
		for (auto &widget : widgets_infos)
		{
			widget.second.Destroy();
		}
	}

	void Add_Option()
	{
		widgets_infos.try_emplace(widgets_infos.size(), widgets_infos.at(0));
	}

	void Remove_Last()
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
			systems::Logger::Get_Instance()->Log("Invalid Key");
		}
	}

	void Draw(const int &id = -1)
	{
		if (id > -1)
			ImGui::PushID(id);

		ImGui::Text(name.c_str());
		ImGui::SameLine();
		ImGui::PushItemWidth(20);
		if (ImGui::Button("+"))
		{
			Add_Option();
		}
		ImGui::SameLine();
		if (ImGui::Button("-"))
		{
			Remove_Last();
		}
		ImGui::PopItemWidth();
		int w_id = 0;
		for (auto &widget_info : widgets_infos)
		{
			switch (widget_info.second.type)
			{
				case W_Selectable:
					widget_info.second.selectable_info.Draw(w_id++);
					break;
				case W_Text_Input:
					widget_info.second.text_input_info.Draw(w_id++);
					break;
				case W_Primitive:
					widget_info.second.primitive_info.Draw(w_id++);
					break;
			}
		}
		if (id > -1)
			ImGui::PopID();
	}
};

struct Multi_Option
{
	std::string                     name;
	std::map<int, std::string>      options;
	std::map<int, Text_Input>        input_texts;
	std::map<int, Enum_Selectable>   selectables;
	std::map<int, Scrollable>       scrollables;
	std::map<int, Dynamic_Structure> dynamic_structures;
	int                             selected_idx = 0;

	Multi_Option(const std::string &name, std::vector<std::string> options, std::map<int, Text_Input> input_texts,
	            std::map<int, Enum_Selectable> selectables, std::map<int, Scrollable> scrollables, std::map<int, Dynamic_Structure> dynamic_structures)
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

	void Draw(const int &id = -1)
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
			input_texts.at(selected_idx).Draw();

		if (selectables.contains(selected_idx))
			selectables.at(selected_idx).Draw();

		if (scrollables.contains(selected_idx))
			scrollables.at(selected_idx).Draw();

		if (dynamic_structures.contains(selected_idx))
			dynamic_structures.at(selected_idx).Draw();

		if (id > -1)
			ImGui::PopID();
	}
};
struct Button
{
	std::string name;

	std::function<void()> action;

	void Draw(const int &id = -1)
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

struct Graph_Node
{
	engine::Render_Graph                      *render_graph;
	::Window_Provider                           *window_provider;
	std::unordered_map<int, Pin_Info>          input_nodes;
	std::unordered_map<int, Pin_Info>          output_nodes;
	std::unordered_map<int, Enum_Selectable>   selectables;
	std::unordered_map<int, Text_Input>        text_inputs;
	std::unordered_map<int, Primitive_Input>   primitives;
	std::unordered_map<int, Scrollable>       scrollables;
	std::unordered_map<int, ui::nodes::Multi_Option>      multi_options;
	std::unordered_map<int, Dynamic_Structure> dynamic_structures;
	// since all ids are not repeated this will represent properly all the widgets

	std::unordered_map<int, bool> drawable_widgets;
	std::unordered_map<int, bool> add_more_widgets;

	std::unordered_map<int, Node_Type>     graph_nodes_links;
	std::unordered_map<int, ui::nodes::Graph_Node *> *graph_nodes_ref       = nullptr;
	Graph_Node_Res_Manager                  *graph_node_res_manager = nullptr;

	std::string name;
	glm::vec2   pos;
	ed::NodeId  node_id;
	int        *widgets_id_gen = nullptr;
	int         global_id     = -1;
	bool        update_data   = false;
	bool        valid        = false;
	bool        first_frame   = true;

	std::unordered_map<std::string, std::function<void(ui::nodes::Graph_Node &)> *> callbacks;

	void Update_Info();

	template <typename F, typename S>
	std::vector<std::pair<F, S>> Get_Vector_From_Map(std::unordered_map<F, S> &map)
	{
		std::vector<std::pair<F, S>> copied_map = {};
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			copied_map.push_back(*it);
		}
		return copied_map;
	}

	void Sort();

	int Run_Callback(std::string callback);

	void Draw();

	void Toggle_Draw(int id, bool draw_condition);

	void Recompile_Node();

	int Add_Link(int id, Node_Type node_kind = N_None);

	std::map<Node_Type, ui::nodes::Graph_Node *> Get_Graph_Node_Ref(Node_Type node_kind);

	Pin_Info *Get_Input_Data_By_Id(int id);

	Pin_Info *Get_Input_Data_By_Name(const std::string &name);

	Pin_Info *Get_Output_Data_By_Id(int id);

	Pin_Info *Get_Output_Data_By_Name(const std::string &name);

	bool Has_Output(Node_Type type);

	bool Has_Input(Node_Type type);

	void Set_Ouput_Data(const std::string &name, std::any data);

	void Set_Ouput_Data(int id, std::any data);

	Scrollable *Get_Scrollable(const std::string &name);

	ui::nodes::Multi_Option *Get_Multi_Option(const std::string name);

	Text_Input *Get_Text_Input(const std::string &name);

	Dynamic_Structure *Get_Dynamic_Structure(const std::string &name);
};

}        // namespace ui::nodes

#endif        // GRAPH_NODE_HPP
