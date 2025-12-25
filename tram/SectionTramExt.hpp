/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionTramExt.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __SECTION_TRAM_EXT_HPP__
#define __SECTION_TRAM_EXT_HPP__
#include <vector>

class Intersection;
class Lane;
class Section;

//######################################################################
/**
 * @~japanese 単路部の路面電車拡張
 * @~english  Tram extension of section
 * @~ @ingroup TramSim
 */
class SectionTramExt
{
public:
    SectionTramExt() {};
    SectionTramExt(Section* section);
    ~SectionTramExt() {};

    /**
     * @~japanese
     * 交差点 @p inter から流入する路面電車レーンを @p result_lanes に
     * 格納する
     *
     * @~english
     * Store the tram lanes flowing from the intersection @p inter in
     * @p result_lanes
     */
    void getTramLanesFrom(
        std::vector<const Lane*>& result_lanes,
        const Intersection*       inter);

    /**
     * @~japanese
     * 交差点 @p inter に流出する路面電車レーンを @p result_lanes に
     * 格納する
     * 
     * @~english
     * Store the tram lanes flowing into the intersection @p inter in
     * @p result_lanes
     */
    void getTramLanesTo(
        std::vector<const Lane*>& result_lanes,
        const Intersection*       inter);

private:
    /**
     * @~japanese 対応する単路部
     * @~english  Corresponding section
     */
    Section* _section;
};

#endif //__INTERSECTION_TRAM_EXT_HPP__
#endif //INCLUDE_TRAMS
