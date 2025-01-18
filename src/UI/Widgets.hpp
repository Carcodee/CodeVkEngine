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
            std::vector<ed::PinId> inputNodes;
            std::vector<ed::PinId> outputNodes;
            std::string name;
            bool firstFrame = true;
            
            void SetName(std::string name)
            {
                this->name = name;
            }
            void SetNodeId(int id)
            {
                nodeId = id;
            }
            void AddInput(int id)
            {
                inputNodes.push_back(id);
            }
            void AddOutput(int id)
            {
                outputNodes.push_back(id);
            }
            void Draw()
            {
                // if (firstFrame)
                // {
                //     ed::SetNodePosition(nodeId, ImVec2(0, 10));
                // }
                ImGui::PushID(nodeId.Get());
                ed::BeginNode(nodeId);
                ImGui::Text(name.c_str());
                for (auto input : inputNodes)
                {
                    ImGui::PushID(input.Get());
                    ed::BeginPin(input, ed::PinKind::Input);
                    ImGui::Text("-> In");
                    ed::EndPin();
                    ImGui::PopID();
                    
                }
                for (auto output : outputNodes)
                {
                    ImGui::PushID(output.Get());
                    ed::BeginPin(output, ed::PinKind::Output);
                    ImGui::Text("-> Out");
                    ed::EndPin();
                    ImGui::PopID();
                }
                ed::EndNode();
                ImGui::PopID();
                firstFrame = false;
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
