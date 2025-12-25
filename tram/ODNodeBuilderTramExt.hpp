/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ODNodeBuilderTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __OD_NODE_BUILDER_TRAM_EXT_HPP__
#define __OD_NODE_BUILDER_TRAM_EXT_HPP__
#include "IntersectionBuilderTramExt.hpp"
#include "../Intersection.hpp"

class RoadMapBuilder;

//######################################################################
/**
 * @~japanese ODNodeBuilder の路面電車拡張
 * @~english  Tram extension of ODNodeBuilder
 * @~ @ingroup TramSim
 */
class ODNodeBuilderTramExt : public IntersectionBuilderTramExt
{
public:
    ODNodeBuilderTramExt(Intersection* inter, RoadMapBuilder* roadMapBuilder)
        : IntersectionBuilderTramExt(inter, roadMapBuilder)
    {
    }
    virtual ~ODNodeBuilderTramExt() {}

    /**
     * @~japanese 交差点内の路面電車レーンを生成する
     * @~english  Generate tram lanes in intersection
     */
    virtual bool generateTramLanes() override;
};

#endif //__OD_NODE_INTERNAL_BUILDER_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS
