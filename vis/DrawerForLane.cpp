/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForLane.cpp
 */
#include "DrawerForLane.hpp"
#include "GLColor.hpp"
#include "autogl_mates.h"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include <AmuPoint.hpp>
#include <AmuLinePoly.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void DrawerForLane::draw(
    const Lane& lane, double zMargin, double width) const
{
    assert(lane.beginConnector() && lane.endConnector());
    _drawLine(*(lane.lineSegment()), zMargin, width, true);

    if (!(AppMates::getGVManager().getFlag("VIS_LANE_ID")))
    {
        return;
    }

    // 識別番号の描画
    // Draw ID number
    GLColor::setLaneId();
    double x
        = (lane.beginConnector()->x() + lane.endConnector()->x()) * 0.5;
    double y
        = (lane.beginConnector()->y() + lane.endConnector()->y()) * 0.5;
    double z
        = (lane.beginConnector()->z() + lane.endConnector()->z()) * 0.5;
    AutoGL_DrawString(x, y, z + zMargin * 2, lane.id().c_str());
}

//======================================================================
void DrawerForLane::_drawLine(
    const AmuLineSegment& line, double zMargin, double width,
    bool isArrow) const
{
    AmuLinePoly* polyLine = dynamic_cast<AmuLinePoly*>(
        const_cast<AmuLineSegment*>(&line));
    if (polyLine)
    {
        // 内部線分の描画
        // Draw internal line segments
        auto lines = *(polyLine->internalLines());
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            AmuLineSegment& line = lines[i];
            if (i == lines.size() - 1)
            {
                _drawLine(line, zMargin, width, true);
            }
            else
            {
                _drawLine(line, zMargin, width, false);
            }
        }
    }
    else
    {
        // 単独の線分の描画
        // Draw a single line segment
        if (isArrow)
        {
            AutoGL_DrawBoldArrow2D(
                line.pointBegin().x(), line.pointBegin().y(),
                line.pointBegin().z() + zMargin, line.pointEnd().x(),
                line.pointEnd().y(), line.pointEnd().z() + zMargin,
                width);
        }
        else
        {
            AutoGL_DrawBoldLine2D(
                line.pointBegin().x(), line.pointBegin().y(),
                line.pointBegin().z() + zMargin, line.pointEnd().x(),
                line.pointEnd().y(), line.pointEnd().z() + zMargin,
                width);
        }
    }
}

