/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilder.hpp
 */
#ifndef __SECTION_BUILDER_HPP__
#define __SECTION_BUILDER_HPP__
#include "LaneBundleBuilder.hpp"
#include "../Connector.hpp"
#include "../Intersection.hpp"
#include "../Lane.hpp"
#include "../Section.hpp"
#include "../SubLaneBundle.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/SectionBuilderTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <string>
#include <vector>

//######################################################################
/**
 * @~japanese 単路部を生成し内部構造を作成する
 *
 * @note
 * Sectionクラスのインスタンスと1対1に対応する．
 * SectionBuildDirectorがコンテナを管理し，RoadMap生成完了後に
 * ただちにdeleteする
 *
 * @~english  Generate an section
 *
 * @note
 * One-to-one correspondence with instances of Section class.
 * SectionBuildDirector manages the container and deletes builder
 * instance immediately after the RoadMap generation is completed.
 *
 * @~
 * @ingroup Initialization IO RoadNetwork
 * @see LaneBundleBuilder
 */
class SectionBuilder : public LaneBundleBuilder
{
public:
    SectionBuilder(RoadMapBuilder* roadMapBuilder);
    ~SectionBuilder();

    //==================================================================
    /**
     * @~japanese Sectionクラスのインスタンスを生成して返す
     * @~english  Generate and return an instance of section class 
     */
    Section* build(
        const std::string& id, Intersection* begin, Intersection* end,
        RoadMap* _roadMap);

    /**
     * @~japanese
     * 交差点 @p inter から見て左あるいは右側の歩道の幅を戻す
     *
     * @param isLeft 左側かどうか
     *
     * @~english
     * Return left or right sidewalk width viewed from intersection
     * @p inter
     *
     * @param isLeft Whether it is on the left side
     */
    double sidewalkWidth(Intersection* inter, bool isLeft) const;

    //==================================================================
public:
    /**
     * @~japanese 対象とする単路部の内部構造を生成する
     * @~english  Generate internal structure of target section
     */
    virtual bool createInternalStructure() override;

private:
    /**
     * @~japanese レーン幅，歩道幅を設定する
     * @~english  Set lane width and sidewalk width
     */
    bool _setWidth();

    //==================================================================
    /**
     * @~japanese
     * @name 単路部の多角形頂点を生成するための関数群
     * @note SectionBuilder_Vertex.cpp で実装されている
     *
     * @~english
     * @name Functions for generating section polygon vertexes
     * @note Implemented in SectionBuilder_Vertex.cpp
     */
    ///@{
private:
    /**
     * @~japanese 単路部の多角形頂点を生成する
     * @~english  Generate section polygon vertexes
     */
    bool _generateVertexes();

    ///@}

    //==================================================================
    /**
     * @~japanese
     * @name サブセクションを生成するための関数群
     * @note SectionBuilder_Subsection.cpp で実装されている
     *
     * @~english  
     * @name Functions for generating subsections
     * @note Implemented in SectionBuilder_Subsection.cpp
     */
    ///@{
private:
    /**
     * @~japanese サブセクションを生成する
     * @~english  Generate subsections
     */
    bool _generateSubsections();

    /**
     * @~japanese 中央サブセクション(ID: 0*)を生成する
     * @~english  Generate a central subsection (ID: 0*)
     */
    void _generateCentralSubsection();

    /**
     * @~japanese 歩道サブセクションを生成する
     *
     * IDは_adjInter[0]->[1]方向を見て右側を"1*"，左側を"2*"とする
     *
     * @~english  Generate sidewalk subsection
     *
     * ID number is "1*" on the right and "2*" on the left in the
     * direction of _adjInter[0]->[1]
     */
    void _generateSidewalkSubsection();

    ///@}

    //==================================================================
    /**
     * @~japanese
     * @name レーンを生成するための関数群
     * @note SectionBuilder_Lane.cpp で実装されている
     *
     * @~english
     * @name Functions for generating lanes
     * @note Implemented in SectionBuilder_Lane.cpp
     */
    ///@{
private:
    /**
     * @~japanese レーンを生成する
     * @~english  Generate lanes
     */
    bool _generateLanes();

    /**
     * @~japanese
     * 境界上のコネクタを単に結んでレーンを生成する
     *
     * @~english
     * Generate lanes by simply connecting connectors on borders
     */
    void _generateSimpleLanes(int dir, int nBegin, int nEnd, int offset);

    /**
     * @~japanese
     * 内部コネクタを追加して分岐するレーンを作成する
     *
     * 分岐の方向を @p direction で指定する．
     * 
     * @~english
     * Add internal connectors to create branching lanes
     *
     * Specify branch direction by @p direction.
     */
    void _generateBranchedLane(
        int dir, int n, int offset, LanePosition::Type direction);

