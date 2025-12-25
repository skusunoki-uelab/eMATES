/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file IntersectionBuilder.hpp
 */
#ifndef __INTERSECTION_BUILDER_HPP__
#define __INTERSECTION_BUILDER_HPP__
#include "LaneBundleBuilder.hpp"
#include "../Border.hpp"
#include "../Connector.hpp"
#include "../Intersection.hpp"
#include "../LaneBundle.hpp"
#include "../RelativeDirection.hpp"
#include "../RelativeDirectionTable.hpp"
#ifdef INCLUDE_TRAMS
#include "../tram/IntersectionBuilderTramExt.hpp"
#endif //INCLUDE_TRAMS
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <AmuVector.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class SectionBuilder;

//##############################################################################
/**
 * @~japanese 交差点を生成し内部構造を作成する
 *
 * @note
 * Intersectionクラスのインスタンスと1対1に対応する．
 * IntersectionBuildDirectorがコンテナを管理し，RoadMap生成完了後に
 * ただちにdeleteする
 *
 * @~english  Generate an intersection
 *
 * @note
 * One-to-one correspondence with instances of Intersection class.
 * IntersectionBuildDirector manages the container and deletes builder
 * instance immediately after the RoadMap generation is completed.
 *
 * @~
 * @ingroup Initialization IO RoadNetwork
 * @see LaneBundleBuilder
 */
class IntersectionBuilder : public LaneBundleBuilder
{
public:
#ifdef INCLUDE_TRAMS
    friend class IntersectionBuilderTramExt;
#endif // INCLUDE_TRAMS

    explicit IntersectionBuilder(RoadMapBuilder* roadMapBuilder);
    virtual ~IntersectionBuilder();

    //==========================================================================
    /**
     * @~japanese Intersectionクラスのインスタンスを生成して返す
     * @~english  Generate and return an instance of intersection class
     */
    virtual Intersection* build(
        const std::string& id, const std::string& type, RoadMap* roadMap);

    /**
     * @~japanese 隣接交差点のbuilder @p nextBuilder を設定する
     * @note 内部でIntersection同士の隣接関係も構築する．
     * @attention 登録順に注意．
     *
     * @~english  Set the builder @p nextBuilder of an adjacent intersection
     * @note Also set adjacency relationships between intersections.
     * @attention Pay attention to the registration order.
     */
    void setNextIntersectionBuilder(IntersectionBuilder* iBuilder);

    /**
     * @~japanese
     * 方向 @p dir に隣接する交差点のbuilderを戻す 
     *
     * @~english
     * Return the builder of the intersection adjacent to direction @p dir 
     */
    IntersectionBuilder* intersectionBuilder(int dir)
    {
        return _nextIBuilders[dir];
    }

    /**
     * @~japanese 接続単路部のビルダ @p nextSectionBuilder を設定する
     * @param dir 単路が接続する方向
     * @note 内部でIntersectionとSectionの接続関係も構築する．
     * @attention 登録順に注意．
     * 
     * @~english  Set the builder @p nextSectionBuilder of an incident section
     * @param dir Direction where the section connects 
     * @note Also set incidence relationship between Intersection and Section.
     * @attention Pay attention to the registration order.
     */
    void setNextSectionBuilder(int dir, SectionBuilder* sBuilder);

    /**
     * @~japanese 方向 @p dir に隣接する単路部のbuilderを戻す 
     * @~english  Return the builder of the section incident to direction @p dir 
     */
    SectionBuilder* sectionBuilder(int dir)
    {
        return _nextSBuilders[dir];
    }

    //==========================================================================
    /**
     * @~japanese 内部構造を生成するための情報を設定する
     * @~english  Set information for generating internal structure
     */
    virtual bool setInternalInfo(std::ifstream* fin);

protected:
    /**
     * @~japanese
     * 交差点設定ファイルに相対方向テーブルの定義が含まれるかどうか
     *
     * @~english
     * Whether intersection configuration file contains relative
     * direction table definition
     */
    bool _includesRDTable(std::ifstream* fin);

    /**
     * @~japanese
     * 交差点設定ファイルにレーン接続の定義が含まれるかどうか
     *
     * @~english
     * Whether intersection configuration file contains lane connection
     * definitions
     */
    bool _includesLaneConnection(std::ifstream* fin);

