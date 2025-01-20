//
// Created by carlo on 2025-01-07.
//















#ifndef RG_NODEEDITOR_HPP
#define RG_NODEEDITOR_HPP

namespace UI
{
    namespace ed = ax::NodeEditor;
    struct RenderNodeEditor
    {

        ENGINE::RenderGraphNode* renderNode;
    };
    
    class RG_NodeEditor
    {

    public:
        ImVector<Nodes::LinkInfo> links;
        Nodes::GraphNodeBuilder builder;
        
        std::vector<Nodes::GraphNode<std::any>> nodes;
        //int_1 - in/out id || int_2 node idx in nodes vec  
        std::map<int, int> nodesIds;

        template <typename T>
        void RegisterNode(Nodes::GraphNode<T>& node, int nodeIndex)
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
        
        void Init()
        {
            if (!firstFrame){return;}

            data = ENGINE::GetColorAttachmentInfo(glm::vec4(1.0f, 0.1f, 0.1f, 1.0f));
            builder.SetNodeId(idGen++, "A");
            builder.AddInput(idGen++, {"In", Nodes::NodeType::SHADER});
            builder.AddOutput(idGen++,{"ColAttachment", Nodes::NodeType::COL_ATTACHMENT});
            nodes.push_back(builder.Build(&data));
            RegisterNode(nodes.back(), nodes.size() - 1);


            builder.SetNodeId(idGen++, "B");
            builder.AddInput(idGen++, {"ColAttachment", Nodes::NodeType::COL_ATTACHMENT});
            builder.AddOutput(idGen++, {"Out", Nodes::NodeType::SHADER});
            nodes.push_back(builder.Build(&data));
            RegisterNode(nodes.back(), nodes.size() - 1);
        }
        void Draw()
        {
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            for (auto& node : nodes)
            {
                //fix this:
                ENGINE::AttachmentInfo* d = std::any_cast<ENGINE::AttachmentInfo>(node.data);
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

                    Nodes::GraphNode<std::any>* startNode = GetNodeByAnyId(startId.Get());
                    Nodes::GraphNode<std::any>* endNode = GetNodeByAnyId(endInd.Get());
                    if (startNode && endNode)
                    {
                        Nodes::PinInfo* startPin =  startNode->outputNodes.contains(startId.Get()) ? &startNode->outputNodes.at(startId.Get()) : nullptr;
                        if (startPin == nullptr)
                        {
                            startPin = startNode->inputNodes.contains(startId.Get())? &startNode->inputNodes.at(startId.Get()): nullptr; 
                        }
                        Nodes::PinInfo* endPin = endNode->inputNodes.contains(endInd.Get()) ? &endNode->inputNodes.at(endInd.Get()) : nullptr;
                        if (endPin == nullptr)
                        {
                            endPin = endNode->outputNodes.contains(endInd.Get()) ? &endNode->outputNodes.at(endInd.Get()) : nullptr;
                        }
                        
                        if (startPin && endPin)
                        {
                            if (startPin->nodeType == endPin->nodeType)
                            {
                                if (ed::AcceptNewItem())
                                {
                                    links.push_back({ed::LinkId(idGen++), startId, endInd});
                                    ed::Link(links.back().id, links.back().inputId, links.back().outputId);
                                }
                            }    
                        }
                        
                    }else
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
        Nodes::GraphNode<std::any>* GetNodeByAnyId(int id)
        {
            if (!nodesIds.contains(id))
            {
                return nullptr;
            }
            return &nodes.at(nodesIds.at(id));
        }
        int idGen = 100;
        bool firstFrame = true;
        std::any data;
        std::any data2;
        // std::map<std::string, int> renderNodesEditorsNames;
        // std::vector<RenderNodeEditor*> renderNodeEditors;
        
        // ENGINE::RenderGraph* renderGraph;
        
    };
    
}

#endif //RG_NODEEDITOR_HPP
