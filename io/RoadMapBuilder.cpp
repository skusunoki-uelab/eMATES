/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
  * @file RoadMapBuilder.cpp
  */
#include "RoadMapBuilder.hpp"
#include "IntersectionBuildDirector.hpp"
#include "IntersectionPropertyReader.hpp"
#include "SectionBuildDirector.hpp"
#include "SectionPropertyReader.hpp"
#include "SignalBuilder.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/TramRouteManager.hpp"
#include "../tram/TramRoute.hpp"
#endif //INCLUDE_TRAMS
#include <iostream>
#include <cassert>

using namespace std;

//======================================================================
RoadMapBuilder::~RoadMapBuilder()
{
    for (auto itr : _iBuilders)
    {
        delete itr.second;
    }
    _iBuilders.clear();
    for (auto itr : _sBuilders)
    {
        delete itr.second;
    }
    _sBuilders.clear();
}

//======================================================================
RoadMap* RoadMapBuilder::buildRoadMap()
{
    _roadMap = new RoadMap();

    IntersectionBuildDirector iDirector(_roadMap, this);
    SectionBuildDirector      sDirector(_roadMap, this);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 手順
     * 1. 交差点の生成，隣接交差点の設定，座標の設定
     * 2. 単路部の生成，接続交差点の設定
     *     - 単路部の_adjInterはここでセットする
     *     - 交差点の生成後でなければならない
     * 3. 交差点から単路への接続関係の設定
     *     - 交差点の_incSectionsはここでセットする
     *     - 単路部の生成後でなければならない
     *
     * Procedure
     * 1. Build intersections (generate, set adjacent intersections,
     *    set coordinates)
     * 2. Build sections (generate, set incident intersections)
     *      - Set _adjInters in each section here.
     *      - This must be after generation of Intersections
     * 3. Set incident relationship from intersection to section
     *      - Set _incSections in each intersection here.
     *      - This must be after generation of sections.
     */
    if (!(iDirector.buildIntersections() && sDirector.buildSections()
          && iDirector.setIncidentSections()))
    {
        cerr << "ERROR: buildRoadmap failed(" << __LINE__ << ")." << endl;
        delete _roadMap;
        exit(EXIT_FAILURE);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef INCLUDE_TRAMS
    /*
     * 路面電車の路線を定義する
     *   交差点および単路部の作成後で，かつそれらの内部構造の
     *   設定前でなければならない
     *
     * Define tramlines
     *   This must be after generation of intersections and sections,
     *   and before setting their internal structure.
     */
    TramRouteManager& tramRouteManager = AppMates::getTramRouteManager();

    tramRouteManager.setRoadMap(_roadMap);
    tramRouteManager.readTramRouteFile();
#endif //INCLUDE_TRAMS

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 手順
     * 4.  単路部の内部構造を決定する
     * 5.  交差点の内部構造を決定する(4に依存)
     * 6a. 交差点の内部構造を生成する(5に依存)
     * 6b. 単路部の内部構造を生成する(4に依存)
     * 7a. 交差点内のレーンの接続関係を設定する(6aに依存)
     * 7b. 単路部内のレーンの接続関係を設定する(6bに依存)
     * 8.  交差点内の交錯レーンを求める(6a, 6bに依存)
     * 9a. 交差点内のサブネットワークを作成する(6aに依存)
     * 9b. 単路部内のサブネットワークを作成する(6bに依存)
     * 10a. 交差点内のサブセクションにレーンを割り当てる(6aに依存)
     * 10b. 単路部内のサブセクションにレーンを割り当てる(6bに依存)
     *
     * Procedure
     * 4.  Define section internal structure
     * 5.  Define intersection internal structure (depends on 4)
     * 6a. Create intersection internal structure (depends on 5)
     * 6b. Create section internal structure (depends on 4)
     * 7a. Set connection for lanes in intersection (depends on 6a)
     * 7b. Set connection for lanes in section (depends on 6a)
     * 8.  Obtain collision lanes in intersection (depends on 6a and 6b)
     * 9a. Creates sub-networks in intersection (depends on 6a)
     * 9b. Creates sub-networks in section (depends on 6b)
     * 10a. Assign lanes to subsections in intersection (depends on 6a)
     * 10b. Assign lanes to subsections in section (depends on 6b)
     */
    if (!(sDirector.setInternalInfo() && iDirector.setInternalInfo()
          && iDirector.createInternalStructure()
          && sDirector.createInternalStructure()
          && iDirector.setLaneConnection() && sDirector.setLaneConnection()
          && iDirector.setLaneCollision() && iDirector.createSubnetwork()
          && sDirector.createSubnetwork()
          && iDirector.assignLanesToSubLaneBundles()
          && sDirector.assignLanesToSubLaneBundles()))
    {
        cerr << "ERROR: buildRoadmap failed(" << __LINE__ << ")." << endl;
        delete _roadMap;
        exit(EXIT_FAILURE);
    }

    return _roadMap;
}

//==============================================================================
void RoadMapBuilder::addIntersectionBuilder(
    const string& key, IntersectionBuilder* builder)
{
    if (key != builder->intersection()->id())
    {
        cerr << "ERROR: IntersectionBuilder[" << key
             << "] is inconsistent with Intersection["
             << builder->intersection()->id() << "]" << endl;
        exit(EXIT_FAILURE);
    }

    if (_iBuilders.find(key) != _iBuilders.end())
    {
        cerr << "WARNING: IntersectionBuilder[" << key << "] is duplicated."
             << endl;
        return;
    }
    _iBuilders.insert(make_pair(key, builder));
}

//==============================================================================
void RoadMapBuilder::addSectionBuilder(
    const string& key, SectionBuilder* builder)
{
    if (key != builder->section()->id())
    {
        cerr << "ERROR: SectionBuilder[" << key
             << ") is inconsistent with Section[" << builder->section()->id()
             << "]" << endl;
        exit(EXIT_FAILURE);
    }

    if (_sBuilders.find(key) != _sBuilders.end())
    {
        cerr << "WARNING: SectionBuilder[" << key << "] is duplicated." << endl;
        return;
    }
    _sBuilders.insert(make_pair(key, builder));
}

//=============================================================================
bool RoadMapBuilder::buildSignals()
{
    SignalBuilder builder;
    return builder.buildSignals(_roadMap);
}

//==============================================================================
bool RoadMapBuilder::setIntersectionProperty()
{
    IntersectionPropertyReader reader(_roadMap);
    return reader.setTrafficControl();
}

//==============================================================================
bool RoadMapBuilder::setSectionProperty()
{
    SectionPropertyReader reader(_roadMap);
    return (
        reader.setSpeedLimit() && reader.setTrafficControl()
        && reader.setRoutingProbability());
}
