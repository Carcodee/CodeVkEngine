//

// Created by carlo on 2025-01-07.
//

#ifndef RG_NODE_EDITOR_HPP
#define RG_NODE_EDITOR_HPP

namespace ui
{
namespace ed = ax::NodeEditor;

class rg_node_editor
{
  public:
	void set_style()
	{
	}
	void init(engine::render_graph *render_graph, ::window_provider *window_provider)
	{
		if (!first_frame)
		{
			return;
		}
		this->render_graph            = render_graph;
		this->window_provider         = window_provider;
		this->factory.render_graph    = render_graph;
		this->factory.window_provider = window_provider;
		this->factory.res_manager     = &res_manager;

		std::vector<nodes::node_type> nodes_list = {
		    nodes::n_root_node,
		    nodes::n_render_node,
		    nodes::n_render_node,
		    nodes::n_render_node,
		    // nodes::n_vert_shader,
		    // nodes::n_frag_shader,
		    // nodes::n_comp_shader,
		    // nodes::n_col_attachment_structure,
		    // nodes::n_depth_structure,
		    // nodes::n_push_constant,
		    // nodes::n_image_sampler,
		    // nodes::n_image_storage,
		    // nodes::n_depth_image_sampler,
		    // nodes::n_vertex_input,
		    // N_BUFFER
		};
		// test
		for (const auto &node_kind : nodes_list)
		{
			factory.get_node(node_kind, glm::vec2(10.0, 0.0));
		}
	}

	void draw()
	{
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		for (auto &node : res_manager.graph_nodes)
		{
			node.second->draw();
		}
		check_links();
		first_frame = false;
		ed::End();
	}

	void check_links()
	{
		for (auto link : links)
		{
			ed::Link(link.id, link.input_id, link.output_id);
		}

		if (ed::BeginCreate())
		{
			ed::PinId start_id, end_id;
			if (ed::QueryNewLink(&start_id, &end_id))
			{
				nodes::graph_node *start_node = res_manager.get_node_by_input_output_id(start_id.Get());
				nodes::graph_node *end_node   = res_manager.get_node_by_input_output_id(end_id.Get());

				if (start_node && end_node)
				{
					ed::PinKind                               start_pin_type;
					ed::PinKind                               end_pin_type;
					std::map<ed::PinKind, nodes::graph_node *> pin_nodes;
					std::map<ed::PinKind, int>                pin_ids;

					nodes::pin_info *start_pin = start_node->input_nodes.contains(start_id.Get()) ? &start_node->input_nodes.at(start_id.Get()) : nullptr;
					start_pin_type             = ed::PinKind::Input;
					if (start_pin == nullptr)
					{
						start_pin     = start_node->output_nodes.contains(start_id.Get()) ? &start_node->output_nodes.at(start_id.Get()) : nullptr;
						start_pin_type = ed::PinKind::Output;
					}

					nodes::pin_info *end_pin = end_node->input_nodes.contains(end_id.Get()) ? &end_node->input_nodes.at(end_id.Get()) : nullptr;
					end_pin_type             = ed::PinKind::Input;
					if (end_pin == nullptr)
					{
						end_pin_type = ed::PinKind::Output;
						end_pin     = end_node->output_nodes.contains(end_id.Get()) ? &end_node->output_nodes.at(end_id.Get()) : nullptr;
					}

					pin_nodes.try_emplace(start_pin_type, start_node);
					pin_nodes.try_emplace(end_pin_type, end_node);

					pin_ids.try_emplace(start_pin_type, start_id.Get());
					pin_ids.try_emplace(end_pin_type, end_id.Get());

					if ((start_pin && end_pin) && (pin_nodes.size() == 2))
					{
						bool valid_link = nodes::is_valid_node_combination(start_pin->node_kind, end_pin->node_kind);

						if (valid_link)
						{
							nodes::graph_node *output_node = pin_nodes.at(ed::PinKind::Output);
							nodes::graph_node *input_node  = pin_nodes.at(ed::PinKind::Input);

							if (ed::AcceptNewItem())
							{
								output_node->run_callback("output_c");
								nodes::pin_info *output_pin = &output_node->output_nodes.at(pin_ids.at(ed::PinKind::Output));
								nodes::pin_info *input_pin  = &input_node->input_nodes.at(pin_ids.at(ed::PinKind::Input));
								output_pin->linked_pin      = input_pin;
								input_pin->linked_pin       = output_pin;

								input_node->add_link(output_node->global_id, output_node->output_nodes.at(pin_ids.at(ed::PinKind::Output)).node_kind);
								output_node->add_link(input_node->global_id, input_node->input_nodes.at(pin_ids.at(ed::PinKind::Input)).node_kind);
								input_node->get_input_data_by_id(pin_ids.at(ed::PinKind::Input))->data =
								    output_node->get_output_data_by_id(pin_ids.at(ed::PinKind::Output))->data;

								input_node->run_callback("link_c");
								output_node->run_callback("link_c");

								links.push_back({ed::LinkId(link_id_gen++), start_id, end_id});
								ed::Link(links.back().id, links.back().input_id, links.back().output_id);
							}
						}
					}
				}
				else
				{
					systems::logger::get_instance()->log_message("Hello");
				}
			}
		}
		ed::EndCreate();
		if (ed::BeginDelete())
		{
			ed::LinkId deleted_link_id;
			while (ed::QueryDeletedLink(&deleted_link_id))
			{
				for (auto &link : links)
				{
					if (link.id == deleted_link_id)
					{
						nodes::pin_info *output = res_manager.get_pin_from_id(link.output_id.Get());
						nodes::pin_info *input  = res_manager.get_pin_from_id(link.input_id.Get());
						output->linked_pin      = nullptr;
						input->linked_pin       = nullptr;

						nodes::graph_node *input_node  = res_manager.get_node_by_input_output_id(input->id);
						nodes::graph_node *output_node = res_manager.get_node_by_input_output_id(output->id);
						// do some callback;
						input_node->run_callback("unlink_c");
						output_node->run_callback("unlink_c");

						links.erase(&link);
						break;
					}
				}
			}
		}
		ed::EndDelete();
	}

	ImVector<nodes::link_info>  links;
	nodes::graph_node_factory    factory;
	nodes::graph_node_res_manager res_manager;
	// int_1 - in/out id || int_2 node idx in nodes vec
	std::map<int, int>   nodes_ids;
	int                  link_id_gen  = 200;
	bool                 first_frame = true;
	engine::render_graph *render_graph;
	::window_provider      *window_provider;
};
}        // namespace ui

#endif        // RG_NODE_EDITOR_HPP
