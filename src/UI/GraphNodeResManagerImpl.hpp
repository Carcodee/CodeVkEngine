//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_RES_MANAGER_IMPL_HPP
#define GRAPH_NODE_RES_MANAGER_IMPL_HPP

namespace ui::nodes
{
graph_node_res_manager::graph_node_res_manager()
{
	graph_nodes_arena = std::make_unique<systems::arena>();
}

void graph_node_res_manager::add_node_id(int input_output_id, int graph_node_id)
{
	input_outputs_ids.try_emplace(input_output_id, graph_node_id);
}

void graph_node_res_manager::add_node_ids(ui::nodes::graph_node *node)
{
	for (auto &input : node->input_nodes)
	{
		add_node_id(input.first, node->global_id);
	}
	for (auto &output : node->output_nodes)
	{
		add_node_id(output.first, node->global_id);
	}
}

ui::nodes::graph_node *graph_node_res_manager::get_node(int id)
{
	if (graph_nodes.contains(id))
	{
		assert(false);
		return graph_nodes.at(id);
	}
	ui::nodes::graph_node *graph_node = graph_nodes_arena->alloc<ui::nodes::graph_node>();
	graph_nodes.try_emplace(id, graph_node);
	graph_node->graph_nodes_ref       = &graph_nodes;
	graph_node->widgets_id_gen        = &widgets_id_gen;
	graph_node->global_id            = id;
	graph_node->graph_nodes_ref       = &graph_nodes;
	graph_node->graph_node_res_manager = this;
	return graph_node;
}

ui::nodes::graph_node *graph_node_res_manager::get_node_by_input_output_id(int id)
{
	if (!input_outputs_ids.contains(id))
	{
		return nullptr;
	}

	return graph_nodes.at(input_outputs_ids.at(id));
}

pin_info *graph_node_res_manager::get_pin_from_id(int id)
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
int graph_node_res_manager::next_widget_id()
{
	return widgets_id_gen++;
}

int graph_node_res_manager::next_node_id()
{
	return id_node_gen++;
}
}        // namespace ui::nodes

#endif        // GRAPH_NODE_RES_MANAGER_IMPL_HPP
