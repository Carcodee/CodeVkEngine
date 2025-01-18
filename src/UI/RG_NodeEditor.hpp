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
        std::vector<Nodes::GraphNode> nodes;
        void Init()
        {
            if (firstFrame)
            {
                nodes.push_back(Nodes::GraphNode());
                nodes.back().SetName("A");
                nodes.back().SetNodeId(11);
                nodes.back().AddInput(1);
                nodes.back().AddOutput(2);
                
                nodes.push_back(Nodes::GraphNode());
                nodes.back().SetName("B");
                nodes.back().SetNodeId(12);
                nodes.back().AddInput(3);
                nodes.back().AddOutput(4);
            }
            
        }
        void Draw()
        {
            ed::Begin("My Editor", ImVec2(0.0, 0.0f));
            for (auto& node : nodes)
            {
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
                            links.push_back({ed::LinkId(nextLinkId++), inId, outId});
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
        int nextLinkId = 100;
        int inIds = 100;
        int outIds = 100;
        bool firstFrame = true;
        // std::map<std::string, int> renderNodesEditorsNames;
        // std::vector<RenderNodeEditor*> renderNodeEditors;
        
        // ENGINE::RenderGraph* renderGraph;
        
    };
    
}

#endif //RG_NODEEDITOR_HPP
