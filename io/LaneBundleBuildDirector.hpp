/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file LaneBundleBuildDirector.hpp
 */
#ifndef __LANE_BUNDLE_BUILD_DIRECTOR_HPP__
#define __LANE_BUNDLE_BUILD_DIRECTOR_HPP__
#include "LaneBundleBuilder.hpp"
#include "../RoadMap.hpp"
#include <string>
#include <unordered_map>

class RoadMapBuilder;

//######################################################################
/**
 * @~japanese レーン束の生成を指示する
 *
 * @note
 * SectionBuildDirector と IntersectionBuildDirector の基底クラス．
 * 単路部・交差点の作成から内部構造情報の読み込みまでは個別に実装する．
 *
 * @~english  Direct generation of lane bundles
 *
 * @note
 * Base class for SectionBuildDirector and IntersectionBuildDirector.
 * From the generation of sections and intersections to the reading of
 * internal structure information, each implementation is done
 * separately.
 *
 * @~
 * @ingroup Initialization IO RoadNetwork
 */
class LaneBundleBuildDirector
{
public:
    LaneBundleBuildDirector(RoadMap* roadMap, RoadMapBuilder* builder)
        : _roadMap(roadMap), _roadMapBuilder(builder)
    {
        _builders.clear();
    }
    virtual ~LaneBundleBuildDirector()
    {
        _builders.clear();
    };

    /**
     * @~japanese 内部構造を作成する
     * @~english  Create internal structure
     */
    bool createInternalStructure();

    /**
     * @~japanese レーンの接続を設定する
     * @~english  Configure lane connections
     */
    bool setLaneConnection();

    /**
     * @~japanese レーンの交錯関係を設定する
     * @~english  Configure lane crossings
     */
    bool setLaneCollision();

    /**
     * @~japanese サブネットワークを作成する
     * @~english  Create subnetwork
     */
    bool createSubnetwork();

    /**
     * @~japanese レーンをサブセクションに割り当てる
     * @~english  Assign lanes to subsections
     */
    bool assignLanesToSubLaneBundles();

protected:
    /**
     * @~japanese 生成対象のレーン束が含まれる地図
     * @~english  Roadmap containing lane bundles to be generated
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 地図のbuilderオブジェクト
     * @~english  Builder object for roadMap
     */
    RoadMapBuilder* _roadMapBuilder;

    /**
     * @~japanese レーン束のbuilderオブジェクト
     * 
     * サブコンテナであるので，デストラクタでの delete は不要．
     * 
     * @~english  Builder object for lane bundles
     * 
     * Since it is a subcontainer, there is no need to delete in the destructor.
     */
    std::vector<LaneBundleBuilder*> _builders;
};

#endif //__LANE_BUNDLE_BUILD_DIRECTOR_HPP__