    /**
     * @~japanese
     * 交差点設定ファイルに頂点座標の定義が含まれるかどうか
     *
     * @~english
     * Whether intersection setting file contains definitions of vertex
     * coordinates
     */
    bool _includesRoadwayVertex(std::ifstream* fin);

    //==========================================================================
public:
    /**
     * @~japanese 対象とする交差点の内部構造を生成する
     * @~english  Generate internal structure of target intersection
     */
    virtual bool createInternalStructure() override;

protected:
    /**
     * @~japanese 歩道および横断歩道の幅を設定する
     * @~english  Set sidewalk and crosswalk width
     */
    bool _setSidewalkAndCrosswalkWidth();

    //==========================================================================
    /**
     * @~japanese
     * @name 相対方向テーブルを生成するための関数群
     * @note IntersectionBuilder_RDTable.cpp で実装されている
     *
     * @~english
     * @name Functions for generating relative orientation table
     * @note Implemented in IntersectionBuilder_RDTable.cpp
     */
    ///@{
protected:
    /**
     * @~japanese 相対方向テーブルをファイルから生成する
     * @~english  Generate relative direction table from file
     */
    RelativeDirectionTable* _buildRDTableFromFile(std::ifstream* fin);

    /**
     * @~japanese デフォルトの相対方向テーブルを作成する
     * @~english  Generate default relative direction table
     */
    RelativeDirectionTable* _buildDefaultRDTable();

    /**
     * ファイル入力文字からRelativeDirection形式へ
     * @todo RelativeDirectionTableクラスに移行？
     */
    RD_t _stoRD(const std::string& str);

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name 車道頂点を生成するための関数群
     * @note IntersectionBuilder_RoadwayVertex.cpp で実装されている
     *
     * @~english
     * @name Functions for generating roadway vertexes
     * @note Implemented in IntersectionBuilder_RoadwayVertex.cpp.
     */
    ///@{
protected:
    /**
     * @~japanese
     * 座標を設定ファイルから入力して車道頂点を生成する
     *
     * @~english
     * Generate roadway vertexes by inputting coordinates from the
     * configuration file
     */
    bool _generateRoadwayVertexesFromFile(std::ifstream* fin);

    /**
     * @~japanese デフォルトルールに従って車道頂点を生成する
     * @~english  Generate roadway vertexes according to default rules
     */
    virtual bool _generateDefaultRoadwayVertexes();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 車道頂点を生成するためのデフォルトルールとして用られる
    // Used as default rules for generating roadway vertexes

    /**
     * @~japanese 単路部の境界の交点を求める
     * @~english  Find intersection point of section boundaries
     */
    void _findCrossPointOfSectionBorders(
        std::vector<amu::geometry::AmuPoint>& result_points);

    /**
     * @~japanese 
     * 十分な角度をつけて交わる単路部の間の交点を求める
     *
     * @~english
     * Find intersection point between sections that meet at a
     * sufficient  angle
     */
    void _findCrossPointWithAngledSections(
        int i, const amu::geometry::AmuPoint& center,
        const amu::math::AmuVector& curDirectionVector,
        const amu::math::AmuVector& prevDirectionVector, double theta,
        amu::geometry::AmuPoint& result_point);

    /**
     * @~japanese
     * 十分に大きくない角度で交わる単路部の間の交点を求める
     * 
     * @~english
     * Find intersection point between sections that meet at an
     * insufficient angle
     */
    void _findCrossPointWithFlatSections(
        int i, const amu::geometry::AmuPoint& center,
        amu::geometry::AmuPoint& result_point);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese 車道頂点を求める
     * @~english  Find roadway vertexes
     */
    void _findRoadwayVertexes(
        const std::vector<amu::geometry::AmuPoint>& crossPoints);

    /**
     * @~japanese 境界の方向ベクトルを求める
     * @~english  Find direction vector of a border
     */
    void _findBorderDirection(int i, amu::math::AmuVector& result_vector);
    /**
     * @~japanese
     * 単路部の交点 @p crossPoints を @p margin だけセットバックした点を
     * 通り，境界の方向ベクトル @p borderDirection に平行な線分を求める
     *
     * @note
     * 交点は2つあるので，線分は2つ生成される
     *
     * @~english
     * Find line segments parallel to the direction vector of the border
     * passing through the point set back by @p margin from intersection
     * points @p crossPoints of sections.
     *
     * @note
     * 2 intersection points create 2 line segments
     */
    void _findReferenceBorderLines(
        int i, double margin,
        const std::vector<amu::geometry::AmuPoint>& crossPoints,
        const amu::math::AmuVector&                 borderDirection,
        amu::geometry::AmuLineSegment&              result_borderLineLeft,
        amu::geometry::AmuLineSegment&              result_borderLineRight);

