//

// Created by carlo on 2025-01-02.
//

#ifndef WIDGETS_HPP
#define WIDGETS_HPP

namespace ui
{
enum Widgets_Property
{
	Drag,
	Drop
};

template <typename Value_Type>
concept Has_Name = requires(Value_Type t) {
	{ t.name } -> std::convertible_to<std::string>;
};

template <Has_Name Value_Type>
static Value_Type *Get_From_Name_In_Map(std::map<int, Value_Type> &map_to_search, const std::string name)
{
	for (auto &item : map_to_search)
	{
		if (item.second.name == name)
		{
			return &item.second;
		}
	}
	return nullptr;
}

template <Has_Name Value_Type>
static Value_Type *Get_From_Name_In_Map(std::unordered_map<int, Value_Type> &map_to_search, const std::string name)
{
	for (auto &item : map_to_search)
	{
		if (item.second.name == name)
		{
			return &item.second;
		}
	}
	return nullptr;
}
template <Has_Name Value_Type>
static int Get_Id_From_Map(std::map<int, Value_Type> &map_to_search, const std::string name)
{
	for (auto &item : map_to_search)
	{
		if (item.second.name == name)
		{
			return &item.first;
		}
	}
	return -1;
}

template <typename Value_Type>
static void Use_Drag_Property(std::string name, Value_Type &payload, size_t payload_size)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload(name.c_str(), &payload, payload_size);
		ImGui::EndDragDropSource();
	}
}

template <typename Value_Type>
static Value_Type Use_Drop_Property(std::string name)
{
	if (ImGui::BeginDragDropTarget())
	{
		Value_Type data = nullptr;
		if (const ImGuiPayload *imgui_payload = ImGui::AcceptDragDropPayload(name.c_str()))
		{
			//				const char* data = (const char*)payload->Data;
			data = *(Value_Type *) imgui_payload->Data;

			ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
			ImGui::BeginTooltip();
			ImGui::Text("+");

			ImGui::EndTooltip();
		}
		ImGui::EndDragDropTarget();
		return data;
	}
	return nullptr;
}

struct I_Widget
{
	virtual ~I_Widget()               = default;
	virtual void Display_Properties() = 0;
};

struct I_Node_Widget
{
	std::string name;
	virtual ~I_Node_Widget()    = default;
	virtual void Draw(int id) = 0;
};

struct Texture_Viewer : I_Widget
{
	engine::Image_View        *curr_image_view = nullptr;
	std::string               label_name;
	std::set<Widgets_Property> properties;
	~Texture_Viewer() override = default;

	void Display_Properties() override
	{
		for (auto &prop : properties)
		{
			switch (prop)
			{
				case Drag:
					Use_Drag_Property("TEXTURE_NAME", curr_image_view, sizeof(engine::Image_View *));
					break;
				case Drop:
					engine::Image_View *new_img_view = Use_Drop_Property<engine::Image_View *>("TEXTURE_NAME");
					if (new_img_view != nullptr)
					{
						curr_image_view = new_img_view;
					}
					break;
			}
		}
	}

	engine::Image_View *Display_Texture(std::string name, engine::Image_View *image_view_handle, ImTextureID texture_id,
	                                  glm::vec2 size)
	{
		label_name           = name;
		this->curr_image_view = image_view_handle;

		ImGui::PushID(name.c_str());
		ImGui::SeparatorText(name.c_str());
		ImGui::ImageButton(name.c_str(), texture_id, ImVec2{size.x, size.y});
		Display_Properties();
		ImGui::PopID();

		return this->curr_image_view;
	}

	void Add_Property(Widgets_Property property)
	{
		properties.emplace(property);
	}
};

namespace nodes
{
namespace ed = ax::NodeEditor;
// note that this only handle copyable data types

struct Graph_Node_Builder
{
	ed::NodeId                                                          node_id;
	std::unordered_map<int, Pin_Info>                                    input_nodes        = {};
	std::unordered_map<int, Pin_Info>                                    output_nodes       = {};
	std::unordered_map<int, Enum_Selectable>                             selectables       = {};
	std::unordered_map<int, Text_Input>                                  text_inputs        = {};
	std::unordered_map<int, Primitive_Input>                             primitives        = {};
	std::unordered_map<int, Scrollable>                                 scrollables       = {};
	std::unordered_map<int, ui::nodes::Multi_Option>                                multi_options      = {};
	std::unordered_map<int, Dynamic_Structure>                           dynamic_structures = {};
	std::unordered_map<int, bool>                                       drawable_widgets   = {};
	std::unordered_map<int, bool>                                       add_more_widgets    = {};
	std::unordered_map<std::string, std::function<void(ui::nodes::Graph_Node &)> *> callbacks;

