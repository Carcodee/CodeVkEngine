//
// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_RES_MANAGER_HPP
#define GRAPH_NODE_RES_MANAGER_HPP

namespace ui::nodes
{
struct Graph_Node;
struct Pin_Info;
struct Graph_Node_Res_Manager
{
	std::unique_ptr<systems::Arena> graph_nodes_arena;

	std::unordered_map<int, int> input_outputs_ids;

	std::unordered_map<int, ui::nodes::Graph_Node *> graph_nodes;

	int widgets_id_gen = 100;
	int id_node_gen    = 100;

	Graph_Node_Res_Manager();
	void       Add_Node_Id(int input_output_id, int graph_node_id);
	void       Add_Node_Ids(ui::nodes::Graph_Node *node);
	ui::nodes::Graph_Node *Get_Node(int id);
	ui::nodes::Graph_Node *Get_Node_By_Input_Output_Id(int id);
	Pin_Info   *Get_Pin_From_Id(int id);
	int        Next_Widget_Id();
	int        Next_Node_Id();
};
}        // namespace ui::nodes

#endif        // GRAPH_NODE_RES_MANAGER_HPP