    /**
     * @~japanese 単路部の中心線を求める
     * @~english  Find center line of section
     */
    void _findCenterLine(
        int i, amu::geometry::AmuLineSegment* result_centerLine);

    /**
     * @~japanese
     * 境界線 @p borderLineLeft および @p borderLineRight と
     * 単路部の中心線 @p centerLine の交点を求め，交差点の中心から
     * より遠いほうの点を @p result_pointに格納する
     *
     * @~english
     * Find intersection point of the border lines @p borderLineLeft
     * and @p borderLineRight and the center line of the section
     * @p centerLine, and store in @p result_point the point that is
     * farther from the center of the intersection
     */
    void _findCrossPointOfBorderAndCenterLines(
        const amu::geometry::AmuLineSegment& borderLineLeft,
        const amu::geometry::AmuLineSegment& borderLineRight,
        const amu::geometry::AmuLineSegment& centerLine,
        amu::geometry::AmuPoint&             result_point);

    /**
     * @~japanese
     * 点 @p midPoint を通り方向ベクトル @p borderDirection を持つ
     * 境界上の車道頂点を決定する
     *
     * @~english
     * Determine roadway vertexes on border with direction vector
     * @p borderDirection through point @p midPoint .
     */
    void _determineRoadwayVertexes(
        int i, const amu::geometry::AmuPoint& midPoint,
        const amu::math::AmuVector& borderDirection);

    /**
     * @~japanese
     * @p i 番目の隣接交差点に向かう内部方向ベクトルを戻す
     *
     * @note
     * 接続する単路部の方向ベクトル (_interの_centerと隣接交差点の
     * _centerを結んだもの) とは限らない
     * 
     * @~english
     * Return the internal direction vector pointing to @p i -th
     * adjacent intersection
     *
     * @note
     * It is not necessarily same as the direction vector of incident
     * section (the _center of _inter and the _center of the adjacent
     * intersection).
     */
    const amu::math::AmuVector _internalUnitDirectionVector(int i);

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name 交差点の多角形頂点を生成するための関数群
     * @note IntersectionBuilder_Vertex.cpp で実装されている
     *
     * @note
     * 歩道幅を考慮するため，多角形頂点は車道頂点とは異なる場合がある
     *
     * @~english
     * @name Functions for generating intersection polygon vertexes
     * @note Implemented in IntersectionBuilder_Vertex.cpp
     *
     * @note
     * Polygon vertex may be different from roadway vertex to consider
     * sidewalk width.
     */
    ///@{
protected:
    /**
     * @~japanese 交差点の多角形頂点を生成する
     * @~english  Generate intersection polygon vertexes
     */
    virtual bool _generateVertexes();

    /**
     * @~japanese
     * 歩道幅を考慮して拡張した交差点の頂点を求める
     *
     * @~english
     * Find vertexes of the intersection expanded considering the
     * sidewalk width
     */
    void _findVertexesOfExpandedIntersection();

    /**
     * @~japanese 交差点の多角形頂点を補正する
     * @return    補正したかどうか
     *
     * @~english  Revise intersection polygon vertexes
     * @return    Whether revised
     */
    bool _reviseVertexes(std::vector<bool>& result_isRevisedCorner);

    /**
     * @~japanese 境界生成用の点を補正する
     * @~english  Revise points for border generation
     */
    bool _reviseBorderPoints(const std::vector<bool>& isRevisedCorner);

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name 境界およびコネクタを生成するための関数群
     * @note IntersectionBuilder_Border.cpp で実装されている
     *
     * @~english
     * @name Functions for generating borders and connectors
     * @note Implemented in IntersectionBuilder_Border.cpp 
     */
    ///@{
protected:
    /**
     * @~japanese 境界を生成する
     * @~english  Generate borders
     */
    virtual bool _generateBorders();

    /**
     * @~japanese 内部コネクタを生成する
     * @~english  Generate internal connectors
     */
    virtual bool _generateInternalConnectors();

