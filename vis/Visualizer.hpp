/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Visualizer.hpp
 */
#ifndef __VISUALIZER_HPP__
#define __VISUALIZER_HPP__
#include "../RandomNumberGenerator.hpp"
#include <cassert>
#include <memory>
#include <string>

class AppSim;
class RoutingRecorder;
class Simulator;

class GuiBackgroundView;
class GuiMapView;
class GuiObjectSearch;
class GuiRoutingTest;
class GuiSimulationControl;
class GuiVehicleGeneration;
class GuiVehicleView;
class GuiViewControl;
#ifdef INCLUDE_PEDESTRIANS
class GuiPedestrianView;
#endif //INCLUDE_PEDESTRIANS

/**
 * @defgroup Visualization
 * @ingroup Procedure
 * @~japanese シミュレーションの可視化
 * @~english  Simulation visualization
 */

/**
 * @defgroup Drawing
 * @~japanese AutoGLを用いた描画
 * @~english  Drawing with AutoGL
 */

//######################################################################
/**
 * @~japanese AutoGLを用いた可視化とGUI機能を提供する
 * @~english  Provides visualization and GUI features using AutoGL
 * @~ @ingroup Drawing Visualization
 */
class Visualizer
{
public:
    Visualizer();
    ~Visualizer();

    /**
     * @~japanese 可視化関数およびGUI関数を用意する
     * @~english  Prepare visualization and GUI functions
     */
    static void startVisualization();

    /**
     * @~japanese フラグを更新する
     * @~english  Update flags
     */
    static void renewFlags();

    /**
     * @~japanese 一時フラグをクリアする
     * @~english  Clear temporary flags
     */
    static void clearTemporaryFlags();

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese 描画領域を取得する
     * @~english  Get Drawing area
     */
    static void getRegion(
        double& result_xmin, double& result_xmax, double& result_ymin,
        double& result_ymax)
    {
        result_xmin = _xmin;
        result_xmax = _xmax;
        result_ymin = _ymin;
        result_ymax = _ymax;
    }

private:
    /**
     * @~japanese コンポーネントのインスタンスの存在をチェックする
     * @~english  Check for existence of component instances
     */
    static void _checkComponents()
    {
        assert(
            _viewControl && _simulationControl && _mapView
            && _vehicleView && _backgroundView && _vehicleGeneration
            && _routingTest && _objectSearch);
#ifdef INCLUDE_PEDESTRIANS
        assert(_pedestrianView);
#endif //INCLUDE_PEDESTRIANS
        return;
    }

    //==================================================================
    /**
     * @~japanese @name コールバック関数群
     * @~english  @name Callback functions
     */
    ///@{
public:
    /**
     * @~japanese ビューを再描画する
     * @~english  Redraw the view
     */
    static void viewRedrawCallback();

    /**
     * @~japanese ビューをオフラインでレンダリングする
     * @~english  Render view offline
     */
    static void viewRenderCallback();

    /**
     * Draw button
     */
    static void drawButtonCallback();

    /**
     * Quit button
     */
    static void quitButtonCallback();

    ///@}

    //==================================================================
    /**
     * @~japanese 機能を集約した部分クラス
     * @~english  Part classes aggregating features
     */
    ///@{
private:
    static GuiViewControl*       _viewControl;
    static GuiSimulationControl* _simulationControl;
    static GuiMapView*           _mapView;
    static GuiVehicleView*       _vehicleView;
#ifdef INCLUDE_PEDESTRIANS
    static GuiPedestrianView* _pedestrianView;
#endif //INCLUDE_PEDESTRIANS
    static GuiBackgroundView*    _backgroundView;
    static GuiVehicleGeneration* _vehicleGeneration;
    static GuiRoutingTest*       _routingTest;
    static GuiObjectSearch*      _objectSearch;
    ///@}

    //==================================================================
private:
    /**
     * @~japanese 可視化対象となるシミュレータ
     * @~english  Simulator to be visualized
     */
    static Simulator* _sim;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    static RandomNumberGenerator _rng;

    /**
     * @~japanese @name 地図領域
     * @~english  @name Map area
     */
    ///@{
    static double _xmin;
    static double _xmax;
    static double _ymin;
    static double _ymax;
    ///@}

    /**
     * @~japanese ビューのサイズ
     * 
     * 画面の中心からビューの端までの距離 [m]
     * 
     * @~english  View size
     *
     * Distance from the center to the edge of screen [m] 
     */
    static double _viewSize;

    /**
     * @~japanese ビューの注視点
     * @~english  Gaze point of the view
     */
    ///@{
    static double _viewPositionX;
    static double _viewPositionY;
    static double _viewPositionZ;
    ///@}

    /**
     * @~japanese ビューの視線方向
     * @~english  Gazing direction of the view
     */
    ///@{
    static double _viewDirectionX;
    static double _viewDirectionY;
    static double _viewDirectionZ;
    ///@}

    /**
     * @~japanese ビューの上方向定義ベクトル
     * @~english  View-up vector
     */
    ///@{
    static double _viewUpVectorX;
    static double _viewUpVectorY;
    static double _viewUpVectorZ;
    ///@}

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    static Simulator* simulator()
    {
        return _sim;
    }

    static void setSimulator(Simulator* sim)
    {
        _sim = sim;
    }

    static RandomNumberGenerator* randomNumberGenerator()
    {
        return &_rng;
    }

    static void setViewSize(double size)
    {
        _viewSize = size;
    }

    static void setViewPosition(double x, double y, double z)
    {
        _viewPositionX = x;
        _viewPositionY = y;
        _viewPositionZ = z;
    }

    static void setViewDirection(double x, double y, double z)
    {
        _viewDirectionX = x;
        _viewDirectionY = y;
        _viewDirectionZ = z;
    }

    static void setViewUpVector(double x, double y, double z)
    {
        _viewUpVectorX = x;
        _viewUpVectorY = y;
        _viewUpVectorZ = z;
    }

    ///@}
};

extern std::unique_ptr<AppSim> App;

#endif //__VISUALIZER_HPP__
