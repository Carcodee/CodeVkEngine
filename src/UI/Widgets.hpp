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
            N_IMAGE_SAMPLER,
            N_IMAGE_STORAGE,
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
       

        struct GraphNode
        {
            
            ed::NodeId nodeId;
            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, SelectableInfo> selectables;
            std::map<NodeType, std::any&> inputData;
            std::string name;
            glm::vec2 pos;
            bool firstFrame = true;
            
            std::function<std::any&(GraphNode&)>* outputFunction = nullptr;

            std::any& BuildOutput()
            {
                std::any& result = (*outputFunction)(*this);
                return result;
            }

            template <typename T>
            T& GetNodeInputData(NodeType nodeType)
            {
                std::any& anyData = inputData.at(nodeType);
                return std::any_cast<T>(anyData);
            }

            void Draw()
            {
                if (firstFrame)
                {
                    ed::SetNodePosition(nodeId, ImVec2(pos.x, pos.y));
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
                    ImGui::PushItemWidth(100.0);
                    int index = selectable.second.selectedIndex;
                    std::string label = selectable.second.options[index].c_str();
                    //temp solution
                    ImGui::SliderInt(label.c_str(), &selectable.second.selectedIndex, 0, selectable.second.options.size() -1);
                        
                    // if(ImGui::Button(selectable.second.options[selectable.second.selectedIndex].c_str(), ImVec2{50, 50}))
                    // {
                    //     ImGui::OpenPopup("SelectType");
                    // }
                    // if (ImGui::BeginPopup("SelectType"))
                    // {
                    //     for (int i = 0; i < selectable.second.options.size(); i++)
                    //     {
                    //         if (ImGui::Button(selectable.second.options[i].c_str(), ImVec2{25, 25}))
                    //         {
                    //             selectable.second.selectedIndex = i;
                    //         }
                    //     }
                    //     ImGui::EndPopup();
                    // }
                    ImGui::PopItemWidth();
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

            ed::NodeId nodeId = -1;
            std::map<int, PinInfo> inputNodes = {};
            std::map<int, PinInfo> outputNodes = {};
            std::map<int, SelectableInfo> selectables = {};
            std::string name = "";
            glm::vec2 pos = glm::vec2(0.0);
            std::function<std::any&(GraphNode&)>* outputOp;
            
            GraphNodeBuilder& SetNodeId(ed::NodeId id, std::string name)
            {
                nodeId = id;
                this->name = name;
                return *this;
            }

            GraphNodeBuilder& SetNodeName(std::string name)
            {
                this->name = name;
                return *this;
            }
            GraphNodeBuilder& SetLinkOp(std::function<std::any&(GraphNode&)>* outputOp)
            {
                this->outputOp = outputOp;
                return *this;
            }
            
            GraphNodeBuilder& SetPosition(glm::vec2 pos)
            {
                this->pos = pos;
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

            GraphNode Build(std::any* data)
            {
                // assert(nodeId.Get() > -1 && "Set the id before building");
                assert(outputOp && "Define a link operation");
                assert(!name.empty() && "Set a valid name");
                assert(data && "Data must be valid");

                GraphNode graphNode ={};
                graphNode.name = name;
                graphNode.nodeId = nodeId;
                graphNode.inputNodes = inputNodes;
                graphNode.outputNodes = outputNodes;
                graphNode.selectables = selectables;
                graphNode.outputFunction = outputOp;
                graphNode.inputData = {};
                graphNode.pos = pos;
                Reset();
                return graphNode;
            }
            void Reset()
            {
                inputNodes.clear();
                outputNodes.clear();
                selectables.clear();
                pos = glm::vec2(0.0);
                nodeId = -1;
                name = "";
            }

        };
        struct GraphNodeFactory
        {
            GraphNode GetNode(NodeType nodeType, std::any* data, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
            {

                GraphNode node;
                std::function<std::any&(GraphNode&)>* linkOp;
                switch (nodeType)
                {
                case N_RENDER_NODE:
                    linkOp = new std::function<std::any&(GraphNode& selfNode)>(
                        [](GraphNode& selfNode) -> std::any& {
                            std::any result;

                            for (auto& input : selfNode.inputNodes)
                            {
                                
                                
                            }
                            
                            
                            return result;
                        });

                    builder
                        .AddOutput(idGen++, {"Result", N_RENDER_NODE})
                        .AddInput(idGen++, {"Vertex Shader", N_SHADER})
                        .AddInput(idGen++, {"Fragment Shader", N_SHADER})
                        .AddInput(idGen++, {"Compute Shader", N_SHADER})
                        .AddInput(idGen++, {"Col Attachment Structure", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(idGen++, {"Depth Attachment", N_DEPTH_CONFIGS})
                        .AddSelectable(idGen++, "Raster Configs", {"Fill", "Line", "Point"})
                        .SetNodeId(idGen++, "Render Node");
                    break;
                case N_SHADER:
                    linkOp = new std::function<std::any&(GraphNode& selfNode)>(
                        [](GraphNode& selfNode) -> std::any& {
                            std::string shaderP = "";
                            std::any shader = ENGINE::ResourcesManager::GetInstance()->GetShader(
                                shaderP, ENGINE::S_COMP);
                            return shader;
                        });
                    
                    builder
                    
                        .AddInput(idGen++, {"Shader In", N_SHADER})
                        .AddOutput(idGen++, {"Shader Out", N_SHADER})
                        .SetLinkOp(linkOp)
                        .SetNodeId(idGen++, "Shader");
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    builder
                        .AddInput(idGen++, {"Col Attachment Info In", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(idGen++, {"Col Attachment Info In", N_IMAGE_SAMPLER})
                        .AddOutput(idGen++, {"Col Attachment Info Out", N_COL_ATTACHMENT_STRUCTURE})
                        .AddSelectable(idGen++, "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"})
                        .SetNodeId(idGen++, "Col Attachment Structure");
                    break;
                }

                if (!name.empty())
                {
                    builder.SetNodeName(name);
                }
                builder.SetPosition(pos);
                node = builder.Build(data);
                return node;
            }

            int idGen = 100;
            GraphNodeBuilder builder;
        };

    }

}

#endif //WIDGETS_HPP
