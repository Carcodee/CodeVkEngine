//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_RES_MANAGER_IMPL_HPP
#define GRAPH_NODE_RES_MANAGER_IMPL_HPP

namespace ui::nodes
{
Graph_Node_Res_Manager::Graph_Node_Res_Manager()
{
	graph_nodes_arena = std::make_unique<systems::Arena>();
}

void Graph_Node_Res_Manager::Add_Node_Id(int input_output_id, int graph_node_id)
{
	input_outputs_ids.try_emplace(input_output_id, graph_node_id);
}

void Graph_Node_Res_Manager::Add_Node_Ids(ui::nodes::Graph_Node *node)
{
	for (auto &input : node->input_nodes)
	{
		Add_Node_Id(input.first, node->global_id);
	}
	for (auto &output : node->output_nodes)
	{
		Add_Node_Id(output.first, node->global_id);
	}
}

ui::nodes::Graph_Node *Graph_Node_Res_Manager::Get_Node(int id)
{
	if (graph_nodes.contains(id))
	{
		assert(false);
		return graph_nodes.at(id);
	}
	ui::nodes::Graph_Node *graph_node = graph_nodes_arena->Alloc<ui::nodes::Graph_Node>();
	graph_nodes.try_emplace(id, graph_node);
	graph_node->graph_nodes_ref       = &graph_nodes;
	graph_node->widgets_id_gen        = &widgets_id_gen;
	graph_node->global_id            = id;
	graph_node->graph_nodes_ref       = &graph_nodes;
	graph_node->graph_node_res_manager = this;
	return graph_node;
}

ui::nodes::Graph_Node *Graph_Node_Res_Manager::Get_Node_By_Input_Output_Id(int id)
{
	if (!input_outputs_ids.contains(id))
	{
		return nullptr;
	}

	return graph_nodes.at(input_outputs_ids.at(id));
}

Pin_Info *Graph_Node_Res_Manager::Get_Pin_From_Id(int id)
{
	for (auto &graph_node : graph_nodes)
	{
		if (graph_node.second->input_nodes.contains(id))
		{
			return &graph_node.second->input_nodes.at(id);
		}
		if (graph_node.second->output_nodes.contains(id))
		{
			return &graph_node.second->output_nodes.at(id);
		}
	}
	return nullptr;
}
int Graph_Node_Res_Manager::Next_Widget_Id()
{
	return widgets_id_gen++;
}

int Graph_Node_Res_Manager::Next_Node_Id()
{
	return id_node_gen++;
}
}        // namespace ui::nodes

#endif        // GRAPH_NODE_RES_MANAGER_IMPL_HPP
