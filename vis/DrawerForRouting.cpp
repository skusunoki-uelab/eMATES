/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file DrawerForRouting.cpp
 */
#include "DrawerForRouting.hpp"
#include "GLColor.hpp"
#include "GuiRoutingTest.hpp"
#include "autogl_mates.h"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;
using amu::converter::formatId;

//==============================================================================
void DrawerForRouting::drawNetwork(const RoutingNetwork& network) const
{
    double linkWidth = 0.002 * AutoGL_GetViewSize();

    // リンクの描画
    // Draw links
    for (auto itr : network.links())
    {
        _drawLink(*itr, linkWidth);
    }

    // ノードの描画
    // Draw nodes
    for (auto itr : network.nodes())
    {
        _drawNode(*(itr.second));
    }
}

//==============================================================================
void DrawerForRouting::_drawLink(const RoutingLink& link, double width) const
{
    AmuPoint bp = link.beginNode()->point();
    AmuPoint ep = link.endNode()->point();
    AmuPoint mp = bp + AmuVector(bp, ep) / 2;

    unsigned int br = link.beginNode()->networkRank();
    unsigned int er = link.endNode()->networkRank();

    GLColor::setRoutingLink();
    AutoGL_DrawBoldArrow(
        bp.x(), bp.y(), bp.z() + _ZMARGIN * (1 + br), ep.x(), ep.y(),
        ep.z() + _ZMARGIN * (1 + er), width);

    int mode = static_cast<int>(
        AppMates::getGVManager().getNumeric("VIS_ROUTING_LINK_PROP_MODE"));
    if (mode == GuiRoutingTest::ROUTELINK_NONE || mode > 8)
    {
        return;
    }
    GLColor::setRoutingLinkString();
    std::ostringstream oss;
    if (mode == GuiRoutingTest::ROUTELINK_ID)
    {
        oss << link.id();
    }
    else if (mode == GuiRoutingTest::ROUTELINK_ALLOW)
    {
        for (auto itr : link.restriction().allowedVehicleTypes())
        {
            oss << itr << " ";
        }
    }
    else if (mode == GuiRoutingTest::ROUTELINK_DENY)
    {
        for (auto itr : link.restriction().deniedVehicleTypes())
        {
            oss << itr << " ";
        }
    }
    else
    {
        oss << link.cost(mode - 2);
    }
    AutoGL_DrawString(mp.x(), mp.y(), mp.z() + _ZMARGIN * 2, oss.str().c_str());
}

//==============================================================================
void DrawerForRouting::_drawNode(const RoutingNode& node) const
{
    int mode = static_cast<int>(
        AppMates::getGVManager().getNumeric("VIS_ROUTING_NODE_PROP_MODE"));
    if (mode == GuiRoutingTest::ROUTENODE_NONE)
    {
        return;
    }

    GLColor::setRoutingNodeString();
    AmuPoint      p = node.point();
    ostringstream oss;
    switch (mode)
    {
    case GuiRoutingTest::ROUTENODE_ID:
        oss << node.id();
        break;
    case GuiRoutingTest::ROUTENODE_RANK:
        oss << node.rank();
        break;
    case GuiRoutingTest::ROUTENODE_ALLOW:
        for (auto itr : node.restriction().allowedVehicleTypes())
        {
            oss << itr << " ";
        }
        break;
    case GuiRoutingTest::ROUTENODE_DENY:
        for (auto itr : node.restriction().deniedVehicleTypes())
        {
            oss << itr << " ";
        }
        break;
    default:
        break;
    }

    AutoGL_DrawString(
        p.x(), p.y(), p.z() + _ZMARGIN * (2 + node.networkRank()),
        oss.str().c_str());
}

//==============================================================================
void DrawerForRouting::drawRecord(
    const RoutingRecorder& recorder, int maxStep) const
{
    double linkWidth = 0.004 * AutoGL_GetViewSize();

    const vector<RouterBase::NodeStatusBase*>&   log = recorder.log();
    const map<const string, const RoutingNode*>& table
        = recorder.nodeStatusId2RoutingNode();
    unsigned int lmax = static_cast<unsigned int>(maxStep);

    for (unsigned int i = 0; i <= lmax; i++)
    {
        if (log[i]->prevId() == "-1")
        {
            continue;
        }
        else if (i >= log.size())
        {
            cout << "end of record." << endl;
            break;
        }

        // リンクの始端
        // Start of link
        RoutingNode* node = log[i]->routingNode();
        AmuPoint     bp   = node->point();
        unsigned int br   = node->networkRank();
        GLColor::setRoutingRecord(br);

        // リンクの終端
        // End of link
        auto itr = table.find(log[i]->prevId());
        assert(itr != table.end());
        AmuPoint     ep = (*itr).second->point();
        unsigned int er = (*itr).second->networkRank();

        AutoGL_DrawBoldArrow(
            bp.x(), bp.y(), bp.z() + _ZMARGIN * (1 + br), ep.x(), ep.y(),
            ep.z() + _ZMARGIN * (1 + er), linkWidth);
    }
}
