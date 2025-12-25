/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiObjectSearch.hpp
 */
#ifndef __GUI_OBJECT_SEARCH_HPP__
#define __GUI_OBJECT_SEARCH_HPP__

class Intersection;
class Section;
class Vehicle;

//######################################################################
/**
 * @~japanese シミュレーション中のオブジェクトを検索する
 *
 * Visualizer の部分クラス
 *
 * @~english  Find objects in simulation
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiObjectSearch
{
public:
    GuiObjectSearch();
    ~GuiObjectSearch() {};

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese 指定した識別番号を持つ車両を探す
     *
     * Search Vehicle ボタンが押されたときの動作
     *
     * @~english  Find the vehicle with the specified ID number
     *
     * The action when Search Vehicle button is pressed. 
     */
    static void searchVehicleButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ車両の状態を標準出力に出力する
     *
     * Show Vehicle Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the vehicle with the specified ID number to
     * stdout.
     *
     * The action when Show Vehicle Status button is pressed.
     */
    static void showVehicleButtonCallback();

    /**
     * @~japanese 指定した識別番号の交差点を探す
     *
     * Search Intersection ボタンが押されたときの動作．
     *
     * @~english  Find the intersection with the specified ID number
     *
     * The action when Search Intersection button is pressed.
     */
    static void searchIntersectionButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ交差点の状態を標準出力に出力する
     *
     * Show Intersection Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the intersection with the specified ID
     * number to stdout.
     *
     * The action when Search Intersection Status button is pressed.
     */
    static void showIntersectionButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ交差点内レーンの状態を標準出力に出力する
     *
     * Show Intersection Lane Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the lane in the intersection with the
     * specified ID number to stdout
     *
     * The action when Show Intersection Lane Status button is pressed.
     */
    static void showIntersectionLaneButtonCallback();

    /**
     * @~japanese 指定した識別番号の単路部を探す
     *
     * Search Section ボタンが押されたときの動作
     *
     * @~english  Find the section with the specified ID number
     *
     * The action when Search Section button is pressed.
     */
    static void searchSectionButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ単路部の状態を標準出力に出力する
     *
     * Show Intersection Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the section with the specified ID
     * number to stdout.
     *
     * The action when Search Section Status button is pressed.
     */
    static void showSectionButtonCallback();

    /**
     * @~japanese
     * 指定した識別番号を持つ単路部内レーンの状態を標準出力に出力する．
     *
     * Show Section Lane Status ボタンが押されたときの動作．
     *
     * @~english
     * Output the status of the lane in the section with the specified
     * ID number to stdout.
     *
     * The action when Show Section Lane Status button is pressed.
     */
    static void showSectionLaneButtonCallback();

private:
    /**
     * @~japanese
     * 指定された識別番号を持つ車両へのポインタを戻す
     *
     * @~english
     * Return pointer to the vehicle with the specified ID number
     */
    static Vehicle* _searchVehicle();

    /**
     * @~japanese
     * 指定された識別番号を持つ交差点へのポインタを戻す
     *
     * @~english
     * Return pointer to the intersection with the specified ID number
     */
    static Intersection* _searchIntersection();

    /**
     * @~japanese
     * 指定された識別番号を持つ単路部へのポインタを戻す
     *
     * @~english
     * Return pointer to the section with the specified ID number
     */
    static Section* _searchSection();

protected:
    /**
     * @~japanese 出力あるいは検索される車両の識別番号
     * @~english  Vehicle ID number to be output or found.
     */
    static char _vehicleId[16];

    /**
     * @~japanese 出力あるいは検索される交差点の識別番号
     * @~english  Intersection ID number to be output or found
     */
    static char _intersectionId[16];

    /**
     * @~japanese
     * 状態出力される交差点内レーンの識別番号
     *
     * @~english
     * ID number of the lane in the intersection whose status is to be
     * output 
     */
    static char _laneIdInIntersection[16];

    /**
     * @~japanese
     * @name 状態出力の対象であるレーンを含む単路を指定するための
     * 交差点の識別番号
     *
     * @~english
     * @name ID numbers of the intersections to specify the section
     * containing the lane whose status is to be output
     */
    ///@{
    static char _intersectionIdOfSection1[16];
    static char _intersectionIdOfSection2[16];
    ///@}

    /**
     * @~japanese
     * 状態出力される単路部内レーンの識別番号
     *
     * @~english
     * ID number of the lane in the section whose status is to be output 
     */
    static char _laneIdInSection[16];
};

#endif //__GUI_OBJECT_SEARCH_HPP__
