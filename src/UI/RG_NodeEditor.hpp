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

            std::vector<Nodes::NodeType> nodesList = {
               // Nodes::N_RENDER_NODE,
               // Nodes::N_VERT_SHADER,
               // Nodes::N_FRAG_SHADER,
               // Nodes::N_COMP_SHADER,
               // Nodes::N_COL_ATTACHMENT_STRUCTURE,
               // Nodes::N_DEPTH_STRUCTURE,
               // Nodes::N_RASTER_STRUCTURE,
               // Nodes::N_PUSH_CONSTANT,
               // Nodes::N_IMAGE_SAMPLER,
               // Nodes::N_IMAGE_STORAGE,
               // Nodes::N_DEPTH_IMAGE_SAMPLER,
               Nodes::N_VERTEX_INPUT,
                // N_BUFFER
            };
            //test
            for (const auto& nodeType : nodesList)
            {
                nodes.push_back(factory.GetNode(nodeType));
                RegisterNode(nodes.back(), nodes.size() - 1);    
            }
            
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
                ed::PinId startId, endId;
                if (ed::QueryNewLink(&startId, &endId))
                {
                    Nodes::GraphNode* startNode = GetNodeByAnyId(startId.Get());
                    Nodes::GraphNode* endNode = GetNodeByAnyId(endId.Get());
                    ed::PinKind startPinType;
                    ed::PinKind endPinType;
                    std::map<ed::PinKind, Nodes::GraphNode*> pinNodes;
                    std::map<ed::PinKind, int> pinIds;
                    if (startNode && endNode)
                    {
                        Nodes::PinInfo* startPin = startNode->inputNodes.contains(startId.Get())? &startNode->inputNodes.at(startId.Get()) : nullptr;
                        startPinType = ed::PinKind::Input;
                        if (startPin == nullptr)
                        {
                            startPin = startNode->outputNodes.contains(startId.Get()) ? &startNode->outputNodes.at(startId.Get()) : nullptr;
                            startPinType = ed::PinKind::Output;
                        }
                        Nodes::PinInfo* endPin = endNode->inputNodes.contains(endId.Get()) ? &endNode->inputNodes.at(endId.Get()) : nullptr;
                        endPinType = ed::PinKind::Input;
                        if (endPin == nullptr)
                        {
                            endPinType = ed::PinKind::Output;
                            endPin = endNode->outputNodes.contains(endId.Get())? &endNode->outputNodes.at(endId.Get()) : nullptr;
                        }
                        pinNodes.try_emplace(startPinType, startNode);
                        pinNodes.try_emplace(endPinType, endNode);
                        
                        pinIds.try_emplace(startPinType, startId.Get());
                        pinIds.try_emplace(endPinType, endId.Get());

                        if ((startPin && endPin) && (pinNodes.size() == 2))
                        {
                            if (startPin->nodeType == endPin->nodeType)
                            {
                                pinNodes.at(ed::PinKind::Output)->BuildOutput();
                                Nodes::GraphNode* outputGraphNodeRef = pinNodes.at(ed::PinKind::Output);
                                Nodes::GraphNode* inputGraphNodeRef = pinNodes.at(ed::PinKind::Input);
                                *inputGraphNodeRef->GetInputDataById(pinIds.at(ed::PinKind::Input)) = outputGraphNodeRef->GetOutputDataById(pinIds.at(ed::PinKind::Output));
                                
                                if (ed::AcceptNewItem())
                                {
                                    links.push_back({ed::LinkId(idGen++), startId, endId});
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
