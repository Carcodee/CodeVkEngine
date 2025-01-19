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
            builder.AddInput(idGen++, "In");
            builder.AddOutput(idGen++, "Out");
            nodes.push_back(builder.Build(&data));
            RegisterNode(nodes.back(), nodes.size() - 1);


            builder.SetNodeId(idGen++, "B");
            builder.AddInput(idGen++, "In");
            builder.AddOutput(idGen++, "Out");
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
                std::string info = "Data: " + std::to_string(d->attachmentInfo.clearValue.color.float32[0]);
                SYSTEMS::Logger::GetInstance()->LogMessage(info);
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
                ed::PinId inId, outId;
                if (ed::QueryNewLink(&inId, &outId))
                {
                    if (inId && outId)
                    {
                        if(ed::AcceptNewItem())
                        {
                            links.push_back({ed::LinkId(idGen++), inId, outId});
                            ed::Link(links.back().id, links.back().inputId, links.back().outputId);
                        }
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
