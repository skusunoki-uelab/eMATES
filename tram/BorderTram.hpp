/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file BorderTram.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __BORDER_TRAM_HPP__
#define __BORDER_TRAM_HPP__
#include "../Border.hpp"
#include <AmuPoint.hpp>

class IntersectionTramExt;
class Connector;

//######################################################################
/**
 * @~japanese レーン束オブジェクト間の境界
 * @~english  Boundary between lane bundle object
 * @~ @ingroup TramSim
 */
class BorderTram : public Border
{
public:
    /**
     * @~japanese 境界のみ作成するコンストラクタ
     * @~english  Constructor that crates a border only
     */
    BorderTram(
        const amu::geometry::AmuPoint begin,
        const amu::geometry::AmuPoint end, double roadsideWidth)
        : Border(begin, end, roadsideWidth)
    {
    }

    virtual ~BorderTram() override {};

    /**
     * @~japanese
     * 識別番号が @p idInt の路面電車レーン用コネクタを戻す
     *
     * @~english
     * Return the connector for tram lane with ID number @p idInt
     */
    const Connector* tramConnector(int i) const;

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
    void createConnectors(
        int numIn, int numOut, IntersectionTramExt* tramExt, int dir);

    /// コネクタを作成する関数
    /**
     * ODNodeのSectionが接していない境界用
     */
    /**
     * @~japanese コネクタを
     * @~english  
     */

    void createConnectorsReverse(
        int numIn, int numOut, IntersectionTramExt* tramExt, int dir);

private:
    /**
     * @~japanese @name createConnectorsの内部で用いる関数
     * @~english  @name Functions used inside createConnectors
     */
    ///@{
    void _createVehicleConnectors(
        amu::geometry::AmuLineSegment& line, int numIn, int numOut,
        IntersectionTramExt* tramExt, int dir, bool isReversed);

    void _createTramConnectors(
        amu::geometry::AmuLineSegment& line, int numIn, int numOut,
        IntersectionTramExt* tramExt, int dir, bool isReversed);
    ///@}

    //==================================================================
private:
    /**
     * @~japanese 路面電車流入点
     * @note 1車線のみ考慮している
     *
     * @~english  Inflow point of tram
     * @note Considering only 1 lane
     */
    const Connector* _inPointTram;

    /**
     * @~japanese 路面電車流出点
     * @note 1車線のみ考慮している
     *
     * @~english  Outflow point of tram
     * @note Considering only 1 lane
     */
    const Connector* _outPointTram;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const Connector* inPointTram() const
    {
        return _inPointTram;
    }

    const Connector* outPointTram() const
    {
        return _outPointTram;
    }

    ///@}
};

#endif //__BORDER_TRAM_HPP__
#endif //INCLUDE_TRAMS
