/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file DrawerForSection.cpp
 */
#include "DrawerForSection.hpp"
#include "DrawerForLane.hpp"
#include "DrawerForSubLaneBundle.hpp"
#include "GLColor.hpp"
#include "GuiMapView.hpp"
#include "autogl_mates.h"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/TramLaneInSection.hpp"
#endif //INCLUDE_TRAMS
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

//==============================================================================
void DrawerForSection::draw(const Section& section) const
{
    _drawSubsections(section);
    _drawLanes(section);
    _drawConnectors(section);
}

//==============================================================================
void DrawerForSection::drawSimple(const Section& section, double size) const
{
    GLColor::setSimpleNetwork();
    AmuPoint bp = section.intersection(true)->center();
    AmuPoint ep = section.intersection(false)->center();

    AutoGL_DrawBoldLine2D(bp.x(), bp.y(), bp.z(), ep.x(), ep.y(), ep.z(), size);
}

//==============================================================================
void DrawerForSection::_drawSubsections(const Section& section) const
{
    DrawerForSubLaneBundle drawer;
    for (auto itr : section.subLaneBundles())
    {
        drawer.draw(*(itr.second));
    }
}

//==============================================================================
void DrawerForSection::_drawLanes(const Section& section) const
{
    if (!(AppMates::getGVManager().getFlag("VIS_LANE_SECTION")))
    {
        return;
    }

    DrawerForLane drawer;
    for (auto itr : section.lanes())
    {
#ifdef DIRECTION_DEBUG
        if (section.isUp(itr.second))
            GLColor::setUpLane();
        else
            GLColor::setDownLane();
#else
        GLColor::setLane();
#endif //DIREC_DEBUG

#ifdef INCLUDE_TRAMS
        if (typeid(*(itr.second)) == typeid(TramLaneInSection))
        {
            GLColor::setTramLane();
        }
#endif //INCLUDE_TRAMS

        drawer.draw(*(itr.second), 0.1, 0.15);
    }
}

//==============================================================================
void DrawerForSection::_drawConnectors(const Section& section) const
{
    if ((int)(AppMates::getGVManager().getNumeric("VIS_CONNECTOR_ID_MODE"))
        == GuiMapView::CN_NONE)
    {
        return;
    }

    GLColor::setInterId();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 内部コネクタ
    // Sectionnal connectors
    for (auto itr : section.internalConnectors())
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
}
