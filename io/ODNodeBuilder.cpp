/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ODNodeBuilder.cpp
 */
#include "ODNodeBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../Section.hpp"
#include "../SubIntersection.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#include "../tram/IntersectionBuilderTramExt.hpp"
#include "../tram/ODNodeBuilderTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
Intersection* ODNodeBuilder::build(
    const std::string& fmId, const std::string& type, RoadMap* roadMap)
{
    _inter  = new ODNode(fmId, type, roadMap);
    _bundle = _inter;

#ifdef INCLUDE_TRAMS
    _builderTramExt = new ODNodeBuilderTramExt(_inter, _roadMapBuilder);
#endif //INCLUDE_TRAMS

    return _inter;
}

//==============================================================================
bool ODNodeBuilder::_generateDefaultRoadwayVertexes()
{
    /*
     * 単路部の単位方向ベクトル (隣接交差点へ向かう方向が正) を求める
     *
     * Find unit direction vector of section (direction towards adjacent
     * intersection is positive)
     */
    Intersection*   target      = _inter->next(0);
    SectionBuilder* incSBuilder = _nextSBuilders[0];
    if (!incSBuilder)
    {
        cerr << "ERROR: ODNode[" << _inter->id()
             << "], SectionBuilder of incSection not found." << endl;
        exit(EXIT_FAILURE);
    }

    AmuPoint  center = _inter->center();
    AmuVector dv(
        target->center().x() - center.x(), target->center().y() - center.y(),
        0);
    dv.normalize();

    // 単位法線ベクトルを求める
    // Find unit normal vector
    AmuVector nv = _internalUnitDirectionVector(0);
    nv.revoltXY(M_PI_2);

    // 頂点を求める
    // Find each vertex
    int numConnectors = _inter->numIn(0) + _inter->numOut(0);

#ifdef INCLUDE_TRAMS
    numConnectors += _inter->tramExt()->numTotalTramLanes(0);
#endif //INCLUDE_TRAMS

    AmuPoint tmpVertex;
    double   tmpWidth = 2.0;
    double   tmpX, tmpY;
    tmpVertex.setZ(center.z());
    for (int i = 0; i < 4; i++)
    {
        int factor1 = (i == 0 || i == 3) ? -1 : 1;
        int factor2 = (i < 2) ? 1 : -1;

        tmpX = center.x()
            + factor1 * nv.x()
                * (numConnectors * 0.5 * incSBuilder->laneWidth()
                   + incSBuilder->roadsideWidth())
            + factor2 * dv.x() * tmpWidth;
        tmpY = center.y()
            + factor1 * nv.y()
                * (numConnectors * 0.5 * incSBuilder->laneWidth()
                   + incSBuilder->roadsideWidth())
            + factor2 * dv.y() * tmpWidth;
        tmpVertex.setX(tmpX);
        tmpVertex.setY(tmpY);
        _roadwayVertexes.push_back(tmpVertex);
    }
    return true;
}

//==============================================================================
bool ODNodeBuilder::_generateVertexes()
{
    // ODNodeでは車道境界の点と車道多角形の頂点が一致する
    // In ODNode, roadway vertexes and polygon vertexes coincide.
    for (unsigned int i = 0; i < _roadwayVertexes.size(); i++)
    {
        _borderPoints.push_back(_roadwayVertexes[i]);
    }

    // 単位法線ベクトル
    // Unit normal vector
    AmuVector nv;
    nv = _internalUnitDirectionVector(0);
    nv.revoltXY(M_PI_2);

    AmuPoint tmpVertex;
    double   tmpX, tmpY;
    tmpVertex.setZ(_inter->center().z());

    // 頂点の決定
    // Determine polygon vertexes
    SectionBuilder* incSBuilder = _nextSBuilders[0];
    for (unsigned int i = 0; i < 4; i++)
    {
        bool isLeft = true;
        int  factor = 1;
        if (i == 0 || i == 3)
        {
            isLeft = false;
            factor = -1;
        }

        tmpX = _roadwayVertexes[i].x()
            + factor * nv.x() * incSBuilder->sidewalkWidth(_inter, isLeft);
        tmpY = _roadwayVertexes[i].y()
            + factor * nv.y() * incSBuilder->sidewalkWidth(_inter, isLeft);
        tmpVertex.setX(tmpX);
        tmpVertex.setY(tmpY);
        _inter->addVertex(tmpVertex);
    }
    return true;
}

