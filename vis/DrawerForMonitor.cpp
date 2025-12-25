/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForMonitor.cpp
 */
#include "DrawerForMonitor.hpp"
#include "GLColor.hpp"
#include "../Config.hpp"
#include "../TrafficCounterComponent.hpp"
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void DrawerForMonitor::drawTrafficCounter(
    const TrafficCounter& counter) const
{
    for (auto itr : counter.components())
    {
        AmuPoint  p       = itr->position();
        double    zOffset = 0.2;
        double    size    = 1.5;
        AmuVector vec0    = itr->lane()->directionVector();
        vec0.normalize();
        AmuVector vec1 = vec0;
        vec1.revoltXY(M_PI_2);

        // 左三角の描画
        // Draw a left triangle
        GLColor::setTrafficCounter();
        AutoGL_DrawTriangle(
            p.x(), p.y(), p.z(),
            p.x() + vec0.x() * size * 0.5 + vec1.x() * size,
            p.y() + vec0.y() * size * 0.5 + vec1.y() * size,
            p.z() + zOffset,
            p.x() - vec0.x() * size * 0.5 + vec1.x() * size,
            p.y() - vec0.y() * size * 0.5 + vec1.y() * size,
            p.z() + zOffset);

        // 識別番号の描画
        // Draw ID number
        if (itr == *(counter.components().begin()))
        {
            GLColor::setMonitorId();
            AutoGL_DrawString(p.x(), p.y(), 5, counter.id().c_str());
        }
    }
}