	std::string name = "";
	glm::vec2   pos  = glm::vec2(0.0);

	Graph_Node_Builder &Set_Node_Id(ed::NodeId id, std::string name)
	{
		node_id     = id;
		this->name = name;
		return *this;
	}

	Graph_Node_Builder &Set_Node_Name(std::string name)
	{
		this->name = name;
		return *this;
	}

	Graph_Node_Builder &Set_Position(glm::vec2 pos)
	{
		this->pos = pos;
		return *this;
	}

	Graph_Node_Builder &Add_Input(int id, Pin_Info pin_info, bool add_more = false)
	{
		pin_info.pin_kind = ed::PinKind::Input;
		pin_info.id      = id;
		input_nodes.try_emplace(id, pin_info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Output(int id, Pin_Info pin_info)
	{
		pin_info.pin_kind = ed::PinKind::Output;
		pin_info.id      = id;
		output_nodes.try_emplace(id, pin_info);
		drawable_widgets.try_emplace(id, true);
		return *this;
	}

	// GraphNodeBuilder& AddButton(int id, Button button)
	// {
	// buttons.try_emplace(id, button);
	// return *this;
	// }

	Graph_Node_Builder &Add_Selectable(int id, std::string name, std::vector<std::string> options, int selected_idx = 0, bool add_more = false)
	{
		selectables.try_emplace(id, Enum_Selectable{name, options, selected_idx});
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Text_Input(int id, Text_Input info, bool add_more = false)
	{
		text_inputs.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Scrollable(int id, Scrollable info, bool add_more = false)
	{
		scrollables.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Primitive_Data(int id, Primitive_Input info, bool add_more = false)
	{
		primitives.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Multi_Option(int id, ui::nodes::Multi_Option info, bool add_more = false)
	{
		multi_options.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	Graph_Node_Builder &Add_Dynamic_Structure(int id, Dynamic_Structure info, bool add_more = false)
	{
		dynamic_structures.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}
	Graph_Node_Builder &Add_Callback(std::string name, std::function<void(ui::nodes::Graph_Node &)> *callback)
	{
		if (callbacks.contains(name))
		{
			callbacks.at(name) = std::move(callback);
		}
		callbacks.try_emplace(name, std::move(callback));
		return *this;
	}

	ui::nodes::Graph_Node *Build(ui::nodes::Graph_Node *graph_node, engine::Render_Graph *render_graph, ::Window_Provider *window_provider)
	{
		// assert(nodeId.Get() > -1 && "Set the id before building");
		assert(!name.empty() && "Set a valid name");

		graph_node->name              = name;
		graph_node->node_id            = node_id;
		graph_node->input_nodes        = input_nodes;
		graph_node->output_nodes       = output_nodes;
		graph_node->selectables       = selectables;
		graph_node->text_inputs        = text_inputs;
		graph_node->primitives        = primitives;
		graph_node->scrollables       = scrollables;
		graph_node->multi_options      = multi_options;
		graph_node->dynamic_structures = dynamic_structures;
		graph_node->drawable_widgets   = drawable_widgets;
		graph_node->add_more_widgets    = add_more_widgets;
		graph_node->pos               = pos;
		for (auto i = callbacks.begin(); i != callbacks.end();)
		{
			graph_node->callbacks.try_emplace(i->first, i->second);
			i++;
		}

		graph_node->render_graph    = render_graph;
		graph_node->window_provider = window_provider;
		graph_node->Sort();
		Reset();
		return graph_node;
	}

	void Reset()
	{
		input_nodes.clear();
		output_nodes.clear();
		selectables.clear();
		text_inputs.clear();
		primitives.clear();
		scrollables.clear();
		multi_options.clear();
		drawable_widgets.clear();
		add_more_widgets.clear();
		callbacks.clear();
		pos    = glm::vec2(0.0);
		node_id = -1;
		name   = "";
	}
};

struct Graph_Node_Registry
{
	struct Callback_Info
	{
		std::unordered_map<std::string, int>                           callbacks_map = {};
		std::vector<std::unique_ptr<std::function<void(ui::nodes::Graph_Node &)>>> callbacks    = {};
		void                                                           Add_Callback(std::string name, std::function<void(ui::nodes::Graph_Node &)> &&callback)
		{
			int id = callbacks.size();
			callbacks_map.try_emplace(name, id);
			callbacks.emplace_back(std::make_unique<std::function<void(ui::nodes::Graph_Node &)>>(callback));
		}
		std::function<void(ui::nodes::Graph_Node &)> *Get_Callback_By_Name(std::string name)
		{
			if (callbacks_map.contains(name))
			{
				auto callback = callbacks.at(callbacks_map.at(name)).get();
				return callback;
			}
			return nullptr;
		}
	};

	std::map<Node_Type, Callback_Info>  callbacks_registry = {};
	std::function<void(ui::nodes::Graph_Node &)> *Get_Callback(Node_Type node_kind, std::string name)
	{
		if (callbacks_registry.at(node_kind).Get_Callback_By_Name(name))
		{
			return callbacks_registry.at(node_kind).Get_Callback_By_Name(name);
		}
		return nullptr;
	}
	Graph_Node_Registry()
	{
		callbacks_registry.try_emplace(N_Root_Node, Callback_Info{});
		callbacks_registry.try_emplace(N_Render_Node, Callback_Info{});
		callbacks_registry.try_emplace(N_Vert_Shader, Callback_Info{});
		callbacks_registry.try_emplace(N_Frag_Shader, Callback_Info{});
		callbacks_registry.try_emplace(N_Comp_Shader, Callback_Info{});
		callbacks_registry.try_emplace(N_Col_Attachment_Structure, Callback_Info{});
		callbacks_registry.try_emplace(N_Depth_Structure, Callback_Info{});
		callbacks_registry.try_emplace(N_Push_Constant, Callback_Info{});
		callbacks_registry.try_emplace(N_Image_Sampler, Callback_Info{});
		callbacks_registry.try_emplace(N_Image_Storage, Callback_Info{});
		callbacks_registry.try_emplace(N_Depth_Image_Sampler, Callback_Info{});
		callbacks_registry.try_emplace(N_Vertex_Input, Callback_Info{});

		callbacks_registry.at(N_Root_Node).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			auto last_node_name = self_node.render_graph->sequential_render_nodes.back();
			self_node.Set_Ouput_Data("out_root_node", last_node_name->pass_name);
			systems::Logger::Get_Instance()->Log_Message(last_node_name->pass_name);
		});
		callbacks_registry.at(N_Render_Node).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			bool       in_root_node_chain  = false;
			ui::nodes::Graph_Node *current_graph_node = &self_node;
			Pin_Info   *input_node        = Get_From_Name_In_Map(current_graph_node->input_nodes, "Input Render Node");
			while (input_node != nullptr && input_node->linked_pin != nullptr)
			{
				current_graph_node = self_node.graph_node_res_manager->Get_Node_By_Input_Output_Id(input_node->linked_pin->id);
				if (current_graph_node->Has_Output(N_Root_Node))
				{
					in_root_node_chain = true;
					break;
				}
				input_node = Get_From_Name_In_Map(current_graph_node->input_nodes, "Input Render Node");
			}
			if (!in_root_node_chain)
			{
				systems::Logger::Get_Instance()->Log_Message("Node is not connected to the root chain");
			}

			struct Expected_Configs
			{
				int added    = 0;
				int expected = 1;
			};
			std::map<Node_Type, Expected_Configs> configs_added;
			std::map<Node_Type, Expected_Configs> configs_expected;

			Pin_Info *vert    = Get_From_Name_In_Map(self_node.input_nodes, "Vertex Shader");
			Pin_Info *frag    = Get_From_Name_In_Map(self_node.input_nodes, "Fragment Shader");
			Pin_Info *compute = Get_From_Name_In_Map(self_node.input_nodes, "Compute Shader");
			int      vert_id  = -1;
			int      frag_id  = -1;
			int      comp_id  = -1;

			if (compute->Has_Data())
			{
				configs_added.try_emplace(N_Comp_Shader, Expected_Configs{0, 1});
				try
				{
					comp_id = compute->Get_Data<int>();
				}
				catch (std::bad_cast c)
				{
					assert(false);
				}
			}
			else
			{
				configs_added.try_emplace(N_Vertex_Input, Expected_Configs{0, 1});
				configs_added.try_emplace(N_Col_Attachment_Structure, Expected_Configs{0, 1});
				configs_added.try_emplace(N_Vert_Shader, Expected_Configs{0, 1});
				configs_added.try_emplace(N_Frag_Shader, Expected_Configs{0, 1});
				configs_added.try_emplace(N_Image_Storage, Expected_Configs{0, 0});
				if (vert->Has_Data())
				{
					configs_added.at(N_Vert_Shader).added++;
					vert_id = vert->Get_Data<int>();
				}
				if (frag->Has_Data())
				{
					Pin_Info *frag_pin = self_node.Get_Input_Data_By_Name("Fragment Shader");
					configs_added.at(N_Frag_Shader).added++;
					frag_id = frag_pin->Get_Data<int>();
				}
			}

			std::map<int, Pin_Info *> images;
			std::map<int, Pin_Info *> storage_images;
			Pin_Info                 *depth_image;
			int                      expected_col_attachments = 0;

			engine::Vertex_Input   *vertex_layout = nullptr;
			engine::Attachment_Info info;
			vk::AttachmentLoadOp   load_op_data;
			vk::AttachmentStoreOp  store_op_data;
			engine::Blend_Configs   blend_data;
			vk::Format             col_format_data;
			glm::vec4              clear_col_data;
			std::string            attachment_name;
			std::string            depth_attachment_name;

			for (auto id : self_node.input_nodes)
			{
				if (id.second.node_kind == N_Image_Storage)
				{
					configs_added.at(N_Image_Storage).expected++;
					if (!id.second.Has_Data())
					{
						continue;
					}
					ui::nodes::Graph_Node *graph_node_ref = self_node.graph_nodes_ref->at(id.second.Get_Data<int>());
					Pin_Info   *image        = graph_node_ref->Get_Output_Data_By_Name("out_img_storage");

					if (image->Has_Data())
					{
						configs_added.at(N_Image_Storage).added++;
						info = engine::Get_Color_Attachment_Info(engine::B_Opaque, clear_col_data, col_format_data, load_op_data,
						                                      store_op_data);
						storage_images.try_emplace(graph_node_ref->global_id, image);
					}
				}
				if (id.second.node_kind == N_Col_Attachment_Structure)
				{
					expected_col_attachments++;
					if (!id.second.Has_Data())
					{
						continue;
					}
					ui::nodes::Graph_Node *graph_node_ref    = self_node.graph_nodes_ref->at(id.second.Get_Data<int>());
					attachment_name             = graph_node_ref->name;
					Primitive_Input *clear_color = Get_From_Name_In_Map(
					    graph_node_ref->primitives, "Clear Color");
					Enum_Selectable *col_format = Get_From_Name_In_Map(
					    graph_node_ref->selectables, "Load Operation");
					Enum_Selectable *load_op = Get_From_Name_In_Map(
					    graph_node_ref->selectables, "Load Operation");
					Enum_Selectable *store_op = Get_From_Name_In_Map(
					    graph_node_ref->selectables, "Store Operation");
					Enum_Selectable *blend_configs = Get_From_Name_In_Map(
					    graph_node_ref->selectables, "Blend Configs");

					clear_col_data = clear_color->Get_Data<glm::vec4>();
					load_op_data   = (vk::AttachmentLoadOp) load_op->selected_idx;
					store_op_data  = (vk::AttachmentStoreOp) store_op->selected_idx;
					blend_data    = (engine::Blend_Configs) blend_configs->selected_idx;

					std::map<int, vk::Format> valid_formats = {
					    {0, engine::g_32b_format}, {1, engine::g_16b_format}, {2, engine::g_shipper_format}};
					col_format_data = col_format->Get_Enum_From_Index<vk::Format>(valid_formats);

					Pin_Info *image = graph_node_ref->Get_Input_Data_By_Name("Col Attachment Sampler");
					if (image->Has_Data())
					{
						configs_added.at(N_Col_Attachment_Structure).added++;
						info = engine::Get_Color_Attachment_Info(blend_data, clear_col_data, col_format_data, load_op_data, store_op_data);
						images.try_emplace(graph_node_ref->global_id, image);
					}
				}
				if (id.second.node_kind == N_Depth_Structure)
				{
					if (!id.second.Has_Data())
					{
						continue;
					}
					configs_added.try_emplace(N_Depth_Structure, Expected_Configs{0, 1});
					ui::nodes::Graph_Node *graph_node_ref = self_node.graph_nodes_ref->at(id.second.Get_Data<int>());
					depth_attachment_name     = graph_node_ref->name;

					depth_image = graph_node_ref->Get_Input_Data_By_Name("Depth Attachment Sampler");
					if (depth_image->Has_Data())
					{
						configs_added.at(N_Depth_Structure).added++;
					}
				}
			}
			configs_added.at(N_Col_Attachment_Structure).expected = (expected_col_attachments != 0) ? expected_col_attachments : 1;

			Pin_Info *vertex_pin = self_node.Get_Input_Data_By_Name("Vertex Input");

			if (vertex_pin->Has_Data())
			{
				std::string vertex_name                = vertex_pin->Get_Data<std::string>();
				vertex_layout                           = self_node.render_graph->resources_manager->Get_Vertex_Input(vertex_name);
				configs_added.at(N_Vertex_Input).added = true;
			}

			int                                 configs_to_match = configs_added.size();
			int                                 configs_matched = 0;
			std::map<Node_Type, Expected_Configs> confings_missing;
			for (auto added : configs_added)
			{
				if (added.second.added != added.second.expected)
				{
					confings_missing.insert(added);
					continue;
				}
				configs_matched++;
			}
			if (configs_matched != configs_to_match || !in_root_node_chain)
			{
				std::string missing_info = "";
				for (auto added : confings_missing)
				{
					missing_info += node_type_strings.at(added.first) + "\n";
				}
				if (!in_root_node_chain)
				{
					missing_info += "NOT_LINKED_WITH_ROOT_\n";
				}
				systems::Logger::Get_Instance()->Log_Message("Confgis missng: \n" + missing_info);
				return;
			}
			std::string name     = "";
			Text_Input  *node_name = Get_From_Name_In_Map(self_node.text_inputs, "RenderNode Name");
			name                 = (node_name) ? node_name->content : "PassName_" + std::to_string(self_node.render_graph->render_nodes.size());

			auto render_node = self_node.render_graph->Add_Pass(name);
			if (render_node->active)
			{
				return;
			}

			for (auto &image : storage_images)
			{
				if (!image.second->Has_Data())
				{
					continue;
				}
				int                image_id   = image.second->Get_Data<int>();
				std::string        image_name = "nodeImage_" + std::to_string(image.first);
				engine::Image_View *img_view   = self_node.render_graph->resources_manager->Get_Storage_From_Id(
                    image_id);
				render_node->G_Add_Storage_Resource(img_view);
			}
			if (configs_added.contains(N_Comp_Shader))
			{
				render_node->G_Set_Comp_Shader(self_node.render_graph->resources_manager->Get_Shader_From_Id(comp_id));
			}
			else
			{
				assert(vert_id > -1 && "Vert id invalid");
				assert(frag_id > -1 && "Frag Id invalid");
				assert(vertex_layout && "invalid vertex input");
				render_node->G_Set_Vert_Shader(self_node.render_graph->resources_manager->Get_Shader_From_Id(vert_id));
				render_node->G_Set_Frag_Shader(self_node.render_graph->resources_manager->Get_Shader_From_Id(frag_id));
				render_node->G_Set_Vertex_Input(*vertex_layout);
				render_node->G_Add_Color_Attachment_Output(0, info);
				int idx = 0;
				for (auto &image : images)
				{
					if (!image.second->Has_Data())
					{
						continue;
					}
					int                image_id   = image.second->Get_Data<int>();
					std::string        image_name = "nodeImage_" + std::to_string(image.first);
					engine::Image_View *img_view   = self_node.render_graph->resources_manager->Get_Image_View_From_Id(
                        image_id);
					render_node->G_Set_Color_Image_Attachment_Binding(idx,img_view);
					idx++;
				}

				if (configs_added.contains(N_Depth_Structure))
				{
					if (depth_image->Has_Data())
					{
						std::string image_name = depth_image->Get_Data<std::string>();
						render_node->G_Set_Depth_Image_Resource(
						    self_node.render_graph->resources_manager->Get_Image_View_From_Name(
						        image_name));
					}
				}

				render_node->G_Set_Configs(engine::Render_Node_Configs{true});
				render_node->Build_Render_Graph_Node();
				render_node->active = false;
				self_node.valid     = true;
			}
		});
		callbacks_registry.at(N_Col_Attachment_Structure).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			self_node.Set_Ouput_Data("out_col_structure", self_node.global_id);
		});
		callbacks_registry.at(N_Depth_Structure).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			self_node.Set_Ouput_Data("out_depth_structure", self_node.global_id);
		});
		callbacks_registry.at(N_Push_Constant).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {});