    /**
     * @~japanese
     * @p pointBegin と @p pointEnd とを結ぶレーンを生成する
     *
     * @~english
     * Generate a lane connecting @p pointBegin and @p pointEnd
     */
    void _generateLane(
        int idInt, const Connector* pointBegin, const Connector* pointEnd);

    /**
     * @~japanese
     * @p pointBegin と @p pointEnd とを結ぶ多直線分岐レーンを生成する
     *
     * @param nextEndPoint 分岐したレーンの終点
     *
     * @~english
     * Generate a polyline branching lane connecting @p pointBegin and
     * @p pointEnd
     *
     * @param nextEndPoint End connector of branched lane
     */
    void _generatePolylineBranchLane(
        int idInt, const Connector* pointBegin, const Connector* pointEnd,
        const Connector* nextEndPoint);

    /**
     * @~japanese 内部コネクタを生成して返す
     * @~english  Create and return internal connector
     */
    const Connector* _createInternalConnector(
        const amu::geometry::AmuPoint& begin,
        const amu::geometry::AmuPoint& end, double distance, int* result_id);

public:
    /**
     * @~japanese レーン接続を設定する
     * @~english  Configure lane connection
     */
    virtual bool setLaneConnection() override;

private:
    /**
     * @~japanese @p lane の左のレーンを求める
     * @~english  Find left lane of @p lane
     */
    bool _decideLeftLane(Lane* lane);

    /**
     * @~japanese @p lane の右のレーンを求める
     * @~english  Find right lane of @p lane
     */
    bool _decideRightLane(Lane* lane);

    /**
     * @~japanese @p lane の隣のレーンを求める
     *
     * @p direction により探索方向を指定する．
     * 
     * @~english  Find lane next to @p lane
     *
     * Specify the search direction by @p direction.
     */
    Lane* _calcSideLane(Lane* lane, LanePosition::Type direction) const;

    /**
     * @~japanese @p lane の隣に @p anotherLaneを追加する
     *
     * @p direction により設定方向を指定する．
      *
     * @~english  Add @p anotherLane next to @p lane
     *
     * Specify the setting direction by @p direction.
     */
    void _setSideLane(
        Lane* lane, Lane* anotherLane, LanePosition::Type direction);

    ///@}

private:
    /**
     * @~japanese 生成対象の単路部
     * @~english  Section to be generated
     */
    Section* _section;

    /**
     * @~japanese レーン幅 [m]
     * @~english  Lane width [m]
     */
    double _laneWidth;

    /**
     * @~japanese 分岐レーンの長さ
     *
     * インデックス0 (isUp==false) は始点側，1 (isUp==true) は終点側
     *
     * @~english  Branch lane length
     *
     * A index of 0 (isUp==false) indicates the starting point side
     * and 1 (isUp==true) indicates the ending point side.
     */
    double _branchLaneLength[2];

    /**
     * @~japanese 路肩幅 [m]
     * @~english  Shoulder width [m]
     */
    double _roadsideWidth;

    /**
     * @~japanese 歩道幅 [m]
     *
     * インデックス0 (isLeft==false) は右，1 (isLeft==true) は左
     *
     * @~english  Sidewalk width [m]
     *
     * Index 0 (isLeft==false) indicates right, and 1 (isLeft==true)
     * indicates left side.
     */
    double _sidewalkWidth[2];

    //==================================================================
#ifdef INCLUDE_TRAMS
private:
    /**
     * @~japanese 路面電車用拡張オブジェクト
     * @~english  Extension object for tram
     */
    SectionBuilderTramExt* _builderTramExt;
#endif //INCLUDE_TRAMS

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    Section* section() const
    {
        return _section;
    }

    double laneWidth() const
    {
        return _laneWidth;
    }

    void setLaneWidth(double width)
    {
        _laneWidth = width;
    }

    double roadsideWidth() const
    {
        return _roadsideWidth;
    }

    void setRoadsideWidth(double width)
    {
        _roadsideWidth = width;
    }

    double sidewalkWidth(bool isLeft)
    {
        return _sidewalkWidth[isLeft];
    }

    void setSidewalkWidth(bool isLeft, double width)
    {
        _sidewalkWidth[isLeft] = width;
    }

#ifdef INCLUDE_TRAMS
    SectionBuilderTramExt* builderTramExt()
    {
        return _builderTramExt;
    }
#endif //INCLUDE_TRAMS

    ///@}
};

#endif //__SECTION_BUILDER_HPP__
