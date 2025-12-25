/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SectionBuildDirector.hpp
 */
#ifndef __SECTION_BUILD_DIRECTOR_HPP__
#define __SECTION_BUILD_DIRECTOR_HPP__
#include "LaneBundleBuildDirector.hpp"
#include "../RoadMap.hpp"
#include <string>
#include <unordered_map>

class RoadMapBuilder;

//######################################################################
/**
 * @~japanese 単路部の生成を指示する
 * @~english  Direct generation of sections
 * @~
 * @ingroup Initialization IO RoadNetwork
 * @see LaneBundleBuildDirector
 */
class SectionBuildDirector : public LaneBundleBuildDirector
{
public:
    SectionBuildDirector(RoadMap* roadMap, RoadMapBuilder* builder)
        : LaneBundleBuildDirector(roadMap, builder)
    {
    }
    virtual ~SectionBuildDirector() {}

    //==================================================================
    /**
     * @~japanese 交差点の情報から単路部を生成し基本設定を行う
     * @~english  Generate sections from intersection information
     */
    bool buildSections();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese
     * @name buildSections()で呼ばれるprivate関数
     *
     * @~english
     * @name Private functions called in buildSections()
     */
    ///@{
private:
    /**
     * @~japanese 単路部を生成する
     * @~english  Generate sections
     */
    bool _generateSections();

    ///@}

public:
    //==================================================================
    /**
     * @~japanese 単路の内部構造に関する情報を設定する
     *
     * SectionBuilderに情報を格納するのみで，単路部の設定はまだ行わない
     *
     * @~english  Set internal structure information of sections
     *
     * Just store information in SectionBuilder, not configure sections
     * yet.
     */
    bool setInternalInfo();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese
     * @name setInternalInfo()で呼ばれるprivate関数
     *
     * @~english
     * @name Private functions called in setInternalInfo()
     */
    ///@{
private:
    /**
     * @~japanese
     * 単路部のレーン幅，歩道幅と通行権を設定する
     *
     * @~english
     * Set lane width, sidewalk width and right-of-way for sections
     */
    bool _setUpWidthAndPermission(bool isFileRead);

    /// 単路の構造情報をデフォルト値で設定する
    /**
     * setUpStructInfoでファイル指定したあとに呼び出される
     */
    /**
     * @~japanese
     * 単路部の内部構造情報をデフォルト値で設定する
     *
     * @note
     * setInternalInfoでファイル指定しない場合に呼び出される
     *
     * @~english
     * Set internal structure info of section with default value
     *
     * @note
     * Called when no file is specified with setInternalInfo().
     */
    bool _setUpDefaultStructInfo();

    ///@}
};

#endif //__SECTION_BUILD_DIRECTOR_HPP__