		callbacks_registry.at(N_Image_Sampler).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			std::string img_name = "nodeImage_" + std::to_string(self_node.global_id);

			assert(!img_name.empty() && "Img name is not valid");
			auto               image_info = engine::Image::Create_Info2d(self_node.window_provider->Get_Window_Size(), 1, 1,
			                                                           engine::g_32b_format,
			                                                           vk::ImageUsageFlagBits::eColorAttachment |
			                                                               vk::ImageUsageFlagBits::eSampled);
			engine::Image_View *img_view   = self_node.render_graph->resources_manager->Get_Image(
                img_name, image_info, 0, 0);
			self_node.Set_Ouput_Data("out_img_sampler", img_view->id);

			assert(img_view && "Image view must be valid");
		});
		callbacks_registry.at(N_Image_Storage).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			std::string img_name = "nodeImage_" + std::to_string(self_node.global_id);
			;

			auto storage_image_info = engine::Image::Create_Info2d(self_node.window_provider->Get_Window_Size(), 1, 1,
			                                                    engine::g_32b_format,
			                                                    vk::ImageUsageFlagBits::eStorage |
			                                                        vk::ImageUsageFlagBits::eTransferDst);

			engine::Image_View *storage_img_view = self_node.render_graph->resources_manager->Get_Image(
			    img_name, storage_image_info, 0, 0);
			self_node.Set_Ouput_Data("out_img_storage", storage_img_view->id);

			assert(storage_img_view && "Image view must be valid");
		});
		callbacks_registry.at(N_Depth_Image_Sampler).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			std::string img_name = self_node.Get_Text_Input("Img Name")->content;
			assert(!img_name.empty() && "Img name is not valid");
			auto               depth_image_info = engine::Image::Create_Info2d(self_node.window_provider->Get_Window_Size(), 1, 1,
			                                                                self_node.render_graph->core->swapchain_ref->depth_format,
			                                                                vk::ImageUsageFlagBits::eDepthStencilAttachment |
			                                                                    vk::ImageUsageFlagBits::eSampled);
			engine::Image_View *img_view        = self_node.render_graph->resources_manager->Get_Image(
                img_name, depth_image_info, 0, 0);
			self_node.Set_Ouput_Data("out_depth_sampler", img_name);

			assert(img_view && "Image view must be valid");
		});
		callbacks_registry.at(N_Vertex_Input).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			std::string          vertex_name  = "vertex_input_" + std::to_string(self_node.global_id);
			engine::Vertex_Input *vertex_layout = self_node.render_graph->resources_manager->Get_Vertex_Input(vertex_name);
			vertex_layout->binding_description.clear();
			vertex_layout->input_description.clear();

			size_t offset           = 0;
			int    location         = 0;
			int    binding          = 0;
			auto   dynamic_structure = Get_From_Name_In_Map(
                self_node.dynamic_structures, "Vertex Builder");
			for (auto widget : dynamic_structure->widgets_infos)
			{
				int                          type_selected = widget.second.selectable_info.selected_idx;
				engine::Vertex_Input::Attribs attribs      = (engine::Vertex_Input::Attribs) type_selected;
				vertex_layout->Add_Vertex_Attrib(attribs, binding, offset, location);
				offset += vertex_layout->Get_Size_From(attribs);
				location++;
			}
			for (auto widget : dynamic_structure->widgets_infos)
			{
				vertex_layout->Add_Vertex_Input_Binding(0, offset);
			}
			self_node.Set_Ouput_Data("out_vertex_result", vertex_name);
		});
		callbacks_registry.at(N_Comp_Shader).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			engine::Shader_Stage stage       = engine::Shader_Stage::S_Comp;
			auto                multi_option = Get_From_Name_In_Map<ui::nodes::Multi_Option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).Get_Current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->Get_Or_Create_Default_Shader(shader_selected, stage, engine::C_Glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.Set_Ouput_Data("out_shader", shader_idx);
		});
		callbacks_registry.at(N_Vert_Shader).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			engine::Shader_Stage stage       = engine::Shader_Stage::S_Vert;
			auto                multi_option = Get_From_Name_In_Map<ui::nodes::Multi_Option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).Get_Current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->Get_Or_Create_Default_Shader(shader_selected, stage, engine::C_Glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.Set_Ouput_Data("out_shader", shader_idx);
		});
		callbacks_registry.at(N_Frag_Shader).Add_Callback("output_c", [](ui::nodes::Graph_Node &self_node) {
			engine::Shader_Stage stage       = engine::Shader_Stage::S_Frag;
			auto                multi_option = Get_From_Name_In_Map<ui::nodes::Multi_Option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).Get_Current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->Get_Or_Create_Default_Shader(shader_selected, stage, engine::C_Glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.Set_Ouput_Data("out_shader", shader_idx);
		});

		callbacks_registry.at(N_Render_Node).Add_Callback("link_c", [](ui::nodes::Graph_Node &self_node) {
			Pin_Info *vert    = Get_From_Name_In_Map(self_node.input_nodes, "Vertex Shader");
			Pin_Info *frag    = Get_From_Name_In_Map(self_node.input_nodes, "Fragment Shader");
			Pin_Info *compute = Get_From_Name_In_Map(self_node.input_nodes, "Compute Shader");
			self_node.Toggle_Draw(vert->id, true);
			self_node.Toggle_Draw(frag->id, true);
			self_node.Toggle_Draw(compute->id, true);
			if (compute->Has_Data())
			{
				self_node.Toggle_Draw(vert->id, false);
				self_node.Toggle_Draw(frag->id, false);
			}
			else if (vert->Has_Data() || frag->Has_Data())
			{
				self_node.Toggle_Draw(compute->id, false);
			}
		});

		callbacks_registry.at(N_Render_Node).Add_Callback("unlink_c", [](ui::nodes::Graph_Node &self_node) {
			Pin_Info *vert    = Get_From_Name_In_Map(self_node.input_nodes, "Vertex Shader");
			Pin_Info *frag    = Get_From_Name_In_Map(self_node.input_nodes, "Fragment Shader");
			Pin_Info *compute = Get_From_Name_In_Map(self_node.input_nodes, "Compute Shader");

			self_node.Toggle_Draw(vert->id, true);
			self_node.Toggle_Draw(frag->id, true);
			self_node.Toggle_Draw(compute->id, true);
			if (compute->Has_Data())
			{
				self_node.Toggle_Draw(vert->id, false);
				self_node.Toggle_Draw(frag->id, false);
			}
			else if (vert->Has_Data() || frag->Has_Data())
			{
				self_node.Toggle_Draw(compute->id, false);
			}
		});
	}
};

