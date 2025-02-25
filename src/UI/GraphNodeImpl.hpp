//



// Created by carlo on 2025-02-22.
//


#ifndef GRAPHNODEIMPL_HPP
#define GRAPHNODEIMPL_HPP

namespace UI::Nodes{
    
    void GraphNode::UpdateInfo()
    {
        if (!updateData) { return; }
        //unoptimall
        Sort();
    }

    void GraphNode::Sort()
    {
        std::vector<std::pair<int, PinInfo>> inputsMap = GetVectorFromMap(inputNodes); 
        std::sort(inputsMap.begin(), inputsMap.end(),
                  [](std::pair<int, PinInfo>& a, std::pair<int, PinInfo>& b)
                  {
                      return a.second.nodeType < b.second.nodeType;
                  });
        inputNodes.clear();
        for (auto pair : inputsMap)
        {
            inputNodes.insert(pair);
            if (!graphNodeResManager->inputOutputsIds.contains(pair.first))
            {
                graphNodeResManager->inputOutputsIds.try_emplace(pair.first, globalId);
            }
        }
        
        
    }

    int GraphNode::RunCallback(std::string callback)
    {
        if (!callbacks.contains(callback) || callbacks.at(callback) == nullptr)
        {
            SYSTEMS::Logger::GetInstance()->LogMessage(callback + " Does not exist");
            return 1;
        }
        (*callbacks.at(callback))(*this);
        return 0;
    }

    void GraphNode::Draw()
    {
        UpdateInfo();
        if (firstFrame)
        {
            ed::SetNodePosition(nodeId, ImVec2(pos.x, pos.y));
        }
        assert(nodeId.Get() != -1);
        ImGui::PushID(nodeId.Get());
        ed::BeginNode(nodeId);
        ImGui::Text(name.c_str());

        // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
        std::deque<int> addedWidgets;

        for (auto& input : inputNodes)
        {
            if (drawableWidgets.at(input.second.id))
                input.second.Draw();
            if (addMoreWidgets.contains(input.second.id))
            {
                if (ImGui::Button("+"))
                {
                    addedWidgets.push_front(input.second.id);
                    updateData = true;
                }
            }
        }
        while (!addedWidgets.empty())
        {
            int id = graphNodeResManager->NextWidgetID();
            PinInfo& copiedPin = inputNodes.at(addedWidgets.front());
            PinInfo pinInfo{
                .name = copiedPin.name + "_" + std::to_string(id),
                .nodeType = copiedPin.nodeType,
                .data = std::any(),
                .pinKind = copiedPin.pinKind,
                .id = id
            };
            inputNodes.try_emplace(id, pinInfo);
            drawableWidgets.try_emplace(id, true);
            addedWidgets.pop_front();
        }

        for (auto& selectable : selectables)
        {
            if (drawableWidgets.at(selectable.first))
                selectable.second.Draw(selectable.first);

            if (addMoreWidgets.contains(selectable.first))
            {
            }
        }
        for (auto& tInput : textInputs)
        {
            if (drawableWidgets.at(tInput.first))
                tInput.second.Draw(tInput.first);

            if (addMoreWidgets.contains(tInput.first))
            {
            }
        }
        for (auto& primitiveInfo : primitives)
        {
            if (drawableWidgets.at(primitiveInfo.first))
                primitiveInfo.second.Draw(primitiveInfo.first);

            if (addMoreWidgets.contains(primitiveInfo.first))
            {
            }
        }

        for (auto& scrollable : scrollables)
        {
            if (drawableWidgets.at(scrollable.first))
                scrollable.second.Draw(scrollable.first);

            if (addMoreWidgets.contains(scrollable.first))
            {
            }
        }
        for (auto& multiOption : multiOptions)
        {
            if (drawableWidgets.at(multiOption.first))
                multiOption.second.Draw(multiOption.first);

            if (addMoreWidgets.contains(multiOption.first))
            {
            }
        }
        for (auto& dynamicStructure : dynamicStructures)
        {
            if (drawableWidgets.at(dynamicStructure.first))
                dynamicStructure.second.Draw(dynamicStructure.first);

            if (addMoreWidgets.contains(dynamicStructure.first))
            {
            }
        }

        for (auto& output : outputNodes)
        {
            if (drawableWidgets.at(output.first))
                output.second.Draw();
            if (addMoreWidgets.contains(output.second.id))
            {
                if (ImGui::Button("+"))
                {
                    addedWidgets.push_front(output.second.id);
                    updateData = true;
                }
            }
        }
        while (!addedWidgets.empty())
        {
            int id = graphNodeResManager->NextWidgetID();
            PinInfo& copiedPin = outputNodes.at(addedWidgets.front());
            PinInfo pinInfo{
                .name = copiedPin.name,
                .nodeType = copiedPin.nodeType,
                .data = std::any(),
                .pinKind = copiedPin.pinKind,
                .id = id
            };
            outputNodes.try_emplace(id, pinInfo);
            drawableWidgets.try_emplace(id, true);
            addedWidgets.pop_front();
        }

        ed::EndNode();
        ImGui::PopID();
        firstFrame = false;
    }

