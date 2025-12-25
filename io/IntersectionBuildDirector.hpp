/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuildDirector.hpp
 */
#ifndef __INTERSECTION_BUILD_DIRECTOR_HPP__
#define __INTERSECTION_BUILD_DIRECTOR_HPP__
#include "LaneBundleBuildDirector.hpp"
#include "../RoadMap.hpp"
#include <fstream>
#include <unordered_map>
#include <string>
#include <unordered_map>

class RoadMapBuilder;

//######################################################################
/**
 * @~japanese 交差点の生成を指示する
 * @~english  Direct generation of intersections
 * @~
 * @ingroup Initialization IO RoadNetwork
 * @see LaneBundleBuildDirector
 */
class IntersectionBuildDirector : public LaneBundleBuildDirector
{
public:
    IntersectionBuildDirector(RoadMap* roadMap, RoadMapBuilder* builder)
        : LaneBundleBuildDirector(roadMap, builder) {};
    virtual ~IntersectionBuildDirector() {};

    //==================================================================
    /**
     * @~japanese
     * ファイルを読み込んで交差点を生成し基本設定を行う
     *
     * @~english
     * Load file to generate intersections and set essential property
     */
    bool buildIntersections();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese
     * @name buildIntersections()で呼ばれるprivate関数
     *
     * @~english
     * @name Private functions called in buildIntersections()
     */
    ///@{
private:
    /**
     * @~japanese CSリストを生成する [eMATES]
     * @~english  Generate CS List
     */
    bool _generateCSList();

    /**
     * @~japanese 交差点を生成する
     * @~english  Generate intersections
     */
    bool _generateIntersections();

    /**
     * @~japanese 隣接する交差点を設定する
     * @~english  Set adjacent intersections
     */
    bool _setAdjacentIntersections();

    /**
     * @~japanese
     * 交差点の代表点(中心点)の座標を設定する
     *
     * @~english
     * Set coordinates of the representative point (center point)
     * of intersection
     */
    bool _setPositions();

    // CSの行き止まりを判定し、CSインスタンスに登録する [eMATES]
    bool _setCSDeadend();

    ///@}

    //==================================================================
public:
    /**
     * @~japanese 接続する単路部を設定する
     * @~english  Set incident sections
     */
    bool setIncidentSections();

    //==================================================================
    /**
     * @~japanese 交差点の内部構造に関する情報を設定する
     *
     * IntersectionBuilderに情報を格納するのみで，交差点の設定は
     * まだ行わない
     *
     * @~english  Set internal structure information of intersections
     *
     * Just store information in IntersectionBuilder, not configure
     * intersections yet
     */
    virtual bool setInternalInfo();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese
     * @name setInternalInfo()で呼ばれるprivate関数
     *
     * @~english
     * @name Private functions called in setInternalInfo()
     */
    ///@{
protected:
    /**
     * @~japanese 交差点の歩道幅を設定する
     * @~english  Set intersection sidewalk width
     */
    bool _setWalkWidth();

    /**
     * @~japanese
     * 交差点の相対方向，幾何形状 (頂点座標)，Lane接続を設定する
     *
     * @~english
     * Set relative direction, geometry (vertex coordinates),
     * and lane connections of intersections
     */
    bool _setInternalStructure(bool readsFile);
    ///@}

    //==================================================================
protected:
    /**
     * @~japanese CSリスト [eMATES]
     * CSの識別番号: [CSの収容台数, CSの定格]
     * @~english  CS List
     * CS-ID: [CS-capacity, CS-rating-power]
     */
    struct CSListVal {
      int capacity;
      double ratingPower;
    };
    std::unordered_map<std::string, CSListVal> _csList {};
};

#endif //__INTERSECTION_BUILD_DIRECTOR_HPP__
