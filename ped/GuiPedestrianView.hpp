/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiPedestrianView.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __GUI_PEDESTRIAN_VIEW_HPP__
#define __GUI_PEDESTRIAN_VIEW_HPP__

//######################################################################
/**
 * @~japanese 自動車の描画機能を提供する
 *
 * Visualizer の部分クラス
 *
 * @~english  Provides features of pedestrian drawing
 *
 * A part of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiPedestrianView
{
public:
    GuiPedestrianView();
    ~GuiPedestrianView() {}

    /**
     * @~japanese フラグを更新する
     * @~english  Update flags
     */
    static void renewFlags();

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese ビューを再描画する
     * @~english  Redraw the view
     */
    static void redrawView();

    /**
     * @~japanese 指定した識別番号を持つ歩行者を探す
     *
     * Search Pedestrian ボタンが押されたときの動作
     *
     * @~english  Find the pedestrian with the specified ID number
     *
     * The action when Search Pedestrian button is pressed. 
     */
    static void searchPedestrianButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ歩行者の状態を標準出力に出力する
     *
     * Show Pedestrian Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the pedestrian with the specified ID number
     * to stdout.
     *
     * The action when Show Pedestrian Status button is pressed.
     */
    static void showPedestrianButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ交差点サブセクションの状態を標準出力に
     * 出力する．
     *
     * Show SubIntersection Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the subsection in intersection with the
     * specified ID number to stdout.
     *
     * The action when Show SubIntersection Status button is pressed.
     */
    static void showSubIntersectionButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ単路部サブセクションの状態を標準出力に
     * 出力する．
     *
     * Show SubSection Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the subsection in section with the specified
     * ID number to stdout.
     *
     * The action when Show SubSection Status button is pressed.
     */
    static void showSubSectionButtonCallback();

    //==================================================================
private:
    /**
     * @~japanese 歩行者の識別番号を描画するかどうか
     * @~english  Whether to draw pedestrian ID numbers
     */
    static int _showsPedestrianIds;

    /**
     * @~japanese 歩行者の速度ベクトルを描画するか
     * @~english  Whether to draw pedestrian velocity vector
     */
    static int _showsPedestrianVelocity;

    /**
     * @~japanese 出力あるいは検索される歩行者の識別番号
     * @~english  Pedestrian ID number to be output or found.
     */
    static char _pedestrianId[16];

    //==================================================================
    /**
     * @~japanese サブセクションを表示するかどうか
     * @~english  Whether to draw subsections
     */
    static int _showsSubsections;

    /**
     * @~japanese サブネットワークを表示するかどうか
     * @~english  Whether to draw sub-network
     */
    static int _showsSubnetworks;

    /**
     * @~japanese サブセクションの属性を描画するかどうか
     * @~english  Whether to draw subsection attributes
     */
    static int _showsSubsectionAttributes;

    /**
     * @~japanese サブセクションの識別番号を描画するかどうか
     * @~english  Whether to draw ID numbers of subsections
     */
    static int _showsSubsectionIds;

    /**
     * @~japanese
     * 状態出力の対象であるサブセクションを含む交差点の識別番号
     *
     * @~english
     * ID number of the intersection containing the subsection whose
     * status is to be output  
     */
    static char _intersectionIdToShowSubIntersection[16];

    /**
     * @~japanese
     * 状態出力される交差点サブセクションの識別番号
     *
     * @~english
     * ID number of the subsection in intersection whose status is
     * to be output
     */
    static char _subIntersectionId[16];

    /**
     * @~japanese
     * 状態出力の対象であるサブセクションを含む単路部の識別番号
     *
     * @~english
     * ID number of the section containing the subsection whose status
     * is to be output
     */
    static char _sectionIdToShowSubSection[16];

    /**
     * @~japanese
     * @name 状態出力の対象であるサブセクションを含む単路を指定する
     * ための交差点の識別番号
     *
     * @~english
     * @name ID numbers of the intersections to specify the section
     * containing the subsection whose status is to be output
     */
    ///@{
    static char _intersectionIdOfSectionToShowSubSection1[16];
    static char _intersectionIdOfSectionToShowSubSection2[16];
    ///@}

    /**
     * @~japanese
     * 状態出力される単路部サブセクションの識別番号
     *
     * @~english
     * Id number of the subsection in the section whose status is
     * to be output
     */
    static char _subSectionId[16];
};

#endif //__PEDESTRIAN_VIEW_HPP__
#endif //INCLUDE_PEDESTRIANS
