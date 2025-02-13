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

        enum NodeType
        {
            N_RENDER_NODE,
            N_VERT_SHADER,
            N_FRAG_SHADER,
            N_COMP_SHADER,
            N_COL_ATTACHMENT_STRUCTURE,
            N_DEPTH_STRUCTURE,
            N_PUSH_CONSTANT,
            N_IMAGE_SAMPLER,
            N_IMAGE_STORAGE,
            N_DEPTH_IMAGE_SAMPLER,
            N_VERTEX_INPUT,
            N_BUFFER,
            N_NONE,
        };

        enum PrimitiveNodeType
        {
            INT,
            UINT,
            VEC2,
            VEC3,
            VEC4,
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
            std::string name{};
            NodeType nodeType = N_NONE;
            std::any data = std::any();
            ed::PinKind pinKind{};
            int id = -1;

            bool HasData()
            {
                return data.has_value();
            }

            template <typename T>
            T GetData()
            {
                assert(HasData() && "Pin does not have data");
                return std::any_cast<T>(data);
            }

            void Draw()
            {
                assert(nodeType != N_NONE);
                if (id > -1)
                    ImGui::PushID(id);

                ed::BeginPin(id, pinKind);
                if (pinKind == ed::PinKind::Input)
                {
                    ImGui::Text(name.c_str());
                }
                else
                {
                    std::string nameText = name + "        ->";
                    ImGui::Text(nameText.c_str());
                }
                ed::EndPin();

                if (id > -1)
                    ImGui::PopID();
            }
        };

        //enums
        struct EnumSelectable
        {
            std::string name;
            std::vector<std::string> options;
            int selectedIdx;

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

            template <typename T>
            T GetEnumFromIndex(std::map<int, T>& map)
            {
                return map.at(selectedIdx);
            }
        };

        struct TextInput
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

        struct PrimitiveInput
        {
            std::string name;
            PrimitiveNodeType primitiveType;
            std::any content;

            bool HasData()
            {
                return content.has_value();
            }

            template <typename T>
            T GetData()
            {
                assert(HasData() && "Primitive has no data");
                T data = std::any_cast<T>(content);
                return data;
            }

            void Draw(const int& id = -1)
            {
                ImGui::PushItemWidth(100.0);
                if (id > -1)
                    ImGui::PushID(id);
                int intData;
                int uIntdata;
                glm::vec2 vec2Data;
                glm::vec3 vec3Data;
                glm::vec4 vec4Data;
                std::string stringData;
                size_t sizeData;
                int sizeTemp;
                std::any result;
                switch (primitiveType)
                {
                case INT:
                    intData = GetData<int>();
                    ImGui::InputInt(name.c_str(), &intData, 1, 100);
                    result = intData;
                    break;
                case UINT:
                    uIntdata = GetData<int>();
                    ImGui::InputInt(name.c_str(), &uIntdata, 1, 100);
                    result = uIntdata;
                    break;
                case VEC2:
                    vec2Data = GetData<glm::vec2>();
                    ImGui::InputFloat2(name.c_str(), glm::value_ptr(vec2Data));
                    result = vec2Data;
                    break;
                case VEC3:
                    vec3Data = GetData<glm::vec3>();
                    ImGui::InputFloat3(name.c_str(), glm::value_ptr(vec3Data));
                    result = vec3Data;
                    break;
                case VEC4:
                    vec4Data = GetData<glm::vec4>();
                    ImGui::InputFloat4(name.c_str(), glm::value_ptr(vec4Data));
                    result = vec4Data;
                    break;
                case SIZE_T:
                    sizeData = GetData<size_t>();
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

            template <typename T>
            T GetCurrent()
            {
                return std::any_cast<T>(content.at(selectedIdx));
            }
        };

        struct MultiOption
        {
            std::string name;
            std::map<int, std::string> options;
            std::map<int, TextInput> inputTexts;
            std::map<int, EnumSelectable> selectables;
            std::map<int, Scrollable> scrollables;
            int selectedIdx = 0;

            MultiOption(const std::string& name, std::vector<std::string> options, std::map<int, TextInput> inputTexts,
                        std::map<int, EnumSelectable> selectables, std::map<int, Scrollable> scrollables)
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
                    EnumSelectable selectableInfo;
                    TextInput textInputInfo;
                    PrimitiveInput primitiveInfo;
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
                        new(&selectableInfo) EnumSelectable(other.selectableInfo);
                        break;
                    case W_TEXT_INPUT:
                        new(&textInputInfo) TextInput(other.textInputInfo);
                        break;
                    case W_PRIMITIVE:
                        new(&primitiveInfo) PrimitiveInput(other.primitiveInfo);
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
                        new(&selectableInfo) EnumSelectable(std::move(other.selectableInfo));
                        break;
                    case W_TEXT_INPUT:
                        new(&textInputInfo) TextInput(std::move(other.textInputInfo));
                        break;
                    case W_PRIMITIVE:
                        new(&primitiveInfo) PrimitiveInput(std::move(other.primitiveInfo));
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
                        selectableInfo.~EnumSelectable();
                        break;
                    case W_TEXT_INPUT:
                        textInputInfo.~TextInput();
                        break;
                    case W_PRIMITIVE:
                        primitiveInfo.~PrimitiveInput();
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
                            new(&selectableInfo) EnumSelectable(other.selectableInfo);
                            break;
                        case W_TEXT_INPUT:
                            new(&textInputInfo) TextInput(other.textInputInfo);
                            break;
                        case W_PRIMITIVE:
                            new(&primitiveInfo) PrimitiveInput(other.primitiveInfo);
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
                            new(&selectableInfo) EnumSelectable(std::move(other.selectableInfo));
                            break;
                        case W_TEXT_INPUT:
                            new(&textInputInfo) TextInput(std::move(other.textInputInfo));
                            break;
                        case W_PRIMITIVE:
                            new(&primitiveInfo) PrimitiveInput(std::move(other.primitiveInfo));
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

            DynamicStructure(const std::string& name, EnumSelectable& selectable)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_SELECTABLE;
                new(&baseWidgetInfo.selectableInfo) EnumSelectable(selectable);
                widgetsInfos.try_emplace(widgetsInfos.size(), std::move(baseWidgetInfo));
            }

            DynamicStructure(const std::string& name, TextInput& textInputInfo)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_TEXT_INPUT;
                new(&baseWidgetInfo.textInputInfo) TextInput(textInputInfo);
                widgetsInfos.try_emplace(widgetsInfos.size(), std::move(baseWidgetInfo));
            }

            DynamicStructure(const std::string& name, PrimitiveInput& primitiveInfo)
            {
                this->name = name;
                NodeWidgetsInfos baseWidgetInfo = {};
                baseWidgetInfo.type = W_PRIMITIVE;
                new(&baseWidgetInfo.primitiveInfo) PrimitiveInput(primitiveInfo);
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
                if (widgetsInfos.size() == 1) { return; }

                auto it = widgetsInfos.find(widgetsInfos.size() - 1);
                if (it != widgetsInfos.end())
                {
                    widgetsInfos.erase(it);
                }
                else
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

        struct Button
        {
            std::string name;

            std::function<void()> action;

            void Draw(const int& id = -1)
            {
                if (id > -1)
                    ImGui::PushID(id);

                if (ImGui::Button(name.c_str(), ImVec2{50, 50}))
                {
                    action();
                }
                if (id > -1)
                    ImGui::PopID();
            }
        };

        template <typename T>
        struct Actioner
        {
            T* context;
            std::function<void(T*)> action;

            void Action()
            {
                action(context);
            }
        };

        //note that this only handle copyable data types
        struct GraphNode
        {
            ENGINE::RenderGraph* renderGraph;
            WindowProvider* windowProvider;

            std::map<int, PinInfo> inputNodes;
            std::map<int, PinInfo> outputNodes;
            std::map<int, EnumSelectable> selectables;
            std::map<int, TextInput> textInputs;
            std::map<int, PrimitiveInput> primitives;
            std::map<int, Scrollable> scrollables;
            std::map<int, MultiOption> multiOptions;
            std::map<int, DynamicStructure> dynamicStructures;
            //since all ids are not repeated this will represent properly all the widgets
            std::map<int, bool> drawableWidget;

            std::map<int, NodeType> graphNodesLinks;
            std::map<int, GraphNode>* graphNodesRef = nullptr;
            std::string name;
            glm::vec2 pos;
            ed::NodeId nodeId;
            int globalId = -1;
            bool valid = false;
            bool firstFrame = true;


            std::map<std::string, std::function<void(GraphNode&)>*> callbacks;

            void RecompileNode()
            {
                valid = false;
                RunCallback("output_c");
            }

            int RunCallback(std::string callback)
            {
                if (!callbacks.contains(callback) || callbacks.at(callback) == nullptr)
                {
                    return 1;
                }
                (*callbacks.at(callback))(*this);
                return 0;
                
            }


            int AddLink(int id, NodeType nodeType = N_NONE)
            {
                if (nodeType != N_NONE && !graphNodesLinks.contains(id))
                {
                    graphNodesLinks.try_emplace(id, nodeType);
                }
                return 0;
            }

            PinInfo* GetInputDataById(int id)
            {
                if (!inputNodes.contains(id))
                {
                    return nullptr;
                }
                return &inputNodes.at(id);
            }

            PinInfo* GetInputDataByName(const std::string& name)
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

            PinInfo* GetOutputDataById(int id)
            {
                if (!outputNodes.contains(id))
                {
                    return nullptr;
                }
                return &outputNodes.at(id);
            }

            PinInfo* GetOutputDataByName(const std::string& name)
            {
                for (auto& node : outputNodes)
                {
                    if (node.second.name == name)
                    {
                        return &node.second;
                    }
                }
                return nullptr;
            }

            void SetOuputData(const std::string& name, std::any data)
            {
                GetOutputDataByName(name)->data = data;
            }

            void SetOuputData(int id, std::any data)
            {
                GetOutputDataById(id)->data = data;
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
                    input.second.Draw();
                }

                for (auto& selectable : selectables)
                {
                    if (drawableWidget.at(selectable.first))
                        selectable.second.Draw(selectable.first);
                }
                for (auto& tInput : textInputs)
                {
                    if (drawableWidget.at(tInput.first))
                        tInput.second.Draw(tInput.first);
                }
                for (auto& primitiveInfo : primitives)
                {
                    if (drawableWidget.at(primitiveInfo.first))
                        primitiveInfo.second.Draw(primitiveInfo.first);
                }

                for (auto& scrollable : scrollables)
                {
                    if (drawableWidget.at(scrollable.first))
                        scrollable.second.Draw(scrollable.first);
                }
                for (auto& multiOption : multiOptions)
                {
                    if (drawableWidget.at(multiOption.first))
                        multiOption.second.Draw(multiOption.first);
                }
                for (auto& dynamicStructure : dynamicStructures)
                {
                    if (drawableWidget.at(dynamicStructure.first))
                        dynamicStructure.second.Draw(dynamicStructure.first);
                }

                // ImGui::PopStyleVar();
                for (auto& output : outputNodes)
                {
                    output.second.Draw();
                }
                ed::EndNode();
                ImGui::PopID();
                firstFrame = false;
            }
        };

        struct GraphNodeBuilder
        {
            ed::NodeId nodeId;
            std::map<int, PinInfo> inputNodes = {};
            std::map<int, PinInfo> outputNodes = {};
            std::map<int, EnumSelectable> selectables = {};
            std::map<int, TextInput> textInputs = {};
            std::map<int, PrimitiveInput> primitives = {};
            std::map<int, Scrollable> scrollables = {};
            std::map<int, MultiOption> multiOptions = {};
            std::map<int, DynamicStructure> dynamicStructures = {};
            std::map<int, bool> drawableWidgets = {};
            std::map<std::string, std::function<void(GraphNode&)>*> callbacks;
            

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
                pinInfo.pinKind = ed::PinKind::Input;
                pinInfo.id = id;
                inputNodes.try_emplace(id, pinInfo);
                drawableWidgets.try_emplace(id, true);
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

            GraphNodeBuilder& AddSelectable(int id, std::string name, std::vector<std::string> options)
            {
                selectables.try_emplace(id, EnumSelectable{name, options, 0});
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            GraphNodeBuilder& AddTextInput(int id, TextInput info)
            {
                textInputs.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            GraphNodeBuilder& AddScrollable(int id, Scrollable info)
            {
                scrollables.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            GraphNodeBuilder& AddPrimitiveData(int id, PrimitiveInput info)
            {
                primitives.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            GraphNodeBuilder& AddMultiOption(int id, MultiOption info)
            {
                multiOptions.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
                return *this;
            }

            GraphNodeBuilder& AddDynamicStructure(int id, DynamicStructure info)
            {
                dynamicStructures.try_emplace(id, info);
                drawableWidgets.try_emplace(id, true);
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


            GraphNode Build(ENGINE::RenderGraph* renderGraph, WindowProvider* windowProvider)
            {
                // assert(nodeId.Get() > -1 && "Set the id before building");
                assert(!name.empty() && "Set a valid name");
                if (callbacks.empty())
                {
                    SetBaseCallbacks();
                }

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
                graphNode.drawableWidget = drawableWidgets;
                graphNode.pos = pos;
                for (auto i = callbacks.begin(); i != callbacks.end();)
                {
                    graphNode.callbacks.try_emplace(i->first,i->second);
                    i++;
                }
                graphNode.renderGraph = renderGraph;
                graphNode.windowProvider = windowProvider;
                Reset();
                return graphNode;
            }
            void SetBaseCallbacks()
            {
                callbacks.clear();
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
                SetBaseCallbacks();
                pos = glm::vec2(0.0);
                nodeId = -1;
                name = "";
            }
        };

        
        struct GraphNodeRegistry
        {
            struct CallbackInfo
            {
                std::map<std::string, int> callbacksMap = {};
                std::map<int, std::unique_ptr<std::function<void(GraphNode&)>>> callbacks = {};
                void AddCallback(std::string name, std::function<void(GraphNode&)>&& callback)
                {
                    int id = callbacks.size();
                    callbacksMap.try_emplace(name, id);
                    callbacks.try_emplace(id, std::make_unique<std::function<void(GraphNode&)>>(callback));
                }
                std::function<void(GraphNode&)>* GetCallbackByName(std::string name)
                {
                    assert(callbacksMap.contains(name));
                    auto callback = callbacks.at(callbacksMap.at(name)).get();
                    return callback;
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
            GraphNodeRegistry()
            {
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

                callbacksRegistry.at(N_RENDER_NODE).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    std::map<NodeType, bool> configsAdded;

                    PinInfo* vert = GetFromNameInMap(selfNode.inputNodes, "Vertex Shader");
                    PinInfo* frag = GetFromNameInMap(selfNode.inputNodes, "Fragment Shader");
                    PinInfo* compute = GetFromNameInMap(selfNode.inputNodes, "Compute Shader");
                    int vertId = -1;
                    int fragId = -1;
                    int compId = -1;

                    if (compute->HasData())
                    {
                        configsAdded.try_emplace(N_COMP_SHADER, true);
                        try
                        {
                            compId = compute->GetData<int>();
                        }
                        catch (std::bad_cast c)
                        {
                            assert(false);
                        }

                        selfNode.drawableWidget.at(vert->id) = false;
                        selfNode.drawableWidget.at(frag->id) = false;
                    }
                    else
                    {
                        selfNode.drawableWidget.at(compute->id) = false;
                        configsAdded.try_emplace(N_VERTEX_INPUT, false);
                        configsAdded.try_emplace(N_COL_ATTACHMENT_STRUCTURE, false);
                        configsAdded.try_emplace(N_VERT_SHADER, false);
                        configsAdded.try_emplace(N_FRAG_SHADER, false);
                        if (vert->HasData())
                        {
                            configsAdded.at(N_VERT_SHADER) = true;
                            vertId = vert->GetData<int>();
                        }
                        if (frag->HasData())
                        {
                            PinInfo* fragPin = selfNode.GetInputDataByName("Fragment Shader");
                            configsAdded.at(N_FRAG_SHADER) = true;
                            fragId = fragPin->GetData<int>();
                        }
                    }
                    glm::vec4 clearColData;
                    vk::AttachmentLoadOp loadOpData;
                    vk::AttachmentStoreOp storeOpData;
                    ENGINE::BlendConfigs blendData;
                    vk::Format colFormatData;
                    PinInfo* image;
                    PinInfo* depthImage;
                    ENGINE::AttachmentInfo info;
                    std::string attachmentName;
                    ENGINE::VertexInput* vertexInput = nullptr;
                    for (auto id : selfNode.graphNodesLinks)
                    {
                        if (id.second == N_COL_ATTACHMENT_STRUCTURE)
                        {
                            GraphNode& graphNodeRef = selfNode.graphNodesRef->at(id.first);
                            attachmentName = graphNodeRef.name;
                            PrimitiveInput* clearColor = GetFromNameInMap(
                                graphNodeRef.primitives, "Clear Color");
                            EnumSelectable* colFormat = GetFromNameInMap(
                                graphNodeRef.selectables, "Load Operation");
                            EnumSelectable* loadOp = GetFromNameInMap(
                                graphNodeRef.selectables, "Load Operation");
                            EnumSelectable* storeOp = GetFromNameInMap(
                                graphNodeRef.selectables, "Store Operation");
                            EnumSelectable* blendConfigs = GetFromNameInMap(
                                graphNodeRef.selectables, "Blend Configs");

                            clearColData = clearColor->GetData<glm::vec4>();
                            loadOpData = (vk::AttachmentLoadOp)loadOp->selectedIdx;
                            storeOpData = (vk::AttachmentStoreOp)storeOp->selectedIdx;
                            blendData = (ENGINE::BlendConfigs)blendConfigs->selectedIdx;

                            std::map<int, vk::Format> validFormats = {
                                {0, ENGINE::g_32bFormat}, {1, ENGINE::g_16bFormat}, {2, ENGINE::g_ShipperFormat}
                            };
                            colFormatData = colFormat->GetEnumFromIndex<vk::Format>(validFormats);

                            image = graphNodeRef.GetInputDataByName("Col Attachment Sampler");
                            if (image->HasData())
                            {
                                configsAdded.at(N_COL_ATTACHMENT_STRUCTURE) = true;
                                info = ENGINE::GetColorAttachmentInfo(
                                    clearColData, colFormatData, loadOpData, storeOpData);
                            }
                        }
                        if (id.second == N_DEPTH_STRUCTURE)
                        {
                            GraphNode& graphNodeRef = selfNode.graphNodesRef->at(id.first);
                            attachmentName = graphNodeRef.name;

                            depthImage = graphNodeRef.GetInputDataByName("Depth Attachment Sampler");
                            if (depthImage->HasData())
                            {
                                configsAdded.try_emplace(N_DEPTH_STRUCTURE, true);
                            }
                        }
                    }

                    PinInfo* vertexPin = selfNode.GetInputDataByName("Vertex Input");

                    if (vertexPin->HasData())
                    {
                        std::string vertexName = vertexPin->GetData<std::string>();
                        vertexInput = selfNode.renderGraph->resourcesManager->GetVertexInput(vertexName);
                        configsAdded.at(N_VERTEX_INPUT) = true;
                    }


                    int configsToMatch = configsAdded.size();
                    int configsMatched = 0;
                    for (auto added : configsAdded)
                    {
                        if (!added.second)
                        {
                            break;
                        }
                        configsMatched++;
                    }
                    if (configsMatched == configsToMatch)
                    {
                        std::string name = "";
                        TextInput* nodeName = GetFromNameInMap(selfNode.textInputs, "RenderNode Name");
                        if (nodeName)
                        {
                            name = nodeName->content;
                        }
                        else
                        {
                            name = "PassName_" + std::to_string(selfNode.renderGraph->renderNodes.size());
                        }
                        auto renderNode = selfNode.renderGraph->AddPass(name);
                        if (configsAdded.contains(N_COMP_SHADER))
                        {
                            renderNode->SetCompShader(
                                selfNode.renderGraph->resourcesManager->GetShaderFromId(compId));
                        }
                        else
                        {
                            assert(vertId > -1 && "Vert id invalid");
                            assert(fragId > -1 && "Frag Id invalid");
                            assert(vertexInput && "invalid vertex input");
                            renderNode->SetVertShader(
                                selfNode.renderGraph->resourcesManager->GetShaderFromId(vertId));
                            renderNode->SetFragShader(
                                selfNode.renderGraph->resourcesManager->GetShaderFromId(fragId));
                            renderNode->SetVertexInput(*vertexInput);
                            renderNode->AddColorAttachmentOutput(attachmentName, info, blendData);
                            if (image->HasData())
                            {
                                std::string imageName = image->GetData<std::string>();
                                renderNode->AddColorImageResource(
                                    imageName,
                                    selfNode.renderGraph->resourcesManager->GetImageViewFromName(imageName));
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
                        }
                        renderNode->SetConfigs({true});
                        renderNode->BuildRenderGraphNode();
                        renderNode->active = false;
                        selfNode.valid = true;
                    }    
                });
                callbacksRegistry.at(N_VERT_SHADER).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_FRAG_SHADER).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_COMP_SHADER).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_COL_ATTACHMENT_STRUCTURE).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_DEPTH_STRUCTURE).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_PUSH_CONSTANT).AddCallback("output_c", [](GraphNode& selfNode){});
                callbacksRegistry.at(N_IMAGE_SAMPLER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    std::string imgName = selfNode.GetInputTextContent("Img Name");
                    assert(!imgName.empty() && "Img name is not valid");
                    auto imageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                                                 ENGINE::g_32bFormat,
                                                                 vk::ImageUsageFlagBits::eColorAttachment |
                                                                 vk::ImageUsageFlagBits::eSampled);
                    ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetImage(
                        imgName, imageInfo, 0, 0);
                    selfNode.SetOuputData("Image Sampler Result", imgName);

                    assert(imgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_IMAGE_STORAGE).AddCallback("output_c", [](GraphNode& selfNode)
                {
                           std::string imgName = selfNode.GetInputTextContent("Img Name");
                            assert(!imgName.empty() && "Img name is not valid");

                            auto storageImageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                ENGINE::g_32bFormat,
                                vk::ImageUsageFlagBits::eStorage |
                                vk::ImageUsageFlagBits::eTransferDst);

                            ENGINE::ImageView* storageImgView = selfNode.renderGraph->resourcesManager->GetImage(
                                imgName, storageImageInfo, 0, 0);
                            selfNode.SetOuputData("Image Storage Result", imgName);

                            assert(storageImgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_DEPTH_IMAGE_SAMPLER).AddCallback("output_c", [](GraphNode& selfNode){
                    std::string imgName = selfNode.GetInputTextContent("Img Name");
                    assert(!imgName.empty() && "Img name is not valid");
                    auto depthImageInfo = ENGINE::Image::CreateInfo2d(selfNode.windowProvider->GetWindowSize(), 1, 1,
                                                                      selfNode.renderGraph->core->swapchainRef->
                                                                               depthFormat,
                                                                      vk::ImageUsageFlagBits::eDepthStencilAttachment
                                                                      |
                                                                      vk::ImageUsageFlagBits::eSampled);
                    ENGINE::ImageView* imgView = selfNode.renderGraph->resourcesManager->GetImage(
                        imgName, depthImageInfo, 0, 0);
                    selfNode.SetOuputData("Depth Sampler Result", imgName);

                    assert(imgView && "Image view must be valid");
                });
                callbacksRegistry.at(N_VERTEX_INPUT).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    TextInput* inputText = GetFromNameInMap(selfNode.textInputs, "Vertex Name");
                    std::string vertexName = inputText->content;
                    ENGINE::VertexInput* vertexInput = selfNode.renderGraph->resourcesManager->GetVertexInput(inputText->content);

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

                    selfNode.SetOuputData("Vertex Result", vertexName);
                });
                callbacksRegistry.at(N_COMP_SHADER).AddCallback("output_c", [](GraphNode& selfNode){
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_COMP;
                    assert(stage != ENGINE::S_UNKNOWN && "Uknown shader type");
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = SYSTEMS::OS::GetInstance()->shadersPath.string() + "\\" + multiOption->
                            inputTexts.at(1).content;
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
                callbacksRegistry.at(N_VERT_SHADER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_VERT;
                    assert(stage != ENGINE::S_UNKNOWN && "Uknown shader type");
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = SYSTEMS::OS::GetInstance()->shadersPath.string() + "\\" + multiOption->
                            inputTexts.at(1).content;
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
                callbacksRegistry.at(N_FRAG_SHADER).AddCallback("output_c", [](GraphNode& selfNode)
                {
                    ENGINE::ShaderStage stage = ENGINE::ShaderStage::S_FRAG;
                    assert(stage != ENGINE::S_UNKNOWN && "Uknown shader type");
                    auto multiOption = GetFromNameInMap<MultiOption>(selfNode.multiOptions, "Shader Options");
                    int optionSelected = multiOption->selectedIdx;
                    std::string shaderSelected = "";
                    switch (optionSelected)
                    {
                    case 0:
                        shaderSelected = multiOption->scrollables.at(0).GetCurrent<std::string>();
                        break;
                    case 1:
                        shaderSelected = SYSTEMS::OS::GetInstance()->shadersPath.string() + "\\" + multiOption->
                            inputTexts.at(1).content;
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
                
                
            }
        };

        struct GraphNodeFactory
        {
            int idGen = 100;
            int idNodeGen = 0;
            GraphNodeBuilder builder = {};
            GraphNodeRegistry nodeRegistry = {};
            ENGINE::RenderGraph* renderGraph;
            WindowProvider* windowProvider;
            std::map<int, GraphNode> graphNodes = {};

            int NextID()
            {
                return idGen++;
            }

            int NextNodeID()
            {
                return idNodeGen++;
            }

            GraphNode* GetNode(NodeType nodeType, glm::vec2 pos = glm::vec2(0.0), std::string name = "")
            {
                assert(renderGraph && "Null rgraph");
                assert(windowProvider && "Null window Provider");
                GraphNode node;
                std::function<void(GraphNode&)>* outputOp;
                outputOp = nodeRegistry.GetCallback(nodeType, "output_c");
                switch (nodeType)
                {
                case N_RENDER_NODE:
                    builder
                        .AddTextInput(NextID(), {"RenderNode Name"})
                        .AddOutput(NextID(), {"Result Node", N_RENDER_NODE})
                        .AddInput(NextID(), {"Input Node", N_RENDER_NODE})
                        .AddInput(NextID(), {"Vertex Shader", N_VERT_SHADER})
                        .AddInput(NextID(), {"Fragment Shader", N_FRAG_SHADER})
                        .AddInput(NextID(), {"Compute Shader", N_COMP_SHADER})
                        .AddInput(NextID(), {"Col Attachment Node", N_COL_ATTACHMENT_STRUCTURE})
                        .AddInput(NextID(), {"Depth Attachment", N_DEPTH_STRUCTURE})
                        .AddInput(NextID(), {"Vertex Input", N_VERTEX_INPUT})
                        .AddSelectable(NextID(), "Raster Configs", {"Fill", "Line", "Point"})
                        .SetNodeId(NextID(), "Render Node");
                    break;
                case N_COL_ATTACHMENT_STRUCTURE:
                    builder
                        .AddPrimitiveData(NextID(), {"Clear Color", VEC4, glm::vec4(0.0)})
                        .AddSelectable(NextID(), "Color Format", {"g_32bFormat", "g_16bFormat"})
                        .AddSelectable(NextID(), "Load Operation", {"Load", "Clear", "Dont Care", "None"})
                        .AddSelectable(NextID(), "Store Operation", {"Load", "eDontCare", "eNone"})
                        .AddSelectable(NextID(), "Blend Configs", {"None", "Opaque", "Add", "Mix", "Alpha Blend"})
                        .AddInput(NextID(), {"Col Attachment Sampler", N_IMAGE_SAMPLER})
                        .AddOutput(NextID(), {"Col Attachment Result", N_COL_ATTACHMENT_STRUCTURE})
                        .SetNodeId(NextID(), "Col Attachment Node");
                    break;
                case N_DEPTH_STRUCTURE:
                    builder
                        .AddSelectable(NextID(), "Depth Configs", {"None", "Enable", "Disable"})
                        .AddInput(NextID(), {"Depth Image In", N_DEPTH_IMAGE_SAMPLER})
                        .AddOutput(NextID(), {"Depth Attachment ", N_DEPTH_STRUCTURE})
                        .SetNodeId(NextID(), "Depth Attachment Node");
                    break;
                case N_PUSH_CONSTANT:
                    builder
                        .AddPrimitiveData(NextID(), {"Push Constant data", SIZE_T, size_t(0)})
                        .AddOutput(NextID(), {"Push Constant Structure ", N_PUSH_CONSTANT})
                        .SetNodeId(NextID(), "Push Constant");
                    break;
                case N_IMAGE_SAMPLER:
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Image Sampler Result", N_IMAGE_SAMPLER})
                        .SetNodeId(NextID(), "Sampler Image Node");
                    break;
                case N_IMAGE_STORAGE:
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Storage Result", N_IMAGE_STORAGE})
                        .SetNodeId(NextID(), "Storage Image Node");
                    break;
                case N_DEPTH_IMAGE_SAMPLER:
                    builder
                        .AddTextInput(NextID(), {"Img Name", "Image Name"})
                        .AddOutput(NextID(), {"Depth Sampler Result", N_DEPTH_IMAGE_SAMPLER})
                        .SetNodeId(NextID(), "Depth Image Node");
                    break;
                case N_VERTEX_INPUT:
                    {
                        EnumSelectable selectable("Vertex Attrib: ", {
                              "INT", "FLOAT", "VEC2", "VEC3", "VE4", "U8VEC3", "U8VEC4","COLOR_32"});
                        DynamicStructure dynamicStructureInfo("Vertex Builder", selectable);
                        builder.AddTextInput(NextID(), {"Vertex Name", ""})
                               .AddDynamicStructure(NextID(), dynamicStructureInfo)
                               .AddOutput(NextID(), {"Vertex Result", N_VERTEX_INPUT})
                               .SetNodeId(NextID(), "Vertex Input Builder");
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
                builder.AddCallback("output_c", outputOp);
                int id = NextNodeID();
                graphNodes.try_emplace(id, builder.Build(renderGraph, windowProvider));
                graphNodes.at(id).globalId = id;
                graphNodes.at(id).graphNodesRef = &graphNodes;
                return &graphNodes.at(id);
            }
        };
    }
}

#endif //WIDGETS_HPP