    void GraphNode::ToggleDraw(int id, bool drawCondition)
    {
        assert(drawableWidgets.contains(id) && "Invalid Id");
        drawableWidgets.at(id) = drawCondition;
    }

    void GraphNode::RecompileNode()
    {
        valid = false;
        RunCallback("output_c");
    }


    int GraphNode::AddLink(int id, NodeType nodeType)
    {
        if (nodeType != N_NONE && !graphNodesLinks.contains(id))
        {
            graphNodesLinks.try_emplace(id, nodeType);
        }
        return 0;
    }

    PinInfo* GraphNode::GetInputDataById(int id)
    {
        if (!inputNodes.contains(id))
        {
            return nullptr;
        }
        return &inputNodes.at(id);
    }

    PinInfo* GraphNode::GetInputDataByName(const std::string& name)
    {
        for (auto& node : inputNodes)
        {
            if (node.second.name == name)
            {
                return &node.second;
            }
        }
        return nullptr;
    }

    PinInfo* GraphNode::GetOutputDataById(int id)
    {
        if (!outputNodes.contains(id))
        {
            return nullptr;
        }
        return &outputNodes.at(id);
    }

    PinInfo* GraphNode::GetOutputDataByName(const std::string& name)
    {
        for (auto& node : outputNodes)
        {
            if (node.second.name == name)
            {
                return &node.second;
            }
        }
        assert(false);
        return nullptr;
    }

    void GraphNode::SetOuputData(const std::string& name, std::any data)
    {
        GetOutputDataByName(name)->data = data;
    }

    void GraphNode::SetOuputData(int id, std::any data)
    {
        GetOutputDataById(id)->data = data;
    }


    Scrollable* GraphNode::GetScrollable(const std::string& name)
    {
        for (auto& scrollable : scrollables)
        {
            if (scrollable.second.name == name)
            {
                return &scrollable.second;
            }
        }
        assert(false && "ivalid name");
        return nullptr;
    }

    MultiOption* GraphNode::GetMultiOption(const std::string name)
    {
        for (auto& multiOption : multiOptions)
        {
            if (multiOption.second.name == name)
            {
                return &multiOption.second;
            }
        }
        assert(false && "invalid name");
        return nullptr;
    }

    TextInput* GraphNode::GetTextInput(const std::string& name)
    {
        for (auto& textInput : textInputs)
        {
            if (textInput.second.name == name)
            {
                return &textInput.second;
            }
        }
        return nullptr;
    }

    DynamicStructure* GraphNode::GetDynamicStructure(const std::string& name)
    {
        for (auto& dynamicStructure : dynamicStructures)
        {
            if (dynamicStructure.second.name == name)
            {
                return &dynamicStructure.second;
            }
        }
        return nullptr;
    }
};


#endif //GRAPHNODEIMPL_HPP
