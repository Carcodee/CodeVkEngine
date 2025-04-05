//

// Created by carlo on 2025-02-22.
//






#ifndef GRAPHNODE_HPP
#define GRAPHNODE_HPP

namespace UI::Nodes{
    
        namespace ed = ax::NodeEditor;
        enum NodeType
        {
            N_ROOT_NODE = 1 << 0,
            N_RENDER_NODE = 1 << 1,
            N_VERT_SHADER = 1 << 2,
            N_FRAG_SHADER = 1 << 3,
            N_COMP_SHADER = 1 << 4,
            N_COL_ATTACHMENT_STRUCTURE = 1 << 5,
            N_DEPTH_STRUCTURE = 1 << 6,
            N_PUSH_CONSTANT = 1 << 7,
            N_IMAGE_SAMPLER = 1 << 8,
            N_IMAGE_STORAGE = 1 << 9,
            N_DEPTH_IMAGE_SAMPLER = 1 << 10,
            N_VERTEX_INPUT = 1 << 11,
            N_BUFFER = 1 << 12,
            N_NONE = 1 << 13,
        };
        static std::map<NodeType, std::string> nodeTypeStrings{
            {N_ROOT_NODE, "N_ROOT_NODE"},
            {N_RENDER_NODE, "N_RENDER_NODE"},
            {N_VERT_SHADER, "N_VERT_SHADER"},
            {N_FRAG_SHADER, "N_FRAG_SHADER"},
            {N_COMP_SHADER, "N_COMP_SHADER"},
            {N_COL_ATTACHMENT_STRUCTURE, "N_COL_ATTACHMENT_STRUCTURE"},
            {N_DEPTH_STRUCTURE, "N_DEPTH_STRUCTURE"},
            {N_PUSH_CONSTANT, "N_PUSH_CONSTANT"},
            {N_IMAGE_SAMPLER, "N_IMAGE_SAMPLER"},
            {N_IMAGE_STORAGE, "N_IMAGE_STORAGE"},
            {N_DEPTH_IMAGE_SAMPLER, "N_DEPTH_IMAGE_SAMPLER"},
            {N_VERTEX_INPUT, "N_VERTEX_INPUT"},
            {N_BUFFER, "N_BUFFER"},
            {N_NONE, "N_NONE"},
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

        static bool IsValidNodeCombination(NodeType nodeType, NodeType dstNode)
        {
            if (nodeType & dstNode)
            {
                return true;
            }
            return false;
        }
        static std::set<NodeType> GetNodeType(NodeType nodeType)
        {
            std::set<NodeType> nodesFind = {};
            if (nodeType & N_ROOT_NODE)
            {
                nodesFind.insert(N_ROOT_NODE);
            }
            if (nodeType & N_RENDER_NODE)
            {
                nodesFind.insert(N_RENDER_NODE);
            }
            if (nodeType & N_VERT_SHADER)
            {
                nodesFind.insert(N_VERT_SHADER);
            }
            if (nodeType & N_FRAG_SHADER)
            {
                nodesFind.insert(N_FRAG_SHADER);
            }
            if (nodeType & N_COMP_SHADER)
            {
                nodesFind.insert(N_COMP_SHADER);
            }
            if (nodeType & N_COL_ATTACHMENT_STRUCTURE)
            {
                nodesFind.insert(N_COL_ATTACHMENT_STRUCTURE);
            }
            if (nodeType & N_DEPTH_STRUCTURE)
            {
                nodesFind.insert(N_DEPTH_STRUCTURE);
            }
            if (nodeType & N_PUSH_CONSTANT)
            {
                nodesFind.insert(N_PUSH_CONSTANT);
            }
            if (nodeType & N_IMAGE_SAMPLER)
            {
                nodesFind.insert(N_IMAGE_SAMPLER);
            }
            if (nodeType & N_IMAGE_STORAGE)
            {
                nodesFind.insert(N_IMAGE_STORAGE);
            }
            if (nodeType & N_DEPTH_IMAGE_SAMPLER)
            {
                nodesFind.insert(N_DEPTH_IMAGE_SAMPLER);
            }
            if (nodeType & N_VERTEX_INPUT)
            {
                nodesFind.insert(N_VERTEX_INPUT);
            }
            if (nodeType & N_BUFFER)
            {
                nodesFind.insert(N_BUFFER);
            }
            if (nodesFind.empty())
            {
                nodesFind.insert(N_NONE);
                SYSTEMS::Logger::GetInstance()->LogMessage("There was no nodes finded in nodeType structure");
            }
            return nodesFind;
            
        }
        struct PinInfo 
        {
            std::string name{};
            NodeType nodeType = N_NONE;
            std::any data = std::any();
            ed::PinKind pinKind{};
            int id = -1;
            PinInfo* linkedPin = nullptr;
            PinInfo(std::string name, NodeType nodeType)
            {
                this->name = name;
                this->nodeType = nodeType;
            }

            
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
        enum WidgetType
        {
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

            struct MultiOption
        {
            std::string name;
            std::map<int, std::string> options;
            std::map<int, TextInput> inputTexts;
            std::map<int, EnumSelectable> selectables;
            std::map<int, Scrollable> scrollables;
            std::map<int, DynamicStructure> dynamicStructures;
            int selectedIdx = 0;

            MultiOption(const std::string& name, std::vector<std::string> options, std::map<int, TextInput> inputTexts,
                        std::map<int, EnumSelectable> selectables, std::map<int, Scrollable> scrollables, std::map<int, DynamicStructure> dynamicStructures)
            {
                this->name = name;
                for (int i = 0; i < options.size(); ++i)
                {
                    this->options.try_emplace(i, options[i]);
                }
                this->inputTexts = inputTexts;
                this->selectables = selectables;
                this->scrollables = scrollables;
                this->dynamicStructures = dynamicStructures;
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
                    
                if (dynamicStructures.contains(selectedIdx))
                    dynamicStructures.at(selectedIdx).Draw();

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

        struct GraphNode
        {
            ENGINE::RenderGraph* renderGraph;
            WindowProvider* windowProvider;
            std::unordered_map<int, PinInfo> inputNodes;
            std::unordered_map<int, PinInfo> outputNodes;
            std::unordered_map<int, EnumSelectable> selectables;
            std::unordered_map<int, TextInput> textInputs;
            std::unordered_map<int, PrimitiveInput> primitives;
            std::unordered_map<int, Scrollable> scrollables;
            std::unordered_map<int, MultiOption> multiOptions;
            std::unordered_map<int, DynamicStructure> dynamicStructures;
            //since all ids are not repeated this will represent properly all the widgets

            std::unordered_map<int, bool> drawableWidgets;
            std::unordered_map<int, bool> addMoreWidgets;

            std::unordered_map<int, NodeType> graphNodesLinks;
            std::unordered_map<int, GraphNode*>* graphNodesRef = nullptr;
            GraphNodeResManager* graphNodeResManager = nullptr;

            std::string name;
            glm::vec2 pos;
            ed::NodeId nodeId;
            int* widgetsIdGen = nullptr;
            int globalId = -1;
            bool updateData = false;
            bool valid = false;
            bool firstFrame = true;


            std::unordered_map<std::string, std::function<void(GraphNode&)>*> callbacks;

            void UpdateInfo();

            template <typename F, typename S>
            std::vector<std::pair<F, S>> GetVectorFromMap(std::unordered_map<F, S>& map)
            {
                std::vector<std::pair<F, S>> copiedMap = {};
                for (auto it = map.begin(); it != map.end(); ++it)
                {
                    copiedMap.push_back(*it);
                }
                return copiedMap;
            }

            void Sort();

            int RunCallback(std::string callback);


            void Draw();


            void ToggleDraw(int id, bool drawCondition);

            void RecompileNode();

            int AddLink(int id, NodeType nodeType = N_NONE);

            std::map<NodeType, GraphNode*> GetGraphNodeRef(NodeType nodeType);

            PinInfo* GetInputDataById(int id);


            PinInfo* GetInputDataByName(const std::string& name);


            PinInfo* GetOutputDataById(int id);


            PinInfo* GetOutputDataByName(const std::string& name);

            bool HasOutput(NodeType type);

            bool HasInput(NodeType type);
            
            void SetOuputData(const std::string& name, std::any data);


            void SetOuputData(int id, std::any data);


            Scrollable* GetScrollable(const std::string& name);

            MultiOption* GetMultiOption(const std::string name);

            TextInput* GetTextInput(const std::string& name);
            
            DynamicStructure* GetDynamicStructure(const std::string& name);
        };

}

#endif //GRAPHNODE_HPP
