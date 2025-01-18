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

        
        std::vector<Nodes::LinkInfo> links;

        void ChekLinks()
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
                        links.push_back({ed::LinkId(nextLinkId++), inId, outId});
                        ed::Link(links.back().id, links.back().inputId, links.back().outputId);
                    }
                }
            }
        }
        int nextLinkId = 100;
        int inIds = 100;
        int outIds = 100;
        // std::map<std::string, int> renderNodesEditorsNames;
        // std::vector<RenderNodeEditor*> renderNodeEditors;
        
        // ENGINE::RenderGraph* renderGraph;
        
    };
    
}

#endif //RG_NODEEDITOR_HPP
