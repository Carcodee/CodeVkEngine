//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPH_NODE_IMPL_HPP
#define GRAPH_NODE_IMPL_HPP

namespace ui::nodes
{

void ui::nodes::graph_node::update_info()
{
	if (!update_data)
	{
		return;
	}
	// unoptimall
	sort();
}

void ui::nodes::graph_node::sort()
{
	std::vector<std::pair<int, pin_info>> inputs_map = get_vector_from_map(input_nodes);
	std::sort(inputs_map.begin(), inputs_map.end(),
	          [](std::pair<int, pin_info> &a, std::pair<int, pin_info> &b) {
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

int ui::nodes::graph_node::run_callback(std::string callback)
{
	if (!callbacks.contains(callback) || callbacks.at(callback) == nullptr)
	{
		systems::logger::get_instance()->log_message(callback + " Does not exist");
		return 1;
	}
	(*callbacks.at(callback))(*this);
	return 0;
}

void ui::nodes::graph_node::draw()
{
	update_info();
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
			input.second.draw();
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
		int      id        = graph_node_res_manager->next_widget_id();
		pin_info &copied_pin = input_nodes.at(added_widgets.front());
		pin_info  pin_info(copied_pin.name + "_" + std::to_string(id), copied_pin.node_kind);
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
			selectable.second.draw(selectable.first);

		if (add_more_widgets.contains(selectable.first))
		{
		}
	}
	for (auto &t_input : text_inputs)
	{
		if (drawable_widgets.at(t_input.first))
			t_input.second.draw(t_input.first);

		if (add_more_widgets.contains(t_input.first))
		{
		}
	}
	for (auto &primitive_info : primitives)
	{
		if (drawable_widgets.at(primitive_info.first))
			primitive_info.second.draw(primitive_info.first);

		if (add_more_widgets.contains(primitive_info.first))
		{
		}
	}

	for (auto &scrollable : scrollables)
	{
		if (drawable_widgets.at(scrollable.first))
			scrollable.second.draw(scrollable.first);

		if (add_more_widgets.contains(scrollable.first))
		{
		}
	}
	for (auto &multi_option : multi_options)
	{
		if (drawable_widgets.at(multi_option.first))
			multi_option.second.draw(multi_option.first);

		if (add_more_widgets.contains(multi_option.first))
		{
		}
	}
	for (auto &dynamic_structure : dynamic_structures)
	{
		if (drawable_widgets.at(dynamic_structure.first))
			dynamic_structure.second.draw(dynamic_structure.first);

		if (add_more_widgets.contains(dynamic_structure.first))
		{
		}
	}

	for (auto &output : output_nodes)
	{
		if (drawable_widgets.at(output.first))
			output.second.draw();
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
		int      id        = graph_node_res_manager->next_widget_id();
		pin_info &copied_pin = output_nodes.at(added_widgets.front());
		pin_info  pin_info(copied_pin.name + "_" + std::to_string(id), copied_pin.node_kind);
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

void ui::nodes::graph_node::toggle_draw(int id, bool draw_condition)
{
	assert(drawable_widgets.contains(id) && "Invalid Id");
	drawable_widgets.at(id) = draw_condition;
}

void ui::nodes::graph_node::recompile_node()
{
	valid = false;
	run_callback("output_c");
}

int ui::nodes::graph_node::add_link(int id, node_type node_kind)
{
	if (node_kind != n_none && !graph_nodes_links.contains(id))
	{
		graph_nodes_links.try_emplace(id, node_kind);
		return 0;
	}
	return 1;
}

std::map<node_type, ui::nodes::graph_node *> ui::nodes::graph_node::get_graph_node_ref(node_type node_kind)
{
	std::map<node_type, ui::nodes::graph_node *> graph_nodes;
	for (auto node_id : graph_nodes_links)
	{
		if (is_valid_node_combination(node_id.second, node_kind))
		{
			systems::logger::get_instance()->log_message("Returning: " + node_type_strings.at(node_kind));
			graph_nodes.try_emplace(node_id.second, graph_node_res_manager->graph_nodes.at(node_id.second));
		}
	}
	return graph_nodes;
}
pin_info *ui::nodes::graph_node::get_input_data_by_id(int id)
{
	if (!input_nodes.contains(id))
	{
		return nullptr;
	}
	return &input_nodes.at(id);
}

pin_info *ui::nodes::graph_node::get_input_data_by_name(const std::string &name)
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

pin_info *ui::nodes::graph_node::get_output_data_by_id(int id)
{
	if (!output_nodes.contains(id))
	{
		return nullptr;
	}
	return &output_nodes.at(id);
}

pin_info *ui::nodes::graph_node::get_output_data_by_name(const std::string &name)
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

bool ui::nodes::graph_node::has_output(node_type type)
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
bool ui::nodes::graph_node::has_input(node_type type)
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

void ui::nodes::graph_node::set_ouput_data(const std::string &name, std::any data)
{
	get_output_data_by_name(name)->data = data;
}

void ui::nodes::graph_node::set_ouput_data(int id, std::any data)
{
	get_output_data_by_id(id)->data = data;
}

scrollable *ui::nodes::graph_node::get_scrollable(const std::string &name)
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

ui::nodes::multi_option *ui::nodes::graph_node::get_multi_option(const std::string name)
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

text_input *ui::nodes::graph_node::get_text_input(const std::string &name)
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

dynamic_structure *ui::nodes::graph_node::get_dynamic_structure(const std::string &name)
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
