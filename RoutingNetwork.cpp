/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNetwork.cpp
 */
#include "RoutingNetwork.hpp"
#include "Intersection.hpp"
#include "RoutingLink.hpp"
#include "RoutingNode.hpp"
#include "Section.hpp"
#include <AmuConverter.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using amu::converter::formatId;

//======================================================================
RoutingNetwork::RoutingNetwork()
{
    _nodes.clear();
    _links.clear();
    _upperNetwork    = nullptr;
    _lowerNetwork    = nullptr;
    _highestNodeRank = 0;
}

//======================================================================
RoutingNetwork::~RoutingNetwork()
{
    for (auto itr : _nodes)
    {
        delete itr.second;
    }
    _nodes.clear();

    for (auto itr : _links)
    {
        delete itr;
    }
    _links.clear();
}

//======================================================================
RoutingNode* RoutingNetwork::convertS2N(const Section* sect, bool isUp) const
{
    // IDで検索する
    // Search by ID number
    string id      = sect->id();
    string beginId = id.substr(0, NUM_FIGURE_FOR_INTERSECTION);
    string endId
        = id.substr(NUM_FIGURE_FOR_INTERSECTION, NUM_FIGURE_FOR_INTERSECTION);
    string nodeId;
    if (isUp)
    {
        nodeId = formatId(to_string(_networkRank), NUM_FIGURE_FOR_ROUTING_LAYER)
            + beginId + endId;
    }
    else
    {
        nodeId = formatId(to_string(_networkRank), NUM_FIGURE_FOR_ROUTING_LAYER)
            + endId + beginId;
    }

    auto itr = _nodes.find(nodeId);
    if (itr != _nodes.end())
    {
        return (*itr).second;
    }
    else
    {
        cerr << "ERROR: RoutingNode[" << nodeId << "] not found." << endl;
        cout << " Section:" << sect->id() << endl;
        cout << " isUp:" << isUp << endl;
        cout << " (begin, end): (" << beginId << ", " << endId << ")" << endl;
        return nullptr;
    }
}

//======================================================================
RoutingLink* RoutingNetwork::convertI2L(
    const Intersection* via, const Intersection* from,
    const Intersection* to) const
{
    for (auto itr : _links)
    {
        if (itr->intersection() == via
            && itr->beginNode()->section()->anotherIntersection(via) == from
            && itr->endNode()->section()->anotherIntersection(via) == to)
        {
            return itr;
        }
    }
    cerr << "ERROR: RoutingNetwork::convertI2L(via:" << via->id()
         << ", from:" << from->id() << ", to:" << to->id() << " failed."
         << std::endl;
    return nullptr;
}

//======================================================================
void RoutingNetwork::print() const
{
    cout << "RoutingNetwork(RANK: " << _networkRank << ")" << endl;
    for (auto itr : _nodes)
    {
        itr.second->print(cout);
    }
    for (auto itr : _links)
    {
        itr->print(cout);
    }
    cout << endl;
}
