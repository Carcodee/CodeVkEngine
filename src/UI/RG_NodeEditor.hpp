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
	void SetStyle()
	{
	}
	void Init(ENGINE::RenderGraph *renderGraph, WindowProvider *windowProvider)
	{
		if (!firstFrame)
		{
			return;
		}
		this->renderGraph            = renderGraph;
		this->windowProvider         = windowProvider;
		this->factory.renderGraph    = renderGraph;
		this->factory.windowProvider = windowProvider;
		this->factory.resManager     = &resManager;

		std::vector<Nodes::NodeType> nodesList = {
		    Nodes::N_ROOT_NODE,
		    Nodes::N_RENDER_NODE,
		    Nodes::N_RENDER_NODE,
		    Nodes::N_RENDER_NODE,
		    // Nodes::N_VERT_SHADER,
		    // Nodes::N_FRAG_SHADER,
		    // Nodes::N_COMP_SHADER,
		    // Nodes::N_COL_ATTACHMENT_STRUCTURE,
		    // Nodes::N_DEPTH_STRUCTURE,
		    // Nodes::N_PUSH_CONSTANT,
		    // Nodes::N_IMAGE_SAMPLER,
		    // Nodes::N_IMAGE_STORAGE,
		    // Nodes::N_DEPTH_IMAGE_SAMPLER,
		    // Nodes::N_VERTEX_INPUT,
		    // N_BUFFER
		};
		// test
		for (const auto &nodeType : nodesList)
		{
			factory.GetNode(nodeType, glm::vec2(10.0, 0.0));
		}
	}

	void Draw()
	{
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		for (auto &node : resManager.graphNodes)
		{
			node.second->Draw();
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
				Nodes::GraphNode *startNode = resManager.GetNodeByInputOutputId(startId.Get());
				Nodes::GraphNode *endNode   = resManager.GetNodeByInputOutputId(endId.Get());

				if (startNode && endNode)
				{
					ed::PinKind                               startPinType;
					ed::PinKind                               endPinType;
					std::map<ed::PinKind, Nodes::GraphNode *> pinNodes;
					std::map<ed::PinKind, int>                pinIds;

					Nodes::PinInfo *startPin = startNode->inputNodes.contains(startId.Get()) ? &startNode->inputNodes.at(startId.Get()) : nullptr;
					startPinType             = ed::PinKind::Input;
					if (startPin == nullptr)
					{
						startPin     = startNode->outputNodes.contains(startId.Get()) ? &startNode->outputNodes.at(startId.Get()) : nullptr;
						startPinType = ed::PinKind::Output;
					}

					Nodes::PinInfo *endPin = endNode->inputNodes.contains(endId.Get()) ? &endNode->inputNodes.at(endId.Get()) : nullptr;
					endPinType             = ed::PinKind::Input;
					if (endPin == nullptr)
					{
						endPinType = ed::PinKind::Output;
						endPin     = endNode->outputNodes.contains(endId.Get()) ? &endNode->outputNodes.at(endId.Get()) : nullptr;
					}

					pinNodes.try_emplace(startPinType, startNode);
					pinNodes.try_emplace(endPinType, endNode);

					pinIds.try_emplace(startPinType, startId.Get());
					pinIds.try_emplace(endPinType, endId.Get());

					if ((startPin && endPin) && (pinNodes.size() == 2))
					{
						bool validLink = Nodes::IsValidNodeCombination(startPin->nodeType, endPin->nodeType);

						if (validLink)
						{
							Nodes::GraphNode *outputNode = pinNodes.at(ed::PinKind::Output);
							Nodes::GraphNode *inputNode  = pinNodes.at(ed::PinKind::Input);

							if (ed::AcceptNewItem())
							{
								outputNode->RunCallback("output_c");
								Nodes::PinInfo *outputPin = &outputNode->outputNodes.at(pinIds.at(ed::PinKind::Output));
								Nodes::PinInfo *inputPin  = &inputNode->inputNodes.at(pinIds.at(ed::PinKind::Input));
								outputPin->linkedPin      = inputPin;
								inputPin->linkedPin       = outputPin;

								inputNode->AddLink(outputNode->globalId, outputNode->outputNodes.at(pinIds.at(ed::PinKind::Output)).nodeType);
								outputNode->AddLink(inputNode->globalId, inputNode->inputNodes.at(pinIds.at(ed::PinKind::Input)).nodeType);
								inputNode->GetInputDataById(pinIds.at(ed::PinKind::Input))->data =
								    outputNode->GetOutputDataById(pinIds.at(ed::PinKind::Output))->data;

								inputNode->RunCallback("link_c");
								outputNode->RunCallback("link_c");

								links.push_back({ed::LinkId(linkIdGen++), startId, endId});
								ed::Link(links.back().id, links.back().inputId, links.back().outputId);
							}
						}
					}
				}
				else
				{
					SYSTEMS::Logger::GetInstance()->LogMessage("Hello");
				}
			}
		}
		ed::EndCreate();
		if (ed::BeginDelete())
		{
			ed::LinkId deletedLinkId;
			while (ed::QueryDeletedLink(&deletedLinkId))
			{
				for (auto &link : links)
				{
					if (link.id == deletedLinkId)
					{
						Nodes::PinInfo *output = resManager.GetPinFromId(link.outputId.Get());
						Nodes::PinInfo *input  = resManager.GetPinFromId(link.inputId.Get());
						output->linkedPin      = nullptr;
						input->linkedPin       = nullptr;

						Nodes::GraphNode *inputNode  = resManager.GetNodeByInputOutputId(input->id);
						Nodes::GraphNode *outputNode = resManager.GetNodeByInputOutputId(output->id);
						// do some callback;
						inputNode->RunCallback("unlink_c");
						outputNode->RunCallback("unlink_c");

						links.erase(&link);
						break;
					}
				}
			}
		}
		ed::EndDelete();
	}

	ImVector<Nodes::LinkInfo>  links;
	Nodes::GraphNodeFactory    factory;
	Nodes::GraphNodeResManager resManager;
	// int_1 - in/out id || int_2 node idx in nodes vec
	std::map<int, int>   nodesIds;
	int                  linkIdGen  = 200;
	bool                 firstFrame = true;
	ENGINE::RenderGraph *renderGraph;
	WindowProvider      *windowProvider;
};
}        // namespace UI

#endif        // RG_NODEEDITOR_HPP
