/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramRouteManager.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __TRAM_ROUTE_MANAGER_HPP__
#define __TRAM_ROUTE_MANAGER_HPP__
#include "../ManagerBase.hpp"
#include <array>
#include <iostream>
#include <vector>

class RoadMap;
class Intersection;
class TramRoute;

/**
 * @defgroup TramSim
 * @~japanese 路面電車シミュレーション用モジュール
 * @~english  Module for tram simulation
 */

//######################################################################
/**
 * @~japanese
 * 単路部内の路面電車レーン設置位置に関する列挙型を定義する構造体
 *
 * @~english
 * Structure defining enumerations of tram lane installation positions
 * within a section
 *
 * @~ @ingroup IO Manager TramSim
 */
struct TramLaneSide
{
public:
    /**
     * @~japanese
     * 路面電車レーン設置位置をあらわす列挙型
     *
     * @~english
     * Enumerations about the installation side of tram lanes 
     */
    enum Type
    {
        None   = -1,
        Left   = 0,
        Center = 1,
        Right  = 2,
    };
};

//######################################################################
/**
 * @~japanese 路面電車の路線情報を管理する
 * @~english  Manager tram route information
 * @~ @ingroup Manager TramSim
 */
class TramRouteManager : public ManagerBase
{
    friend class ManagerPool;

private:
    TramRouteManager();
    ~TramRouteManager();

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void _finalizeInside() override {};

    //==================================================================

public:
    /// 路面電車の路線定義ファイルを読み込む
    void readTramRouteFile();

    /**
     * @~japanese
     * 交差点 @p inter の @p dir 方向の路面電車レーンのコネクタ数を
     * 取得し，流入点数，流出点数をそれぞれ @p result_numIn ，
     * @p result_numOut に格納する
     *
     * @~english
     * Get the number of connectors of the tram lane in direction
     * @p dir of intersection @p inter and store the numbers of inflow
     * and outflow points in @p result_numIn and @p result_numOut ,
     * respectively
     */
    void getNumTramConnectors(
        Intersection* inter, int dir, std::array<int, 3>& result_numIn,
        std::array<int, 3>& result_numOut);

    /**
     * @~japanese
     * 交差点 @p inter の @p inDir 方向から流入して @p outDir へ流出する
     * 路線が存在するかどうかを戻す  
     * 
     * @~english
     * Returns whether there is a route flowing in from @p inDir and
     * flowing out to @p outDir of intersection @p inter
     */
    bool hasTramRoute(Intersection* inter, int inDir, int outDir) const;

    /**
     * @~japanese
     * 指定された交差点を指定された順で通過する路線を戻す
     *
     * @~english
     * Return a route that passes through the specified intersections
     * in the specified order
     */
    TramRoute* tramRouteWithDesignatedGates(
        const std::vector<const Intersection*>& gates);

    /**
     * @~japanese 保存した路線情報を @p out に出力する
     * @~english  Output saved route information to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 地図オブジェクト
     * @~english  Road map object
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 路面電車の路線
     * @~english  Tram line
     */
    std::vector<TramRoute*> _tramRoutes;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setRoadMap(RoadMap* roadMap)
    {
        _roadMap = roadMap;
    }

    ///@}
};

#endif //__TRAM_ROUTE_MANAGER_HPP__
#endif //INCLUDE_TRAMS