    /**
     * @~japanese
     * 境界以外の線分 @p line 上にコネクタを生成する
     *
     * @~english
     * Generate connectors on non-border line segments @p line
     */
    bool _generateConnectorsOnLine(
        amu::geometry::AmuLineSegment line, int numIn, int numOut,
        int borderId);

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name サブセクションを生成するための関数群
     * @note IntersectionBuilder_Subsection.cpp で実装されている
     *
     * @~english  
     * @name Functions for generating subsections
     * @note Implemented in IntersectionBuilder_Subsection.cpp
     */
    ///@{
protected:
    /**
     * @~japanese サブセクションを生成する
     * @~english  Generate subsections
     */
    virtual bool _generateSubsections();

    /**
     * @~japanese 中央サブセクション(ID: 00)を生成する
     * @~english  Generate a central subsection (ID: 00)
     */
    virtual void _generateCentralSubsection();

    /**
     * @~japanese
     * 横断歩道サブセクションを生成する
     *
     * @p r ，@p t は極座標(r, theta)に対応する．
     *
     * @attention
     * 現状では r=1 までしか対応していない
     * 
     * @~english
     * Generate a crosswalk subsection
     *
     * @p r and @p t correspond to polar coordinates (r, theta).
     *
     * @attention
     * Currently only supports up to r=1
     */
    virtual void _generateCrosswalkSubsection(int r, int t);

    /**
     * @~japanese
     * 歩道サブセクションを生成する
     *
     * @p r ，@p t は極座標(r, theta)に対応する．
     * 
     * @attention
     * 現状では r=1 までしか対応していない
     * 
     * @~english
     * Generate a sidewalk subsection
     *
     * @p r and @p t correspond to polar coordinates (r, theta). 
     *
     * @attention
     * Currently only supports up to r=1
     */
    virtual void _generateSidewalkSubsection(int r, int t);

    ///@}

    //==========================================================================
    /**
     * @~japanese
     * @name レーンを生成・設定するための関数群
     * @note IntersectionBuilder_Lane.cpp で実装されている
     *
     * @~english
     * @name Functions for generating and setting lanes
     * @note Implemented in IntersectionBuilder_Lane.cpp
     */
    ///@{
protected:
    /**
     * @~japanese レーン接続を設定ファイルから入力する
     * @~english  Input lane connections from configuration file
     */
    bool _readLaneConnectionFromFile(std::ifstream* fin);

    /**
     * @~japanese レーンを生成する
     * @~english  Generate lanes
     */
    virtual bool _generateLanes();

    /**
     * @~japanese
     * ファイルで定義された情報にもとづいてレーンを生成する
     *
     * @~english
     * Generate lanes based on information defined in configuration file
     */
    virtual void _generateLanesFromFile();

    /**
     * @~japanese
     * デフォルトルールに従ってレーンを生成する
     * 
     * @~english
     * Generate lanes according to default rules
     */
    virtual void _generateDefaultLanes();

    /**
     * @~japanese
     * デフォルトルールに従って右折レーンを生成する
     * 
     * @~english
     * Generate right-turn lanes according to default rules
     */
    virtual void _generateDefaultRightLanes(unsigned int from, unsigned int to);

    /**
     * @~japanese
     * デフォルトルールに従って直進レーンを生成する
     * 
     * @~english
     * Generate straight lanes according to default rules
     */
    virtual void _generateDefaultStraightLanes(
        unsigned int from, unsigned int to);

    /**
     * @~japanese
     * デフォルトルールに従って左折レーンを生成する
     * 
     * @~english
     * Generate left-turn lanes according to default rules
     */
    virtual void _generateDefaultLeftLanes(unsigned int from, unsigned int to);

    /**
     * @~japanese 横断歩道レーンを生成する 
     * @~english  Generate crosswalk lanes
     */
    void _generateCrosswalkLanes(int dir);

    /**
     * @~japanese
     * コネクタ @p pointBegin ， @p pointEnd をそれぞれ始点・終点とする
     * ID @p idInt を持つレーンを生成する
     * 
     * @~english
     * Generate a lane with ID number @p idInt with connectors
     * @p pointBegin and @p pointEnd as start and end points,
     * respectively.
     */
    void _generateLane(
        int idInt, const Connector* pointBegin, const Connector* pointEnd);

public:
    /**
     * @~japanese レーンの交錯関係を設定する
     * @~english  Set lane crossing
     */
    virtual bool setLaneCollision() override;

protected:
    /**
     * @~japanese レーン @p lane の交錯レーンを設定する
     * @~english  Set crossing lanes of @p lane
     */
    bool _decideCollisionLane(const Lane* lane);

