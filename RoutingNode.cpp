/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingNode.cpp
 */
#include "RoutingNode.hpp"
#include "Config.hpp"
#include "Intersection.hpp"
#include "RoutingLink.hpp"
#include "Section.hpp"
#include <AmuConverter.hpp>
#include <AmuLineSegment.hpp>
#include <AmuVector.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using amu::converter::formatId;

//======================================================================
RoutingNode::RoutingNode(const Section* section, bool isUp)
    : _section(section), _isUp(isUp)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * _coordは可視化上の位置を示すのみで，経路探索には用いられない．
     * 点が重ならないよう，isUpの真偽に応じて位置をずらす．
     *
     * The _coord only indicates the position on the visualization and
     * is not used for routing. Shift the position according to true or
     * false of isUp so that the points do not overlap.
     */
    _coord = section->center();

    AmuVector vec = AmuVector(
        section->intersection(false)->center(),
        section->intersection(true)->center());
    vec.normalize();

    if (isUp)
    {
        vec.revoltXY(M_PI_2);
    }
    else
    {
        vec.revoltXY(-M_PI_2);
    }

    _coord += vec * 1.5;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    _rank = max(section->upWidth(), section->downWidth());

    _subId = section->intersection(!isUp)->id()
        + section->intersection(isUp)->id();

    // default value
    _networkRank = 0;

    _id = formatId(to_string(_networkRank), NUM_FIGURE_FOR_ROUTING_LAYER)
        + _subId;

    _inlinks.clear();
    _outlinks.clear();
    _upwardLink   = nullptr;
    _downwardLink = nullptr;

    _upperNode  = nullptr;
    _lowerNode  = nullptr;
    _lowestNode = nullptr;
}

//======================================================================
RoutingNode::~RoutingNode() {}

//======================================================================
RoutingLink* RoutingNode::inLink(const RoutingNode* prev) const
{
    for (auto itr : _inlinks)
    {
        if (itr->beginNode() == prev)
        {
            return itr;
        }
    }
    return nullptr;
}

//======================================================================
void RoutingNode::getInLinks(
    vector<const RoutingLink*>& result_links, unsigned int rank) const
{
    for (auto itr : _inlinks)
    {
        if (itr->beginNode()->rank() >= rank)
        {
            result_links.emplace_back(itr);
        }
    }
}

//======================================================================
void RoutingNode::getInNodes(
    vector<const RoutingNode*>& result_nodes, unsigned int rank) const
{
    for (auto itr : _inlinks)
    {
        if (itr->beginNode()->rank() >= rank)
        {
            result_nodes.emplace_back(itr->beginNode());
        }
    }
}

//======================================================================
const RoutingNode* RoutingNode::straightInNode() const
{
    const RoutingNode* result = nullptr;

    // 初期値．angleは-PIからPIまで．
    // Default value. The angle is from -PI to PI.
    double minAngle = 4;

    const AmuVector outvec(
        _lowestNode->section()->intersection(!_isUp)->center(),
        _lowestNode->section()->intersection(_isUp)->center());

    for (auto itr : _inlinks)
    {
        const AmuVector invec(
            itr->beginNode()->point(), itr->endNode()->point());
        double angle = outvec.calcAngle(invec);
        if (fabs(angle) < minAngle)
        {
            minAngle = angle;
            result   = itr->beginNode();
        }
    }
    return result;
}

//======================================================================
int RoutingNode::indegree(unsigned int rank) const
{
    int indeg = 0;
    for (auto itr : _inlinks)
    {
        if (itr->beginNode()->rank() >= rank)
        {
            indeg++;
        }
    }
    return indeg;
}

//======================================================================
int RoutingNode::indegreeInSameNetworkRank() const
{
    int indeg = 0;
    for (auto itr : _inlinks)
    {
        if (itr->beginNode()->networkRank() == _networkRank)
        {
            indeg++;
        }
    }
    return indeg;
}

//======================================================================
RoutingLink* RoutingNode::outLink(const RoutingNode* next) const
{
    for (auto itr : _outlinks)
    {
        if (itr->endNode() == next)
        {
            return itr;
        }
    }
    return nullptr;
}

//======================================================================
void RoutingNode::getOutLinks(
    vector<const RoutingLink*>& result_links, unsigned int rank) const
{
    for (auto itr : _outlinks)
    {
        if (itr->endNode()->rank() >= rank)
        {
            result_links.emplace_back(itr);
        }
    }
}

//======================================================================
void RoutingNode::getOutNodes(
    vector<const RoutingNode*>& result_nodes, unsigned int rank) const
{
    for (auto itr : _outlinks)
    {
        if (itr->endNode()->rank() >= rank)
        {
            result_nodes.emplace_back(itr->endNode());
        }
    }
}

//======================================================================
const RoutingNode* RoutingNode::straightOutNode() const
{
    const RoutingNode* result = nullptr;

    // 初期値．angleは-PIからPIまで．
    // Default value. The angle is from -PI to PI.
    double minAngle = 4;

    const AmuVector invec(
        _lowestNode->section()->intersection(!_isUp)->center(),
        _lowestNode->section()->intersection(_isUp)->center());

    for (auto itr : _outlinks)
    {
        const AmuVector outvec(
            itr->beginNode()->point(), itr->endNode()->point());
        double angle = invec.calcAngle(outvec);
        if (fabs(angle) < minAngle)
        {
            minAngle = angle;
            result   = itr->endNode();
        }
    }

    return result;
}

//======================================================================
int RoutingNode::outdegree(unsigned int rank) const
{
    int outdeg = 0;
    for (auto itr : _outlinks)
    {
        if (itr->endNode()->rank() >= rank)
        {
            outdeg++;
        }
    }
    return outdeg;
}

//======================================================================
int RoutingNode::outdegreeInSameNetworkRank() const
{
    int outdeg = 0;
    for (auto itr : _outlinks)
    {
        if (itr->endNode()->networkRank() == _networkRank)
        {
            outdeg++;
        }
    }
    return outdeg;
}

//======================================================================
void RoutingNode::setProperty()
{
    /*
    for (auto itr : _section->allowedVehicleTypes(_isUp))
    {
        _permission.addAllowedVehicleType(itr);
    }
    for (auto itr : _section->deniedVehicleTypes(_isUp))
    {
        _permission.addDeniedVehicleType(itr);
    }
    */
    _restriction.addPermission(_section->restriction(_isUp));

    for (auto itr : _section->routingProbabilities(_isUp))
    {
        _probability.addProbability(itr.first, itr.second);
    }
}

//======================================================================
void RoutingNode::print(ostream&) const {}
