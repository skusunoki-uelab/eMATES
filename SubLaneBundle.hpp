/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SubLaneBundle.hpp
 */
#ifndef __SUB_LANE_BUNDLE_HPP__
#define __SUB_LANE_BUNDLE_HPP__
#include "Lane.hpp"
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

class LaneBundle;
class Signal;

//######################################################################
/**
 * @~japanese サブセクションの種別をあらわす列挙型
 * @~english  Enumeration representation subsection type
 */
enum class SubsectionType : unsigned int
{
    Roadway   = 0,
    Sidewalk  = 11,
    Crosswalk = 12,
};

//######################################################################
/**
 * @~japanese サブセクション
 *
 * 交差点，単路部を細分化するためのクラス．歩行者の歩行領域の定義などに
 * 用いる．単路部を分割するものを SubSection，交差点を分割するものを
 * SubIntersection として継承する．
 *
 * @note
 * LaneBundle を 細分化したものであるので SubLaneBundle と名付けた．
 * Section -> SubSection，Intersection -> SubIntersection と対応．
 *
 * @note
 * Subsection の形状は四角形に限定される．SubIntersection は凸多角形で
 * あればよい．
 *
 * @~english  Subsection
 *
 * A class for subdividing intersections and sections. Used to define
 * the walking area for pedestrians. Inherit the one that divides a
 * section as SubSection, the one that divides an intersection as
 * SubIntersection.
 *
 * @note
 * Named SubLaneBundle because it divides LaneBundle. Corresponds to
 * Section -> SubSection and Intersection -> SubIntersection.
 *
 * @note
 * The shape of Subsection is restricted to a rectangle. SubIntersection
 * can be any convex polygon.
 *
 * @~ @ingroup RoadNetwork
 */
class SubLaneBundle
{
public:
#ifdef INCLUDE_PEDESTRIANS
    friend class SubLaneBundlePedExt;
#endif //INCLUDE_PEDESTRIANS

public:
    SubLaneBundle(const std::string& id, SubsectionType type);
    virtual ~SubLaneBundle();

    //==================================================================
    /**
     * @~japanese @name 幾何形状に関する関数群
     * @~english  @name Functions related to geometry
     */
    ///@{

    /**
     * @~japanese サブセクションの中心点を設定する
     * @~english  Set center point of this subsection
     */
    void setCenter()
    {
        double x = 0.0, y = 0.0, z = 0.0;
        for (auto itr : _vertexes)
        {
            x += itr.x();
            y += itr.y();
            z += itr.z();
        }
        int size = _vertexes.size();
        _center.setXYZ(x / size, y / size, z / size);
    }

    /**
     * @~japanese 頂点の個数を戻す
     * @~english  Return the number of vertexes
     */
    int numVertexes() const
    {
        return _vertexes.size();
    }

    /**
     * @~japanese @p i 番目の頂点を返す
     * @~english  Return the @p i -th vertex
     */
    const amu::geometry::AmuPoint& vertex(int i) const
    {
        assert(0 <= i && i < static_cast<signed int>(_vertexes.size()));
        return _vertexes[i];
    }

    /**
     * @~japanese 頂点 @p vertex を追加する
     * @~english  Add vertex @p vertex
     */
    void addVertex(amu::geometry::AmuPoint vertex)
    {
        _adjSubLaneBundles.push_back(nullptr);
        _vertexes.push_back(vertex);
    }

    /**
     * @~japanese @p i 番目の辺を返す
     *
     * 始点が i 番目の頂点，終点が i+1 番目の頂点の辺
     *
     * @note
     * メンバ変数として辺を保持していないので都度生成する．
     *
     * @~english  Return the @p i -th edge
     *
     * The edge whose begin point is i-th vertex and whose end point
     * is (i+1)-th vertex.
     *
     * @note
     * Since the edges are not stored as member variables, they are
     * generated each time.
     */
    const amu::geometry::AmuLineSegment edge(int i) const
    {
        return amu::geometry::AmuLineSegment(
            vertex(i), vertex((i + 1) % numVertexes()));
    }

    /**
     * @~japanese
     * 別のサブセクション @p subsec に接する辺を戻す
     *
     * @~english
     * Return the edge tangent to another subsection @p subsec
     */
    const amu::geometry::AmuLineSegment edge(
        SubLaneBundle* subsec) const
    {
        return edge(edgeNum(subsec));
    }

    /**
     * @~japanese
     * サブセクション @p subsec に接する辺の辺番号を戻す
     *
     * @~english
     * Return the edge number of the edge tangent to the subsection
     * @p subsec
     */
    int edgeNum(SubLaneBundle* subsec) const;

