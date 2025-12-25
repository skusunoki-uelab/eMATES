/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramRoute.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __TRAM_ROUTE_HPP__
#define __TRAM_ROUTE_HPP__
#include "TramRouteManager.hpp"
#include <array>
#include <iostream>
#include <vector>

class Intersection;

//######################################################################
/**
 * @~japanese 路面電車の1つの路線の属性を格納する構造体
 *
 * 上り・下りで2路線と定義する
 *
 * @~english  Struct storing attributes about one tram route
 *
 * Define as 2 route, inbound and outbound.
 *
 * @~ @ingroup TramSim
 */
struct TramRoute
{
    //==================================================================
private:
    /**
     * @~japanese
     * 路線上の交差点の属性を保持する内部構造体
     *
     * @~english
     * Internal struct storing attributes of intersections on the route
     *
     * @~ @ingroup TramSim
     */
    struct TramRouteInter
    {
    public:
        TramRouteInter(
            Intersection* inter, Intersection* prev, int prevDir,
            TramLaneSide::Type prevSide, Intersection* next,
            int nextDir, TramLaneSide::Type nextSide)
            : _inter(inter),
              _prev(prev),
              _prevDir(prevDir),
              _prevSide(prevSide),
              _next(next),
              _nextDir(nextDir),
              _nextSide(nextSide)
        {
        }
        ~TramRouteInter() {};

    private:
        /**
         * @~japanese 対応する交差点
         * @~english  Corresponding intersection 
         */
        const Intersection* _inter;

        /**
         * @~japanese 路線上の上流の交差点
         * @~english  Upstream intersection on the route
         */
        const Intersection* _prev;

        /**
         * @~japanese 上流側の流入境界方向
         * @~english  Inflow border direction from upstream
         */
        const int _prevDir;

        /**
         * @~japanese 上流側の路線通過位置
         *
         * 境界を交差点の外から見たときの位置
         *
         * @~english  Upstream route passing position
         *
         * Position on the border when viewed from outside intersection
         */
        const TramLaneSide::Type _prevSide;

        /**
         * @~japanese 路線上の下流の交差点
         * @~english  Downstream intersection on the route
         */
        const Intersection* _next;

        /**
         * @~japanese 下流側の流出境界方向
         * @~english  Outflow border direction to downstream
         */
        const int _nextDir;

        /**
         * @~japanese 下流側の路線通過位置
         *
         * 境界を交差点の外から見たときの位置
         *
         * @~english  Downstream route passing position
         *
         * Position on the border when viewed from outside intersection
         */
        const TramLaneSide::Type _nextSide;

        //--------------------------------------------------------------
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        const Intersection* inter() const
        {
            return _inter;
        }

        const Intersection* prev() const
        {
            return _prev;
        }

        int prevDir() const
        {
            return _prevDir;
        }

        TramLaneSide::Type prevSide() const
        {
            return _prevSide;
        }

        const Intersection* next() const
        {
            return _next;
        }

        int nextDir() const
        {
            return _nextDir;
        }

        TramLaneSide::Type nextSide() const
        {
            return _nextSide;
        }

        ///@}
    };

    //==================================================================
public:
    TramRoute()
    {
        _inters.clear();
    }
    ~TramRoute()
    {
        for (auto itr : _inters)
        {
            delete itr;
        }
        _inters.clear();
    }

    /**
     * @~japanese 路線に交差点を追加する
     * @~english  Add intersection to route
     */
    bool createTramRouteInter(
        Intersection* inter, Intersection* prev,
        TramLaneSide::Type prevSide, Intersection* next,
        TramLaneSide::Type nextSide);

    /**
     * @~japanese
     * 通過交差点の集合を @p result_inters に格納する
     *
     * @~english
     * Store the set of intersections passed through in @p result_inters
     */
    void getIntersections(
        std::vector<const Intersection*>& result_inters) const
    {
        for (auto itr : _inters)
        {
            result_inters.emplace_back(itr->inter());
        }
    }

    /**
     * @~japanese
     * 交差点 @p inter の境界方向 @p dir の路面電車コネクタの数を得る
     *
     * @~english
     * Get the number of tram connectors in border direction @p dir
     * of intersection @p inter
     */
    void getNumTramConnectors(
        Intersection* inter, int dir, std::array<int, 3>& result_numIn,
        std::array<int, 3>& result_numOut);

    /**
     * @~japanese
     * 交差点 @p inter において境界方向 @p from から境界方向 @p to に
     * 路線が通過するかどうかを戻す
     * 
     * @~english
     * Return whether the route passes from the border direction @p from
     * to the border direction @p to direction at intersection  @p inter
     */
    bool connects(Intersection* inter, int from, int to);

    /**
     * @~japanese 保持する路線属性を @p out に出力する
     * @~english  Output stored route attributes to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese
     * 路線を構成する交差点の属性の集合
     *
     * @~english
     * Set of attributes of intersections that make up a route
     */
    std::vector<TramRouteInter*> _inters;
};

#endif //__TRAM_LINE_HPP__
#endif //INCLUDE_TRAMS
