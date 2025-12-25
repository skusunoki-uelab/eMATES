/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiVehicleView.hpp
 */
#ifndef __GUI_VEHICLE_VIEW_HPP__
#define __GUI_VEHICLE_VIEW_HPP__

//######################################################################
/**
 * @~japanese 自動車の描画機能を提供する
 *
 * Visualizer の部分クラス
 *
 * @~english  Provides features of vehicle drawing
 *
 * A part of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiVehicleView
{
public:
    GuiVehicleView();
    ~GuiVehicleView() {}

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

private:
    /**
     * @~japanese 自動車を描画する
     * @~english  Draw vehicles
     */
    static void _drawVehicles();

public:
    /**
     * @~japanese 車両色表示モードの種類
     * @~english  Type of vehicle drawing mode
     */
    enum VehicleColorShowingType
    {
        VEH_COL_VEHICLE_FAMILY = 0,
        VEH_COL_MEAN_SPEED     = 1,
        VEH_COL_HYBRID         = 2,
    };

private:
    /**
     * @~japanese 車両色表示モード
     * @~english  Vehicle color drawing mode
     */
    static int _vehicleColorShowingMode;

public:
    /**
     * @~japanese 平均速度で着色する場合の描画モードの種類
     * @~english  Type of drawing mode when coloring at mean speed
     */
    enum VelocityColorShowingType
    {
        VEL_COL_BINARY    = 0,
        VEL_COL_GRADATION = 1,
    };

private:
    /**
     * @~japanese 平均速度で着色する場合の描画モード
     * @~english  Drawing mode when coloring at mean speed
     */
    static int _velocityColorShowingMode;

    /**
     * @~japanese 希望速度に対する走行速度の割合の閾値
     * @~english  Threshold for rate of driving speed to desired speed
     */
    static double _velocityRateThreshold;

    /**
     * @~japanese 識別番号を描画するかどうか
     * @~english  Whether to draw ID numbers
     */
    static int _showsVehicleIds;

    // SOCを描画するかどうか [eMATES]
    // ガソリン車はフラグのいかんに関わらず表示しない
    static int _showsSOC;
};

#endif //__VEHICLE_VIEW_HPP__
