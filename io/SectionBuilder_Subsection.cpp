/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilder_Subsection.cpp
 */
#include "SectionBuilder.hpp"
#include "../Intersection.hpp"
#include "../SubSection.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>

using namespace std;
using namespace amu::geometry;

//======================================================================
bool SectionBuilder::_generateSubsections()
{
    /*
     * 現在では直線単路に限られる
     *   辺番号0と辺番号2が交差点に接するようにする．
     *
     * Currently, limited to a straight section.
     *   Make sides 0 and 2 touch face incident intersection.
     */

    // 中央 (車道) サブセクション
    // Central (roadway) subsection
    _generateCentralSubsection();

    // 歩道サブセクション
    // Sidewalk subsection
    _generateSidewalkSubsection();

    return true;
}

//======================================================================
void SectionBuilder::_generateCentralSubsection()
{
    SubSection* ptSubS = new SubSection("00", SubsectionType::Roadway);
    ptSubS->setParent(_section);

    // サブセクションの形状の設定
    // Set subsection shape
    Intersection*         inter0 = _section->intersection(0);
    Intersection*         inter1 = _section->intersection(1);
    const AmuLineSegment& border0
        = inter0->border(inter0->direction(inter1))->lineSegment();
    const AmuLineSegment& border1
        = inter1->border(inter1->direction(inter0))->lineSegment();

    ptSubS->addVertex(border0.pointBegin());
    ptSubS->addVertex(border0.pointEnd());
    ptSubS->addVertex(border1.pointBegin());
    ptSubS->addVertex(border1.pointEnd());
    ptSubS->setCenter();
    _section->addSubLaneBundle(ptSubS->id(), ptSubS);
}

//======================================================================
void SectionBuilder::_generateSidewalkSubsection()
{
    Intersection*         inter0 = _section->intersection(0);
    Intersection*         inter1 = _section->intersection(1);
    const AmuLineSegment& border0
        = inter0->border(inter0->direction(inter1))->lineSegment();
    const AmuLineSegment& border1
        = inter1->border(inter1->direction(inter0))->lineSegment();
    AmuLineSegment edge0 = inter0->edgeToNextInter(inter1);
    AmuLineSegment edge1 = inter1->edgeToNextInter(inter0);

    if (_section->sidewalkWidth(inter0, false) > 1e-6)
    {
        // 歩道1 (右側)
        // Sidewalk 1 (right side)
        SubSection* ptSubS
            = new SubSection("10", SubsectionType::Sidewalk);
        ptSubS->setParent(_section);
        ptSubS->addVertex(edge0.pointBegin());
        ptSubS->addVertex(border0.pointBegin());
        ptSubS->addVertex(border1.pointEnd());
        ptSubS->addVertex(edge1.pointEnd());
        ptSubS->setCenter();
        _section->addSubLaneBundle(ptSubS->id(), ptSubS);
    }

    if (_section->sidewalkWidth(inter0, true) != 0)
    {
        // 歩道2 (左側)
        // Sidewalk 2 (left side)
        SubSection* ptSubS
            = new SubSection("20", SubsectionType::Sidewalk);
        ptSubS->setParent(_section);
        ptSubS->addVertex(border0.pointEnd());
        ptSubS->addVertex(edge0.pointEnd());
        ptSubS->addVertex(edge1.pointBegin());
        ptSubS->addVertex(border1.pointBegin());
        ptSubS->setCenter();
        _section->addSubLaneBundle(ptSubS->id(), ptSubS);
    }
}
