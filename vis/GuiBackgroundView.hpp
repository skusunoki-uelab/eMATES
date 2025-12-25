/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiBackgroundView.hpp
 */
#ifndef __GUI_BACKGROUND_VIEW_HPP__
#define __GUI_BACKGROUND_VIEW_HPP__
#include "BgImage.hpp"

//######################################################################
/**
 * @~japanese 背景画像を操作する
 *
 * Visualizer の部分クラス
 *
 * @~english  Control background image
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiBackgroundView
{
public:
    GuiBackgroundView();
    ~GuiBackgroundView() {};

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese ビューを再描画する
     *
     * _showsBgImage が true の場合は背景テクスチャを貼り付け， false の
     * 場合は地面を単色で描画する．
     *
     * @~english  Redraw the view
     *
     * If _showBgImage is true, the background texture is pasted, if
     * false, the ground is drawn in a single color.
     */
    static void redrawView();

    /**
     * @~japanese 背景画像を設定する
     *
     * Update Background Image ボタンが押されたときの動作．
     *
     * @~english  Set background image
     *
     * The action when Update Background Image button is pressed.
     */
    static void updateBackgroundImageButtonCallback();

    /**
     * @~japanese 背景画像を消去する
     *
     * Clear Background Image ボタンが押されたときの動作．
     *
     * @~english  Clear background image
     *
     * The action when Clear Background Image button is pressed.
     */
    static void clearBackgroundImageButtonCallback();

private:
#ifdef USE_OPENCV
    /**
     * @~japanese 背景画像
     * @~english  Background image
     */
    static BgImage _bgImage;
#endif //USE_OPENCV

    /**
     * @~japanese 背景画像を貼り付ける矩形領域
     * @~english  Rectangular area to paste the background image
     */
    ///@{
    static double _bgXmin;
    static double _bgXmax;
    static double _bgYmin;
    static double _bgYmax;
    ///@}

    /**
     * @~japanese 背景画像ファイル名
     * @~english  Background image file name
     */
    static char _fileName[64];

    /**
     * @~japanese 背景画像を表示するかどうか
     * @~english  Whether to display a background image
     */
    static bool _showsBgImage;

    /**
     * @~japanese 背景画像をグレースケールで表示するか
     * @~english  Whether to display background image in grayscale 
     */
    static int _showsBgGrayscale;

    /**
     * @~japanese アスペクト比を維持するかどうか
     * @~english  Whether to maintain aspect ratio
     */
    static int _keepsAspectRatio;

    /**
     * @~japanese 背景画像の著作権表記
     * @~english  Copyright notice of background image
     */
    static char _creditString[128];

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    static bool showsBgImage()
    {
        return _showsBgImage;
    }

    ///@}
};

#endif //__GUI_BACKGROUND_VIEW_HPP__
