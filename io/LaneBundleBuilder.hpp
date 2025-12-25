/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneBundleBuilder.hpp
 */
#ifndef __LANE_BUNDLE_BUILDER_HPP__
#define __LANE_BUNDLE_BUILDER_HPP__
#include "../LaneBundle.hpp"
#include <string>
#include <unordered_map>
#include <vector>

class RoadMapBuilder;

//##############################################################################
/**
 * @~japanese レーン束を生成する
 *
 * @note SectionBuilder と IntersectionBuilder の基底クラス

 * @~english  Generate lane bundles
 *
 * @note Base class for SectionBuilderr and IntersectionBuilder
 *
 * @~ @ingroup Initialization IO RoadNetwork
 */
class LaneBundleBuilder
{
public:
    LaneBundleBuilder(RoadMapBuilder* roadMapBuilder)
        : _bundle(nullptr), _roadMapBuilder(roadMapBuilder) {};
    virtual ~LaneBundleBuilder() {};

    /**
     * @~japanese 内部構造を作成する
     * @~english  Create internal structure
     */
    virtual bool createInternalStructure() = 0;

    /**
     * @~japanese @name レーン同士の関係を設定するための関数
     * @~english  @name Functions to set relationships between lanes
     */
    ///@{
public:
    /**
     * @~japanese レーンの接続関係を設定する
     * @note 単路の場合は左右のレーンも設定する
     *
     * @~english  Set lane connections
     * @note If in section, also set left and right lanes
     */
    virtual bool setLaneConnection();

    /**
     * @~japanese レーンをサブセクションに割り当てる
     * @~english  Assign lanes to subsections
     */
    virtual bool assignLanesToSubLaneBundles();

protected:
    /**
     * @~japanese レーン @p lane の下流レーンの集合を求める
     * @~english  Find set of downstream lanes for lane @p lane
     */
    bool _decideNextLanes(Lane* lane);

    /**
     * @~japanese レーン @p lane の上流レーンの集合を求める
     * @~english  Find set of upstream lanes for lane @p lane
     */
    bool _decidePrevLanes(Lane* lane);

public:
    /**
     * @~japanese レーンの交錯関係を設定する
     * @note 現在はIntersectionBuilderでのみ処理する
     *
     * @~english  Set lane crossing
     * @note Currently only processed by IntersectionBuilder
     */
    virtual bool setLaneCollision();

    ///@}

public:
    /**
     * @~japanese サブネットワークを作成する
     * @~english  Create subnetwork
     */
    bool createSubnetwork();

protected:
    /**
     * @~japanese 処理対象のレーン束オブジェクト
     * @~english  Lane bundle object to be processed
     */
    LaneBundle* _bundle;

    /**
     * @~japanese 地図のbuilderオブジェクト
     * @~english  Builder object for roadMap
     */
    RoadMapBuilder* _roadMapBuilder;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    LaneBundle* laneBundle() const
    {
        return _bundle;
    }

    ///@}
};

#endif //__LANE_BUNDLE_BUILDER_HPP__
