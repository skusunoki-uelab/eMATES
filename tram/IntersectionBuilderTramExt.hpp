/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionBuilderTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __INTERSECTION_BUILDER_TRAM_EXT_HPP__
#define __INTERSECTION_BUILDER_TRAM_EXT_HPP__
#include "../Intersection.hpp"
#include <AmuLineSegment.hpp>

class RoadMapBuilder;

//######################################################################
/**
 * @~japanese IntersectionBuilder の路面電車拡張
 * @~english  Tram extension of IntersectionBuilder
 * @~ @ingroup TramSim
 */
class IntersectionBuilderTramExt
{
public:
    IntersectionBuilderTramExt(
        Intersection* inter, RoadMapBuilder* roadMapBuilder)
        : _inter(inter), _roadMapBuilder(roadMapBuilder) {};
    virtual ~IntersectionBuilderTramExt() {};

    /**
     * @~japanese
     * 境界以外の線分 @p line 上にコネクタを生成する
     *
     * @~english
     * Generate connectors on non-border line segments @p line
     */
    void generateConnectorsOnLine(
        amu::geometry::AmuLineSegment line, int numIn, int numOut,
        int borderId);

protected:
    /**
     * @~japanese @name generateConnectorsOnLine の内部で用いる関数
     * @~english  @name Functions used inside generateConnectorsOnLine
     */
    ///@
    void _generateVehicleConnectorsOnLine(
        amu::geometry::AmuLineSegment& line, int numIn, int numOut, int sum,
        int borderId, int& idInt);

    void _generateTramConnectorsOnLine(
        amu::geometry::AmuLineSegment& line, int numIn, int numOut, int sum,
        int borderId, int& idInt);
    ///@

    //==================================================================
public:
    /**
     * @~japanese 交差点内の路面電車レーンを生成する
     * @~english  Generate tram lanes in intersection
     */
    virtual bool generateTramLanes();

protected:
    /**
     * @~japanese 中心サブセクションの路面電車レーンを生成する
     * @~english  Generate tram lanes for central subsection
     */
    void _generateDefaultCentralTramLanes(int from, int to);

    /**
     * @~japanese
     * 境界方向 @p dir の横断歩道サブセクションの路面電車レーンを
     * 生成する
     *
     * @~english
     * Generate tram lanes for crosswalk subsection with border
     * direction @p dir
     */
    void _generateCrosswalkTramLanes(int dir);

protected:
    /**
     * @~japanese
     * コネクタ @p pointBegin ， @p pointEnd をそれぞれ始点・終点とする
     * ID @p idInt を持つ路面電車レーンを生成する
     * 
     * @~english
     * Generate a tram lane with ID number @p idInt with connectors
     * @p pointBegin and @p pointEnd as start and end points,
     */
    void _generateTramLane(
        int idInt, const Connector* pointBegin, const Connector* pointEnd);

    //==================================================================
protected:
    /**
     * @~japanese 生成する交差点
     * @~english  Intersection to generate
     */
    Intersection* _inter;

    /**
     * @~japanese 地図のbuilderオブジェクト
     * @~english  Builder object for roadMap
     */
    RoadMapBuilder* _roadMapBuilder;
};

#endif //__INTERSECTION_BUILDER_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS
