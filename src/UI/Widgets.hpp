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

        struct TextInputInfo 
        {
            std::string name;
            std::string content;
        };
        
        struct  PrimitiveInfo
        {
            std::string name;
            std::any data;
        };
       

        //note that this only handle copyable data types
        struct GraphNode
        {
            
            ed::NodeId nodeId;
            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, SelectableInfo> selectables;
            std::map<int, TextInputInfo> textInputs;
            std::map<int, PrimitiveInfo> nodeData;
            
            std::map<NodeType, std::any> inputData;
            std::map<NodeType, std::any> outputData;
            
            std::map<NodeType, GraphNode&> graphNodes;
            
            std::string name;
            glm::vec2 pos;
            bool firstFrame = true;
            
            ENGINE::RenderGraph* renderGraph;
            
            std::function<std::any(GraphNode&)>* outputFunction = nullptr;

            std::any BuildOutput()
            {
                std::any result = (*outputFunction)(*this);
                return result;
            }

            template <typename T>
            T* GetNodeInputData(NodeType nodeType)
            {
                if (inputData.contains(nodeType))
                {
                    std::any& anyData = inputData.at(nodeType);
                    return std::any_cast<T*>(anyData);
                }
                return nullptr;
            }

            bool ContainsInput(NodeType nodeType)
            {
                if (!inputData.contains(nodeType))
                {
                    return false;
                }
                if (!inputData.at(nodeType).has_value())
                {
                    return false;
                }
                return true;
            }

            int GetSelectableIndex(const std::string& name)
            {
                for (auto& selectable : selectables)
                {
                    if (selectable.second.name == name)
                    {
                        return selectable.second.selectedIndex;
                    }
                }
                return -1;
                
            }
            
            std::string GetInputTextContent (const std::string& name)
            {
                for (auto& textInput : textInputs)
                {
                    if (textInput.second.name == name)
                    {
                        return textInput.second.content;
                    }
                }
                return "";
                
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
                for (auto& tInput : textInputs)
                {
                    ImGui::PushItemWidth(100.0);
                    ImGui::PushID(tInput.first);
                    constexpr size_t size = 128;
                    char inputT[size];
                    std::strncpy(inputT, tInput.second.content.c_str(), size);
                    if (ImGui::InputText(tInput.second.name.c_str(), inputT, size))
                    {
                        tInput.second.content = std::string(inputT);
                    }
                    ImGui::PopID();
                    ImGui::PopItemWidth();
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
            std::map<int, TextInputInfo> textInputs;
            
            std::string name = "";
            glm::vec2 pos = glm::vec2(0.0);
            std::function<std::any(GraphNode&)>* outputOp;
            
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
            GraphNodeBuilder& SetLinkOp(std::function<std::any(GraphNode&)>* outputOp)
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
            GraphNodeBuilder& AddTextInput(int id, TextInputInfo info)
            {
                textInputs.try_emplace(id, info);
                return *this;
            }

            GraphNode Build( ENGINE::RenderGraph* renderGraph)
            {
                // assert(nodeId.Get() > -1 && "Set the id before building");
                assert(outputOp && "Define a link operation");
                assert(!name.empty() && "Set a valid name");

                GraphNode graphNode ={};
                graphNode.name = name;
                graphNode.nodeId = nodeId;
                graphNode.inputNodes = inputNodes;
                graphNode.outputNodes = outputNodes;
                graphNode.selectables = selectables;
                graphNode.textInputs = textInputs;
                graphNode.outputFunction = outputOp;
                graphNode.inputData = {};
                graphNode.pos = pos;
                graphNode.renderGraph = renderGraph;
                for (auto& input : graphNode.inputNodes)
                {
                    graphNode.inputData.try_emplace(input.second.nodeType, std::string("Empty"));
                }
                for (auto& output : graphNode.outputNodes)
                {
                    graphNode.outputData.try_emplace(output.second.nodeType, std::string("Empty"));
                }
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
            int idGen = 100;
            GraphNodeBuilder builder;
            ENGINE::RenderGraph* renderGraph;
            WindowProvider* windowProvider;
            int NextID()
            {
                return idGen++;
            }
            GraphNode GetNode(NodeType nodeType, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
            {
                assert(renderGraph && "Null rgraph");
                assert(windowProvider && "Null window Provider");
                GraphNode node;
                std::function<std::any(GraphNode&)>* linkOp;
                switch (nodeType)
                {
                case N_RENDER_NODE:
                    linkOp = new std::function<std::any(GraphNode& selfNode)>(
                        [this](GraphNode& selfNode) -> std::any {

                            auto node = renderGraph->AddPass(selfNode.name);
                            for (auto& input : selfNode.inputData)
                            {
                                switch (input.first)
                                {
                                case N_COL_ATTACHMENT_STRUCTURE:
                                    GraphNode& inputNode = *selfNode.GetNodeInputData<GraphNode>(N_COL_ATTACHMENT_STRUCTURE);
                                    if(inputNode.ContainsInput(input.first))
                                    {
                                        ENGINE::AttachmentInfo info = *inputNode.GetNodeInputData<ENGINE::AttachmentInfo>(input.first);
                                        ENGINE::BlendConfigs blendConfigs = (ENGINE::BlendConfigs)inputNode.GetSelectableIndex("BlendConfig");
                                        node->AddColorAttachmentOutput(inputNode.name, info, blendConfigs);
                                    }
                                    if (inputNode.ContainsInput(N_IMAGE_SAMPLER))
                                    {
                                        ENGINE::ImageView* img = *inputNode.GetNodeInputData<ENGINE::ImageView*>(input.first);
                                        node->AddSamplerResource(inputNode.name, img);
                                    }
                                    break;
                                }
                            }
                            std::any result = selfNode.name;
                            return result;
                        });

                    builder
                        .AddOutput(NextID(), {"Result", N_RENDER_NODE})
                        .AddInput(NextID(), {"Vertex Shader", N_SHADER})
                        .AddInput(NextID(), {"Fragment Shader", N_SHADER})
                        .AddInput(NextID(), {"Compute Shader", N_SHADER})
                        .AddInput(NextID(), {"Col Attachment Structure", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(NextID(), {"Depth Attachment", N_DEPTH_CONFIGS})
                        .AddInput(NextID(), {"Col Attachment Info In", N_IMAGE_SAMPLER})
                        .AddSelectable(NextID(), "Raster Configs", {"Fill", "Line", "Point"})
                        .SetNodeId(NextID(), "Render Node");
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    linkOp = new std::function<std::any(GraphNode& selfNode)>(
                        [this](GraphNode& selfNode) -> std::any
                        {
                            return "";
                        });
                    builder
                        .AddInput(NextID(), {"Clear Color", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(NextID(), {"Clear Color", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(NextID(), {"Col Attachment Info In", N_IMAGE_SAMPLER})
                        .AddOutput(NextID(), {"Col Attachment Info Out", N_COL_ATTACHMENT_STRUCTURE})
                        .AddSelectable(NextID(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"})
                        .SetNodeId(NextID(), "Col Attachment Structure");
                    break;
                case N_IMAGE_SAMPLER:
                    linkOp = new std::function<std::any(GraphNode& selfNode)>(
                        [this](GraphNode& selfNode) -> std::any {

                            std::string imgName = selfNode.GetInputTextContent("Img Name");
                            assert(!imgName.empty() && "Img name is not valid");
                            auto imageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                                 ENGINE::g_32bFormat,
                                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                                 vk::ImageUsageFlagBits::eSampled);
                            ENGINE::ImageView* imgView = renderGraph->resourcesManager->GetImage(imgName, imageInfo, 0, 0);

                            assert(imgView && "Image view must be valid");
                            return imgName;
                        });
                     builder
                         .AddTextInput(NextID(), {"Img Name", "Image Name"})
                         .AddOutput(NextID(), {"Image Sampler Result", N_IMAGE_SAMPLER})
                         .SetLinkOp(linkOp)
                         .SetNodeId(NextID(), "Image Node");
                    break;
                }

                if (!name.empty())
                {
                    builder.SetNodeName(name);
                }
                builder.SetPosition(pos);
                node = builder.Build(renderGraph);
                return node;
            }

        };

    }

}

#endif //WIDGETS_HPP
