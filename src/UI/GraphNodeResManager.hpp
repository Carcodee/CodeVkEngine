//
// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_RES_MANAGER_HPP
#define GRAPH_NODE_RES_MANAGER_HPP

namespace ui::nodes
{
struct graph_node;
struct pin_info;
struct graph_node_res_manager
{
	std::unique_ptr<systems::arena> graph_nodes_arena;

	std::unordered_map<int, int> input_outputs_ids;

	std::unordered_map<int, ui::nodes::graph_node *> graph_nodes;

	int widgets_id_gen = 100;
	int id_node_gen    = 100;

	graph_node_res_manager();
	void       add_node_id(int input_output_id, int graph_node_id);
	void       add_node_ids(ui::nodes::graph_node *node);
	ui::nodes::graph_node *get_node(int id);
	ui::nodes::graph_node *get_node_by_input_output_id(int id);
	pin_info   *get_pin_from_id(int id);
	int        next_widget_id();
	int        next_node_id();
};
}        // namespace ui::nodes

#endif        // GRAPH_NODE_RES_MANAGER_HPP
