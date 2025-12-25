/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForIntersection.cpp
 */
#include "DrawerForIntersection.hpp"
#include "DrawerForLane.hpp"
#include "DrawerForSignal.hpp"
#include "DrawerForSubLaneBundle.hpp"
#include "GLColor.hpp"
#include "GuiMapView.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "../ped/LanePedExt.hpp"
#endif //INCLUDE_PEDESTRIANS
#ifdef INCLUDE_TRAMS
#include "../tram/TramLaneInIntersection.hpp"
#endif //INCLUDE_TRAMS
#include "../CSNodeBase.hpp" // [eMATES]
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;
using namespace amu::math;

//#define DIRECTION_DEBUG
#define ZEBRA_DEBUG

//======================================================================
void DrawerForIntersection::draw(const Intersection& inter) const
{
    _drawSubsections(inter);
    _drawLanes(inter);
    _drawConnectors(inter);
    if (inter.signal())
    {
        _drawSignals(inter);
    }
    _drawId(inter);
    // 2025.03.27 by abe [eMATES]
    _drawCSId(inter);
    _drawCSCharge(inter);
}

//======================================================================
void DrawerForIntersection::drawSimple(
    const Intersection& inter, double size) const
{
    GLColor::setSimpleNetwork();
    AutoGL_DrawCircle3D(
        inter.center().x(), inter.center().y(), inter.center().z(), 0, 0, 1,
        size, 3);
    _drawId(inter);
    // 2025.03.27 by abe [eMATES]
    _drawCSId(inter);
    _drawCSCharge(inter);
}

//======================================================================
void DrawerForIntersection::_drawSubsections(const Intersection& inter) const
{
    DrawerForSubLaneBundle drawer;
    for (auto itr : inter.subLaneBundles())
    {
        drawer.draw(*(itr.second));
    }
}

//======================================================================
void DrawerForIntersection::_drawLanes(const Intersection& inter) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_LANE_INTER")))
    {
        return;
    }

    DrawerForLane drawer;
    for (auto itr : inter.lanes())
    {
#ifdef DIRECTION_DEBUG
        if (inter.isRight(itr.second))
        {
            GLColor::setRightLane();
        }
        else if (inter.isLeft(itr.second))
        {
            GLColor::setLeftLane();
        }
        else if (inter.isStraight(itr.second))
        {
            GLColor::setStraightLane();
        }
        else
        {
            GLColor::setLane();
        }
#else  //DIRECTION_DEBUG
        GLColor::setLane();
#endif //DIRECTION_DEBUG

#ifdef INCLUDE_TRAMS
        if (typeid(*(itr.second)) == typeid(TramLaneInIntersection))
        {
            GLColor::setTramLane();
        }
#endif //INCLUDE_TRAMS

#if defined(INCLUDE_PEDESTRIANS) && defined(ZEBRA_DEBUG)
        if (itr.second->pedExt()->hasApproachingVehicles())
        {
            GLColor::setCrosswalkVehicleLane();
        }
        else if (itr.second->pedExt()->hasApproachingPedestrian())
        {
            GLColor::setCrosswalkPedestrianLane();
        }
#endif //INCLUDE_PEDESTRIANS && ZEBRA_DEBUG

        drawer.draw(*(itr.second), 0.1, 0.15);
    }
}

//======================================================================
void DrawerForIntersection::_drawConnectors(const Intersection& inter) const
{
    if ((int)(AppMates::getGVManager().getNumeric("VIS_CONNECTOR_ID_MODE"))
        == GuiMapView::CN_NONE)
    {
        return;
    }

    GLColor::setInterId();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 内部コネクタ
    // Internal connectors
    for (auto itr : inter.internalConnectors())
    {
        switch (
            (int)(AppMates::getGVManager().getNumeric("VIS_CONNECTOR_ID_MODE")))
        {
        case GuiMapView::CN_GLOBAL:
            AutoGL_DrawString(
                itr.second->x(), itr.second->y(), itr.second->z() + 5,
                formatId(
                    to_string(itr.second->idGlobal()),
                    NUM_FIGURE_FOR_CONNECTOR_GLOBAL)
                    .c_str());
            break;

        case GuiMapView::CN_LOCAL:
            AutoGL_DrawString(
                itr.second->x(), itr.second->y(), itr.second->z() + 5,
                (itr.first).c_str());
            break;

        default:
            break;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 境界上のコネクタ
    // Connectors on borders
    for (auto itr : inter.borders())
    {
        for (int i = 0; i < itr->numIn() + itr->numOut(); i++)
        {
            const Connector* con = itr->connector(i);
            string           localId;

            switch ((int)(AppMates::getGVManager().getNumeric(
                "VIS_CONNECTOR_ID_MODE")))
            {
            case GuiMapView::CN_GLOBAL:
                AutoGL_DrawString(
                    con->x(), con->y(), con->z() + 5,
                    formatId(
                        to_string(con->idGlobal()),
                        NUM_FIGURE_FOR_CONNECTOR_GLOBAL)
                        .c_str());
                break;

            case GuiMapView::CN_LOCAL:
                localId = "**"
                    + formatId(
                              to_string(i), NUM_FIGURE_FOR_CONNECTOR_LOCAL - 2);
                AutoGL_DrawString(
                    con->x(), con->y(), con->z() + 5, localId.c_str());
                break;

            default:
                break;
            }
        }
    }
}

//======================================================================
void DrawerForIntersection::_drawSignals(const Intersection& inter) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_SIGNAL")))
    {
        return;
    }

    DrawerForSignal drawer;
    drawer.draw(inter);
}

//======================================================================
void DrawerForIntersection::_drawId(const Intersection& inter) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_INTER_ID")))
    {
        return;
    }

    const AmuPoint center = inter.center();
    GLColor::setInterId();
    AutoGL_DrawString(
        center.x(), center.y(), center.z() + 30, inter.id().c_str());
}

//======================================================================
void DrawerForIntersection::_drawCSId(const Intersection& inter) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_CS_ID")))
    {
        return;
    }
    if (! dynamic_cast<const CSNodeBase*>(&inter))
    {
      return;
    }

    const AmuPoint center = inter.center();
    GLColor::setCSId();
    AutoGL_DrawString(center.x(), center.y(), center.z()+32,
                      inter.id().c_str());
}

//======================================================================
void DrawerForIntersection::_drawCSCharge(const Intersection& inter) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_CS_VALUE")))
    {
        return;
    }
    const CSNodeBase* cs = dynamic_cast<const CSNodeBase*>(&inter);
    if (! cs)
    {
      return;
    }

    double charge = cs->instantaneousCharge();
    std::string strCharge = formatId(std::to_string(static_cast<int>(charge)), 6);
    const AmuPoint center = inter.center();
    GLColor::setCSId();
    AutoGL_DrawString(center.x(), center.y()-5, center.z()+32,
                      strCharge.c_str());
    double radius = charge / 50;
    GLColor::setCSValue();
    AutoGL_DrawCircle3D(center.x(), center.y()-5, center.z()-1,
                        0, 0, 1, radius, 6);
}
