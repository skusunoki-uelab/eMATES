/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForSection.hpp
 */
#ifndef __DRAWER_FOR_SECTION_HPP__
#define __DRAWER_FOR_SECTION_HPP__
#include "../Section.hpp"

//######################################################################
/**
 * @~japanese 単路部を描画する
 * @~english  Draw a section
 * @~ @ingroup Drawing
 */
class DrawerForSection
{
public:
    DrawerForSection() {};
    ~DrawerForSection() {};

    /**
     * @~japanese 単路部 @p section を描画する
     * @~english  Draw @p section
     */
    void draw(const Section& section) const;

    /**
     * @~japanese 単路部 @p section をサイズ @p size で簡易描画する
     * @~english  Simply draw @p section with size @p size
     */
    void drawSimple(const Section& section, double size) const;

private:
    /**
     * @~japanese サブセクションを描画する
     * @~english  Draw subsections
     */
    void _drawSubsections(const Section& section) const;

    /**
     * @~japanese レーンを描画する
     * @~english  Draw lanes
     */
    void _drawLanes(const Section& section) const;

    /**
     * @~japanese コネクタを描画する
     * @~english  Draw connectors
     */
    void _drawConnectors(const Section& section) const;
};

#endif //__DRAWER_FOR_SECTION_HPP__