    /**
     * @~japanese
     * レーン @p lane の交錯レーンである @p collisionLane の
     * 上流のレーンを登録する
     *
     * @param[out] result_clInter
     * 上流レーンのうち交差点内のレーン
     *
     * @param[out] result_clSection
     * 上流レーンのうち単路部内のレーン
     *
     * @~english
     * Register the upstream lanes of @p collisionLane which is
     * crossing lane of lane @p lane
     *
     * @param[out] result_clInter
     * Intersection lanes within upstream lanes
     *
     * @param[out] result_clSection
     * Section lanes within upstream lanes
     */
    bool _addUpstreamCollisionLanes(
        const Lane* lane, const Lane* collisionLane,
        std::vector<const Lane*>& result_clInter,
        std::vector<const Lane*>& result_clSection);

    ///@}

    //==========================================================================
protected:
    /**
     * @~japanese 生成対象の交差点
     * @~english  Intersection to generate
     */
    Intersection* _inter;

    /**
     * @~japanese 隣接交差点のBuilderオブジェクト
     * @attention Intersection::_nextsと同順にすること
     * 
     * @~english  Builder objects of adjacent intersections
     * @attention Must be in the same order as Intersection::_nexts
     */
    std::vector<IntersectionBuilder*> _nextIBuilders;

    /**
     * @~japanese 接続単路部のBuilderオブジェクト
     * @attention Intersection::_incSectionsと同順にすること
     * 
     * @~english  Builder objects of incident sections
     * @attention Must be in the same order as Intersection::_incSections
     */
    std::vector<SectionBuilder*> _nextSBuilders;

    /**
     * @~japanese 歩道幅
     * @~english  Sidewalk width
     */
    double _sidewalkWidth;

    /**
     * @~japanese 横断歩道幅
     * @~english  Crosswalk width
     */
    double _crosswalkWidth;

    /**
     * @~japanese 指定された相対方向テーブル
     *
     * @attention
     * 所有権が_interに移るため，このクラスでのdelete禁止
     *
     * @~english  Given relative Orientation Table
     *
     * @attention
     * Not delete in this class, as ownership is transferred to _inter
     */
    RelativeDirectionTable* _rdTable;

    /// レーンを構成するコネクタの識別番号
    /**
     * 始点終点の区別なく保持する．(2n)->(2n+1)へ接続する．
     */
    std::vector<std::string> _connectorIds;

    /**
     * @~japanese 車道頂点のリスト
     * @~english  List of roadway vertexes
     */
    std::vector<amu::geometry::AmuPoint> _roadwayVertexes;

    /**
     * @~japanese 境界生成用頂点のリスト
     * @~english  List of vertexes for border generation
     */
    std::vector<amu::geometry::AmuPoint> _borderPoints;

    //==========================================================================
#ifdef INCLUDE_TRAMS
protected:
    /**
     * @~japanese 路面電車用拡張オブジェクト
     * @~english  Extension object for tram
     */
    IntersectionBuilderTramExt* _builderTramExt;
#endif //INCLUDE_TRAMS

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    Intersection* intersection() const
    {
        return _inter;
    }

    double sidewalkWidth() const
    {
        return _sidewalkWidth;
    }

    void setSidewalkWidth(double width)
    {
        _sidewalkWidth = width;
    }

    double crosswalkWidth() const
    {
        return _crosswalkWidth;
    }

    void setCrosswalkWidth(double width)
    {
        _crosswalkWidth = width;
    }

    amu::geometry::AmuPoint roadwayVertex(unsigned int n)
    {
        assert(n < _roadwayVertexes.size());
        return _roadwayVertexes[n];
    }

#ifdef INCLUDE_TRAMS
    IntersectionBuilderTramExt* builderTramExt()
    {
        return _builderTramExt;
    }
#endif //INCLUDE_TRAMS

    ///@}
};

#endif //__INTERSECTION_BUILDER_HPP__

//#define IB_DEBUG
