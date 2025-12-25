/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoadMapBuilder.hpp
 */
#ifndef __ROADMAP_BUILDER_HPP__
#define __ROADMAP_BUILDER_HPP__
#include "IntersectionBuilder.hpp"
#include "LaneBundleBuilder.hpp"
#include "SectionBuilder.hpp"
#include "../RoadMap.hpp"
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @defgroup IO
 * @~japanese 入出力を扱うモジュール
 * @~english  Input and output
 */

//##############################################################################
/**
 * @~japanese RoadMapを作成する
 * @~english  Build RoadMap
 * @~ @ingroup Initialization IO RoadNetwork
 */
class RoadMapBuilder
{
public:
    RoadMapBuilder()
    {
        _roadMap = nullptr;
        _iBuilders.clear();
        _sBuilders.clear();
    }
    ~RoadMapBuilder();

    /**
     * @~japanese
     * IntersectionとSection，およびそれらの内部構造を生成する
     *
     * @note
     * 具体的な処理はIntersectionBuildDirector，SectionBuildDirectorに
     * まとめた
     *
     * @~english
     * Build Intersections and Sections and their internal structure
     *
     * @note
     * Concrete processing is summarized in IntersectionBuildDirector
     * and SectionBuildDirector.
     */
    RoadMap* buildRoadMap();

    /**
     * @~japanese 信号を生成する
     *
     * @note
     * 具体的な処理はSignalBuildDirectorにまとめた
     *
     * @~english  Build signals
     *
     * @note
     * Concrete processing is summarized in SignalBuildDirector
     */
    bool buildSignals();

    /**
     * @~japanese Intersectionの属性を付与する
     *
     * 通行規制を読み込む
     * 
     * @~english  Add Intersection property
     *
     * Read traffic regulation
     */
    bool setIntersectionProperty();

    /**
     * @~japanese Sectionの属性を付与する
     *
     * 制限速度，通行規制，経路として選択される確率を読み込む
     * 
     * @~english  Add Section property
     *
     * Read speed limit, traffic regulation, and probability being selected as a
     * route
     */
    bool setSectionProperty();

    //==========================================================================
    /**
     * @~japanese @name 部分クラスに関する操作
     * @~english  @name Operations on part classes
     */
    ///@{
public:
    /**
     * @~japanese 交差点builderオブジェクト @p builder を追加する
     * @~english  Add intersection builder object @p builder
     */
    void addIntersectionBuilder(
        const std::string& key, IntersectionBuilder* builder);

    /**
     * @~japanese キー @p key を持つ交差点builderオブジェクトを戻す 
     * @~english  Return the intersection builder object with @p key
     */
    IntersectionBuilder* intersectionBuilder(const std::string& key)
    {
        return _iBuilders[key];
    }

    /**
     * @~japanese 交差点 @p inter のbuilderオブジェクトを戻す
     * @~english  Returns the builder object of the intersection @p inter
     */
    IntersectionBuilder* intersectionBuilder(const Intersection* inter)
    {
        return intersectionBuilder(inter->id());
    }

    /**
     * @~japanese 単路部builderオブジェクト @p builder を追加する
     * @~english  Add section builder object @p builder
     */
    void addSectionBuilder(const std::string& key, SectionBuilder* builder);

    /**
     * @~japanese キー @p key を持つ単路部builderオブジェクトを戻す 
     * @~english  Return the section builder object with @p key
     */
    SectionBuilder* sectionBuilder(const std::string& key)
    {
        return _sBuilders[key];
    }

    /**
     * @~japanese 単路部 @p section のbuilderオブジェクトを戻す
     * @~english  Returns the builder object of @p section
     */
    SectionBuilder* sectionBuilder(const Section* section)
    {
        return sectionBuilder(section->id());
    }

    ///@}

protected:
    /**
     * @~japanese 作成対象のRoadMap
     *
     * @attention
     * Simulatorに渡されるためこのクラスでdeleteしてはならない
     * 
     * @~english  RoadMap to be built
     *
     * @attention
     * It will be transferred to Simulator, so do not delete it in this class.
     */
    RoadMap* _roadMap;

    /**
     * @~japanese 個々の交差点のbuilderオブジェクト
     * @~english  Builder objects for individual intersections
     */
    std::unordered_map<std::string, IntersectionBuilder*> _iBuilders;

    /**
     * @~japanese 個々の単路部のbuilderオブジェクト
     * @~english  Builder objects for individual sections
     */
    std::unordered_map<std::string, SectionBuilder*> _sBuilders;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::unordered_map<std::string, IntersectionBuilder*>&
    intersectionBuilders()
    {
        return _iBuilders;
    }

    const std::unordered_map<std::string, SectionBuilder*>& sectionBuilders()
    {
        return _sBuilders;
    }

    ///@}
};

#endif //__ROADMAP_BUILDER_HPP__
