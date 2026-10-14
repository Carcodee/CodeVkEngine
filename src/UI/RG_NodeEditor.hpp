//

// Created by carlo on 2025-01-07.
//

#ifndef RG_NODE_EDITOR_HPP
#define RG_NODE_EDITOR_HPP

namespace ui
{
namespace ed = ax::NodeEditor;

class Rg_Node_Editor
{
  public:
	void Set_Style()
	{
	}
	void Init(engine::Render_Graph *render_graph, ::Window_Provider *window_provider)
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

		std::vector<nodes::Node_Type> nodes_list = {
		    nodes::N_Root_Node,
		    nodes::N_Render_Node,
		    nodes::N_Render_Node,
		    nodes::N_Render_Node,
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
			factory.Get_Node(node_kind, glm::vec2(10.0, 0.0));
		}
	}

	void Draw()
	{
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		for (auto &node : res_manager.graph_nodes)
		{
			node.second->Draw();
		}
		Check_Links();
		first_frame = false;
		ed::End();
	}

	void Check_Links()
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
				nodes::Graph_Node *start_node = res_manager.Get_Node_By_Input_Output_Id(start_id.Get());
				nodes::Graph_Node *end_node   = res_manager.Get_Node_By_Input_Output_Id(end_id.Get());

				if (start_node && end_node)
				{
					ed::PinKind                               start_pin_type;
					ed::PinKind                               end_pin_type;
					std::map<ed::PinKind, nodes::Graph_Node *> pin_nodes;
					std::map<ed::PinKind, int>                pin_ids;

					nodes::Pin_Info *start_pin = start_node->input_nodes.contains(start_id.Get()) ? &start_node->input_nodes.at(start_id.Get()) : nullptr;
					start_pin_type             = ed::PinKind::Input;
					if (start_pin == nullptr)
					{
						start_pin     = start_node->output_nodes.contains(start_id.Get()) ? &start_node->output_nodes.at(start_id.Get()) : nullptr;
						start_pin_type = ed::PinKind::Output;
					}

					nodes::Pin_Info *end_pin = end_node->input_nodes.contains(end_id.Get()) ? &end_node->input_nodes.at(end_id.Get()) : nullptr;
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
						bool valid_link = nodes::Is_Valid_Node_Combination(start_pin->node_kind, end_pin->node_kind);

						if (valid_link)
						{
							nodes::Graph_Node *output_node = pin_nodes.at(ed::PinKind::Output);
							nodes::Graph_Node *input_node  = pin_nodes.at(ed::PinKind::Input);

							if (ed::AcceptNewItem())
							{
								output_node->Run_Callback("output_c");
								nodes::Pin_Info *output_pin = &output_node->output_nodes.at(pin_ids.at(ed::PinKind::Output));
								nodes::Pin_Info *input_pin  = &input_node->input_nodes.at(pin_ids.at(ed::PinKind::Input));
								output_pin->linked_pin      = input_pin;
								input_pin->linked_pin       = output_pin;

								input_node->Add_Link(output_node->global_id, output_node->output_nodes.at(pin_ids.at(ed::PinKind::Output)).node_kind);
								output_node->Add_Link(input_node->global_id, input_node->input_nodes.at(pin_ids.at(ed::PinKind::Input)).node_kind);
								input_node->Get_Input_Data_By_Id(pin_ids.at(ed::PinKind::Input))->data =
								    output_node->Get_Output_Data_By_Id(pin_ids.at(ed::PinKind::Output))->data;

								input_node->Run_Callback("link_c");
								output_node->Run_Callback("link_c");

								links.push_back({ed::LinkId(link_id_gen++), start_id, end_id});
								ed::Link(links.back().id, links.back().input_id, links.back().output_id);
							}
						}
					}
				}
				else
				{
					systems::Logger::Get_Instance()->Log_Message("Hello");
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
						nodes::Pin_Info *output = res_manager.Get_Pin_From_Id(link.output_id.Get());
						nodes::Pin_Info *input  = res_manager.Get_Pin_From_Id(link.input_id.Get());
						output->linked_pin      = nullptr;
						input->linked_pin       = nullptr;

						nodes::Graph_Node *input_node  = res_manager.Get_Node_By_Input_Output_Id(input->id);
						nodes::Graph_Node *output_node = res_manager.Get_Node_By_Input_Output_Id(output->id);
						// do some callback;
						input_node->Run_Callback("unlink_c");
						output_node->Run_Callback("unlink_c");

						links.erase(&link);
						break;
					}
				}
			}
		}
		ed::EndDelete();
	}

	ImVector<nodes::Link_Info>  links;
	nodes::Graph_Node_Factory    factory;
	nodes::Graph_Node_Res_Manager res_manager;
	// int_1 - in/out id || int_2 node idx in nodes vec
	std::map<int, int>   nodes_ids;
	int                  link_id_gen  = 200;
	bool                 first_frame = true;
	engine::Render_Graph *render_graph;
	::Window_Provider      *window_provider;
};
}        // namespace ui

#endif        // RG_NODE_EDITOR_HPP
