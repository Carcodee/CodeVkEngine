//

// Created by carlo on 2025-01-02.
//

#ifndef WIDGETS_HPP
#define WIDGETS_HPP

namespace ui
{
enum widgets_property
{
	drag,
	drop
};

template <typename value_type>
concept has_name = requires(value_type t) {
	{ t.name } -> std::convertible_to<std::string>;
};

template <has_name value_type>
static value_type *get_from_name_in_map(std::map<int, value_type> &map_to_search, const std::string name)
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

template <has_name value_type>
static value_type *get_from_name_in_map(std::unordered_map<int, value_type> &map_to_search, const std::string name)
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
template <has_name value_type>
static int get_id_from_map(std::map<int, value_type> &map_to_search, const std::string name)
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

template <typename value_type>
static void use_drag_property(std::string name, value_type &payload, size_t payload_size)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload(name.c_str(), &payload, payload_size);
		ImGui::EndDragDropSource();
	}
}

template <typename value_type>
static value_type use_drop_property(std::string name)
{
	if (ImGui::BeginDragDropTarget())
	{
		value_type data = nullptr;
		if (const ImGuiPayload *imgui_payload = ImGui::AcceptDragDropPayload(name.c_str()))
		{
			//				const char* data = (const char*)payload->Data;
			data = *(value_type *) imgui_payload->Data;

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

struct i_widget
{
	virtual ~i_widget()               = default;
	virtual void display_properties() = 0;
};

struct i_node_widget
{
	std::string name;
	virtual ~i_node_widget()    = default;
	virtual void draw(int id) = 0;
};

struct texture_viewer : i_widget
{
	engine::image_view        *curr_image_view = nullptr;
	std::string               label_name;
	std::set<widgets_property> properties;
	~texture_viewer() override = default;

	void display_properties() override
	{
		for (auto &prop : properties)
		{
			switch (prop)
			{
				case drag:
					use_drag_property("TEXTURE_NAME", curr_image_view, sizeof(engine::image_view *));
					break;
				case drop:
					engine::image_view *new_img_view = use_drop_property<engine::image_view *>("TEXTURE_NAME");
					if (new_img_view != nullptr)
					{
						curr_image_view = new_img_view;
					}
					break;
			}
		}
	}

	engine::image_view *display_texture(std::string name, engine::image_view *image_view_handle, ImTextureID texture_id,
	                                  glm::vec2 size)
	{
		label_name           = name;
		this->curr_image_view = image_view_handle;

		ImGui::PushID(name.c_str());
		ImGui::SeparatorText(name.c_str());
		ImGui::ImageButton(name.c_str(), texture_id, ImVec2{size.x, size.y});
		display_properties();
		ImGui::PopID();

		return this->curr_image_view;
	}

	void add_property(widgets_property property)
	{
		properties.emplace(property);
	}
};

namespace nodes
{
namespace ed = ax::NodeEditor;
// note that this only handle copyable data types

struct graph_node_builder
{
	ed::NodeId                                                          node_id;
	std::unordered_map<int, pin_info>                                    input_nodes        = {};
	std::unordered_map<int, pin_info>                                    output_nodes       = {};
	std::unordered_map<int, enum_selectable>                             selectables       = {};
	std::unordered_map<int, text_input>                                  text_inputs        = {};
	std::unordered_map<int, primitive_input>                             primitives        = {};
	std::unordered_map<int, scrollable>                                 scrollables       = {};
	std::unordered_map<int, ui::nodes::multi_option>                                multi_options      = {};
	std::unordered_map<int, dynamic_structure>                           dynamic_structures = {};
	std::unordered_map<int, bool>                                       drawable_widgets   = {};
	std::unordered_map<int, bool>                                       add_more_widgets    = {};
	std::unordered_map<std::string, std::function<void(ui::nodes::graph_node &)> *> callbacks;

	std::string name = "";
	glm::vec2   pos  = glm::vec2(0.0);

	graph_node_builder &set_node_id(ed::NodeId id, std::string name)
	{
		node_id     = id;
		this->name = name;
		return *this;
	}

	graph_node_builder &set_node_name(std::string name)
	{
		this->name = name;
		return *this;
	}

	graph_node_builder &set_position(glm::vec2 pos)
	{
		this->pos = pos;
		return *this;
	}

	graph_node_builder &add_input(int id, pin_info pin_info, bool add_more = false)
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

	graph_node_builder &add_output(int id, pin_info pin_info)
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

	graph_node_builder &add_selectable(int id, std::string name, std::vector<std::string> options, int selected_idx = 0, bool add_more = false)
	{
		selectables.try_emplace(id, enum_selectable{name, options, selected_idx});
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	graph_node_builder &add_text_input(int id, text_input info, bool add_more = false)
	{
		text_inputs.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	graph_node_builder &add_scrollable(int id, scrollable info, bool add_more = false)
	{
		scrollables.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	graph_node_builder &add_primitive_data(int id, primitive_input info, bool add_more = false)
	{
		primitives.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	graph_node_builder &add_multi_option(int id, ui::nodes::multi_option info, bool add_more = false)
	{
		multi_options.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}

	graph_node_builder &add_dynamic_structure(int id, dynamic_structure info, bool add_more = false)
	{
		dynamic_structures.try_emplace(id, info);
		drawable_widgets.try_emplace(id, true);
		if (add_more)
		{
			add_more_widgets.try_emplace(id, true);
		}
		return *this;
	}
	graph_node_builder &add_callback(std::string name, std::function<void(ui::nodes::graph_node &)> *callback)
	{
		if (callbacks.contains(name))
		{
			callbacks.at(name) = std::move(callback);
		}
		callbacks.try_emplace(name, std::move(callback));
		return *this;
	}

	ui::nodes::graph_node *build(ui::nodes::graph_node *graph_node, engine::render_graph *render_graph, ::window_provider *window_provider)
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
		graph_node->sort();
		reset();
		return graph_node;
	}

	void reset()
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

struct graph_node_registry
{
	struct callback_info
	{
		std::unordered_map<std::string, int>                           callbacks_map = {};
		std::vector<std::unique_ptr<std::function<void(ui::nodes::graph_node &)>>> callbacks    = {};
		void                                                           add_callback(std::string name, std::function<void(ui::nodes::graph_node &)> &&callback)
		{
			int id = callbacks.size();
			callbacks_map.try_emplace(name, id);
			callbacks.emplace_back(std::make_unique<std::function<void(ui::nodes::graph_node &)>>(callback));
		}
		std::function<void(ui::nodes::graph_node &)> *get_callback_by_name(std::string name)
		{
			if (callbacks_map.contains(name))
			{
				auto callback = callbacks.at(callbacks_map.at(name)).get();
				return callback;
			}
			return nullptr;
		}
	};

	std::map<node_type, callback_info>  callbacks_registry = {};
	std::function<void(ui::nodes::graph_node &)> *get_callback(node_type node_kind, std::string name)
	{
		if (callbacks_registry.at(node_kind).get_callback_by_name(name))
		{
			return callbacks_registry.at(node_kind).get_callback_by_name(name);
		}
		return nullptr;
	}
	graph_node_registry()
	{
		callbacks_registry.try_emplace(n_root_node, callback_info{});
		callbacks_registry.try_emplace(n_render_node, callback_info{});
		callbacks_registry.try_emplace(n_vert_shader, callback_info{});
		callbacks_registry.try_emplace(n_frag_shader, callback_info{});
		callbacks_registry.try_emplace(n_comp_shader, callback_info{});
		callbacks_registry.try_emplace(n_col_attachment_structure, callback_info{});
		callbacks_registry.try_emplace(n_depth_structure, callback_info{});
		callbacks_registry.try_emplace(n_push_constant, callback_info{});
		callbacks_registry.try_emplace(n_image_sampler, callback_info{});
		callbacks_registry.try_emplace(n_image_storage, callback_info{});
		callbacks_registry.try_emplace(n_depth_image_sampler, callback_info{});
		callbacks_registry.try_emplace(n_vertex_input, callback_info{});

		callbacks_registry.at(n_root_node).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			auto last_node_name = self_node.render_graph->sequential_render_nodes.back();
			self_node.set_ouput_data("out_root_node", last_node_name->pass_name);
			systems::logger::get_instance()->log_message(last_node_name->pass_name);
		});
		callbacks_registry.at(n_render_node).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			bool       in_root_node_chain  = false;
			ui::nodes::graph_node *current_graph_node = &self_node;
			pin_info   *input_node        = get_from_name_in_map(current_graph_node->input_nodes, "Input Render Node");
			while (input_node != nullptr && input_node->linked_pin != nullptr)
			{
				current_graph_node = self_node.graph_node_res_manager->get_node_by_input_output_id(input_node->linked_pin->id);
				if (current_graph_node->has_output(n_root_node))
				{
					in_root_node_chain = true;
					break;
				}
				input_node = get_from_name_in_map(current_graph_node->input_nodes, "Input Render Node");
			}
			if (!in_root_node_chain)
			{
				systems::logger::get_instance()->log_message("Node is not connected to the root chain");
			}

			struct expected_configs
			{
				int added    = 0;
				int expected = 1;
			};
			std::map<node_type, expected_configs> configs_added;
			std::map<node_type, expected_configs> configs_expected;

			pin_info *vert    = get_from_name_in_map(self_node.input_nodes, "Vertex Shader");
			pin_info *frag    = get_from_name_in_map(self_node.input_nodes, "Fragment Shader");
			pin_info *compute = get_from_name_in_map(self_node.input_nodes, "Compute Shader");
			int      vert_id  = -1;
			int      frag_id  = -1;
			int      comp_id  = -1;

			if (compute->has_data())
			{
				configs_added.try_emplace(n_comp_shader, expected_configs{0, 1});
				try
				{
					comp_id = compute->get_data<int>();
				}
				catch (std::bad_cast c)
				{
					assert(false);
				}
			}
			else
			{
				configs_added.try_emplace(n_vertex_input, expected_configs{0, 1});
				configs_added.try_emplace(n_col_attachment_structure, expected_configs{0, 1});
				configs_added.try_emplace(n_vert_shader, expected_configs{0, 1});
				configs_added.try_emplace(n_frag_shader, expected_configs{0, 1});
				configs_added.try_emplace(n_image_storage, expected_configs{0, 0});
				if (vert->has_data())
				{
					configs_added.at(n_vert_shader).added++;
					vert_id = vert->get_data<int>();
				}
				if (frag->has_data())
				{
					pin_info *frag_pin = self_node.get_input_data_by_name("Fragment Shader");
					configs_added.at(n_frag_shader).added++;
					frag_id = frag_pin->get_data<int>();
				}
			}

			std::map<int, pin_info *> images;
			std::map<int, pin_info *> storage_images;
			pin_info                 *depth_image;
			int                      expected_col_attachments = 0;

			engine::vertex_input   *vertex_layout = nullptr;
			engine::attachment_info info;
			vk::AttachmentLoadOp   load_op_data;
			vk::AttachmentStoreOp  store_op_data;
			engine::blend_configs   blend_data;
			vk::Format             col_format_data;
			glm::vec4              clear_col_data;
			std::string            attachment_name;
			std::string            depth_attachment_name;

			for (auto id : self_node.input_nodes)
			{
				if (id.second.node_kind == n_image_storage)
				{
					configs_added.at(n_image_storage).expected++;
					if (!id.second.has_data())
					{
						continue;
					}
					ui::nodes::graph_node *graph_node_ref = self_node.graph_nodes_ref->at(id.second.get_data<int>());
					pin_info   *image        = graph_node_ref->get_output_data_by_name("out_img_storage");

					if (image->has_data())
					{
						configs_added.at(n_image_storage).added++;
						info = engine::get_color_attachment_info(engine::b_opaque, clear_col_data, col_format_data, load_op_data,
						                                      store_op_data);
						storage_images.try_emplace(graph_node_ref->global_id, image);
					}
				}
				if (id.second.node_kind == n_col_attachment_structure)
				{
					expected_col_attachments++;
					if (!id.second.has_data())
					{
						continue;
					}
					ui::nodes::graph_node *graph_node_ref    = self_node.graph_nodes_ref->at(id.second.get_data<int>());
					attachment_name             = graph_node_ref->name;
					primitive_input *clear_color = get_from_name_in_map(
					    graph_node_ref->primitives, "Clear Color");
					enum_selectable *col_format = get_from_name_in_map(
					    graph_node_ref->selectables, "Load Operation");
					enum_selectable *load_op = get_from_name_in_map(
					    graph_node_ref->selectables, "Load Operation");
					enum_selectable *store_op = get_from_name_in_map(
					    graph_node_ref->selectables, "Store Operation");
					enum_selectable *blend_configs = get_from_name_in_map(
					    graph_node_ref->selectables, "Blend Configs");

					clear_col_data = clear_color->get_data<glm::vec4>();
					load_op_data   = (vk::AttachmentLoadOp) load_op->selected_idx;
					store_op_data  = (vk::AttachmentStoreOp) store_op->selected_idx;
					blend_data    = (engine::blend_configs) blend_configs->selected_idx;

					std::map<int, vk::Format> valid_formats = {
					    {0, engine::g_32b_format}, {1, engine::g_16b_format}, {2, engine::g_shipper_format}};
					col_format_data = col_format->get_enum_from_index<vk::Format>(valid_formats);

					pin_info *image = graph_node_ref->get_input_data_by_name("Col Attachment Sampler");
					if (image->has_data())
					{
						configs_added.at(n_col_attachment_structure).added++;
						info = engine::get_color_attachment_info(blend_data, clear_col_data, col_format_data, load_op_data, store_op_data);
						images.try_emplace(graph_node_ref->global_id, image);
					}
				}
				if (id.second.node_kind == n_depth_structure)
				{
					if (!id.second.has_data())
					{
						continue;
					}
					configs_added.try_emplace(n_depth_structure, expected_configs{0, 1});
					ui::nodes::graph_node *graph_node_ref = self_node.graph_nodes_ref->at(id.second.get_data<int>());
					depth_attachment_name     = graph_node_ref->name;

					depth_image = graph_node_ref->get_input_data_by_name("Depth Attachment Sampler");
					if (depth_image->has_data())
					{
						configs_added.at(n_depth_structure).added++;
					}
				}
			}
			configs_added.at(n_col_attachment_structure).expected = (expected_col_attachments != 0) ? expected_col_attachments : 1;

			pin_info *vertex_pin = self_node.get_input_data_by_name("Vertex Input");

			if (vertex_pin->has_data())
			{
				std::string vertex_name                = vertex_pin->get_data<std::string>();
				vertex_layout                           = self_node.render_graph->resources_manager->get_vertex_input(vertex_name);
				configs_added.at(n_vertex_input).added = true;
			}

			int                                 configs_to_match = configs_added.size();
			int                                 configs_matched = 0;
			std::map<node_type, expected_configs> confings_missing;
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
				systems::logger::get_instance()->log_message("Confgis missng: \n" + missing_info);
				return;
			}
			std::string name     = "";
			text_input  *node_name = get_from_name_in_map(self_node.text_inputs, "RenderNode Name");
			name                 = (node_name) ? node_name->content : "PassName_" + std::to_string(self_node.render_graph->render_nodes.size());

			auto render_node = self_node.render_graph->add_pass(name);
			if (render_node->active)
			{
				return;
			}

			for (auto &image : storage_images)
			{
				if (!image.second->has_data())
				{
					continue;
				}
				int                image_id   = image.second->get_data<int>();
				std::string        image_name = "nodeImage_" + std::to_string(image.first);
				engine::image_view *img_view   = self_node.render_graph->resources_manager->get_storage_from_id(
                    image_id);
				render_node->g_add_storage_resource(img_view);
			}
			if (configs_added.contains(n_comp_shader))
			{
				render_node->g_set_comp_shader(self_node.render_graph->resources_manager->get_shader_from_id(comp_id));
			}
			else
			{
				assert(vert_id > -1 && "Vert id invalid");
				assert(frag_id > -1 && "Frag Id invalid");
				assert(vertex_layout && "invalid vertex input");
				render_node->g_set_vert_shader(self_node.render_graph->resources_manager->get_shader_from_id(vert_id));
				render_node->g_set_frag_shader(self_node.render_graph->resources_manager->get_shader_from_id(frag_id));
				render_node->g_set_vertex_input(*vertex_layout);
				render_node->g_add_color_attachment_output(0, info);
				int idx = 0;
				for (auto &image : images)
				{
					if (!image.second->has_data())
					{
						continue;
					}
					int                image_id   = image.second->get_data<int>();
					std::string        image_name = "nodeImage_" + std::to_string(image.first);
					engine::image_view *img_view   = self_node.render_graph->resources_manager->get_image_view_from_id(
                        image_id);
					render_node->g_set_color_image_attachment_binding(idx,img_view);
					idx++;
				}

				if (configs_added.contains(n_depth_structure))
				{
					if (depth_image->has_data())
					{
						std::string image_name = depth_image->get_data<std::string>();
						render_node->g_set_depth_image_resource(
						    self_node.render_graph->resources_manager->get_image_view_from_name(
						        image_name));
					}
				}

				render_node->g_set_configs(engine::render_node_configs{true});
				render_node->build_render_graph_node();
				render_node->active = false;
				self_node.valid     = true;
			}
		});
		callbacks_registry.at(n_col_attachment_structure).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			self_node.set_ouput_data("out_col_structure", self_node.global_id);
		});
		callbacks_registry.at(n_depth_structure).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			self_node.set_ouput_data("out_depth_structure", self_node.global_id);
		});
		callbacks_registry.at(n_push_constant).add_callback("output_c", [](ui::nodes::graph_node &self_node) {});

		callbacks_registry.at(n_image_sampler).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			std::string img_name = "nodeImage_" + std::to_string(self_node.global_id);

			assert(!img_name.empty() && "Img name is not valid");
			auto               image_info = engine::image::create_info2d(self_node.window_provider->get_window_size(), 1, 1,
			                                                           engine::g_32b_format,
			                                                           vk::ImageUsageFlagBits::eColorAttachment |
			                                                               vk::ImageUsageFlagBits::eSampled);
			engine::image_view *img_view   = self_node.render_graph->resources_manager->get_image(
                img_name, image_info, 0, 0);
			self_node.set_ouput_data("out_img_sampler", img_view->id);

			assert(img_view && "Image view must be valid");
		});
		callbacks_registry.at(n_image_storage).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			std::string img_name = "nodeImage_" + std::to_string(self_node.global_id);
			;

			auto storage_image_info = engine::image::create_info2d(self_node.window_provider->get_window_size(), 1, 1,
			                                                    engine::g_32b_format,
			                                                    vk::ImageUsageFlagBits::eStorage |
			                                                        vk::ImageUsageFlagBits::eTransferDst);

			engine::image_view *storage_img_view = self_node.render_graph->resources_manager->get_image(
			    img_name, storage_image_info, 0, 0);
			self_node.set_ouput_data("out_img_storage", storage_img_view->id);

			assert(storage_img_view && "Image view must be valid");
		});
		callbacks_registry.at(n_depth_image_sampler).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			std::string img_name = self_node.get_text_input("Img Name")->content;
			assert(!img_name.empty() && "Img name is not valid");
			auto               depth_image_info = engine::image::create_info2d(self_node.window_provider->get_window_size(), 1, 1,
			                                                                self_node.render_graph->core->swapchain_ref->depth_format,
			                                                                vk::ImageUsageFlagBits::eDepthStencilAttachment |
			                                                                    vk::ImageUsageFlagBits::eSampled);
			engine::image_view *img_view        = self_node.render_graph->resources_manager->get_image(
                img_name, depth_image_info, 0, 0);
			self_node.set_ouput_data("out_depth_sampler", img_name);

			assert(img_view && "Image view must be valid");
		});
		callbacks_registry.at(n_vertex_input).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			std::string          vertex_name  = "vertex_input_" + std::to_string(self_node.global_id);
			engine::vertex_input *vertex_layout = self_node.render_graph->resources_manager->get_vertex_input(vertex_name);
			vertex_layout->binding_description.clear();
			vertex_layout->input_description.clear();

			size_t offset           = 0;
			int    location         = 0;
			int    binding          = 0;
			auto   dynamic_structure = get_from_name_in_map(
                self_node.dynamic_structures, "Vertex Builder");
			for (auto widget : dynamic_structure->widgets_infos)
			{
				int                          type_selected = widget.second.selectable_info.selected_idx;
				engine::vertex_input::attribs attribs      = (engine::vertex_input::attribs) type_selected;
				vertex_layout->add_vertex_attrib(attribs, binding, offset, location);
				offset += vertex_layout->get_size_from(attribs);
				location++;
			}
			for (auto widget : dynamic_structure->widgets_infos)
			{
				vertex_layout->add_vertex_input_binding(0, offset);
			}
			self_node.set_ouput_data("out_vertex_result", vertex_name);
		});
		callbacks_registry.at(n_comp_shader).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			engine::shader_stage stage       = engine::shader_stage::s_comp;
			auto                multi_option = get_from_name_in_map<ui::nodes::multi_option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).get_current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->get_or_create_default_shader(shader_selected, stage, engine::c_glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.set_ouput_data("out_shader", shader_idx);
		});
		callbacks_registry.at(n_vert_shader).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			engine::shader_stage stage       = engine::shader_stage::s_vert;
			auto                multi_option = get_from_name_in_map<ui::nodes::multi_option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).get_current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->get_or_create_default_shader(shader_selected, stage, engine::c_glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.set_ouput_data("out_shader", shader_idx);
		});
		callbacks_registry.at(n_frag_shader).add_callback("output_c", [](ui::nodes::graph_node &self_node) {
			engine::shader_stage stage       = engine::shader_stage::s_frag;
			auto                multi_option = get_from_name_in_map<ui::nodes::multi_option>(self_node.multi_options, "Shader Options");
			assert(multi_option);
			int         option_selected = multi_option->selected_idx;
			std::string shader_selected = "";
			switch (option_selected)
			{
				case 0:
					shader_selected = multi_option->scrollables.at(0).get_current<std::string>();
					break;
				case 1:
					shader_selected = multi_option->input_texts.at(1).content;
					self_node.render_graph->resources_manager->get_or_create_default_shader(shader_selected, stage, engine::c_glsl);
					break;
			}
			int shader_idx = -1;
			if (self_node.render_graph->resources_manager->shaders_names.contains(shader_selected))
			{
				shader_idx = self_node.render_graph->resources_manager->shaders_names.at(shader_selected);
			}
			self_node.set_ouput_data("out_shader", shader_idx);
		});

		callbacks_registry.at(n_render_node).add_callback("link_c", [](ui::nodes::graph_node &self_node) {
			pin_info *vert    = get_from_name_in_map(self_node.input_nodes, "Vertex Shader");
			pin_info *frag    = get_from_name_in_map(self_node.input_nodes, "Fragment Shader");
			pin_info *compute = get_from_name_in_map(self_node.input_nodes, "Compute Shader");
			self_node.toggle_draw(vert->id, true);
			self_node.toggle_draw(frag->id, true);
			self_node.toggle_draw(compute->id, true);
			if (compute->has_data())
			{
				self_node.toggle_draw(vert->id, false);
				self_node.toggle_draw(frag->id, false);
			}
			else if (vert->has_data() || frag->has_data())
			{
				self_node.toggle_draw(compute->id, false);
			}
		});

		callbacks_registry.at(n_render_node).add_callback("unlink_c", [](ui::nodes::graph_node &self_node) {
			pin_info *vert    = get_from_name_in_map(self_node.input_nodes, "Vertex Shader");
			pin_info *frag    = get_from_name_in_map(self_node.input_nodes, "Fragment Shader");
			pin_info *compute = get_from_name_in_map(self_node.input_nodes, "Compute Shader");

			self_node.toggle_draw(vert->id, true);
			self_node.toggle_draw(frag->id, true);
			self_node.toggle_draw(compute->id, true);
			if (compute->has_data())
			{
				self_node.toggle_draw(vert->id, false);
				self_node.toggle_draw(frag->id, false);
			}
			else if (vert->has_data() || frag->has_data())
			{
				self_node.toggle_draw(compute->id, false);
			}
		});
	}
};

