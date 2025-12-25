/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilder.cpp
 */
#include "SectionBuilder.hpp"
#include "RoadMapBuilder.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"

using namespace std;

//======================================================================
SectionBuilder::SectionBuilder(RoadMapBuilder* roadMapBuilder)
    : LaneBundleBuilder(roadMapBuilder)
{
    _section = nullptr;

    // デフォルト値であり上書きされうる
    // Default values, may be overwritten.
    _laneWidth = AppMates::getGVManager().getNumeric("DEFAULT_LANE_WIDTH");
    _branchLaneLength[0] = _branchLaneLength[1]
        = AppMates::getGVManager().getNumeric("RIGHT_TURN_LANE_LENGTH");

    _roadsideWidth
        = AppMates::getGVManager().getNumeric("DEFAULT_ROADSIDE_WIDTH");

#ifdef INCLUDE_TRAMS
    _builderTramExt = nullptr;
#endif //INCLUDE_TRAMS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 自動で設置される歩道幅と通行権の設定
    // Set sidewalk width automatically installed and right-of-way
    int numLanesSidewalkAutoSet = (int)AppMates::getGVManager().getNumeric(
        "AUTO_SIDEWALK_SECTION_LANE");

    if (numLanesSidewalkAutoSet < 0)
    {
        /*
         * 歩道なし，歩行者は車道を歩けない
         * No sidewalks, pedestrians cannot walk on roadway
         */
        _sidewalkWidth[0] = _sidewalkWidth[1] = 0.0;
    }
    else if (
        _section->numIn(0) + _section->numOut(0) < numLanesSidewalkAutoSet
        && _section->numIn(1) + _section->numOut(1) < numLanesSidewalkAutoSet)
    {
        /*
         * デフォルトで歩道なし，歩行者は車道を歩ける
         * No sidewalks, pedestrians CAN walk on roadway
         */
        _sidewalkWidth[0] = _sidewalkWidth[1] = 0.0;
    }
    else
    {
        /*
         * 歩道あり，歩行者は車道を歩けない
         * Install sidewalk, pedestrians cannot walk on roadway
         */
        _sidewalkWidth[0]
            = AppMates::getGVManager().getNumeric("DEFAULT_SIDEWALK_WIDTH");
        _sidewalkWidth[1]
            = AppMates::getGVManager().getNumeric("DEFAULT_SIDEWALK_WIDTH");
    }
}

//======================================================================
SectionBuilder::~SectionBuilder()
{
#ifdef INCLUDE_TRAMS
    delete _builderTramExt;
#endif //INCLUDE_TRAMS
}

//======================================================================
Section* SectionBuilder::build(
    const string& id, Intersection* begin, Intersection* end, RoadMap* roadMap)
{
    _section = new Section(id, begin, end, roadMap);
    _bundle  = _section;

#ifdef INCLUDE_TRAMS
    _builderTramExt = new SectionBuilderTramExt(_section);
#endif //INCLUDE_TRAMS

    return _section;
}

//======================================================================
double SectionBuilder::sidewalkWidth(Intersection* inter, bool leftSide) const
{
    Intersection* adjInter[2];
    adjInter[0] = _section->intersection(0);
    adjInter[1] = _section->intersection(1);
    assert(inter == adjInter[0] || inter == adjInter[1]);

    if ((inter == adjInter[0] && leftSide == true)
        || (inter == adjInter[1] && leftSide == false))
    {
        /*
         * _adjInter[0]から見て左側 = _adjInter[1]から見て右側
         *
         * Left side viewed from _adjInter[0]
         * = Right side viewed from _adjInter[1]
         */
        return _sidewalkWidth[1];
    }
    else
    {
        /*
         * _adjInter[0]から見て右側 = _adjInter[1]から見て左側
         *
         * Right side viewed from _adjInter[0]
         * = Left side viewed from _adjInter[1]
         */
        return _sidewalkWidth[0];
    }
}

//======================================================================
bool SectionBuilder::createInternalStructure()
{
    /*
     * 手順
     * 1. レーン幅および歩道幅の設定
     * 2. 頂点配列の生成
     * 3. サブセクションの生成
     * 4. レーンの生成 (内部コネクタの生成を含む)
     *
     * Procedure
     * 1. Set Lane and sidewalk width
     * 2. Generate vertex list
     * 3. Generate subsections
     * 4. Generation lanes (including internal connector generation) 
     */
    return (
        _setWidth() && _generateVertexes() && _generateSubsections()
        && _generateLanes());
}

//======================================================================
bool SectionBuilder::_setWidth()
{
    // レーン幅・歩道幅を_sectionに設定
    // Set lane and sidewalk width to _section
    _section->setLaneWidth(_laneWidth);
    _section->setRoadsideWidth(_roadsideWidth);
    _section->setSidewalkWidth(false, _sidewalkWidth[0]);
    _section->setSidewalkWidth(true, _sidewalkWidth[1]);

    return true;
}
