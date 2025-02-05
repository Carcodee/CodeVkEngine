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

    template<typename T>
    concept  HasName = requires(T t)
    {
        {t.name} -> std::convertible_to<std::string>;
    };
    
    template <HasName T>
    static T* GetFromMap(std::map<int, T>& mapToSearch, const std::string name)
    {
        for (auto& item : mapToSearch)
        {
            if (item.second.name == name)
            {
                return &item.second;
            }
        }
        assert(false && "invalid name");
        return nullptr;
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
        else
        {
            return nullptr;
        }
    }

    struct IWidget
    {
        virtual ~IWidget() = default;
        virtual void DisplayProperties() = 0;
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

        enum NodeType
        {
            N_RENDER_NODE,
            N_VERT_SHADER,
            N_FRAG_SHADER,
            N_COMP_SHADER,
            N_COL_ATTACHMENT_STRUCTURE,
            N_DEPTH_STRUCTURE,
            N_RASTER_STRUCTURE,
            N_PUSH_CONSTANT,
            N_IMAGE_SAMPLER,
            N_IMAGE_STORAGE,
            N_DEPTH_IMAGE_SAMPLER,
            N_VERTEX_INPUT,
            N_BUFFER
            
        };

        enum PrimitiveNodeType
        {
            INT,
            UINT,
            VEC3,
            VEC2,
            SIZE_T,
            STRING
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

        //enums
        struct SelectableInfo
        {
            std::string name;
            std::vector<std::string> options;
            int selectedIdx;

            ~SelectableInfo() = default;

            void Draw(const int& id = -1)
            {
                ImGui::PushItemWidth(100.0);
                if (id > -1)
                    ImGui::PushID(id);

                ImGui::Text(name.c_str());
                int index = selectedIdx;
                std::string label = options[index].c_str();
                //temp solution
                ImGui::SliderInt(label.c_str(), &selectedIdx, 0, options.size() - 1);

                if (id > -1)
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
        };

        struct TextInputInfo
        {
            std::string name;
            std::string content;

            void Draw(const int& id = -1)
            {
                ImGui::PushItemWidth(100.0);
                if (id > -1)
                    ImGui::PushID(id);
                constexpr size_t size = 128;
                char inputT[size];
                std::strncpy(inputT, content.c_str(), size);
                if (ImGui::InputText(name.c_str(), inputT, size))
                {
                    content = std::string(inputT);
                }
                if (id > -1)
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
        };

        struct PrimitiveSelectable 
        {
            std::string name;
            PrimitiveNodeType primitiveType;
            std::any content;
            
            template<typename T>
            T& GetValue()
            {
                T& data = std::any_cast<T>(content);
                return data;
            }

            void Draw(const int& id = -1)
            {
                ImGui::PushItemWidth(100.0);
                if (id > -1)
                    ImGui::PushID(id);
                int intData;
                int uIntdata;
                glm::vec3 vec3Data;
                glm::vec2 vec2Data;
                std::string stringData;
                size_t sizeData;
                int sizeTemp;
                std::any result;
                switch (primitiveType)
                {
                case INT:
                    intData = std::any_cast<int>(content);
                    ImGui::InputInt(name.c_str(), &intData, 1, 100);
                    result = intData;
                    break;
                case UINT:
                    uIntdata = std::any_cast<int>(content);
                    ImGui::InputInt(name.c_str(), &uIntdata, 1, 100);
                    result = uIntdata;
                    break;
                case VEC3:
                    vec3Data = std::any_cast<glm::vec3>(content);
                    ImGui::InputFloat3(name.c_str(), glm::value_ptr(vec3Data));
                    result = vec3Data;
                    break;
                case VEC2:
                    vec2Data = std::any_cast<glm::vec2>(content);
                    ImGui::InputFloat2(name.c_str(), glm::value_ptr(vec2Data));
                    result = vec2Data;
                    break;
                case SIZE_T:
                    sizeData = std::any_cast<size_t>(content);
                    sizeTemp = static_cast<int>(sizeData);
                    ImGui::InputInt(name.c_str(), &sizeTemp);
                    sizeData = static_cast<size_t>(sizeTemp);
                    result = sizeData;
                    break;
                case STRING:
                    assert(false && "Invalid case");
                    break;
                default:
                    assert(false && "Invalid case");
                    break;
                }
                content = result;
                if (id > -1)
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }
            
        };

        //any
        //fix this
        struct Scrollable
        {
            std::string name;
            PrimitiveNodeType primitiveType;
            std::vector<std::any> content;
            int selectedIdx = 0;
            int itemHightlight = 0;



            void Draw(const int& id = -1)
            {
                int intData = -1;
                std::vector<std::string> items;
                items.reserve(content.size());
                std::string stringData = "";
                int idx = 0;
                for (auto& item : content)
                {
                    switch (primitiveType)
                    {
                    case INT:
                        intData = std::any_cast<int>(item);
                        stringData = std::to_string(intData).c_str();
                    case UINT:
                        assert(false && "Invalid case");
                        break;
                    case VEC3:
                        assert(false && "Invalid case");
                        break;
                    case VEC2:
                        assert(false && "Invalid case");
                        break;
                    case STRING:
                        stringData = std::any_cast<std::string>(item).c_str();
                    default:
                        break;
                    }
                    items.push_back(stringData.c_str());
                    idx++;
                }

                ImGui::PushItemWidth(100.0);
                if (id > -1)
                    ImGui::PushID(id);
                ImGui::Text(name.c_str());
                int index = selectedIdx;
                std::string filename = std::filesystem::path(items[index]).filename().string();
                const char* label = filename.c_str();
                //temp solution

                ImGui::SliderInt(label, &selectedIdx, 0, content.size() - 1);

                if (id > -1)
                    ImGui::PopID();
                ImGui::PopItemWidth();
            }

            template<typename T>
            T GetCurrent()
            {
                return std::any_cast<T>(content.at(selectedIdx));
            }
        };

        struct MultiOption
        {
            std::string name;
            std::map<int, std::string> options;;
            std::map<int, TextInputInfo> inputTexts;;
            std::map<int, SelectableInfo> selectables;
            std::map<int, Scrollable> scrollables;

            int selectedIdx = 0;

            MultiOption(std::string name, std::vector<std::string> options, std::map<int, TextInputInfo> inputTexts,
                            std::map<int, SelectableInfo> selectables, std::map<int, Scrollable> scrollables)
            {
                this->name = name;
                for (int i = 0; i < options.size(); ++i)
                {
                    this->options.try_emplace(i, options[i]);
                }
                this->inputTexts = inputTexts;
                this->selectables = selectables;
                this->scrollables = scrollables;
            }

            void Draw(const int& id = -1)
            {
                if (id > -1)
                    ImGui::PushID(id);
                for (auto& option : options)
                {
                    if (ImGui::RadioButton(option.second.c_str(), selectedIdx == option.first))
                    {
                        selectedIdx = option.first;
                    }
                }
                if (inputTexts.contains(selectedIdx))
                    inputTexts.at(selectedIdx).Draw();

                if (selectables.contains(selectedIdx))
                    selectables.at(selectedIdx).Draw();

                if (scrollables.contains(selectedIdx))
                    scrollables.at(selectedIdx).Draw();
                
                if (id > -1)
                    ImGui::PopID();
            }
        };
        enum WidgetType
        {
            W_MULTI_OPTION,
            W_SELECTABLE,
            W_TEXT_INPUT,
            W_PRIMITIVE
        };


        struct DynamicStructure
        {
            struct NodeWidgetsInfos
            {
                union
                {
                    MultiOption multiOptionInfo;
                    SelectableInfo selectableInfo;
                    TextInputInfo textInputInfo;
                    PrimitiveSelectable primitiveInfo;
                };

                WidgetType type;

                NodeWidgetsInfos()
                {
                }

                NodeWidgetsInfos(const NodeWidgetsInfos& other) : type(other.type)
                {
                    
                    switch (type)
                    {
                    case W_MULTI_OPTION:
                        new(&multiOptionInfo) MultiOption(other.multiOptionInfo);
                        break;
                    case W_SELECTABLE:
                        new(&selectableInfo) SelectableInfo(other.selectableInfo);
                        break;
                    case W_TEXT_INPUT:
                        new(&textInputInfo) TextInputInfo(other.textInputInfo);
                        break;
                    case W_PRIMITIVE:
                        new(&primitiveInfo) PrimitiveSelectable(other.primitiveInfo);
                        break;
                    }
                }

                NodeWidgetsInfos(const NodeWidgetsInfos&& other) noexcept : type(other.type)
                {
                    switch (type)
                    {
                    case W_MULTI_OPTION:
                        new(&multiOptionInfo) MultiOption(std::move(other.multiOptionInfo));
                        break;
                    case W_SELECTABLE:
                        new(&selectableInfo) SelectableInfo(std::move(other.selectableInfo));
                        break;
                    case W_TEXT_INPUT:
                        new(&textInputInfo) TextInputInfo(std::move(other.textInputInfo));
                        break;
                    case W_PRIMITIVE:
                        new(&primitiveInfo) PrimitiveSelectable(std::move(other.primitiveInfo));
                        break;
                    }
                }

                ~NodeWidgetsInfos()
                {
                    Destroy();
                }

                void Destroy()
                {
                    switch (type)
                    {
                    case W_MULTI_OPTION:
                        multiOptionInfo.~MultiOption();
                        break;
                    case W_SELECTABLE:
                        selectableInfo.~SelectableInfo();
                        break;
                    case W_TEXT_INPUT:
                        textInputInfo.~TextInputInfo();
                        break;
                    case W_PRIMITIVE:
                        primitiveInfo.~PrimitiveSelectable();
                        break;
                    }
                }

                NodeWidgetsInfos& operator=(const NodeWidgetsInfos& other)
                {
                    if (this != &other)
                    {
                        Destroy();
                        type = other.type;
                        switch (type)
                        {
                        case W_MULTI_OPTION:
                            new(&multiOptionInfo) MultiOption(other.multiOptionInfo);
                            break;
                        case W_SELECTABLE:
                            new(&selectableInfo) SelectableInfo(other.selectableInfo);
                            break;
                        case W_TEXT_INPUT:
                            new(&textInputInfo) TextInputInfo(other.textInputInfo);
                            break;
                        case W_PRIMITIVE:
                            new(&primitiveInfo) PrimitiveSelectable(other.primitiveInfo);
                            break;
                        }
                    }
                    return *this;
                }
                NodeWidgetsInfos& operator=(const NodeWidgetsInfos&& other) noexcept
                {
                    if (this != &other)
                    {
                        Destroy();
                        type = other.type;
                        switch (type)
                        {
                        case W_MULTI_OPTION:
                            new(&multiOptionInfo) MultiOption(std::move(other.multiOptionInfo));
                            break;
                        case W_SELECTABLE:
                            new(&selectableInfo) SelectableInfo(std::move(other.selectableInfo));
                            break;
                        case W_TEXT_INPUT:
                            new(&textInputInfo) TextInputInfo(std::move(other.textInputInfo));
                            break;
                        case W_PRIMITIVE:
                            new(&primitiveInfo) PrimitiveSelectable(std::move(other.primitiveInfo));
                            break;
                        }
                    }
                    return *this;
                }
            };
            
            std::string name;
            std::map<int, NodeWidgetsInfos> widgetsInfos;

            DynamicStructure(const DynamicStructure& other)
            {
                this->name = other.name;
                this->widgetsInfos = other.widgetsInfos;
            }

            DynamicStructure(const std::string& name, MultiOption& multiOption)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_SELECTABLE;
                new(&baseWidgetInfo.multiOptionInfo) MultiOption(multiOption);
                widgetsInfos.try_emplace(widgetsInfos.size(), std::move(baseWidgetInfo));
            }           
            DynamicStructure(const std::string& name, SelectableInfo& selectable)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_SELECTABLE;
                new(&baseWidgetInfo.selectableInfo) SelectableInfo(selectable);
                widgetsInfos.try_emplace(widgetsInfos.size() , std::move(baseWidgetInfo));
            }

            DynamicStructure(const std::string& name, TextInputInfo& textInputInfo)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_TEXT_INPUT;
                new(&baseWidgetInfo.textInputInfo) TextInputInfo(textInputInfo);
                widgetsInfos.try_emplace(widgetsInfos.size(), std::move(baseWidgetInfo));
            }

            DynamicStructure(const std::string& name, PrimitiveSelectable& primitiveInfo)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_PRIMITIVE;
                new(&baseWidgetInfo.primitiveInfo) PrimitiveSelectable(primitiveInfo);
                widgetsInfos.try_emplace(widgetsInfos.size(), std::move(baseWidgetInfo));
            }



            ~DynamicStructure()
            {
                for (auto& widget : widgetsInfos)
                {
                    widget.second.Destroy();
                }
            }
            
            void AddOption()
            {
                widgetsInfos.try_emplace(widgetsInfos.size(), widgetsInfos.at(0));
            }

            void RemoveLast()
            {
                if (widgetsInfos.size()==1){return;}
                
                auto it = widgetsInfos.find(widgetsInfos.size()-1);
                if (it != widgetsInfos.end())
                {
                    widgetsInfos.erase(it);
                }else
                {
                    SYSTEMS::Logger::GetInstance()->Log("Invalid Key");
                }
            }
            
            void Draw(const int& id = -1)
            {
                if (id > -1)
                    ImGui::PushID(id);

                ImGui::Text(name.c_str());
                ImGui::SameLine();
                ImGui::PushItemWidth(20);
                if (ImGui::Button("+"))
                {
                    AddOption();
                }
                ImGui::SameLine();
                if (ImGui::Button("-"))
                {
                    RemoveLast();
                }
                ImGui::PopItemWidth();
                int wId = 0;
                for (auto& widgetInfo : widgetsInfos)
                {
                    switch (widgetInfo.second.type)
                    {
                    case W_MULTI_OPTION:
                        widgetInfo.second.multiOptionInfo.Draw(wId++);
                        break;
                    case W_SELECTABLE:
                        widgetInfo.second.selectableInfo.Draw(wId++);
                        break;
                    case W_TEXT_INPUT:
                        widgetInfo.second.textInputInfo.Draw(wId++);
                        break;
                    case W_PRIMITIVE:
                        widgetInfo.second.primitiveInfo.Draw(wId++);
                        break;
                    }
                }
                if (id > -1)
                    ImGui::PopID();
            }
            
        };

        
        //note that this only handle copyable data types
        struct GraphNode
        {
            ed::NodeId nodeId;
            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, SelectableInfo> selectables;
            std::map<int, TextInputInfo> textInputs;
            std::map<int, PrimitiveSelectable> primitives;
            std::map<int, Scrollable> scrollables;
            std::map<int, MultiOption> multiOptions;
            std::map<int, DynamicStructure> dynamicStructures;

            std::map<int, std::any> inputData;
            std::map<int, std::any> outputData;

            std::string name;
            glm::vec2 pos;
            bool firstFrame = true;

            ENGINE::RenderGraph* renderGraph;

            std::unique_ptr<std::function<void(GraphNode&)>> outputFunction = nullptr;

            int BuildOutput()
            {
                if (outputFunction == nullptr)
                {
                    return -1;
                }
                (*outputFunction)(*this);
                return 0;
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

            std::any* GetInputDataById(int id)
            {
                if (!inputNodes.contains(id))
                {
                    return nullptr;
                }
                return &inputData.at(id);
            }

            std::any* GetInputDataByName(const std::string& name)
            {
                for (auto& node : inputNodes)
                {
                    if (node.second.name == name)
                    {
                        return &inputData.at(node.first);
                    }
                }
                return nullptr;
            }

            std::any* GetOutputDataById(int id)
            {
                if (!outputNodes.contains(id))
                {
                    return nullptr;
                }
                return &outputData.at(id);
            }

            std::any* GetOutputDataByName(const std::string& name)
            {
                for (auto& node : outputNodes)
                {
                    if (node.second.name == name)
                    {
                        return &outputData.at(node.first);
                    }
                }
                return nullptr;
            }

            void SetOuputData(const std::string& name, std::any data)
            {
                *GetOutputDataByName(name) = data;
            }

            void SetOuputData(int id, std::any data)
            {
                *GetOutputDataById(id) = data;
            }


            int GetSelectableIndex(const std::string& name)
            {
                for (auto& selectable : selectables)
                {
                    if (selectable.second.name == name)
                    {
                        return selectable.second.selectedIdx;
                    }
                }
                return -1;
            }

            Scrollable* GetScrollable(const std::string& name)
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

            MultiOption* GetMultiOption(const std::string name)
            {
                 for (auto& multiOption : multiOptions)
                {
                    if (multiOption.second.name == name)
                    {
                        return &multiOption.second;
                    }
                }
                assert(false && "ivalid name");
                return nullptr;               
            }

            std::string GetInputTextContent(const std::string& name)
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
                assert(nodeId.Get() != -1);
                ImGui::PushID(nodeId.Get());
                ed::BeginNode(nodeId);
                ImGui::Text(name.c_str());

                // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                for (auto& input : inputNodes)
                {
                    ImGui::PushID(input.first);
                    ed::BeginPin(input.first, ed::PinKind::Input);
                    ImGui::Text(input.second.name.c_str());
                    ed::EndPin();
                    ImGui::PopID();
                }

                for (auto& selectable : selectables)
                {
                    selectable.second.Draw(selectable.first);
                }
                for (auto& tInput : textInputs)
                {
                    tInput.second.Draw(tInput.first);
                }
                for (auto& primitiveInfo : primitives)
                {
                    primitiveInfo.second.Draw(primitiveInfo.first);
                }

                for (auto& scrollable : scrollables)
                {
                    scrollable.second.Draw(scrollable.first);
                }
                for (auto& multiOption : multiOptions)
                {
                    multiOption.second.Draw(multiOption.first);
                }
                for (auto& dynamicStructure : dynamicStructures)
                {
                    dynamicStructure.second.Draw(dynamicStructure.first);
                    
                }
                
                // ImGui::PopStyleVar();
                for (auto& output : outputNodes)
                {
                    ImGui::PushID(output.first);
                    ed::BeginPin(output.first, ed::PinKind::Output);
                    std::string spacing = "                              "+ output.second.name + "->";
                    ImGui::Text(spacing.c_str());
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
            ed::NodeId nodeId = -1;
            std::map<int, PinInfo> inputNodes = {};
            std::map<int, PinInfo> outputNodes = {};
            std::map<int, SelectableInfo> selectables = {};
            std::map<int, TextInputInfo> textInputs = {};
            std::map<int, PrimitiveSelectable> primitives = {};
            std::map<int, Scrollable> scrollables = {};
            std::map<int, MultiOption> multiOptions = {};
            std::map<int, DynamicStructure> dynamicStructures = {};

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

            GraphNodeBuilder& AddSelectable(int id, std::string name, std::vector<std::string> options)
            {
                selectables.try_emplace(id, SelectableInfo{name, options, 0});
                return *this;
            }

            GraphNodeBuilder& AddTextInput(int id, TextInputInfo info)
            {
                textInputs.try_emplace(id, info);
                return *this;
            }

            GraphNodeBuilder& AddScrollableOption(int id, Scrollable info)
            {
                scrollables.try_emplace(id, info);
                return *this;
            }

            GraphNodeBuilder& AddPrimitiveData(int id, PrimitiveSelectable info)
            {
                primitives.try_emplace(id, info);
                return *this;
            }

            GraphNodeBuilder& AddMultiOption(int id, MultiOption info)
            {
                multiOptions.try_emplace(id, info);
                return *this;
            }

            GraphNodeBuilder& AddDynamicStructure(int id, DynamicStructure info)
            {
                dynamicStructures.try_emplace(id, info);
                return *this;
            }


            GraphNode Build(ENGINE::RenderGraph* renderGraph, std::unique_ptr<std::function<void(GraphNode&)>> outputOp)
            {
                // assert(nodeId.Get() > -1 && "Set the id before building");
                assert(!name.empty() && "Set a valid name");

                GraphNode graphNode = {};
                graphNode.name = name;
                graphNode.nodeId = nodeId;
                graphNode.inputNodes = inputNodes;
                graphNode.outputNodes = outputNodes;
                graphNode.selectables = selectables;
                graphNode.textInputs = textInputs;
                graphNode.primitives = primitives;
                graphNode.scrollables = scrollables;
                graphNode.multiOptions = multiOptions;
                graphNode.dynamicStructures = dynamicStructures;
                graphNode.outputFunction = std::move(outputOp);
                graphNode.pos = pos;
                graphNode.renderGraph = renderGraph;
                for (auto& input : graphNode.inputNodes)
                {
                    graphNode.inputData.try_emplace(input.first, std::string("Empty"));
                }
                for (auto& output : graphNode.outputNodes)
                {
                    graphNode.outputData.try_emplace(output.first, std::string("Empty"));
                }
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
                std::unique_ptr<std::function<void(GraphNode&)>> linkOp;
                switch (nodeType)
                {
                case N_RENDER_NODE:
                    // linkOp =  std::make_unique<std::function<void(GraphNode& selfNode)>>(
                    // [this](GraphNode& selfNode) -> std::any {

                    // auto node = renderGraph->AddPass(selfNode.name);
                    // for (auto& input : selfNode.inputData)
                    // {
                    // switch (input.first)
                    // {
                    // case N_COL_ATTACHMENT_STRUCTURE:
                    // GraphNode& inputNode = *selfNode.GetNodeInputData<GraphNode>(N_COL_ATTACHMENT_STRUCTURE);
                    // if(inputNode.ContainsInput(input.first))
                    // {
                    // ENGINE::AttachmentInfo info = *inputNode.GetNodeInputData<ENGINE::AttachmentInfo>(input.first);
                    // ENGINE::BlendConfigs blendConfigs = (ENGINE::BlendConfigs)inputNode.GetSelectableIndex("BlendConfig");
                    // node->AddColorAttachmentOutput(inputNode.name, info, blendConfigs);
                    // }
                    // if (inputNode.ContainsInput(N_IMAGE_SAMPLER))
                    // {
                    // ENGINE::ImageView* img = *inputNode.GetNodeInputData<ENGINE::ImageView*>(input.first);
                    // node->AddSamplerResource(inputNode.name, img);
                    // }
                    // break;
                    // }
                    // }
                    // std::any result = selfNode.name;
                    // return result;
                    // });

                    builder
                        .AddOutput(NextID(), {"Result", N_RENDER_NODE})
                        .AddInput(NextID(), {"Vertex Shader", N_VERT_SHADER})
                        .AddInput(NextID(), {"Fragment Shader", N_FRAG_SHADER})
                        .AddInput(NextID(), {"Compute Shader", N_COMP_SHADER})
                        .AddInput(NextID(), {"Col Attachment Structure", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(NextID(), {"Depth Attachment", N_DEPTH_STRUCTURE})
                        .AddInput(NextID(), {"Col Attachment Info In", N_IMAGE_SAMPLER})
                        .AddSelectable(NextID(), "Raster Configs", {"Fill", "Line", "Point"})
                        .SetNodeId(NextID(), "Render Node");
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                        });
                    builder
                        .AddPrimitiveData(NextID(), {"Clear Color", VEC3, glm::vec3(0.0)})
                        .AddSelectable(NextID(), "Color Format", {"g_32bFormat", "g_16bFormat"})
                        .AddSelectable(NextID(), "Load Operation", {"Load", "Clear", "Dont Care", "None"})
                        .AddSelectable(NextID(), "Store Operation", {"Load", "eDontCare", "eNone"})
                        .AddInput(NextID(), {"Col Attachment Info In", N_IMAGE_SAMPLER})
                        .AddOutput(NextID(), {"Col Attachment Info Out", N_COL_ATTACHMENT_STRUCTURE})
                        .AddSelectable(NextID(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"})
                        .SetNodeId(NextID(), "Col Attachment Structure");
                    break;
                case N_DEPTH_STRUCTURE:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                        });
                    builder
                        .AddSelectable(NextID(), "Depth Configs", {"None", "Enable", "Disable"})
                        .AddInput(NextID(), {"Depth Image In", N_DEPTH_IMAGE_SAMPLER})
                        .AddOutput(NextID(), {"Depth Attachment ", N_DEPTH_STRUCTURE})
                        .SetNodeId(NextID(), "Col Attachment Structure");
                    break;
                case N_RASTER_STRUCTURE:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                        });
                    builder
                        .AddSelectable(NextID(), "Raster Configs", {"Fill", "Point", "Line"})
                        .AddOutput(NextID(), {"Raster Config ", N_RASTER_STRUCTURE})
                        .SetNodeId(NextID(), "Raster Structure");
                    break;
                case N_PUSH_CONSTANT:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                        });
                    builder
                        .AddPrimitiveData(NextID(), {"Push Constant data", SIZE_T, size_t(0)})
                        .AddOutput(NextID(), {"Push Constant Structure ", N_PUSH_CONSTANT})
                        .SetNodeId(NextID(), "Push Constant");
                    break;
                case N_IMAGE_SAMPLER:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                            std::string imgName = selfNode.GetInputTextContent("Img Name");
                            assert(!imgName.empty() && "Img name is not valid");
                            auto imageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                                         ENGINE::g_32bFormat,
                                                                         vk::ImageUsageFlagBits::eColorAttachment |
                                                                         vk::ImageUsageFlagBits::eSampled);
                            ENGINE::ImageView* imgView = renderGraph->resourcesManager->GetImage(
                                imgName, imageInfo, 0, 0);
                            selfNode.SetOuputData("Image Sampler Result", imgName);

                            assert(imgView && "Image view must be valid");
                        });
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Image Sampler Result", N_IMAGE_SAMPLER})
                        .SetNodeId(NextID(), "Sampler Image Node");
                    break;
                case N_IMAGE_STORAGE:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                            std::string imgName = selfNode.GetInputTextContent("Img Name");
                            assert(!imgName.empty() && "Img name is not valid");

                            auto storageImageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                ENGINE::g_32bFormat,
                                vk::ImageUsageFlagBits::eStorage |
                                vk::ImageUsageFlagBits::eTransferDst);
                            
                            ENGINE::ImageView* storageImgView = renderGraph->resourcesManager->GetImage(
                                imgName, storageImageInfo, 0, 0);
                            selfNode.SetOuputData("Image Storage Result", imgName);

                            assert(storageImgView && "Image view must be valid");
                        });
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Storage Result", N_IMAGE_STORAGE})
                        .SetNodeId(NextID(), "Storage Image Node");
                    break;
                case N_DEPTH_IMAGE_SAMPLER:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                            std::string imgName = selfNode.GetInputTextContent("Img Name");
                            assert(!imgName.empty() && "Img name is not valid");
                            auto depthImageInfo = ENGINE::Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1,
                                                                      selfNode.renderGraph->core->swapchainRef->depthFormat,
                                                                      vk::ImageUsageFlagBits::eDepthStencilAttachment |
                                                                      vk::ImageUsageFlagBits::eSampled);
                            ENGINE::ImageView* imgView = renderGraph->resourcesManager->GetImage(imgName, depthImageInfo, 0, 0);
                            selfNode.SetOuputData("Depth Image Sampler Result", imgName);
                            
                            assert(imgView && "Image view must be valid");
                        });
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Image Sampler Result", N_DEPTH_IMAGE_SAMPLER})
                        .SetNodeId(NextID(), "Depth Image Node");
                    break;
                case N_VERTEX_INPUT:
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this](GraphNode& selfNode)
                        {
                            
                            auto inputText = GetFromMap(selfNode.textInputs, "Vertex Name");
                            ENGINE::VertexInput* vertexInput = renderGraph->resourcesManager->GetVertexInput(
                                inputText->content);
                            
                            vertexInput->bindingDescription.clear();
                            vertexInput->inputDescription.clear();
                            size_t offset = 0;
                            int location = 0;
                            int binding = 0;
                            auto dynamicStructure = GetFromMap(selfNode.dynamicStructures, "Vertex Builder");
                            for (auto widget : dynamicStructure->widgetsInfos)
                            {
                                int typeSelected = widget.second.selectableInfo.selectedIdx;
                                ENGINE::VertexInput::Attribs attribs = (ENGINE::VertexInput::Attribs)typeSelected;
                                vertexInput->AddVertexAttrib(attribs, binding, offset, location);
                                offset += vertexInput->GetSizeFrom(attribs);
                                location++;
                            }
                            for (auto widget : dynamicStructure->widgetsInfos)
                            {
                                vertexInput->AddVertexInputBinding(0, offset);
                            }
                            
                            selfNode.SetOuputData("Vertex Result", inputText);
                                
                        });
                    {
                        SelectableInfo selectable("Vertex Attrib: ", {"INT", "FLOAT", "VEC2", "VEC3", "VE4", "U8VEC3", "U8VEC4", "COLOR_32"});
                        DynamicStructure dynamicStructureInfo("Vertex Builder", selectable);
                        builder.AddTextInput(NextID(), {"Vertex Name", ""})
                               .AddDynamicStructure(NextID(), dynamicStructureInfo)
                               .AddOutput(NextID(),{"Vertex Result", N_VERTEX_INPUT})
                               .SetNodeId(NextID(), "Vertex Input Builder");
                    }
                    
                    break;
                case N_VERT_SHADER:
                case N_FRAG_SHADER:
                case N_COMP_SHADER:
                    
                    linkOp = std::make_unique<std::function<void(GraphNode& selfNode)>>(
                        [this, nodeType](GraphNode& selfNode)
                        {
                            ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_UNKNOWN;
                            switch (nodeType)
                            {
                            case N_VERT_SHADER: stage = ENGINE::ShaderStage::S_VERT;
                                break;
                            case N_FRAG_SHADER: stage = ENGINE::ShaderStage::S_FRAG;
                                break;
                            case N_COMP_SHADER: stage = ENGINE::ShaderStage::S_COMP;
                                break;
                            }
                            assert(stage != ENGINE::S_UNKNOWN && "Uknown shader type");
                            
                            auto multiOption = GetFromMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                            int optionSelected = multiOption->selectedIdx;
                            std::string shaderSelected = "";
                            switch (optionSelected)
                            {
                            case 0:
                                shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                                break;
                            case 1:
                                shaderSelected =SYSTEMS::OS::GetInstance()->shadersPath.string() + "\\"+ multiOption->inputTexts.at(1).content;
                                selfNode.renderGraph->resourcesManager->GetShader(shaderSelected, stage);
                                break;
                            }
                            int shaderIdx = -1;
                            if (selfNode.renderGraph->resourcesManager->shadersNames.contains(shaderSelected))
                            {
                                shaderIdx = selfNode.renderGraph->resourcesManager->shadersNames.at(shaderSelected);
                            }
                            selfNode.SetOuputData("Shader result", shaderIdx);
                        });
                    std::vector<std::any> shaderPaths;
                    shaderPaths.reserve(renderGraph->resourcesManager->shadersNames.size());
                    std::vector<std::string> options = {"Pick Shader", "Create Shader"};
                    std::map<int, TextInputInfo> textInputs;
                    textInputs.try_emplace(1, TextInputInfo{"Select Shader Name"});

                    std::map<int, Scrollable> scrollables;

                    if (nodeType == N_VERT_SHADER)
                    {
                        for (auto& shaderPath : renderGraph->resourcesManager->shadersNames)
                        {
                            if (renderGraph->resourcesManager->shaders.at(shaderPath.second)->stage == ENGINE::ShaderStage::S_VERT)
                            {
                                shaderPaths.emplace_back(shaderPath.first);
                            }
                        }
                        builder
                            .SetNodeId(NextID(), "Vert Shader")
                            .AddOutput(NextID(), {"Shader result", N_VERT_SHADER});

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
                            .SetNodeId(NextID(), "Frag Shader")
                            .AddOutput(NextID(), {"Shader result", N_FRAG_SHADER});
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
                            .SetNodeId(NextID(), "Compute Shader")
                            .AddOutput(NextID(), {"Shader result", N_COMP_SHADER});
                    }
                    scrollables.try_emplace(0, Scrollable{"Posible Shaders", STRING, shaderPaths});
                    MultiOption multiOptionInfo("Shader Options", options, textInputs, {}, scrollables);
                    builder.AddMultiOption(NextID(), multiOptionInfo);
                   
                    break;
                }

                if (!name.empty())
                {
                    builder.SetNodeName(name);
                }
                builder.SetPosition(pos);
                node = builder.Build(renderGraph, std::move(linkOp));
                return node;
            }
        };
    }
}

#endif //WIDGETS_HPP
