//


// Created by carlo on 2025-01-07.
//






#ifndef RG_NODEEDITOR_HPP
#define RG_NODEEDITOR_HPP

namespace UI
{
    namespace ed = ax::NodeEditor;

    class RG_NodeEditor
    {
    public:
        
        void Init(ENGINE::RenderGraph* renderGraph, WindowProvider* windowProvider)
        {
            
            if (!firstFrame) { return; }
            
            this->renderGraph = renderGraph;
            this->windowProvider = windowProvider;
            this->factory.renderGraph =renderGraph;       
            this->factory.windowProvider =windowProvider;       
            
            nodes.push_back(factory.GetNode(Nodes::N_RENDER_NODE));
            RegisterNode(nodes.back(), nodes.size() - 1);

            nodes.push_back(factory.GetNode(Nodes::N_IMAGE_SAMPLER, glm::vec2(0.0), "Img1"));
            RegisterNode(nodes.back(), nodes.size() - 1);


            
        }

        void RegisterNode(Nodes::GraphNode& node, int nodeIndex)
        {
            nodesIds.try_emplace(node.nodeId.Get(), nodeIndex);
            for (auto& input : node.inputNodes)
            {
                nodesIds.try_emplace(input.first, nodeIndex);
            }
            for (auto& output : node.outputNodes)
            {
                nodesIds.try_emplace(output.first, nodeIndex);
            }
        }

        void Draw()
        {
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            for (auto& node : nodes)
            {
                //fix this:
                // ENGINE::AttachmentInfo* d = std::any_cast<ENGINE::AttachmentInfo>(node.data);
                // std::string info = "Data: " + std::to_string(d->attachmentInfo.clearValue.color.float32[0]);
                // SYSTEMS::Logger::GetInstance()->LogMessage(info);
                node.Draw();
            }
            CheckLinks();
            firstFrame = false;
            ed::End();
        }

        void CheckLinks()
        {
            for (auto link : links)
            {
                ed::Link(link.id, link.inputId, link.outputId);
            }

            if (ed::BeginCreate())
            {
                ed::PinId startId, endInd;
                if (ed::QueryNewLink(&startId, &endInd))
                {
                    Nodes::GraphNode* startNode = GetNodeByAnyId(startId.Get());
                    Nodes::GraphNode* endNode = GetNodeByAnyId(endInd.Get());
                    ed::PinKind startPinType;
                    ed::PinKind endPinType;
                    std::map<ed::PinKind, Nodes::GraphNode*> pinNodes;
                    if (startNode && endNode)
                    {
                        Nodes::PinInfo* startPin = startNode->inputNodes.contains(startId.Get())? &startNode->inputNodes.at(startId.Get()) : nullptr;
                        startPinType = ed::PinKind::Input;
                        if (startPin == nullptr)
                        {
                            startPin = startNode->outputNodes.contains(startId.Get()) ? &startNode->outputNodes.at(startId.Get()) : nullptr;
                            startPinType = ed::PinKind::Output;
                        }
                        Nodes::PinInfo* endPin = endNode->inputNodes.contains(endInd.Get()) ? &endNode->inputNodes.at(endInd.Get()) : nullptr;
                        endPinType = ed::PinKind::Input;
                        if (endPin == nullptr)
                        {
                            endPinType = ed::PinKind::Output;
                            endPin = endNode->outputNodes.contains(endInd.Get())? &endNode->outputNodes.at(endInd.Get()) : nullptr;
                        }
                        pinNodes.try_emplace(startPinType, startNode);
                        pinNodes.try_emplace(endPinType, endNode);

                        if ((startPin && endPin) && (pinNodes.size() == 2))
                        {
                            if (startPin->nodeType == endPin->nodeType)
                            {
                                std::any result = pinNodes.at(ed::PinKind::Output)->BuildOutput();
                                Nodes::GraphNode* graphNodeRef = pinNodes.at(ed::PinKind::Input);
                                graphNodeRef->inputData.at(startPin->nodeType) = result;
                                
                                if (ed::AcceptNewItem())
                                {
                                    links.push_back({ed::LinkId(idGen++), startId, endInd});
                                    ed::Link(links.back().id, links.back().inputId, links.back().outputId);
                                }
                            }
                        }
                    }
                    else
                    {
                        return;
                    }
                }
            }
            ed::EndCreate();
            if (ed::BeginDelete())
            {
                ed::LinkId deletedLinkId;
                while (ed::QueryDeletedLink(&deletedLinkId))
                {
                    for (auto& link : links)
                    {
                        if (link.id == deletedLinkId)
                        {
                            links.erase(&link);
                            break;
                        }
                    }
                }
            }
            ed::EndDelete();
        }

        Nodes::GraphNode* GetNodeByAnyId(int id)
        {
            if (!nodesIds.contains(id))
            {
                return nullptr;
            }
            return &nodes.at(nodesIds.at(id));
        }

        ImVector<Nodes::LinkInfo> links;
        Nodes::GraphNodeFactory factory;
        std::vector<Nodes::GraphNode> nodes;
        //int_1 - in/out id || int_2 node idx in nodes vec  
        std::map<int, int> nodesIds;
        int idGen = 200;
        bool firstFrame = true;
        std::any data;
        // std::map<std::string, int> renderNodesEditorsNames;
        // std::vector<RenderNodeEditor*> renderNodeEditors;
        ENGINE::RenderGraph* renderGraph;
        WindowProvider* windowProvider;
    };
}

#endif //RG_NODEEDITOR_HPP