    /**
     * @~japanese 点 @p point が領域内に入っているかどうか判定する
     *
     * @attention
     * 頂点が左回り又は右回りで順番に _vertexes に格納されている必要が
     * ある．
     *
     * @~english  Check whether the point @p point is inside the region
     *
     * @attention
     * Vertexes must be stored in _vertexes in either counterclockwise
     * or clockwise order
     */
    bool isInside(const amu::geometry::AmuPoint& point) const;

    ///@}

    //==================================================================
    /**
     * @~japanese @name 道路構造に関する関数群 
     * @~english  @name functions related to road structure
     */
    ///@{

    /**
     * @~japanese
     * 辺番号 @p edgeNum の辺に接するサブセクションを戻す
     *
     * @~english
     * Return the subsection tangent to the edge whose edge number
     * @p edgeNum
     */
    SubLaneBundle* adjSubLaneBundle(int edgeNum) const
    {
        assert(
            0 <= edgeNum
            && edgeNum < static_cast<int>(_adjSubLaneBundles.size()));
        return _adjSubLaneBundles[edgeNum];
    }

    /**
     * @~japanese
     * 辺番号 @p edgeNum に接するサブセクション @p subsec を設定する
     *
     * @~english
     * Assign subsection @p subsec to the edge whose edge number
     * @p edgeNum.
     */
    void addAdjSubLaneBundle(int edgeNum, SubLaneBundle* subsec)
    {
        assert(_adjSubLaneBundles[edgeNum] == nullptr);
        _adjSubLaneBundles[edgeNum] = subsec;
    }

    /**
     * @~japanese レーン @p lane を内包するかどうかを戻す
     *
     * @note
     * 多くのレーンの始点・終点が辺上にあるため，厳密に判定する
     *
     * @~english  Return whether @p lane is included
     *
     * @note
     * Since the start and end points of many lanes are on the edge,
     * strict judgment is required. 
     */
    bool includes(const Lane* lane) const;

    /**
     * @~japanese レーン @p lane を追加する
     * @~english  Add @p lane
     */
    void addLane(Lane* lane)
    {
        _lanes.insert(make_pair(lane->id(), lane));
    }

    /**
     * @~japanese
     * 参照方向 @p dir を指定して信号 @p signal を設置する
     * 
     * @~english
     * Attach signal @p signal with reference direction @p dir
     */
    void attachSignal(Signal* signal, int dir)
    {
        _signal          = signal;
        _signalDirection = dir;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese サブセクションの属性を @p out に出力する
     * @~english  Output subsection attributes to @p out 
     */
    virtual void print(std::ostream& out) const;

    //==================================================================
protected:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese 親オブジェクト
     *
     * このサブセクションが含まれるレーン束オブジェクト
     *
     * @~english  Parent object
     *
     * Lane bundle object that contains this subsection
     */
    LaneBundle* _parent;

    /**
     * @~japanese サブセクションの種類
     * @~english  Subsection type
     */
    SubsectionType _type;

    /**
     * @~japanese 信号
     * @~english  Traffic light
     */
    Signal* _signal;

    /**
     * @~japanese 交差点における信号参照方向
     * @~english  Reference direction of traffic light at intersection
     */
    int _signalDirection;

    /**
     * @~japanese
     * このサブセクションに含まれるレーンの集合
     *
     * @~english
     * Set of lanes contained in this subsection  
     */
    std::unordered_map<std::string, Lane*> _lanes;

    /**
     * @~japanese 中心点 (重心)
     * @~english  Center point (centroid)
     */
    amu::geometry::AmuPoint _center;

    /**
     * @~japanese 頂点の配列
     * @~english  Vertex sequence
     */
    std::vector<amu::geometry::AmuPoint> _vertexes;

    /**
     * @~japanese 隣接するサブセクション
     * @~english  Adjacent subsections
     */
    std::vector<SubLaneBundle*> _adjSubLaneBundles;

    /**
     * @~japanese 厳密な内外判定フラグ
     * @~english  Flag for strict inside-outside judgment
     */
    bool _strictJudgeInside;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& id() const
    {
        return _id;
    }

    LaneBundle* parent() const
    {
        return _parent;
    }

    void setParent(LaneBundle* parent)
    {
        _parent = parent;
    }

    void setType(SubsectionType type)
    {
        _type = type;
    }

    SubsectionType type() const
    {
        return _type;
    }

    Signal* signal() const
    {
        return _signal;
    }

    int signalDirection() const
    {
        return _signalDirection;
    }

    const std::unordered_map<std::string, Lane*>& lanes() const
    {
        return _lanes;
    }

    const amu::geometry::AmuPoint& center() const
    {
        return _center;
    }

    const std::vector<amu::geometry::AmuPoint>& vertexes() const
    {
        return _vertexes;
    }

    ///@}
};

#endif //__SUB_LANE_BUNDLE_HPP__
