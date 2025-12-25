/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __INTERSECTION_TRAM_EXT_HPP__
#define __INTERSECTION_TRAM_EXT_HPP__
#include "TramRouteManager.hpp"
#include "../LaneBundle.hpp"
#include <AmuLineSegment.hpp>
#include <array>
#include <vector>

class Intersection;
class Connector;

//######################################################################
/**
 * @~japanese 交差点の路面電車拡張
 * @~english  Tram extension of intersection
 * @~ @ingroup TramSim
 */
class IntersectionTramExt
{
public:
    IntersectionTramExt();
    IntersectionTramExt(Intersection* inter);
    ~IntersectionTramExt(){};

    /**
     * @~japanese 路面電車レーンの流入点・流出点の個数の設定
     * @~english  Set the number of inflow/outflow points of tram lanes
     */
    void setNumTramConnectors();

    /**
     * @~japanese
     * 識別番号が@p idの内部路面電車コネクタを戻す
     *
     * @~english
     *  Return the internal tram connector with ID number @p id
     */
    Connector* internalTramConnector(const std::string& id)
    {
        return _internalTramConnectors[id];
    }
    
    /**
     * @~japanese
     * 境界方向 @p dir の交差点に向かって道路の @p side 側にある路面電車
     * レーンの流入点数を戻す
     * 
     * @~english
     * Return the number of inflow points of tram lanes on the @p side
     * of the road toward the intersection with border direction @p dir
     */
    int numInTramLanes(int dir, TramLaneSide::Type side)
    {
        return _numInTramLanes[dir][side];
    }

    /**
     * @~japanese
     * 境界方向 @p dir の交差点に向かって道路の @p side 側にある路面電車
     * レーンの流出点数を戻す
     * 
     * @~english
     * Return the number of outflow points of tram lanes on the @p side
     * of the road toward the intersection with border direction @p dir
     */
    int numOutTramLanes(int dir, TramLaneSide::Type side)
    {
        return _numOutTramLanes[dir][side];
    }

    /**
     * @~japanese
     * 境界方向 @p dir の交差点に向かって道路の @p side 側にある路面電車
     * レーンの流入点数，流出点数の合計を戻す
     * 
     * @~english
     * Return the total number of inflow and outflow points of tram
     * lanes on the @p side of the road toward the intersection with
     * border direction @p dir
     */
    int numTotalTramLanes(int dir, TramLaneSide::Type side)
    {
        return _numInTramLanes[dir][side]
            + _numOutTramLanes[dir][side];
    }
    
    /**
     * @~japanese
     * 境界方向 @p dir にある路面電車レーンの流入点数，流出点数の合計を
     * 戻す
     * 
     * @~english
     * Return the total number of inflow and outflow points of tram
     * lanes in border direction @p dir
     */
    int numTotalTramLanes(int dir) const
    {
        int sum = 0;
        for (unsigned int i=0; i<3; i++)
        {
            sum += _numInTramLanes[dir][i]
                + _numOutTramLanes[dir][i];
        }
        return sum;
    }
    
    /**
     * @~japanese 路面電車レーン用コネクタ @p connector を追加する
     * @~english  Add connector @p connector for tram lane
     */
    void addInternalTramConnector(const std::string& id,
                                  Connector* connector)
    {
        _internalTramConnectors.insert(make_pair(id, connector));
    }

    /**
     * @~japanese
     * 路面電車レーン用コネクタ @p connector を持つ境界方向を戻す
     *
     * @~english
     * Return border direction with @p connector for tram lane
     */
    int tramDirection(const Connector* connector) const;

    /**
     * @~japanese
     * 識別番号 @p idInt に対応する端コネクタを戻す
     *
     * @~english
     * Return the edge connector corresponding to ID number @p idInt
     */
    const Connector* edgeTramConnector(int idInt);

private:
    /**
     * @~japanese 対応する交差点
     * @~english  Corresponding intersection
     */
    Intersection* _inter;

    /**
     * @~japanese 路面電車レーンの流入点数
     * @note std::array のインデックスは TramLaneSide に対応する
     *
     * @~english  Number of inflow point of tram lanes
     * @note Index of std::array corresponds to TramLaneSide
     */
    std::vector<std::array<int,3>> _numInTramLanes;
    
    /**
     * @~japanese 路面電車レーンの流出点数
     * @note array のインデックスは TramLaneSide に対応する
     *
     * @~english  Number of outflow point of tram lanes
     * @note Index of std::array corresponds to TramLaneSide
     */
    std::vector<std::array<int,3>> _numOutTramLanes;
    
    /**
     * @~japanese 路面電車レーン用内部コネクタ
     * @~english  Internal connector for tram lane
     */
    std::unordered_map<std::string, Connector*> _internalTramConnectors;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    Intersection* inter()
    {
        return _inter;
    }

    ///@}
};

#endif //__INTERSECTION_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS
