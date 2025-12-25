/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuilderTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __SECTION_BUILDER_TRAM_EXT_HPP__
#define __SECTION_BUILDER_TRAM_EXT_HPP__
#include "../Section.hpp"

class Connector;

//######################################################################
/**
 * @~japanese 路面電車の通過する単路部を生成する
 * @~english  Generate an section for trams to pass through
 * @~ @ingroup TramSim
 */
class SectionBuilderTramExt
{
public:
    SectionBuilderTramExt(Section* section) : _section(section) {}
    ~SectionBuilderTramExt() {};

    /**
     * @~japanese 単路内の路面電車レーンを生成する
     * @~english  Generate tram lanes in section
     */
    bool generateTramLanes();

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
        int idInt, const Connector* pointBegin,
        const Connector* pointEnd);

private:
    /**
     * @~japanese 生成する単路部
     * @~english  Section to generate
     */
    Section* _section;
};

#endif //__SECTION_BUILDER_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS

