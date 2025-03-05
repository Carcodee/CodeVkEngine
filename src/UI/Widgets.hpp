//


// Created by carlo on 2025-01-02.
//













#ifndef WIDGETS_HPP
#define WIDGETS_HPP

namespace UI
{
    enum WidgetsProperty
    {
        DRAG,
        DROP
    };

    template <typename T>
    concept HasName = requires(T t)
    {
        { t.name } -> std::convertible_to<std::string>;
    };

    template <HasName T>
    static T* GetFromNameInMap(std::map<int, T>& mapToSearch, const std::string name)
    {
        for (auto& item : mapToSearch)
        {
            if (item.second.name == name)
            {
                return &item.second;
            }
        }
        return nullptr;
    }

    template <HasName T>
    static T* GetFromNameInMap(std::unordered_map<int, T>& mapToSearch, const std::string name)
    {
        for (auto& item : mapToSearch)
        {
            if (item.second.name == name)
            {
                return &item.second;
            }
        }
        return nullptr;
    }
    template <HasName T>
    static int GetIdFromMap(std::map<int, T>& mapToSearch, const std::string name)
    {
        for (auto& item : mapToSearch)
        {
            if (item.second.name == name)
            {
                return &item.first;
            }
        }
        return -1;
    }

    template <typename T>
    static void UseDragProperty(std::string name, T& payload, size_t payloadSize)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload(name.c_str(), &payload, payloadSize);
            ImGui::EndDragDropSource();
        }
    }

    template <typename T>
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
        }
        return nullptr;
    }

    struct IWidget
    {
        virtual ~IWidget() = default;
        virtual void DisplayProperties() = 0;
    };

    struct INodeWidget
    {
        std::string name;
        virtual ~INodeWidget() = default;
        virtual void Draw(int id) = 0;
    };


    struct TextureViewer : IWidget
    {
        ENGINE::ImageView* currImageView = nullptr;
        std::string labelName;
        std::set<WidgetsProperty> properties;
        ~TextureViewer() override = default;

        void DisplayProperties() override
        {
            for (auto& prop : properties)
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

        ENGINE::ImageView* DisplayTexture(std::string name, ENGINE::ImageView* imageView, ImTextureID textureId,
                                          glm::vec2 size)
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

        void AddProperty(WidgetsProperty property)
        {
            properties.emplace(property);
        }
    };

    namespace Nodes
    {
        namespace ed = ax::NodeEditor;
        //note that this only handle copyable data types


        struct GraphNodeBuilder
        {
            ed::NodeId nodeId;
            std::unordered_map<int, PinInfo> inputNodes = {};
            std::unordered_map<int, PinInfo> outputNodes = {};
            std::unordered_map<int, EnumSelectable> selectables = {};
            std::unordered_map<int, TextInput> textInputs = {};
            std::unordered_map<int, PrimitiveInput> primitives = {};
            std::unordered_map<int, Scrollable> scrollables = {};
            std::unordered_map<int, MultiOption> multiOptions = {};
            std::unordered_map<int, DynamicStructure> dynamicStructures = {};
            std::unordered_map<int, bool> drawableWidgets = {};
            std::unordered_map<int, bool> addMoreWidgets = {};
            std::unordered_map<std::string, std::function<void(GraphNode&)>*> callbacks;
            

            std::string name = "";
            glm::vec2 pos = glm::vec2(0.0);


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

            GraphNodeBuilder& SetPosition(glm::vec2 pos)
            {
                this->pos = pos;
                return *this;
            }

            GraphNodeBuilder& AddInput(int id, PinInfo pinInfo, bool addMore = false)
            {
                pinInfo.pinKind = ed::PinKind::Input;
                pinInfo.id = id;
                inputNodes.try_emplace(id, pinInfo);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }

            GraphNodeBuilder& AddOutput(int id, PinInfo pinInfo)
            {
                pinInfo.pinKind = ed::PinKind::Output;
                pinInfo.id = id;
                outputNodes.try_emplace(id, pinInfo);
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            // GraphNodeBuilder& AddButton(int id, Button button)
            // {
            // buttons.try_emplace(id, button);
            // return *this;
            // }

            GraphNodeBuilder& AddSelectable(int id, std::string name, std::vector<std::string> options, int selectedIdx = 0, bool addMore = false)
            {
                selectables.try_emplace(id, EnumSelectable{name, options, selectedIdx});
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }

            GraphNodeBuilder& AddTextInput(int id, TextInput info, bool addMore = false)
            {
                textInputs.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }

            GraphNodeBuilder& AddScrollable(int id, Scrollable info, bool addMore = false)
            {
                scrollables.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }

            GraphNodeBuilder& AddPrimitiveData(int id, PrimitiveInput info, bool addMore = false)
            {
                primitives.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }

            GraphNodeBuilder& AddMultiOption(int id, MultiOption info, bool addMore = false)
            {
                multiOptions.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }               
                return *this;
            }

            GraphNodeBuilder& AddDynamicStructure(int id, DynamicStructure info, bool addMore = false)
            {
                dynamicStructures.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                if (addMore)
                {
                    addMoreWidgets.try_emplace(id, true);
                }
                return *this;
            }
            GraphNodeBuilder& AddCallback(std::string name, std::function<void(GraphNode&)>* callback)
            {
                if (callbacks.contains(name))
                {
                    callbacks.at(name) = std::move(callback);
                }
                callbacks.try_emplace(name, std::move(callback));
                return *this;
            }


            GraphNode* Build(GraphNode* graphNode, ENGINE::RenderGraph* renderGraph, WindowProvider* windowProvider)
            {
                // assert(nodeId.Get() > -1 && "Set the id before building");
                assert(!name.empty() && "Set a valid name");
                
                graphNode->name = name;
                graphNode->nodeId = nodeId;
                graphNode->inputNodes = inputNodes;
                graphNode->outputNodes = outputNodes;
                graphNode->selectables = selectables;
                graphNode->textInputs = textInputs;
                graphNode->primitives = primitives;
                graphNode->scrollables = scrollables;
                graphNode->multiOptions = multiOptions;
                graphNode->dynamicStructures = dynamicStructures;
                graphNode->drawableWidgets = drawableWidgets;
                graphNode->addMoreWidgets = addMoreWidgets;
                graphNode->pos = pos;
                for (auto i = callbacks.begin(); i != callbacks.end();)
                {
                    graphNode->callbacks.try_emplace(i->first,i->second);
                    i++;
                }
                
                graphNode->renderGraph = renderGraph;
                graphNode->windowProvider = windowProvider;
                graphNode->Sort();
                Reset();
                return graphNode;
            }

            void Reset()
            {
                inputNodes.clear();
                outputNodes.clear();
                selectables.clear();
                textInputs.clear();
                primitives.clear();
                scrollables.clear();
                multiOptions.clear();
                drawableWidgets.clear();
                addMoreWidgets.clear();
                callbacks.clear();
                pos = glm::vec2(0.0);
                nodeId = -1;
                name = "";
            }
        };

        
        struct GraphNodeRegistry
        {
            struct CallbackInfo
            {
                std::unordered_map<std::string, int> callbacksMap = {};
                std::vector<std::unique_ptr<std::function<void(GraphNode&)>>> callbacks = {};
                void AddCallback(std::string name, std::function<void(GraphNode&)>&& callback)
                {
                    int id = callbacks.size();
                    callbacksMap.try_emplace(name, id);
                    callbacks.emplace_back(std::make_unique<std::function<void(GraphNode&)>>(callback));
                }
                std::function<void(GraphNode&)>* GetCallbackByName(std::string name)
                {
                    if (callbacksMap.contains(name))
                    {
                        auto callback = callbacks.at(callbacksMap.at(name)).get();
                        return callback;
                    }
                    return nullptr;
                }
            };
            
            std::map<NodeType, CallbackInfo> callbacksRegistry = {};
            std::function<void(GraphNode&)>* GetCallback(NodeType nodeType, std::string name)
            {
                if (callbacksRegistry.at(nodeType).GetCallbackByName(name))
                {
                    return callbacksRegistry.at(nodeType).GetCallbackByName(name);
                }
                return nullptr;
            }
            GraphNodeRegistry() {
                callbacksRegistry.try_emplace(N_ROOT_NODE, CallbackInfo{});
                callbacksRegistry.try_emplace(N_RENDER_NODE, CallbackInfo{});
                callbacksRegistry.try_emplace(N_VERT_SHADER, CallbackInfo{});
                callbacksRegistry.try_emplace(N_FRAG_SHADER, CallbackInfo{});
                callbacksRegistry.try_emplace(N_COMP_SHADER, CallbackInfo{});
                callbacksRegistry.try_emplace(N_COL_ATTACHMENT_STRUCTURE, CallbackInfo{});
                callbacksRegistry.try_emplace(N_DEPTH_STRUCTURE, CallbackInfo{});
                callbacksRegistry.try_emplace(N_PUSH_CONSTANT, CallbackInfo{});
                callbacksRegistry.try_emplace(N_IMAGE_SAMPLER, CallbackInfo{});
                callbacksRegistry.try_emplace(N_IMAGE_STORAGE, CallbackInfo{});
                callbacksRegistry.try_emplace(N_DEPTH_IMAGE_SAMPLER, CallbackInfo{});
                callbacksRegistry.try_emplace(N_VERTEX_INPUT, CallbackInfo{});

                callbacksRegistry.at(N_ROOT_NODE).AddCallback("output_c", [](GraphNode& selfNode){
                    auto lastNodeName = selfNode.renderGraph->renderNodesSorted.back();
                    selfNode.SetOuputData("out_root_node", lastNodeName->passName);
                    SYSTEMS::Logger::GetInstance()->LogMessage(lastNodeName->passName);
                });
                callbacksRegistry.at(N_RENDER_NODE).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    
                    bool inRootNodeChain = false;
                    GraphNode* currentGraphNode = &selfNode;
                    PinInfo* inputNode = GetFromNameInMap(currentGraphNode->inputNodes, "Input Render Node");
                    while (inputNode != nullptr && inputNode->linkedPin != nullptr)
                    {
                        currentGraphNode = selfNode.graphNodeResManager->GetNodeByInputOutputId(inputNode->linkedPin->id);
                        if (currentGraphNode->HasOutput(N_ROOT_NODE))
                        {
                            inRootNodeChain = true;
                            break;
                        }
                        inputNode = GetFromNameInMap(currentGraphNode->inputNodes, "Input Render Node");
                    }
                    if (!inRootNodeChain)
                    {
                        SYSTEMS::Logger::GetInstance()->LogMessage("Node is not connected to the root chain");
                    }
                    
                    struct ExpectedConfigs
                    {
                        int added = 0;
                        int expected = 1;
                    };
                    std::map<NodeType, ExpectedConfigs> configsAdded;
                    std::map<NodeType, ExpectedConfigs> configsExpected;

                    PinInfo* vert = GetFromNameInMap(selfNode.inputNodes, "Vertex Shader");
                    PinInfo* frag = GetFromNameInMap(selfNode.inputNodes, "Fragment Shader");
                    PinInfo* compute = GetFromNameInMap(selfNode.inputNodes, "Compute Shader");
                    int vertId = -1;
                    int fragId = -1;
                    int compId = -1;

                    if (compute->HasData())
                    {
                        configsAdded.try_emplace(N_COMP_SHADER, ExpectedConfigs{0, 1});
                        try
                        {
                            compId = compute->GetData<int>();
                        }
                        catch (std::bad_cast c)
                        {
                            assert(false);
                        }
                    }
                    else
                    {
                        configsAdded.try_emplace(N_VERTEX_INPUT, ExpectedConfigs{0, 1});
                        configsAdded.try_emplace(N_COL_ATTACHMENT_STRUCTURE, ExpectedConfigs{0, 1});
                        configsAdded.try_emplace(N_VERT_SHADER, ExpectedConfigs{0, 1});
                        configsAdded.try_emplace(N_FRAG_SHADER, ExpectedConfigs{0, 1});
                        configsAdded.try_emplace(N_IMAGE_STORAGE, ExpectedConfigs{0, 0});
                        if (vert->HasData())
                        {
                            configsAdded.at(N_VERT_SHADER).added++;
                            vertId = vert->GetData<int>();
                        }
                        if (frag->HasData())
                        {
                            PinInfo* fragPin = selfNode.GetInputDataByName("Fragment Shader");
                            configsAdded.at(N_FRAG_SHADER).added++;
                            fragId = fragPin->GetData<int>();
                        }
                    }
                    
                    std::map<int, PinInfo*> images;
                    std::map<int, PinInfo*> storageImages;
                    PinInfo* depthImage;
                    int expectedColAttachments = 0;
                    
                    ENGINE::VertexInput* vertexInput = nullptr;
                    ENGINE::AttachmentInfo info;
                    vk::AttachmentLoadOp loadOpData;
                    vk::AttachmentStoreOp storeOpData;
                    ENGINE::BlendConfigs blendData;
                    vk::Format colFormatData;
                    glm::vec4 clearColData;
                    std::string attachmentName;
                    std::string depthAttachmentName;
                    
                    for (auto id : selfNode.inputNodes)
                    {
                        if (id.second.nodeType == N_IMAGE_STORAGE)
                        {
                            configsAdded.at(N_IMAGE_STORAGE).expected++;
                            if (!id.second.HasData())
                            {
                                continue;
                            }
                            GraphNode* graphNodeRef = selfNode.graphNodesRef->at(id.second.GetData<int>());
                            PinInfo* image = graphNodeRef->GetOutputDataByName("out_img_storage");
                            
                            if (image->HasData())
                            {
                                configsAdded.at(N_IMAGE_STORAGE).added++;
                                info = ENGINE::GetColorAttachmentInfo(clearColData, colFormatData, loadOpData,
                                                                      storeOpData);
                                storageImages.try_emplace(graphNodeRef->globalId, image);
                            }
                            
                        }
                        if (id.second.nodeType == N_COL_ATTACHMENT_STRUCTURE)
                        {
                            expectedColAttachments++;
                            if (!id.second.HasData())
                            {
                                continue;
                            }
                            GraphNode* graphNodeRef = selfNode.graphNodesRef->at(id.second.GetData<int>());
                            attachmentName = graphNodeRef->name;
                            PrimitiveInput* clearColor = GetFromNameInMap(
                                graphNodeRef->primitives, "Clear Color");
                            EnumSelectable* colFormat = GetFromNameInMap(
                                graphNodeRef->selectables, "Load Operation");
                            EnumSelectable* loadOp = GetFromNameInMap(
                                graphNodeRef->selectables, "Load Operation");
                            EnumSelectable* storeOp = GetFromNameInMap(
                                graphNodeRef->selectables, "Store Operation");
                            EnumSelectable* blendConfigs = GetFromNameInMap(
                                graphNodeRef->selectables, "Blend Configs");

                            clearColData = clearColor->GetData<glm::vec4>();
                            loadOpData = (vk::AttachmentLoadOp)loadOp->selectedIdx;
                            storeOpData = (vk::AttachmentStoreOp)storeOp->selectedIdx;
                            blendData = (ENGINE::BlendConfigs)blendConfigs->selectedIdx;

                            std::map<int, vk::Format> validFormats = {
                                {0, ENGINE::g_32bFormat}, {1, ENGINE::g_16bFormat}, {2, ENGINE::g_ShipperFormat}
                            };
                            colFormatData = colFormat->GetEnumFromIndex<vk::Format>(validFormats);

                            PinInfo* image = graphNodeRef->GetInputDataByName("Col Attachment Sampler");
                            if (image->HasData())
                            {
                                configsAdded.at(N_COL_ATTACHMENT_STRUCTURE).added++;
                                info = ENGINE::GetColorAttachmentInfo(clearColData, colFormatData, loadOpData, storeOpData);
                                images.try_emplace(graphNodeRef->globalId, image);
                            }
                        }
                        if (id.second.nodeType == N_DEPTH_STRUCTURE)
                        {
                            if (!id.second.HasData())
                            {
                                continue;
                            }
                            configsAdded.try_emplace(N_DEPTH_STRUCTURE, ExpectedConfigs{0, 1});
                            GraphNode* graphNodeRef = selfNode.graphNodesRef->at(id.second.GetData<int>());
                            depthAttachmentName = graphNodeRef->name;

                            depthImage = graphNodeRef->GetInputDataByName("Depth Attachment Sampler");
                            if (depthImage->HasData())
                            {
                                configsAdded.at(N_DEPTH_STRUCTURE).added++;
                            }
                        }
                    }
                    configsAdded.at(N_COL_ATTACHMENT_STRUCTURE).expected =(expectedColAttachments != 0) ? expectedColAttachments : 1;

                    PinInfo* vertexPin = selfNode.GetInputDataByName("Vertex Input");

                    if (vertexPin->HasData())
                    {
                        std::string vertexName = vertexPin->GetData<std::string>();
                        vertexInput = selfNode.renderGraph->resourcesManager->GetVertexInput(vertexName);
                        configsAdded.at(N_VERTEX_INPUT).added = true;
                    }

                    int configsToMatch = configsAdded.size();
                    int configsMatched = 0;
                    std::map<NodeType, ExpectedConfigs> confingsMissing;
                    for (auto added : configsAdded)
                    {
                        if (added.second.added != added.second.expected)
                        {
                            confingsMissing.insert(added);
                            continue;
                        }
                        configsMatched++;
                    }
                    if (configsMatched != configsToMatch || !inRootNodeChain)
                    {
                        std::string missingInfo = "";
                        for (auto added : confingsMissing)
                        {
                            missingInfo+= nodeTypeStrings.at(added.first) + "\n";
                        }
                        if (!inRootNodeChain)
                        {
                            missingInfo+= "NOT_LINKED_WITH_ROOT_\n";
                        }
                        SYSTEMS::Logger::GetInstance()->LogMessage("Confgis missng: \n" + missingInfo);
                        return;
                    }
                    std::string name = "";
                    TextInput* nodeName = GetFromNameInMap(selfNode.textInputs, "RenderNode Name");
                    name = (nodeName)? nodeName->content : "PassName_" + std::to_string(selfNode.renderGraph->renderNodes.size());
                    
                    auto renderNode = selfNode.renderGraph->AddPass(name);
                    if (renderNode->active)
                    {
                        return;
                    }

                    for (auto& image : storageImages)
                    {
                        if (!image.second->HasData()) { continue; }
                        int imageId = image.second->GetData<int>();
                        std::string imageName = "nodeImage_" + std::to_string(image.first);
                        ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetStorageFromId(
                            imageId);
                        renderNode->AddStorageResource(imageName, imgView);
                    }
                    if (configsAdded.contains(N_COMP_SHADER))
                    {
                        renderNode->SetCompShader(selfNode.renderGraph->resourcesManager->GetShaderFromId(compId));
                    }
                    else
                    {
                        assert(vertId > -1 && "Vert id invalid");
                        assert(fragId > -1 && "Frag Id invalid");
                        assert(vertexInput && "invalid vertex input");
                        renderNode->SetVertShader(selfNode.renderGraph->resourcesManager->GetShaderFromId(vertId));
                        renderNode->SetFragShader(selfNode.renderGraph->resourcesManager->GetShaderFromId(fragId));
                        renderNode->SetVertexInput(*vertexInput);
                        renderNode->AddColorAttachmentOutput(attachmentName, info, blendData);
                        for (auto& image : images)
                        {
                            if (!image.second->HasData()) { continue; }
                            int imageId = image.second->GetData<int>();
                            std::string imageName = "nodeImage_" + std::to_string(image.first);
                            ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetImageViewFromId(
                                imageId);
                            renderNode->AddColorImageResource(imageName, imgView);
                        }

                        if (configsAdded.contains(N_DEPTH_STRUCTURE))
                        {
                            if (depthImage->HasData())
                            {
                                std::string imageName = depthImage->GetData<std::string>();
                                renderNode->SetDepthImageResource(
                                    imageName,
                                    selfNode.renderGraph->resourcesManager->GetImageViewFromName(
                                        imageName));
                            }
                        }
                        
                        renderNode->SetConfigs({true});
                        renderNode->BuildRenderGraphNode();
                        renderNode->active = false;
                        selfNode.valid = true;
                    }    
                });
                callbacksRegistry.at(N_COL_ATTACHMENT_STRUCTURE).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    selfNode.SetOuputData("out_col_structure", selfNode.globalId);
                });
                callbacksRegistry.at(N_DEPTH_STRUCTURE).AddCallback("output_c", [](GraphNode& selfNode) {
                    selfNode.SetOuputData("out_depth_structure", selfNode.globalId);
                });
                callbacksRegistry.at(N_PUSH_CONSTANT).AddCallback("output_c", [](GraphNode& selfNode){});
                
                callbacksRegistry.at(N_IMAGE_SAMPLER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    std::string imgName = "nodeImage_" + std::to_string(selfNode.globalId);
                    
                    assert(!imgName.empty() && "Img name is not valid");
                    auto imageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                                                 ENGINE::g_32bFormat,
                                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                                 vk::ImageUsageFlagBits::eSampled);
                    ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetImage(
                        imgName, imageInfo, 0, 0);
                    selfNode.SetOuputData("out_img_sampler", imgView->id);

                    assert(imgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_IMAGE_STORAGE).AddCallback("output_c", [](GraphNode& selfNode)
                {
                           std::string imgName = "nodeImage_" + std::to_string(selfNode.globalId);;

                            auto storageImageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                ENGINE::g_32bFormat,
                                vk::ImageUsageFlagBits::eStorage |
                                vk::ImageUsageFlagBits::eTransferDst);

                            ENGINE::ImageView* storageImgView = selfNode.renderGraph->resourcesManager->GetImage(
                                imgName, storageImageInfo, 0, 0);
                            selfNode.SetOuputData("out_img_storage", storageImgView->id);

                            assert(storageImgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_DEPTH_IMAGE_SAMPLER).AddCallback("output_c", [](GraphNode& selfNode){
                    std::string imgName = selfNode.GetTextInput("Img Name")->content;
                    assert(!imgName.empty() && "Img name is not valid");
                    auto depthImageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                                                      selfNode.renderGraph->core->swapchainRef->
                                                                               depthFormat,
                                                                      vk::ImageUsageFlagBits::eDepthStencilAttachment
                                                                      |
                                                                      vk::ImageUsageFlagBits::eSampled);
                    ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetImage(
                        imgName, depthImageInfo, 0, 0);
                    selfNode.SetOuputData("out_depth_sampler", imgName);

                    assert(imgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_VERTEX_INPUT).AddCallback("output_c", [](GraphNode& selfNode){
                    std::string vertexName = "vertex_input_" + std::to_string(selfNode.globalId);
                    ENGINE::VertexInput* vertexInput = selfNode.renderGraph->resourcesManager->GetVertexInput(vertexName);
                    vertexInput->bindingDescription.clear();
                    vertexInput->inputDescription.clear();
                    
                    size_t offset = 0;
                    int location = 0;
                    int binding = 0;
                    auto dynamicStructure = GetFromNameInMap(
                        selfNode.dynamicStructures, "Vertex Builder");
                    for (auto widget : dynamicStructure->widgetsInfos)
                    {
                        int typeSelected = widget.second.selectableInfo.selectedIdx;
                        ENGINE::VertexInput::Attribs attribs = (ENGINE::VertexInput::Attribs)typeSelected;
                        vertexInput->
                            AddVertexAttrib(attribs, binding, offset, location);
                        offset += vertexInput->GetSizeFrom(attribs);
                        location++;
                    }
                    for (auto widget : dynamicStructure->widgetsInfos)
                    {
                        vertexInput->AddVertexInputBinding(0, offset);
                    }
                    selfNode.SetOuputData("out_vertex_result", vertexName);
                    
                });
                callbacksRegistry.at(N_COMP_SHADER).AddCallback("output_c", [](GraphNode& selfNode){
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_COMP;
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    assert(multiOption);
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = multiOption->inputTexts.at(1).content;
                        selfNode.renderGraph->resourcesManager->CreateDefaultShader(shaderSelected, stage);
                        break;
                    }
                    int shaderIdx = -1;
                    if (selfNode.renderGraph->resourcesManager->shadersNames.contains(shaderSelected))
                    {
                        shaderIdx = selfNode.renderGraph->resourcesManager->shadersNames.at(shaderSelected);
                    }
                    selfNode.SetOuputData("out_shader", shaderIdx);
                });
                callbacksRegistry.at(N_VERT_SHADER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_VERT;
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    assert(multiOption);
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = multiOption->inputTexts.at(1).content;
                        selfNode.renderGraph->resourcesManager->CreateDefaultShader(shaderSelected, stage);
                        break;
                    }
                    int shaderIdx = -1;
                    if (selfNode.renderGraph->resourcesManager->shadersNames.contains(shaderSelected))
                    {
                        shaderIdx = selfNode.renderGraph->resourcesManager->shadersNames.at(shaderSelected);
                    }
                    selfNode.SetOuputData("out_shader", shaderIdx);
                });
                callbacksRegistry.at(N_FRAG_SHADER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_FRAG;
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    assert(multiOption);
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = multiOption->inputTexts.at(1).content;
                        selfNode.renderGraph->resourcesManager->CreateDefaultShader(shaderSelected, stage);
                        break;
                    }
                    int shaderIdx = -1;
                    if (selfNode.renderGraph->resourcesManager->shadersNames.contains(shaderSelected))
                    {
                        shaderIdx = selfNode.renderGraph->resourcesManager->shadersNames.at(shaderSelected);
                    }
                    selfNode.SetOuputData("out_shader", shaderIdx);
                });

                
                callbacksRegistry.at(N_RENDER_NODE).AddCallback("link_c", [](GraphNode& selfNode)
                {
                    
                    PinInfo* vert = GetFromNameInMap(selfNode.inputNodes, "Vertex Shader");
                    PinInfo* frag = GetFromNameInMap(selfNode.inputNodes, "Fragment Shader");
                    PinInfo* compute = GetFromNameInMap(selfNode.inputNodes, "Compute Shader");
                    selfNode.ToggleDraw(vert->id, true);
                    selfNode.ToggleDraw(frag->id, true);
                    selfNode.ToggleDraw(compute->id, true);
                    if (compute->HasData())
                    {
                        selfNode.ToggleDraw(vert->id, false);
                        selfNode.ToggleDraw(frag->id, false);
                    }else if (vert->HasData() || frag->HasData())
                    {
                        selfNode.ToggleDraw(compute->id, false);
                    }
                });

                
                callbacksRegistry.at(N_RENDER_NODE).AddCallback("unlink_c", [](GraphNode& selfNode)
                {
                    PinInfo* vert = GetFromNameInMap(selfNode.inputNodes, "Vertex Shader");
                    PinInfo* frag = GetFromNameInMap(selfNode.inputNodes, "Fragment Shader");
                    PinInfo* compute = GetFromNameInMap(selfNode.inputNodes, "Compute Shader");

                    selfNode.ToggleDraw(vert->id, true);
                    selfNode.ToggleDraw(frag->id, true);
                    selfNode.ToggleDraw(compute->id, true);
                    if (compute->HasData())
                    {
                        selfNode.ToggleDraw(vert->id, false);
                        selfNode.ToggleDraw(frag->id, false);
                    }
                    else if (vert->HasData() || frag->HasData())
                    {
                        selfNode.ToggleDraw(compute->id, false);
                    }
                });
            }
        };

        struct GraphNodeFactory
        {
            GraphNodeBuilder builder = {};
            GraphNodeRegistry nodeRegistry = {};
            ENGINE::RenderGraph* renderGraph;
            WindowProvider* windowProvider;
            GraphNodeResManager* resManager;

            GraphNode* GetNode(NodeType nodeType, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
            {
                assert(renderGraph && "Null rgraph");
                assert(windowProvider && "Null window Provider");
                assert(resManager && "Null resManager");
                GraphNode node;
                switch (nodeType)
                {
                case N_ROOT_NODE:
                    builder
                        .AddOutput(resManager->NextWidgetID(), {"out_root_node", N_ROOT_NODE})
                        .SetNodeId(resManager->NextWidgetID(), "Root Node");
                    break;
                case N_RENDER_NODE:
                    builder
                        .AddTextInput(resManager->NextWidgetID(), {"RenderNode Name"})
                        .AddOutput(resManager->NextWidgetID(), {"out_render_node", N_RENDER_NODE})
                        .AddInput(resManager->NextWidgetID(), {"Input Render Node", static_cast<NodeType>(N_RENDER_NODE | N_ROOT_NODE)})
                        .AddInput(resManager->NextWidgetID(), {"Vertex Shader", N_VERT_SHADER})
                        .AddInput(resManager->NextWidgetID(), {"Fragment Shader", N_FRAG_SHADER})
                        .AddInput(resManager->NextWidgetID(), {"Compute Shader", N_COMP_SHADER})
                        .AddInput(resManager->NextWidgetID(), {"Col Attachment Node", N_COL_ATTACHMENT_STRUCTURE}, true)
                        .AddInput(resManager->NextWidgetID(), {"Storage Image Node", N_IMAGE_STORAGE}, true)
                        .AddInput(resManager->NextWidgetID(), {"Depth Attachment", N_DEPTH_STRUCTURE})
                        .AddInput(resManager->NextWidgetID(), {"Vertex Input", N_VERTEX_INPUT})
                        .AddSelectable(resManager->NextWidgetID(), "Raster Configs", {"Fill", "Line", "Point"})
                        .SetNodeId(resManager->NextWidgetID(), "Render Node");
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    builder
                        .AddPrimitiveData(resManager->NextWidgetID(), {"Clear Color", VEC4, glm::vec4(0.0)})
                        .AddSelectable(resManager->NextWidgetID(), "Color Format", {"g_32bFormat", "g_16bFormat"})
                        .AddSelectable(resManager->NextWidgetID(), "Load Operation", {"Load", "Clear", "Dont Care", "None"})
                        .AddSelectable(resManager->NextWidgetID(), "Store Operation", {"Load", "eDontCare", "eNone"})
                        .AddSelectable(resManager->NextWidgetID(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"}, 1)
                        .AddInput(resManager->NextWidgetID(), {"Col Attachment Sampler", N_IMAGE_SAMPLER})
                        .AddOutput(resManager->NextWidgetID(), {"out_col_structure", N_COL_ATTACHMENT_STRUCTURE})
                        .SetNodeId(resManager->NextWidgetID(), "Col Attachment Node");
                    break;
                case N_DEPTH_STRUCTURE:
                    builder
                        .AddSelectable(resManager->NextWidgetID(), "Depth Configs", {"None", "Enable", "Disable"})
                        .AddInput(resManager->NextWidgetID(), {"Depth Image In", N_DEPTH_IMAGE_SAMPLER})
                        .AddOutput(resManager->NextWidgetID(), {"out_depth_structure ", N_DEPTH_STRUCTURE})
                        .SetNodeId(resManager->NextWidgetID(), "Depth Attachment Node");
                    break;
                case N_PUSH_CONSTANT:
                    builder
                        .AddPrimitiveData(resManager->NextWidgetID(), {"Push Constant data", SIZE_T, size_t(0)})
                        .AddOutput(resManager->NextWidgetID(), {"Push Constant Structure ", N_PUSH_CONSTANT})
                        .SetNodeId(resManager->NextWidgetID(), "Push Constant");
                    break;
                case N_IMAGE_SAMPLER:
                    builder
                        .AddOutput(resManager->NextWidgetID(), {"out_img_sampler", N_IMAGE_SAMPLER})
                        .SetNodeId(resManager->NextWidgetID(), "Sampler Image Node");
                    break;
                case N_IMAGE_STORAGE:
                    builder
                        .AddOutput(resManager->NextWidgetID(), {"out_img_storage", N_IMAGE_STORAGE})
                        .SetNodeId(resManager->NextWidgetID(), "Storage Image Node");
                    break;
                case N_DEPTH_IMAGE_SAMPLER:
                    builder
                        .AddTextInput(resManager->NextWidgetID(), {"Img Name", "Image Name"})
                        .AddOutput(resManager->NextWidgetID(), {"out_depth_sampler", N_DEPTH_IMAGE_SAMPLER})
                        .SetNodeId(resManager->NextWidgetID(), "Depth Image Node");
                    break;
                case N_VERTEX_INPUT:
                    {
                        EnumSelectable selectable("Vertex Attrib: ", {
                              "INT", "FLOAT", "VEC2", "VEC3", "VE4", "U8VEC3", "U8VEC4","COLOR_32"});
                        DynamicStructure dynamicStructureInfo("Vertex Builder", selectable);
                        
                        builder.AddDynamicStructure(resManager->NextWidgetID(), dynamicStructureInfo)
                               .AddOutput(resManager->NextWidgetID(), {"out_vertex_result", N_VERTEX_INPUT})
                               .SetNodeId(resManager->NextWidgetID(), "Vertex Input Builder");
                    }
                    break;
                case N_VERT_SHADER:
                case N_FRAG_SHADER:
                case N_COMP_SHADER:
                    std::vector<std::any> shaderPaths;
                    shaderPaths.reserve(renderGraph->resourcesManager->shadersNames.size());
                    std::vector<std::string> options = {"Pick Shader", "Create Shader"};
                    std::map<int, TextInput> textInputs;
                    textInputs.try_emplace(1, TextInput{"Select Shader Name"});
                    std::map<int, Scrollable> scrollables;

                    if (nodeType == N_VERT_SHADER)
                    {
                        for (auto& shaderPath : renderGraph->resourcesManager->shadersNames)
                        {
                            if (renderGraph->resourcesManager->shaders.at(shaderPath.second)->stage ==
                                ENGINE::ShaderStage::S_VERT)
                            {
                                shaderPaths.emplace_back(shaderPath.first);
                            }
                        }
                        builder
                            .SetNodeId(resManager->NextWidgetID(), "Vert Shader")
                            .AddOutput(resManager->NextWidgetID(), {"out_shader", N_VERT_SHADER});
                    }
                    if (nodeType == N_FRAG_SHADER)
                    {
                        for (auto& shaderPath : renderGraph->resourcesManager->shadersNames)
                        {
                            if (renderGraph->resourcesManager->shaders.at(shaderPath.second)->stage ==
                                ENGINE::ShaderStage::S_FRAG)
                            {
                                shaderPaths.emplace_back(shaderPath.first);
                            }
                        }
                        builder
                            .SetNodeId(resManager->NextWidgetID(), "Frag Shader")
                            .AddOutput(resManager->NextWidgetID(), {"out_shader", N_FRAG_SHADER});
                    }
                    if (nodeType == N_COMP_SHADER)
                    {
                        for (auto& shaderPath : renderGraph->resourcesManager->shadersNames)
                        {
                            if (renderGraph->resourcesManager->shaders.at(shaderPath.second)->stage ==
                                ENGINE::ShaderStage::S_COMP)
                            {
                                shaderPaths.emplace_back(shaderPath.first);
                            }
                        }
                        builder
                            .SetNodeId(resManager->NextWidgetID(), "Compute Shader")
                            .AddOutput(resManager->NextWidgetID(), {"out_shader", N_COMP_SHADER});
                    }
                    scrollables.try_emplace(0, Scrollable{"Possible Shaders", STRING, shaderPaths});
                    MultiOption multiOptionInfo("Shader Options", options, textInputs, {}, scrollables, {});
                    builder.AddMultiOption(resManager->NextWidgetID(), multiOptionInfo);
                    break;
                }

                std::function<void(GraphNode&)>* outputOp = nodeRegistry.GetCallback(nodeType, "output_c");
                std::function<void(GraphNode&)>* linkOp = nodeRegistry.GetCallback(nodeType, "link_c");
                if (!name.empty())
                {
                    builder.SetNodeName(name);
                }
                builder.SetPosition(pos);
                builder.AddCallback("output_c", outputOp);
                builder.AddCallback("link_c", linkOp);
                
                int id = resManager->NextNodeID();
                GraphNode* graphNode = resManager->GetNode(id);
                builder.Build(graphNode, renderGraph, windowProvider);
                resManager->AddNodeIds(graphNode);

                return graphNode;
            }
        };
    }
    
}

#endif //WIDGETS_HPP
