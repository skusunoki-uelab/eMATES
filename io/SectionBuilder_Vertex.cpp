/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilder_Vertex.cpp
 */
#include "SectionBuilder.hpp"
#include "../Intersection.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>

using namespace std;
using namespace amu::geometry;

//======================================================================
bool SectionBuilder::_generateVertexes()
{
    double x = 0.0, y = 0.0, z = 0.0;
    for (int i = 0; i < 2; i++)
    {
        // 接続する交差点の境界の属性を用いる
        // Use border property of incident intersection
        Intersection*  nextInter = _section->intersection(i);
        AmuLineSegment edge      = nextInter->edgeToNextInter(
            _section->anotherIntersection(nextInter));

        _section->addVertex(edge.pointBegin());
        x += edge.pointBegin().x();
        y += edge.pointBegin().y();
        z += edge.pointBegin().z();

        _section->addVertex(edge.pointEnd());
        x += edge.pointEnd().x();
        y += edge.pointEnd().y();
        z += edge.pointEnd().z();
    }
    _section->setCenter(AmuPoint(
        x / _section->numVertexes(), y / _section->numVertexes(),
        z / _section->numVertexes()));
    _section->calcLength();

    return true;
}
