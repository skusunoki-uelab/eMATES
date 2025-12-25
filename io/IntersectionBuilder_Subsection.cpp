/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionBuilder_Subsection.cpp
 */
#include "IntersectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../Intersection.hpp"
#include "../Section.hpp"
#include "../SubIntersection.hpp"
#include "../SubLaneBundle.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/BorderTram.hpp"
#include "../tram/IntersectionTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuConverter.hpp>
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <cassert>
#include <iostream>

using namespace std;
using namespace amu::geometry;
using namespace amu::converter;

//=============================================================================
bool IntersectionBuilder::_generateSubsections()
{
    /*
     * 極座標系でr方向に内側から0,1,2,…，theta方向に辺0から左回りに
     * 0,1,2,…と番号づけ，(r,theta)の組でIDを表す
     *
     * In polar coordinate system, numbered 0,1,2,... from the inside
     * in the r direction, 0,1,2,... counterclockwise from side 0 in the
     * theta direction, and the ID is represented by a pair of (r,theta)
     */

    // 中央サブセクション (ID:00)
    // Central subsection (ID:00)
    _generateCentralSubsection();

    // 周辺サブセクション (ID:10-)
    // Peripheral subsections (ID: 10-)
    int r = 1;

    const vector<AmuPoint>& vertexes = _inter->vertexes();
    for (unsigned int t = 0; t < vertexes.size(); t++)
    {
        if (t % 2 == 0)
        {
            // 横断歩道サブセクション
            // Crosswalk subsection
            _generateCrosswalkSubsection(r, t);
        }
        else
        {
            // 歩道サブセクション
            // Sidewalk subsection
            _generateSidewalkSubsection(r, t);
        }
    }
    return true;
}

//=============================================================================
void IntersectionBuilder::_generateCentralSubsection()
{
    string           id     = "00";
    SubIntersection* ptSubI = new SubIntersection(id, SubsectionType::Roadway);
    ptSubI->setParent(_inter);

    for (auto itr : _roadwayVertexes)
    {
        ptSubI->addVertex(itr);
    }
    ptSubI->setCenter();
    _inter->addSubLaneBundle(id, ptSubI);
}

//=============================================================================
void IntersectionBuilder::_generateCrosswalkSubsection(int r, int t)
{
    // 辺番号から境界番号を決定する
    // Find border number from edge number
    int e2d = _inter->edge2dir(t);
    assert(e2d != -1);

    // 横断歩道幅がゼロの場合は横断歩道を生成しない
    // Not generate crosswalks if crosswalk width is zero
    if (_inter->crosswalkWidth(e2d) < 1e-6)
    {
        return;
    }

    string id = formatId(to_string(r * 10 + t), NUM_FIGURE_FOR_SUBSECTION);

#ifdef INCLUDE_PEDESTRIANS
    Zebra*           zebra  = new Zebra(id);
    SubIntersection* ptSubI = zebra;
    const_cast<IntersectionPedExt*>(_inter->pedExt())->addZebra(zebra);
#else  //INCLUDE_PEDESTRIANS not defined
    SubIntersection* ptSubI
        = new SubIntersection(id, SubsectionType::Crosswalk);
#endif //INCLUDE_PEDESTRIANS

    ptSubI->setParent(_inter);

    // _border[e2d]の始点から左回りの四角形
    // Counterclockwise rectangle from the start of _border[e2d]
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * edge0 単路部 Section
     * edge1 横断歩道終点 Crosswalk ending point
     * edge2 中央サブセクション Central subsection
     * edge3:横断歩道起点 Crosswalk starting point
     * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    ptSubI->addVertex(_borderPoints[t]);
    ptSubI->addVertex(_borderPoints[t + 1]);
    ptSubI->addVertex(_roadwayVertexes[t + 1]);
    ptSubI->addVertex(_roadwayVertexes[t]);
    ptSubI->setCenter();
    _inter->addSubLaneBundle(id, ptSubI);

#ifdef INCLUDE_PEDESTRIANS
    zebra->initialize(e2d);
#endif //INCLUDE_PEDESTRIANS
}

//=============================================================================
void IntersectionBuilder::_generateSidewalkSubsection(int r, int t)
{
    int numVertexes = _inter->numVertexes();
    int e2d[2];
    e2d[0] = _inter->edge2dir((t + numVertexes - 1) % numVertexes);
    e2d[1] = _inter->edge2dir((t + 1) % numVertexes);
    assert(e2d[0] != -1 && e2d[1] != -1);

    // 横断歩道幅がゼロの場合は横断歩道を生成しない
    // Not generate crosswalks if crosswalk width is zero
    if (_nextSBuilders[e2d[0]]->sidewalkWidth(_inter, true) < 1e-6
        && _nextSBuilders[e2d[1]]->sidewalkWidth(_inter, false) < 1e-6)
    {
        return;
    }

    string id = formatId(to_string(r * 10 + t), NUM_FIGURE_FOR_SUBSECTION);
    SubIntersection* ptSubI = new SubIntersection(id, SubsectionType::Sidewalk);
    ptSubI->setParent(_inter);

    /*
     * _border[e2d[0]]の終点から左回りの6角形
     *   T字路の場合には見た目は四角形になるが，内部的には6辺を持つ
     *
     * Counterclockwise hexagon from end of _border[e2d[0]]
     *   In the case of a T-junction, it looks like a rectangle,
     *   but internally it has 6 sides.
     */
    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     * edge0 始点側単路の歩道   Sidewalk of start-side section
     * edge1 接続なし           No connection
     * edge2 終点側単路の歩道   Sidewalk of end-side section
     * edge3 終点側横断歩道     End-side crosswalk
     * edge4 中央サブセクション Central subsection
     * edge5 始点側横断歩道     Start-side crosswalk
     *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    int                     tNext    = (t + 1) % numVertexes;
    const vector<AmuPoint>& vertexes = _inter->vertexes();
    if (_nextSBuilders[e2d[0]]->sidewalkWidth(_inter, true) > 1e-6)
    {
        ptSubI->addVertex(_borderPoints[t]);
    }
    ptSubI->addVertex(vertexes[t]);
    ptSubI->addVertex(vertexes[tNext]);
    if (_nextSBuilders[e2d[1]]->sidewalkWidth(_inter, false) > 1e-6)
    {
        ptSubI->addVertex(_borderPoints[tNext]);
    }
    if (_inter->crosswalkWidth(e2d[1]) > 1e-6)
    {
        ptSubI->addVertex(_roadwayVertexes[tNext]);
    }
    if (_inter->crosswalkWidth(e2d[0]) > 1e-6)
    {
        ptSubI->addVertex(_roadwayVertexes[t]);
    }
    ptSubI->setCenter();
    _inter->addSubLaneBundle(id, ptSubI);
}
