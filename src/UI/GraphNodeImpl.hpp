//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_IMPL_HPP
#define GRAPH_NODE_IMPL_HPP

namespace ui::nodes
{

void ui::nodes::Graph_Node::Update_Info()
{
	if (!update_data)
	{
		return;
	}
	// unoptimall
	Sort();
}

void ui::nodes::Graph_Node::Sort()
{
	std::vector<std::pair<int, Pin_Info>> inputs_map = Get_Vector_From_Map(input_nodes);
	std::sort(inputs_map.begin(), inputs_map.end(),
	          [](std::pair<int, Pin_Info> &a, std::pair<int, Pin_Info> &b) {
		          return a.second.node_kind < b.second.node_kind;
	          });
	input_nodes.clear();
	for (auto pair : inputs_map)
	{
		input_nodes.insert(pair);
		if (!graph_node_res_manager->input_outputs_ids.contains(pair.first))
		{
			graph_node_res_manager->input_outputs_ids.try_emplace(pair.first, global_id);
		}
	}
}

int ui::nodes::Graph_Node::Run_Callback(std::string callback)
{
	if (!callbacks.contains(callback) || callbacks.at(callback) == nullptr)
	{
		systems::Logger::Get_Instance()->Log_Message(callback + " Does not exist");
		return 1;
	}
	(*callbacks.at(callback))(*this);
	return 0;
}

void ui::nodes::Graph_Node::Draw()
{
	Update_Info();
	if (first_frame)
	{
		ed::SetNodePosition(node_id, ImVec2(pos.x, pos.y));
	}
	assert(node_id.Get() != -1);
	ImGui::PushID(node_id.Get());
	ed::BeginNode(node_id);
	ImGui::Text(name.c_str());

	// ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
	std::deque<int> added_widgets;

	for (auto &input : input_nodes)
	{
		if (drawable_widgets.at(input.second.id))
			input.second.Draw();
		if (add_more_widgets.contains(input.second.id))
		{
			if (ImGui::Button("+"))
			{
				added_widgets.push_front(input.second.id);
				update_data = true;
			}
		}
	}
	while (!added_widgets.empty())
	{
		int      id        = graph_node_res_manager->Next_Widget_Id();
		Pin_Info &copied_pin = input_nodes.at(added_widgets.front());
		Pin_Info  pin_info(copied_pin.name + "_" + std::to_string(id), copied_pin.node_kind);
		pin_info.data    = std::any();
		pin_info.pin_kind = copied_pin.pin_kind;
		pin_info.id      = id;

		input_nodes.try_emplace(id, pin_info);
		drawable_widgets.try_emplace(id, true);
		added_widgets.pop_front();
	}

	for (auto &selectable : selectables)
	{
		if (drawable_widgets.at(selectable.first))
			selectable.second.Draw(selectable.first);

		if (add_more_widgets.contains(selectable.first))
		{
		}
	}
	for (auto &t_input : text_inputs)
	{
		if (drawable_widgets.at(t_input.first))
			t_input.second.Draw(t_input.first);

		if (add_more_widgets.contains(t_input.first))
		{
		}
	}
	for (auto &primitive_info : primitives)
	{
		if (drawable_widgets.at(primitive_info.first))
			primitive_info.second.Draw(primitive_info.first);

		if (add_more_widgets.contains(primitive_info.first))
		{
		}
	}

	for (auto &scrollable : scrollables)
	{
		if (drawable_widgets.at(scrollable.first))
			scrollable.second.Draw(scrollable.first);

		if (add_more_widgets.contains(scrollable.first))
		{
		}
	}
	for (auto &multi_option : multi_options)
	{
		if (drawable_widgets.at(multi_option.first))
			multi_option.second.Draw(multi_option.first);

		if (add_more_widgets.contains(multi_option.first))
		{
		}
	}
	for (auto &dynamic_structure : dynamic_structures)
	{
		if (drawable_widgets.at(dynamic_structure.first))
			dynamic_structure.second.Draw(dynamic_structure.first);

		if (add_more_widgets.contains(dynamic_structure.first))
		{
		}
	}

	for (auto &output : output_nodes)
	{
		if (drawable_widgets.at(output.first))
			output.second.Draw();
		if (add_more_widgets.contains(output.second.id))
		{
			if (ImGui::Button("+"))
			{
				added_widgets.push_front(output.second.id);
				update_data = true;
			}
		}
	}
	while (!added_widgets.empty())
	{
		int      id        = graph_node_res_manager->Next_Widget_Id();
		Pin_Info &copied_pin = output_nodes.at(added_widgets.front());
		Pin_Info  pin_info(copied_pin.name + "_" + std::to_string(id), copied_pin.node_kind);
		pin_info.data    = std::any();
		pin_info.pin_kind = copied_pin.pin_kind;
		pin_info.id      = id;

		output_nodes.try_emplace(id, pin_info);
		drawable_widgets.try_emplace(id, true);
		added_widgets.pop_front();
	}

	ed::EndNode();
	ImGui::PopID();
	first_frame = false;
}

void ui::nodes::Graph_Node::Toggle_Draw(int id, bool draw_condition)
{
	assert(drawable_widgets.contains(id) && "Invalid Id");
	drawable_widgets.at(id) = draw_condition;
}

void ui::nodes::Graph_Node::Recompile_Node()
{
	valid = false;
	Run_Callback("output_c");
}

int ui::nodes::Graph_Node::Add_Link(int id, Node_Type node_kind)
{
	if (node_kind != N_None && !graph_nodes_links.contains(id))
	{
		graph_nodes_links.try_emplace(id, node_kind);
		return 0;
	}
	return 1;
}

std::map<Node_Type, ui::nodes::Graph_Node *> ui::nodes::Graph_Node::Get_Graph_Node_Ref(Node_Type node_kind)
{
	std::map<Node_Type, ui::nodes::Graph_Node *> graph_nodes;
	for (auto node_id : graph_nodes_links)
	{
		if (Is_Valid_Node_Combination(node_id.second, node_kind))
		{
			systems::Logger::Get_Instance()->Log_Message("Returning: " + node_type_strings.at(node_kind));
			graph_nodes.try_emplace(node_id.second, graph_node_res_manager->graph_nodes.at(node_id.second));
		}
	}
	return graph_nodes;
}
Pin_Info *ui::nodes::Graph_Node::Get_Input_Data_By_Id(int id)
{
	if (!input_nodes.contains(id))
	{
		return nullptr;
	}
	return &input_nodes.at(id);
}

Pin_Info *ui::nodes::Graph_Node::Get_Input_Data_By_Name(const std::string &name)
{
	for (auto &node : input_nodes)
	{
		if (node.second.name == name)
		{
			return &node.second;
		}
	}
	return nullptr;
}

Pin_Info *ui::nodes::Graph_Node::Get_Output_Data_By_Id(int id)
{
	if (!output_nodes.contains(id))
	{
		return nullptr;
	}
	return &output_nodes.at(id);
}

Pin_Info *ui::nodes::Graph_Node::Get_Output_Data_By_Name(const std::string &name)
{
	for (auto &node : output_nodes)
	{
		if (node.second.name == name)
		{
			return &node.second;
		}
	}
	return nullptr;
}

bool ui::nodes::Graph_Node::Has_Output(Node_Type type)
{
	for (auto &output : output_nodes)
	{
		if (output.second.node_kind == type)
		{
			return true;
		}
	}
	return false;
}
bool ui::nodes::Graph_Node::Has_Input(Node_Type type)
{
	for (auto &input : input_nodes)
	{
		if (input.second.node_kind == type)
		{
			return true;
		}
	}
	return false;
}

void ui::nodes::Graph_Node::Set_Ouput_Data(const std::string &name, std::any data)
{
	Get_Output_Data_By_Name(name)->data = data;
}

void ui::nodes::Graph_Node::Set_Ouput_Data(int id, std::any data)
{
	Get_Output_Data_By_Id(id)->data = data;
}

Scrollable *ui::nodes::Graph_Node::Get_Scrollable(const std::string &name)
{
	for (auto &scrollable : scrollables)
	{
		if (scrollable.second.name == name)
		{
			return &scrollable.second;
		}
	}
	assert(false && "ivalid name");
	return nullptr;
}

ui::nodes::Multi_Option *ui::nodes::Graph_Node::Get_Multi_Option(const std::string name)
{
	for (auto &multi_option : multi_options)
	{
		if (multi_option.second.name == name)
		{
			return &multi_option.second;
		}
	}
	assert(false && "invalid name");
	return nullptr;
}

Text_Input *ui::nodes::Graph_Node::Get_Text_Input(const std::string &name)
{
	for (auto &text_input : text_inputs)
	{
		if (text_input.second.name == name)
		{
			return &text_input.second;
		}
	}
	return nullptr;
}

Dynamic_Structure *ui::nodes::Graph_Node::Get_Dynamic_Structure(const std::string &name)
{
	for (auto &dynamic_structure : dynamic_structures)
	{
		if (dynamic_structure.second.name == name)
		{
			return &dynamic_structure.second;
		}
	}
	return nullptr;
}
};        // namespace ui::nodes

#endif        // GRAPH_NODE_IMPL_HPP
