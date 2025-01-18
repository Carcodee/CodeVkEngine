//
// Created by carlo on 2025-01-02.
//


#ifndef WIDGETS_HPP
#define WIDGETS_HPP

namespace UI{

    enum WidgetsProperty
    {
        DRAG,
        DROP
    };

    template<typename T>
    static void UseDragProperty(std::string name, T& payload, size_t payloadSize)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(name.c_str() , &payload, payloadSize);
            ImGui::EndDragDropSource();
        }
    }
    template<typename T>
    static T UseDropProperty(std::string name)
    {
        if (ImGui::BeginDragDropTarget())
        {
            T data = nullptr;
            if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload(name.c_str()))
            {
                //				const char* data = (const char*)payload->Data;
                data = *(T*)imguiPayload->Data;

                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                ImGui::BeginTooltip();
                ImGui::Text("+");

                ImGui::EndTooltip();
            }
            ImGui::EndDragDropTarget();
            return data;
        }else
        {
            return nullptr;
        }
    }

    struct IWidget
    {
        virtual ~IWidget()= default;
        virtual void DisplayProperties() = 0;
    };


    struct TextureViewer: IWidget
    {
        ENGINE::ImageView* currImageView = nullptr;
        std::string labelName;
        std::set<WidgetsProperty> properties;
        ~TextureViewer() override = default;
        void DisplayProperties() override
        {
            for (auto& prop: properties)
            {
                switch (prop)
                {
                case DRAG:
                    UseDragProperty("TEXTURE_NAME", currImageView, sizeof(ENGINE::ImageView*));
                    break;
                case DROP:
                    ENGINE::ImageView* newImgView = UseDropProperty<ENGINE::ImageView*>("TEXTURE_NAME");
                    if (newImgView != nullptr)
                    {
                        currImageView = newImgView;
                    }
                    break;
                }
            }
            
        }
        ENGINE::ImageView* DisplayTexture(std::string name, ENGINE::ImageView* imageView, ImTextureID textureId, glm::vec2 size)
        {
            labelName = name;
            this->currImageView = imageView;

            ImGui::PushID(name.c_str());
            ImGui::SeparatorText(name.c_str());
		    ImGui::ImageButton(name.c_str(), textureId, ImVec2{size.x, size.y});
            DisplayProperties();
		    ImGui::PopID();
            
            return this->currImageView;
        }
        void AddProperty(WidgetsProperty property){
            properties.emplace(property);
        }
    };

    namespace Nodes
    {

        namespace ed = ax::NodeEditor;

        struct LinkInfo
        {
            ed::LinkId id;
            ed::PinId inputId;
            ed::PinId outputId;
        };

                struct GraphNode
        {
            ed::NodeId nodeId;
            std::map<int, std::string> inputNodes;
            std::map<int, std::string> outputNodes;
            std::string name;
            bool firstFrame = true;

            void Draw()
            {
                if (firstFrame)
                {
                    ed::SetNodePosition(nodeId, ImVec2(0, 10));
                }
                ImGui::PushID(nodeId.Get());
                ed::BeginNode(nodeId);
                ImGui::Text(name.c_str());
                for (auto input : inputNodes)
                {
                    ImGui::PushID(input.first);
                    ed::BeginPin(input.first, ed::PinKind::Input);
                    ImGui::Text(input.second.c_str());
                    ed::EndPin();
                    ImGui::PopID();
                }
                for (auto output : outputNodes)
                {
                    ImGui::PushID(output.first);
                    ed::BeginPin(output.first, ed::PinKind::Output);
                    ImGui::Text(output.second.c_str());
                    ed::EndPin();
                    ImGui::PopID();
                }
                ed::EndNode();
                ImGui::PopID();
                firstFrame = false;
            }
        };
        struct GraphNodeBuilder
        {

            ed::NodeId nodeId;
            std::map<int, std::string> inputNodes;
            std::map<int, std::string> outputNodes;
            std::string name;
            
            GraphNodeBuilder* SetNodeId(ed::NodeId id, std::string name)
            {
                nodeId = id;
                this->name = name;
                return this;
            }
            GraphNodeBuilder* AddInput(int id, std::string name)
            {
                inputNodes.try_emplace(id, name);
                return this;
            }
            GraphNodeBuilder* AddOutput(int id, std::string name)
            {
                outputNodes.try_emplace(id, name);
                return this;
            }

            GraphNode Build()
            {
                GraphNode graphNode = {nodeId, inputNodes, outputNodes, name, true};
                inputNodes.clear();
                outputNodes.clear();
                nodeId = -1;
                name = "";
                return graphNode;
            }
            
            

        };


        static void BaseNode(bool firstFrame)
        {
            
                int uniqueId = 1;
                // Submit Node B
                ed::NodeId nodeA_Id = uniqueId++;
                ed::PinId nodeA_InputPinId = uniqueId++;
                ed::PinId nodeA_OutputPinId = uniqueId++;

                ed::Begin("My Editor", ImVec2(0.0, 0.0f));

                // Start drawing nodes.
            
                if (firstFrame)
                {
                    ed::SetNodePosition(nodeA_Id, ImVec2(0, 10));
                }
                ed::BeginNode(nodeA_Id);
                ImGui::Text("Node A");
                ed::BeginPin(nodeA_InputPinId, ed::PinKind::Input);
                ImGui::Text("-> In");
                ed::EndPin();
                ImGui::SameLine();
                ed::BeginPin(nodeA_OutputPinId, ed::PinKind::Output);
                ImGui::Text("Out ->");
                ed::EndPin();
                ed::EndNode();

                ed::NodeId nodeB_Id = uniqueId++;
                ed::PinId nodeB_InputPinId1 = uniqueId++;
                ed::PinId nodeB_InputPinId2 = uniqueId++;
                ed::PinId nodeB_OutputPinId = uniqueId++;

                // Start drawing nodes.
                if (firstFrame)
                {
                    ed::SetNodePosition(nodeB_Id, ImVec2(10, 10));
                }
                ed::BeginNode(nodeB_Id);
                ImGui::Text("Node A");
                ed::BeginPin(nodeB_InputPinId1, ed::PinKind::Input);
                ImGui::Text("-> In_1");
                ed::EndPin();
                ed::BeginPin(nodeB_InputPinId2, ed::PinKind::Input);
                ImGui::Text("-> In_2");
                ed::EndPin();
                ImGui::SameLine();
                ed::BeginPin(nodeB_OutputPinId, ed::PinKind::Output);
                ImGui::Text("Out ->");
                ed::EndPin();
                ed::EndNode();
            
    		ed::End();
            
        }
        
    }

}

#endif //WIDGETS_HPP
