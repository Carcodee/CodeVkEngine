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

        enum NodeType
        {
            N_RENDER_NODE,
            N_SHADER,
            N_COL_ATTACHMENT_STRUCTURE,
            N_RASTER_CONFIGS,
            N_DEPTH_CONFIGS,
            N_VERTEX_INPUT
            
        };
        struct LinkInfo
        {
            ed::LinkId id;
            ed::PinId inputId;
            ed::PinId outputId;
        };
        struct PinInfo
        {
            std::string name;
            NodeType nodeType;
        };
        
        struct SelectableInfo
        {
            std::string name;
            std::vector<std::string>options;
            int selectedIndex;
        };
       

        template <typename T>
        struct GraphNode
        {
            ed::NodeId nodeId;
            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, SelectableInfo> selectables;
            T* data;
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
                for (auto& input : inputNodes)
                {
                    ImGui::PushID(input.first);
                    ed::BeginPin(input.first, ed::PinKind::Input);
                    ImGui::Text(input.second.name.c_str());
                    ed::EndPin();
                    ImGui::PopID();
                }
                for (auto& output : outputNodes)
                {
                    ImGui::PushID(output.first);
                    ed::BeginPin(output.first, ed::PinKind::Output);
                    ImGui::Text(output.second.name.c_str());
                    ed::EndPin();
                    ImGui::PopID();
                }
                for (auto& selectable : selectables)
                {
                    
                    ImGui::PushID(selectable.first);
                    ImGui::Text(selectable.second.name.c_str());
                    //
                    // if(ImGui::Button(selectable.second.options[selectable.second.selectedIndex].c_str(), ImVec2{50, 50}))
                    // {
                    //     ImGui::OpenPopup("Select Type");
                    //     
                    // }
                    // if (ImGui::BeginMenu("Select Type"))
                    // {
                    //     for (int i = 0; i < selectable.second.options.size(); i++)
                    //     {
                    //         if (ImGui::Selectable(selectable.second.options[i].c_str()))
                    //         {
                    //             selectable.second.selectedIndex = i;
                    //         }    
                    //     }
                    // }
                    // ImGui::PushItemWidth(100.0);
                    // ImGui::PopItemWidth();
                    // ImGui::Spacing();

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
            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, SelectableInfo> selectables;
            std::string name;
            
            GraphNodeBuilder& SetNodeId(ed::NodeId id, std::string name)
            {
                nodeId = id;
                this->name = name;
                return *this;
            }
            GraphNodeBuilder& AddInput(int id, PinInfo pinInfo)
            {
                inputNodes.try_emplace(id, pinInfo);
                return *this;
            }
            GraphNodeBuilder& AddOutput(int id, PinInfo pinInfo)
            {
                outputNodes.try_emplace(id, pinInfo);
                return *this;
            }
            
            GraphNodeBuilder& AddSelectable(int id, std::string name,std::vector<std::string> options)
            {
                selectables.try_emplace(id, SelectableInfo{name ,options, 0});
                return *this;
            }


            GraphNode<std::any> Build(std::any* data)
            {
                
                GraphNode<std::any> graphNode = {nodeId, inputNodes, outputNodes, selectables , data , name, true};
                Reset();
                return graphNode;
            }
            void Reset()
            {
                inputNodes.clear();
                outputNodes.clear();
                selectables.clear();
                nodeId = -1;
                name = "";
            }

        };
        struct GraphNodeFactory
        {

            GraphNode<std::any> GetNode(NodeType nodeType, std::any* data, std::string name)
            {
                GraphNode<std::any> node;

                switch (nodeType)
                {
                case N_RENDER_NODE:
                    builder.SetNodeId(idGen++, name)
                    .AddInput(idGen++, {"Vertex Shader", N_SHADER})
                    .AddInput(idGen++, {"Fragment Shader", N_SHADER})
                    .AddInput(idGen++, {"Compute Shader", N_SHADER})
                    .AddInput(idGen++, {"Raster Config", N_RASTER_CONFIGS})
                    .AddInput(idGen++, {"Col Attachment Structure", N_COL_ATTACHMENT_STRUCTURE})
                    .AddInput(idGen++, {"Depth Attachment", N_DEPTH_CONFIGS})
                    .AddOutput(idGen++, {"Result", N_RENDER_NODE});
                    break;
                case N_SHADER:
                    builder.SetNodeId(idGen++, name)
                    .AddInput(idGen++, {"Shader In", N_SHADER})
                    .AddOutput(idGen++, {"Shader Out", N_SHADER});
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    builder.SetNodeId(idGen++, name)
                    .AddInput(idGen++, {"Col Attachment Info In", N_COL_ATTACHMENT_STRUCTURE})
                    .AddOutput(idGen++, {"Col Attachment Info Out", N_COL_ATTACHMENT_STRUCTURE})
                    .AddSelectable(idGen++, "Col Attachment Config",{"Opt 1", "Opt2", "Opt3"});
                    break;
                }
                node = builder.Build(data);
                return node;
            }

            int idGen = 100;
            GraphNodeBuilder builder;
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
