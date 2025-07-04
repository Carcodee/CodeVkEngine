﻿//

// Created by carlo on 2025-02-22.
//

#ifndef GRAPHNODERESMANAGERIMPL_HPP
#define GRAPHNODERESMANAGERIMPL_HPP

namespace UI::Nodes
{
GraphNodeResManager::GraphNodeResManager()
{
	graphNodesArena = std::make_unique<Systems::Arena>();
}

void GraphNodeResManager::AddNodeId(int inputOutputId, int graphNodeId)
{
	inputOutputsIds.try_emplace(inputOutputId, graphNodeId);
}

void GraphNodeResManager::AddNodeIds(GraphNode *node)
{
	for (auto &input : node->inputNodes)
	{
		AddNodeId(input.first, node->globalId);
	}
	for (auto &output : node->outputNodes)
	{
		AddNodeId(output.first, node->globalId);
	}
}

GraphNode *GraphNodeResManager::GetNode(int id)
{
	if (graphNodes.contains(id))
	{
		assert(false);
		return graphNodes.at(id);
	}
	GraphNode *graphNode = graphNodesArena->Alloc<GraphNode>();
	graphNodes.try_emplace(id, graphNode);
	graphNode->graphNodesRef       = &graphNodes;
	graphNode->widgetsIdGen        = &widgetsIdGen;
	graphNode->globalId            = id;
	graphNode->graphNodesRef       = &graphNodes;
	graphNode->graphNodeResManager = this;
	return graphNode;
}

GraphNode *GraphNodeResManager::GetNodeByInputOutputId(int id)
{
	if (!inputOutputsIds.contains(id))
	{
		return nullptr;
	}

	return graphNodes.at(inputOutputsIds.at(id));
}

PinInfo *GraphNodeResManager::GetPinFromId(int id)
{
	for (auto &graphNode : graphNodes)
	{
		if (graphNode.second->inputNodes.contains(id))
		{
			return &graphNode.second->inputNodes.at(id);
		}
		if (graphNode.second->outputNodes.contains(id))
		{
			return &graphNode.second->outputNodes.at(id);
		}
	}
	return nullptr;
}
int GraphNodeResManager::NextWidgetID()
{
	return widgetsIdGen++;
}

int GraphNodeResManager::NextNodeID()
{
	return idNodeGen++;
}
}        // namespace UI::Nodes

#endif        // GRAPHNODERESMANAGERIMPL_HPP