struct Graph_Node_Factory
{
	Graph_Node_Builder     builder      = {};
	Graph_Node_Registry    node_registry = {};
	engine::Render_Graph *render_graph;
	::Window_Provider      *window_provider;
	Graph_Node_Res_Manager *res_manager;

	ui::nodes::Graph_Node *Get_Node(Node_Type node_kind, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
	{
		assert(render_graph && "Null rgraph");
		assert(window_provider && "Null window Provider");
		assert(res_manager && "Null resManager");
		ui::nodes::Graph_Node node;
		switch (node_kind)
		{
			default:
				assert(false);
				break;
			case N_Root_Node:
				builder
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_root_node", N_Root_Node})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Root Node");
				break;
			case N_Render_Node:
				builder
				    .Add_Text_Input(res_manager->Next_Widget_Id(), {"RenderNode Name"})
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_render_node", N_Render_Node})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Input Render Node", static_cast<Node_Type>(N_Render_Node | N_Root_Node)})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Vertex Shader", N_Vert_Shader})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Fragment Shader", N_Frag_Shader})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Compute Shader", N_Comp_Shader})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Col Attachment Node", N_Col_Attachment_Structure}, true)
				    .Add_Input(res_manager->Next_Widget_Id(), {"Storage Image Node", N_Image_Storage}, true)
				    .Add_Input(res_manager->Next_Widget_Id(), {"Depth Attachment", N_Depth_Structure})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Vertex Input", N_Vertex_Input})
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Raster Configs", {"Fill", "Line", "Point"})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Render Node");
				break;
			case N_Col_Attachment_Structure:
				builder
				    .Add_Primitive_Data(res_manager->Next_Widget_Id(), {"Clear Color", Vec4, glm::vec4(0.0)})
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Color Format", {"g_32bFormat", "g_16bFormat"})
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Load Operation", {"Load", "Clear", "Dont Care", "None"})
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Store Operation", {"Load", "eDontCare", "eNone"})
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"}, 1)
				    .Add_Input(res_manager->Next_Widget_Id(), {"Col Attachment Sampler", N_Image_Sampler})
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_col_structure", N_Col_Attachment_Structure})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Col Attachment Node");
				break;
			case N_Depth_Structure:
				builder
				    .Add_Selectable(res_manager->Next_Widget_Id(), "Depth Configs", {"None", "Enable", "Disable"})
				    .Add_Input(res_manager->Next_Widget_Id(), {"Depth Image In", N_Depth_Image_Sampler})
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_depth_structure ", N_Depth_Structure})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Depth Attachment Node");
				break;
			case N_Push_Constant:
				builder
				    .Add_Primitive_Data(res_manager->Next_Widget_Id(), {"Push Constant data", Size_Value, size_t(0)})
				    .Add_Output(res_manager->Next_Widget_Id(), {"Push Constant Structure ", N_Push_Constant})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Push Constant");
				break;
			case N_Image_Sampler:
				builder
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_img_sampler", N_Image_Sampler})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Sampler Image Node");
				break;
			case N_Image_Storage:
				builder
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_img_storage", N_Image_Storage})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Storage Image Node");
				break;
			case N_Depth_Image_Sampler:
				builder
				    .Add_Text_Input(res_manager->Next_Widget_Id(), {"Img Name", "Image Name"})
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_depth_sampler", N_Depth_Image_Sampler})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Depth Image Node");
				break;
			case N_Vertex_Input:
			{
				Enum_Selectable   selectable("Vertex Attrib: ", {"INT", "FLOAT", "VEC2", "VEC3", "VE4", "U8VEC3", "U8VEC4", "COLOR_32"});
				Dynamic_Structure dynamic_structure_info("Vertex Builder", selectable);

				builder.Add_Dynamic_Structure(res_manager->Next_Widget_Id(), dynamic_structure_info)
				    .Add_Output(res_manager->Next_Widget_Id(), {"out_vertex_result", N_Vertex_Input})
				    .Set_Node_Id(res_manager->Next_Widget_Id(), "Vertex Input Builder");
			}
			break;
			case N_Vert_Shader:
			case N_Frag_Shader:
			case N_Comp_Shader:
				std::vector<std::any> shader_paths;
				shader_paths.reserve(render_graph->resources_manager->shaders_names.size());
				std::vector<std::string> options = {"Pick Shader", "Create Shader"};
				std::map<int, Text_Input> text_inputs;
				text_inputs.try_emplace(1, Text_Input{"Select Shader Name"});
				std::map<int, Scrollable> scrollables;

				if (node_kind == N_Vert_Shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::Shader_Stage::S_Vert)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .Set_Node_Id(res_manager->Next_Widget_Id(), "Vert Shader")
					    .Add_Output(res_manager->Next_Widget_Id(), {"out_shader", N_Vert_Shader});
				}
				if (node_kind == N_Frag_Shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::Shader_Stage::S_Frag)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .Set_Node_Id(res_manager->Next_Widget_Id(), "Frag Shader")
					    .Add_Output(res_manager->Next_Widget_Id(), {"out_shader", N_Frag_Shader});
				}
				if (node_kind == N_Comp_Shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::Shader_Stage::S_Comp)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .Set_Node_Id(res_manager->Next_Widget_Id(), "Compute Shader")
					    .Add_Output(res_manager->Next_Widget_Id(), {"out_shader", N_Comp_Shader});
				}
				scrollables.try_emplace(0, Scrollable{"Possible Shaders", String_Value, shader_paths});
				ui::nodes::Multi_Option multi_option_info("Shader Options", options, text_inputs, {}, scrollables, {});
				builder.Add_Multi_Option(res_manager->Next_Widget_Id(), multi_option_info);
				break;
		}

		std::function<void(ui::nodes::Graph_Node &)> *output_op = node_registry.Get_Callback(node_kind, "output_c");
		std::function<void(ui::nodes::Graph_Node &)> *link_op   = node_registry.Get_Callback(node_kind, "link_c");
		if (!name.empty())
		{
			builder.Set_Node_Name(name);
		}
		builder.Set_Position(pos);
		builder.Add_Callback("output_c", output_op);
		builder.Add_Callback("link_c", link_op);

		int        id        = res_manager->Next_Node_Id();
		ui::nodes::Graph_Node *graph_node = res_manager->Get_Node(id);
		builder.Build(graph_node, render_graph, window_provider);
		res_manager->Add_Node_Ids(graph_node);

		return graph_node;
	}
};
}        // namespace nodes

}        // namespace ui

#endif        // WIDGETS_HPP
