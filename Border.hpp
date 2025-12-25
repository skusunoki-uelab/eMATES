/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Border.hpp
 */
#ifndef __BORDER_HPP__
#define __BORDER_HPP__
#include <AmuLineSegment.hpp>
#include <AmuPoint.hpp>
#include <algorithm>
#include <cassert>
#include <vector>

class Connector;

//##############################################################################
/**
 * @~japanese レーン束オブジェクト間の境界
 *
 * 交差点と単路部の境界線上に置かれ，それらに含まれるレーンをつなぐコネクタを
 * 集約する．
 * 
 * @~english  Boundary between lane bundle object
 *
 * Placed on the boundary between an intersection and a section, and aggregates
 * connectors of the lanes included in them.
 *
 * @~ @ingroup RoadNetwork
 */
class Border
{
    //==========================================================================
public:
    /**
     * @~japanese 境界のみ作成するコンストラクタ
     * @~english  Constructor that crates a border only
     */
    Border(
        amu::geometry::AmuPoint begin, amu::geometry::AmuPoint end,
        double roadsideWidth)
        : _lineSegment(begin, end), _roadsideWidth(roadsideWidth) {};

    virtual ~Border() {};

    //==========================================================================
public:
    /**
     * @~japanese
     * 流入ポイントから順に数えて @p i 番目のコネクタを戻す
     *
     * @attention
     * 引数@p iはコネクタの識別番号の末尾に対応する．左から数えることを前提に
     * 使用してはならない．
     *
     * @note
     * 左側通行の場合は流入方向左から見て以下の順に並んでいる
     * - ID: 0, ..., numIn-1 (流入コネクタ)
     * - ID: numIn, ..., numIn+numOut-1 (流出コネクタ)
     * 
     * @note
     * 右側通行の場合は流入方向左から見て以下の順に並んでいる
     * - ID: numIn, ..., numIn+numOut-1 (流出コネクタ)
     * - ID: 0, ..., numIn-1 (流入コネクタ)
     *
     * @~english
     * Return the @p i -th connector counting from the inflow point
     * 
     * @attention
     * Argument @p i corresponds to the end of the ID number of the connector.
     * Do not assume to count from the left.
     *
     * @note
     * In the case of left-hand traffic, they are arranged in the following
     * order when viewed from the left in the inflow direction,
     * - ID: 0, ..., numIn-1 (inflow connectors)
     * - ID: numIn, ..., numIn+numOut-1 (outflow connectors)
     *
     * @note
     * In the case of right-hand traffic, they are arranged in the following 
     * order when viewed from the left in the inflow direction,
     * - ID: numIn, ..., numIn+numOut-1 (outflow connectors)
     * - ID: 0, ..., numIn-1 (inflow connectors)
     */
    const Connector* connector(int i) const;

    /**
     * @~japanese 逆向きにした境界を戻す
     * @attention コネクタは失われる
     *
     * @~english  Return reversed border
     * @attention Connectors are lost
     */
    Border reversedBorder() const
    {
        Border rev(
            lineSegment().pointEnd(), lineSegment().pointBegin(),
            _roadsideWidth);
        return rev;
    }

    /**
     * @~japanese コネクタ @p connector が流入点かどうか
     * @~english  Whether the connector @p connector is an inflow point
     */
    bool isIn(Connector* connector) const
    {
        if (find(_inPoints.begin(), _inPoints.end(), connector)
            == _inPoints.end())
        {
            return false;
        }
        return true;
    }

    /**
     * @~japanese コネクタ @p connector が流出点かどうか
     * @~english  Whether the connector @p connector is an outflow point
     */
    bool isOut(Connector* connector) const
    {
        if (find(_outPoints.begin(), _outPoints.end(), connector)
            == _outPoints.end())
        {
            return false;
        }
        return true;
    }

    /**
     * @~japanese 流入点の数を戻す
     * @~english  Return the number of inflow points
     */
    int numIn() const
    {
        return static_cast<int>(_inPoints.size());
    }

    /**
     * @~japanese 流出点の数を返す
     * @~english  Return the number of outflow points
     */
    int numOut() const
    {
        return static_cast<int>(_outPoints.size());
    }

protected:
    /**
     * @~japanese 幾何学的な線分オブジェクト
     * @~english  Geometric line object
     */
    amu::geometry::AmuLineSegment _lineSegment;

    /**
     * @~japanese 交差点への流入点の集合
     * @~english  Set of inflow points to an intersection
     */
    std::vector<const Connector*> _inPoints;

    /**
     * @~japanese 交差点からの流出点の集合
     * @~english  Set of outflow points from an intersection
     */
    std::vector<const Connector*> _outPoints;

    /**
     * @~japanese 路側幅
     * @~english  Roadside width
     */
    double _roadsideWidth;

    //==========================================================================
    /**
     * @~japanese @name コネクタの生成に関する関数
     * @~english  @name Functions for generating connectors
     */
    ///@{
public:
    /**
     * @~japanese コネクタを作成する
     * @attention すべてのコネクタは等間隔に配置される
     * @param numIn 交差点への流入点数
     * @param numOut 交差点からの流出点数
     *
     * @~english  Generate connectors
     * @attention All connectors are evenly spaced
     * @param numIn Number of inflow points to the intersection
     * @param numOut Number of outflow points from the intersection
     */
    void createConnectors(int numIn, int numOut);

protected:
    /**
     * @~japanese
     * @p d0 : @p d1 の内分点にコネクタを作成して戻す
     *
     * @~english
     * Create and return a connector at the internal division point of
     * @p 0 : @p d1
     */
    const Connector* _createInnerConnector(
        const amu::geometry::AmuLineSegment& line, double d0, double d1);

    ///@}

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const amu::geometry::AmuLineSegment& lineSegment() const
    {
        return _lineSegment;
    }

    const std::vector<const Connector*>& inPoints() const
    {
        return _inPoints;
    }

    const std::vector<const Connector*>& outPoints() const
    {
        return _outPoints;
    }

    double roadsideWidth() const
    {
        return _roadsideWidth;
    }

    ///@}
};

#endif //__BORDER_HPP__
