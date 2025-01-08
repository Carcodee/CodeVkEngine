//
// Created by carlo on 2025-01-07.
//

#ifndef RG_NODEEDITOR_HPP
#define RG_NODEEDITOR_HPP

namespace UI
{
    struct RenderNodeEditor
    {

        ENGINE::RenderGraphNode* renderNode;
    };
    
    class RG_NodeEditor
    {

        

        std::map<std::string, int> renderNodesEditorsNames;
        std::vector<RenderNodeEditor*> renderNodeEditors;
        
        ENGINE::RenderGraph* renderGraph;
        
    };
    
}

#endif //RG_NODEEDITOR_HPP
