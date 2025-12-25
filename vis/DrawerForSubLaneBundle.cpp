/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForSubLaneBundle.cpp
 */
#include "DrawerForSubLaneBundle.hpp"
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
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//======================================================================
void DrawerForSubLaneBundle::draw(const SubLaneBundle& subsec) const
{
    bool includesPedestrians = false;
#ifdef INCLUDE_PEDESTRIANS
    includesPedestrians = true;
#endif

    if (!includesPedestrians
        || AppMates::getGVManager().getFlag("VIS_SUBSECTION_SHAPE"))
    {
        _drawShape(subsec);
    }
    if (includesPedestrians
        && AppMates::getGVManager().getFlag("VIS_SUBNETWORK"))
    {
        _drawSubnetwork(subsec, 0.2, 0.15);
    }
    if ((subsec.type() == SubsectionType::Crosswalk
         && AppMates::getGVManager().getFlag("VIS_SIGNAL")))
    {
        _drawSignals(subsec);
    }
    if (includesPedestrians
        && AppMates::getGVManager().getFlag("VIS_SUBSECTION_ID"))
    {
        _drawId(subsec);
    }
}

//======================================================================
void DrawerForSubLaneBundle::_drawShape(
    const SubLaneBundle& subsec) const
{
    // 属性により色分けする
    // Color by the type of the subsection
    SubsectionType type = subsec.type();
    switch (type)
    {
    case SubsectionType::Roadway:
        GLColor::setSubsection();
        break;
    case SubsectionType::Sidewalk:
        GLColor::setSideWalk();
        break;
    case SubsectionType::Crosswalk:
        GLColor::setCrossWalk();
        break;
    default:
        GLColor::setSubsection();
    }

    /**
     * @todo
     * attributeは今後実装 (vehicleが通行権のあるエリアのみ，
     * pedestrianが通行権のあるエリアのみ，がいいのでは？）
     */

    /*
     * 形状が非凸でも描けるように中心点を使う．ただしこれは最適な
     * アルゴリズムではない．
     *
     * Use the center point so that the shape can be drawn even if
     * it is non-convex. However, this is not the optimal algorithm.
     */
    AmuPoint p0 = subsec.center();
    AmuPoint p1, p2;
    p1 = subsec.vertex(subsec.numVertexes() - 1);
    for (auto itr : subsec.vertexes())
    {
        p2 = itr;
        AutoGL_DrawTriangle(
            p0.x(), p0.y(), p0.z(), p1.x(), p1.y(), p1.z(), p2.x(),
            p2.y(), p2.z());
        p1 = p2;
    }
}

//======================================================================
void DrawerForSubLaneBundle::_drawSubnetwork(
    const SubLaneBundle& subsec, double zMargin, double width) const
{
    GLColor::setSubnetwork();

    // サブセクションの中心点
    // Center point of the subsection
    AutoGL_DrawCircle3D(
        subsec.center().x(), subsec.center().y(),
        subsec.center().z() + zMargin, 0, 0, 1, width * 4, 3);

    for (int i = 0; i < subsec.numVertexes(); i++)
    {
        /*
         * このサブセクションの中心点から隣接するサブセクションの
         * 中心点への線分
         *
         * Line segment from the center point of this subsection to
         * the center point of the adjacent subsection.
         */
        if (subsec.adjSubLaneBundle(i))
        {
            GLColor::setSubnetwork();
            AutoGL_DrawBoldLine2D(
                subsec.center().x(), subsec.center().y(),
                subsec.center().z() + zMargin * 2,
                subsec.adjSubLaneBundle(i)->center().x(),
                subsec.adjSubLaneBundle(i)->center().y(),
                subsec.adjSubLaneBundle(i)->center().z() + zMargin * 2,
                width);
        }
        /*
         * 隣接するサブセクションがない場合はこのサブセクションの境界を
         * 着色する
         *
         * Color the border of this subsection if there is no adjacent
         * subsection.
         */
        else
        {
            GLColor::setSubsectionEdge();
            AutoGL_DrawBoldLine2D(
                subsec.edge(i).pointBegin().x(),
                subsec.edge(i).pointBegin().y(),
                subsec.edge(i).pointBegin().z() + zMargin,
                subsec.edge(i).pointEnd().x(),
                subsec.edge(i).pointEnd().y(),
                subsec.edge(i).pointEnd().z() + zMargin, width);
        }
    }
}

//======================================================================
void DrawerForSubLaneBundle::_drawSignals(
    const SubLaneBundle& subsec) const
{
    for (int i = 0; i < subsec.numVertexes(); i++)
    {
        // 歩道との境界に描画する
        // Draw at the border with the sidewalk
        if (subsec.adjSubLaneBundle(i)
            && subsec.adjSubLaneBundle(i)->type()
                   == SubsectionType::Sidewalk)
        {
            AmuPoint point  = subsec.edge(i).createInteriorPoint(1, 1);
            Signal*  signal = subsec.signal();
            int      dir    = subsec.signalDirection();

            if (!signal)
            {
                continue;
            }
            if (signal->walkerColor(dir)
                == SignalColor::WalkerState::BLUE)
            {
                GLColor::setBlueSignal();
            }
            else if (
                signal->walkerColor(dir)
                == SignalColor::WalkerState::RED)
            {
                GLColor::setRedSignal();
            }
            else if (
                signal->walkerColor(dir)
                == SignalColor::WalkerState::YELLOW)
            {
                GLColor::setYellowSignal();
            }
            else
            {
                AutoGL_SetColor(0, 0, 0);
            }
            AutoGL_DrawQuadrangle(
                point.x() - 0.4, point.y(), point.z() + 4.0, point.x(),
                point.y() - 0.4, point.z() + 4.0, point.x() + 0.4,
                point.y(), point.z() + 4.0, point.x(), point.y() + 0.4,
                point.z() + 4.0);
        }
    }
}

//======================================================================
void DrawerForSubLaneBundle::_drawId(const SubLaneBundle& subsec) const
{
    GLColor::setSubsectionId();
    const AmuPoint center = subsec.center();
    AutoGL_DrawString(
        center.x(), center.y(), center.z() + 5, subsec.id().c_str());
}