struct graph_node_factory
{
	graph_node_builder     builder      = {};
	graph_node_registry    node_registry = {};
	engine::render_graph *render_graph;
	::window_provider      *window_provider;
	graph_node_res_manager *res_manager;

	ui::nodes::graph_node *get_node(node_type node_kind, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
	{
		assert(render_graph && "Null rgraph");
		assert(window_provider && "Null window Provider");
		assert(res_manager && "Null resManager");
		ui::nodes::graph_node node;
		switch (node_kind)
		{
			default:
				assert(false);
				break;
			case n_root_node:
				builder
				    .add_output(res_manager->next_widget_id(), {"out_root_node", n_root_node})
				    .set_node_id(res_manager->next_widget_id(), "Root Node");
				break;
			case n_render_node:
				builder
				    .add_text_input(res_manager->next_widget_id(), {"RenderNode Name"})
				    .add_output(res_manager->next_widget_id(), {"out_render_node", n_render_node})
				    .add_input(res_manager->next_widget_id(), {"Input Render Node", static_cast<node_type>(n_render_node | n_root_node)})
				    .add_input(res_manager->next_widget_id(), {"Vertex Shader", n_vert_shader})
				    .add_input(res_manager->next_widget_id(), {"Fragment Shader", n_frag_shader})
				    .add_input(res_manager->next_widget_id(), {"Compute Shader", n_comp_shader})
				    .add_input(res_manager->next_widget_id(), {"Col Attachment Node", n_col_attachment_structure}, true)
				    .add_input(res_manager->next_widget_id(), {"Storage Image Node", n_image_storage}, true)
				    .add_input(res_manager->next_widget_id(), {"Depth Attachment", n_depth_structure})
				    .add_input(res_manager->next_widget_id(), {"Vertex Input", n_vertex_input})
				    .add_selectable(res_manager->next_widget_id(), "Raster Configs", {"Fill", "Line", "Point"})
				    .set_node_id(res_manager->next_widget_id(), "Render Node");
				break;
			case n_col_attachment_structure:
				builder
				    .add_primitive_data(res_manager->next_widget_id(), {"Clear Color", vec4, glm::vec4(0.0)})
				    .add_selectable(res_manager->next_widget_id(), "Color Format", {"g_32bFormat", "g_16bFormat"})
				    .add_selectable(res_manager->next_widget_id(), "Load Operation", {"Load", "Clear", "Dont Care", "None"})
				    .add_selectable(res_manager->next_widget_id(), "Store Operation", {"Load", "eDontCare", "eNone"})
				    .add_selectable(res_manager->next_widget_id(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"}, 1)
				    .add_input(res_manager->next_widget_id(), {"Col Attachment Sampler", n_image_sampler})
				    .add_output(res_manager->next_widget_id(), {"out_col_structure", n_col_attachment_structure})
				    .set_node_id(res_manager->next_widget_id(), "Col Attachment Node");
				break;
			case n_depth_structure:
				builder
				    .add_selectable(res_manager->next_widget_id(), "Depth Configs", {"None", "Enable", "Disable"})
				    .add_input(res_manager->next_widget_id(), {"Depth Image In", n_depth_image_sampler})
				    .add_output(res_manager->next_widget_id(), {"out_depth_structure ", n_depth_structure})
				    .set_node_id(res_manager->next_widget_id(), "Depth Attachment Node");
				break;
			case n_push_constant:
				builder
				    .add_primitive_data(res_manager->next_widget_id(), {"Push Constant data", size_value, size_t(0)})
				    .add_output(res_manager->next_widget_id(), {"Push Constant Structure ", n_push_constant})
				    .set_node_id(res_manager->next_widget_id(), "Push Constant");
				break;
			case n_image_sampler:
				builder
				    .add_output(res_manager->next_widget_id(), {"out_img_sampler", n_image_sampler})
				    .set_node_id(res_manager->next_widget_id(), "Sampler Image Node");
				break;
			case n_image_storage:
				builder
				    .add_output(res_manager->next_widget_id(), {"out_img_storage", n_image_storage})
				    .set_node_id(res_manager->next_widget_id(), "Storage Image Node");
				break;
			case n_depth_image_sampler:
				builder
				    .add_text_input(res_manager->next_widget_id(), {"Img Name", "Image Name"})
				    .add_output(res_manager->next_widget_id(), {"out_depth_sampler", n_depth_image_sampler})
				    .set_node_id(res_manager->next_widget_id(), "Depth Image Node");
				break;
			case n_vertex_input:
			{
				enum_selectable   selectable("Vertex Attrib: ", {"INT", "FLOAT", "VEC2", "VEC3", "VE4", "U8VEC3", "U8VEC4", "COLOR_32"});
				dynamic_structure dynamic_structure_info("Vertex Builder", selectable);

				builder.add_dynamic_structure(res_manager->next_widget_id(), dynamic_structure_info)
				    .add_output(res_manager->next_widget_id(), {"out_vertex_result", n_vertex_input})
				    .set_node_id(res_manager->next_widget_id(), "Vertex Input Builder");
			}
			break;
			case n_vert_shader:
			case n_frag_shader:
			case n_comp_shader:
				std::vector<std::any> shader_paths;
				shader_paths.reserve(render_graph->resources_manager->shaders_names.size());
				std::vector<std::string> options = {"Pick Shader", "Create Shader"};
				std::map<int, text_input> text_inputs;
				text_inputs.try_emplace(1, text_input{"Select Shader Name"});
				std::map<int, scrollable> scrollables;

				if (node_kind == n_vert_shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::shader_stage::s_vert)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .set_node_id(res_manager->next_widget_id(), "Vert Shader")
					    .add_output(res_manager->next_widget_id(), {"out_shader", n_vert_shader});
				}
				if (node_kind == n_frag_shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::shader_stage::s_frag)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .set_node_id(res_manager->next_widget_id(), "Frag Shader")
					    .add_output(res_manager->next_widget_id(), {"out_shader", n_frag_shader});
				}
				if (node_kind == n_comp_shader)
				{
					for (auto &shader_path : render_graph->resources_manager->shaders_names)
					{
						if (render_graph->resources_manager->shaders.at(shader_path.second)->stage ==
						    engine::shader_stage::s_comp)
						{
							shader_paths.emplace_back(shader_path.first);
						}
					}
					builder
					    .set_node_id(res_manager->next_widget_id(), "Compute Shader")
					    .add_output(res_manager->next_widget_id(), {"out_shader", n_comp_shader});
				}
				scrollables.try_emplace(0, scrollable{"Possible Shaders", string_value, shader_paths});
				ui::nodes::multi_option multi_option_info("Shader Options", options, text_inputs, {}, scrollables, {});
				builder.add_multi_option(res_manager->next_widget_id(), multi_option_info);
				break;
		}

		std::function<void(ui::nodes::graph_node &)> *output_op = node_registry.get_callback(node_kind, "output_c");
		std::function<void(ui::nodes::graph_node &)> *link_op   = node_registry.get_callback(node_kind, "link_c");
		if (!name.empty())
		{
			builder.set_node_name(name);
		}
		builder.set_position(pos);
		builder.add_callback("output_c", output_op);
		builder.add_callback("link_c", link_op);

		int        id        = res_manager->next_node_id();
		ui::nodes::graph_node *graph_node = res_manager->get_node(id);
		builder.build(graph_node, render_graph, window_provider);
		res_manager->add_node_ids(graph_node);

		return graph_node;
	}
};
}        // namespace nodes

}        // namespace ui

#endif        // WIDGETS_HPP
