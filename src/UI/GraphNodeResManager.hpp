﻿//
// Created by carlo on 2025-02-22.
//

#ifndef GRAPHNODERESMANAGER_HPP
#define GRAPHNODERESMANAGER_HPP

namespace UI::Nodes
{
struct GraphNode;
struct PinInfo;
struct GraphNodeResManager
{
	std::unique_ptr<Systems::Arena> graphNodesArena;

	std::unordered_map<int, int> inputOutputsIds;

	std::unordered_map<int, GraphNode *> graphNodes;

	int widgetsIdGen = 100;
	int idNodeGen    = 100;

	GraphNodeResManager();
	void       AddNodeId(int inputOutputId, int graphNodeId);
	void       AddNodeIds(GraphNode *node);
	GraphNode *GetNode(int id);
	GraphNode *GetNodeByInputOutputId(int id);
	PinInfo   *GetPinFromId(int id);
	int        NextWidgetID();
	int        NextNodeID();
};
}        // namespace UI::Nodes

#endif        // GRAPHNODERESMANAGER_HPP