//==============================================================================
bool ODNodeBuilder::_generateBorders()
{
    AmuLineSegment line;
    AmuVector      v;

    // edge[0]にborder0を作成する
    // Generate border0 on edge[0]
    Border* border0 = nullptr;

    // edge[2]にborder1を作成する(border0と逆向き)
    // Generate border1 on edge[2] (opposite to border0)
    Border* border1 = nullptr;

    SectionBuilder* incSBuilder = _nextSBuilders[0];

#ifdef INCLUDE_TRAMS
    IntersectionTramExt* tramExt = _inter->tramExt();
    if (tramExt->numTotalTramLanes(0) > 0)
    {
        BorderTram* borderTram0 = new BorderTram(
            _borderPoints[0], _borderPoints[1], incSBuilder->roadsideWidth());
        borderTram0->createConnectors(
            _inter->numIn(0), _inter->numOut(0), tramExt, 0);
        border0                 = borderTram0;
        BorderTram* borderTram1 = new BorderTram(
            _borderPoints[2], _borderPoints[3], incSBuilder->roadsideWidth());
        borderTram1->createConnectorsReverse(
            _inter->numOut(0), _inter->numIn(0), tramExt, 0);
        border1 = borderTram1;
    }
    else
#endif //INCLUDE_TRAMS
    {
        border0 = new Border(
            _borderPoints[0], _borderPoints[1], incSBuilder->roadsideWidth());
        border0->createConnectors(_inter->numIn(0), _inter->numOut(0));
        border1 = new Border(
            _borderPoints[2], _borderPoints[3], incSBuilder->roadsideWidth());
        border1->createConnectors(_inter->numOut(0), _inter->numIn(0));
    }

    assert(border0 && border1);
    _inter->addBorder(border0, 0);
    _inter->addBorder(border1, 2);

    return true;
}

//==============================================================================
bool ODNodeBuilder::_generateSubsections()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 中央サブセクション (ID:00)
    // Central subsection (ID:00)
    string           centralId = "00";
    SubIntersection* ptSubI
        = new SubIntersection(centralId, SubsectionType::Roadway);
    ptSubI->setParent(_inter);

    for (unsigned int i = 0; i < _roadwayVertexes.size(); i++)
    {
        ptSubI->addVertex(_roadwayVertexes[i]);
    }
    ptSubI->setCenter();
    _inter->addSubLaneBundle(centralId, ptSubI);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 歩道サブセクション
    // Sidewalk subsection
    SectionBuilder* nextSBuilder = _nextSBuilders[0];
    if (nextSBuilder->sidewalkWidth(_inter, true) > 0)
    {
        string           sideId = "11";
        SubIntersection* ptSubI1
            = new SubIntersection(sideId, SubsectionType::Sidewalk);
        ptSubI1->setParent(_inter);
        ptSubI1->addVertex(_borderPoints[1]);
        ptSubI1->addVertex(_inter->vertex(1));
        ptSubI1->addVertex(_inter->vertex(2));
        ptSubI1->addVertex(_borderPoints[2]);
        ptSubI1->setCenter();
        _inter->addSubLaneBundle(sideId, ptSubI1);
    }
    if (nextSBuilder->sidewalkWidth(_inter, false) > 0)
    {
        string           sideId = "13";
        SubIntersection* ptSubI3
            = new SubIntersection(sideId, SubsectionType::Sidewalk);
        ptSubI3->setParent(_inter);
        ptSubI3->addVertex(_borderPoints[3]);
        ptSubI3->addVertex(_inter->vertex(3));
        ptSubI3->addVertex(_inter->vertex(0));
        ptSubI3->addVertex(_borderPoints[0]);
        ptSubI3->setCenter();
        _inter->addSubLaneBundle(sideId, ptSubI3);
    }

    return true;
}

//==============================================================================
bool ODNodeBuilder::_generateLanes()
{
    /*
     * 向かい合ったコネクタ同士を結ぶ
     *   左側通行でも右側通行でも向かい合ったコネクタIDの対応は不変
     *
     * Connect connectors facing each other
     *   The correspondence between facing connector IDs does not change
     *   for left-hand or right-hand traffic.
     */
    int numIn  = _inter->numIn(0);
    int numOut = _inter->numOut(0);

    // 境界0のコネクタから境界1のコネクタへ
    // From border 0 to border 1
    for (int j = 0; j < numIn; j++)
    {
        int idIntBegin = j;
        int idIntEnd   = 1 * 100 + (numOut + numIn - 1 - j);
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->border(0)->connector(j);
        const Connector* pointEnd
            = _inter->border(1)->connector(numOut + numIn - 1 - j);
        if (pointBegin && pointEnd)
        {
            _generateLane(idInt, pointBegin, pointEnd);
        }
    }
    // 境界1のコネクタから境界0のコネクタへ
    // From border 1 to border 0
    for (int j = 0; j < numOut; j++)
    {
        int idIntBegin = 1 * 100 + j;
        int idIntEnd   = numOut + numIn - 1 - j;
        int idInt      = idIntBegin * 10000 + idIntEnd;

        const Connector* pointBegin = _inter->border(1)->connector(j);
        const Connector* pointEnd
            = _inter->border(0)->connector(numOut + numIn - 1 - j);
        if (pointBegin && pointEnd)
        {
            _generateLane(idInt, pointBegin, pointEnd);
        }
    }

#ifdef INCLUDE_TRAMS
    _builderTramExt->generateTramLanes();
#endif //INCLUDE_TRAMS
    return true;
}
