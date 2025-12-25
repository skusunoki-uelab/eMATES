/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiMapView.hpp
 */
#ifndef __GUI_MAP_VIEW_HPP__
#define __GUI_MAP_VIEW_HPP__

//######################################################################
/**
 * @~japanese 道路環境の描画機能を提供する
 *
 * Visualizer の部分クラス
 *
 * @~english  Provides features of road environment drawing
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiMapView
{
public:


public:
    GuiMapView();
    ~GuiMapView() {}

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
     * @~japanese 地面を描画する
     * @~english  Draw ground
     */
    static void _drawGround();

    /**
     * @~japanese 道路地図を描画する
     * @~english  Draw road map
     */
    static void _drawRoadMap();

    /**
     * @~japanese 観測器を描画する
     * @~english  Draw monitors
     */
    static void _drawMonitors();

public:
    /**
     * @~japanese 地図表示モードの種類
     * @~english  Type of map drawing mode
     */
    enum DrawingTypeForRoadMap
    {
        RM_DETAIL = 0,
        RM_SIMPLE = 1,
        RM_NONE   = 2,
    };

private:
    /**
     * @~japanese 道路地図の描画モード
     * @~english  Drawing mode for road map
     */
    static int _drawingModeForRoadMap;

    /**
     * @~japanese 交差点の識別番号を描画するかどうか [eMATES]
     * @~english  Whether to draw ID numbers of intersections
     */
    static int _showsInterIds;

    /**
     * @~japanese CSの識別番号を描画するかどうか [eMATES]
     * @~english  Whether to draw ID numbers of CS
     */
    static int _showsCSIds;

    /**
     * @~japanese CSの電力消費を描画するかどうか
     * @~english  Whether to draw charge amount of CS
     */
    static int _showsCSValues;

    /**
     * @~japanese 交差点の内部レーンを描画するかどうか
     * @~english  Whether to draw lanes in intersections
     */
    static int _showsLanesInter;

    /**
     * @~japanese 単路部の内部レーンを描画するかどうか
     * @~english  Whether to draw lanes in sections
     */
    static int _showsLanesSection;

    /**
     * @~japanese レーンの識別番号を描画するかどうか
     * @~english  Whether to draw ID number of lanes
     */
    static int _showsLaneIds;

public:
    /**
     * @~japanese コネクタID表示モードの種類
     * @~english  Type of connector ID number drawing mode
     */
    enum DrawingTypeForConnecttorId
    {
        CN_NONE   = 0,
        CN_GLOBAL = 1,
        CN_LOCAL  = 2,
    };

private:
    /**
     * @~japanese コネクタIDの描画モード
     * @~english  Drawing mode for ID numbers of connectors
     */
    static int _drawingModeForConnectorId;

    /**
     * @~japanese 信号を描画するかどうか
     * @~english  Whether to draw traffic lights
     */
    static int _showsSignals;

    /**
     * @~japanese 観測器を描画するかどうか
     * @~english  Whether to draw monitors
     */
    static int _showsMonitors;
};

#endif //__GUI_MAP_VIEW_HPP__
