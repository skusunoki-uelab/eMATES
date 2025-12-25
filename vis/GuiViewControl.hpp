/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiViewControl.hpp
 */
#ifndef __GUI_VIEW_CONTROL_HPP__
#define __GUI_VIEW_CONTROL_HPP__

//######################################################################
/**
 * @~japanese ビューのサイズや視線方向など全般を制御する
 *
 * Visualizer の部分クラス
 *
 * @~english  General control of view size, gazing direction, etc.
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiViewControl
{
public:
    GuiViewControl();
    ~GuiViewControl() {};

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese ビューの制御変数を出力する 
     * @~english  Print view control variables
     */
    static void printViewingParamsButtonCallback();

    /**
     * @~japanese ビューの制御変数を更新する 
     * @~english  Update view control variables
     */
    static void setViewingParamsButtonCallback();

protected:
    /**
     * @~japanese @name ビューの設定用変数
     * @~english  @name Variables for view setting
     */
    ///@{
    static double _svpSize;
    static double _svpPositionX;
    static double _svpPositionY;
    static double _svpPositionZ;
    static double _svpDirectionX;
    static double _svpDirectionY;
    static double _svpDirectionZ;
    static double _svpUpVectorX;
    static double _svpUpVectorY;
    static double _svpUpVectorZ;
    ///@}
};

#endif //__GUI_VIEW__CONTROL_HPP__
