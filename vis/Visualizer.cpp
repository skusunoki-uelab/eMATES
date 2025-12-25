/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Visualizer.cpp
 */
#include "Visualizer.hpp"
#include "GLColor.hpp"
#include "GuiBackgroundView.hpp"
#include "GuiMapView.hpp"
#include "GuiObjectSearch.hpp"
#include "GuiRoutingTest.hpp"
#include "GuiSimulationControl.hpp"
#include "GuiVehicleGeneration.hpp"
#include "GuiVehicleView.hpp"
#include "GuiViewControl.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../Simulator.hpp"
#include "../TimeManager.hpp"
#ifdef INCLUDE_PEDESTRIANS
#include "../ped/GuiPedestrianView.hpp"
#endif //INCLUDE_PEDESTRIANS
#include <AmuConverter.hpp>
#include <autogl.h>
#include <cassert>
#include <cstdlib>
#include <string>

using namespace std;
using namespace amu::converter;

Simulator*            Visualizer::_sim;
RandomNumberGenerator Visualizer::_rng;

GuiViewControl*       Visualizer::_viewControl;
GuiSimulationControl* Visualizer::_simulationControl;
GuiMapView*           Visualizer::_mapView;
GuiVehicleView*       Visualizer::_vehicleView;
#ifdef INCLUDE_PEDESTRIANS
GuiPedestrianView* Visualizer::_pedestrianView;
#endif //INCLUDE_PEDESTRIANS
GuiBackgroundView*    Visualizer::_backgroundView;
GuiVehicleGeneration* Visualizer::_vehicleGeneration;
GuiRoutingTest*       Visualizer::_routingTest;
GuiObjectSearch*      Visualizer::_objectSearch;

double Visualizer::_xmin = -100.0;
double Visualizer::_xmax = 100.0;
double Visualizer::_ymin = -100.0;
double Visualizer::_ymax = 100.0;

double Visualizer::_viewSize = 100;

double Visualizer::_viewPositionX = 0.0;
double Visualizer::_viewPositionY = 0.0;
double Visualizer::_viewPositionZ = 0.0;

double Visualizer::_viewDirectionX = 0.0;
double Visualizer::_viewDirectionY = 0.0;
double Visualizer::_viewDirectionZ = 1.0;

double Visualizer::_viewUpVectorX = 0.0;
double Visualizer::_viewUpVectorY = 1.0;
double Visualizer::_viewUpVectorZ = 0.0;

//======================================================================
Visualizer::Visualizer()
{
    _sim = nullptr;
    _rng.resetForSim();

    GVManager& gv = AppMates::getGVManager();

    _viewSize = gv.getNumeric("VIS_VIEW_SIZE");

    _viewPositionX = gv.getNumeric("VIS_VIEW_CENTER_X");
    _viewPositionY = gv.getNumeric("VIS_VIEW_CENTER_Y");
    _viewPositionZ = gv.getNumeric("VIS_VIEW_CENTER_Z");

    _viewDirectionX = gv.getNumeric("VIS_VIEW_DIRECTION_X");
    _viewDirectionY = gv.getNumeric("VIS_VIEW_DIRECTION_Y");
    _viewDirectionZ = gv.getNumeric("VIS_VIEW_DIRECTION_Z");

    _viewUpVectorX = gv.getNumeric("VIS_VIEW_UPVECTOR_X");
    _viewUpVectorY = gv.getNumeric("VIS_VIEW_UPVECTOR_Y");
    _viewUpVectorZ = gv.getNumeric("VIS_VIEW_UPVECTOR_Z");

    _xmin = _viewPositionX - _viewSize - 10.0;
    _xmax = _viewPositionX + _viewSize + 10.0;
    _ymin = _viewPositionY - _viewSize - 10.0;
    _ymax = _viewPositionY + _viewSize + 10.0;

    _viewControl       = new GuiViewControl();
    _simulationControl = new GuiSimulationControl();
    _mapView           = new GuiMapView();
    _vehicleView       = new GuiVehicleView();
    _backgroundView    = new GuiBackgroundView();
    _vehicleGeneration = new GuiVehicleGeneration();
    _routingTest       = new GuiRoutingTest();
    _objectSearch      = new GuiObjectSearch();

#ifdef INCLUDE_PEDESTRIANS
    _pedestrianView = new GuiPedestrianView();
#endif //INCLUDE_PEDESTRIANS
}

//======================================================================
Visualizer::~Visualizer()
{
    if (_viewControl)
    {
        delete _viewControl;
    }
    if (_simulationControl)
    {
        delete _simulationControl;
    }
    if (_mapView)
    {
        delete _mapView;
    }
    if (_vehicleView)
    {
        delete _vehicleView;
    }
#ifdef INCLUDE_PEDESTRIANS
    if (_pedestrianView)
    {
        delete _pedestrianView;
    }
#endif //INCLUDE_PEDESTRIANS
    if (_backgroundView)
    {
        delete _backgroundView;
    }
    if (_vehicleGeneration)
    {
        delete _vehicleGeneration;
    }
    if (_routingTest)
    {
        delete _routingTest;
    }
    if (_objectSearch)
    {
        delete _objectSearch;
    }
}

//======================================================================
void Visualizer::startVisualization()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // AutoGL のウィンドウを用意する
    // Prepare AutoGL window
    AutoGL_SetViewSize(_viewSize);
    AutoGL_SetViewCenter(
        _viewPositionX, _viewPositionY, _viewPositionZ);
    AutoGL_SetViewDirection(
        _viewDirectionX, _viewDirectionY, _viewDirectionZ);
    AutoGL_SetViewUpVector(
        _viewUpVectorX, _viewUpVectorY, _viewUpVectorZ);

    // 背景色の設定
    // Set background color
    GLColor::setBackground();

    // コンターマップの用意
    // Prepare a contour map
    AutoGL_ClearContourColor();
    AutoGL_AddContourColorOfGrade(0.0, 1.0, 0.0, 0.0);
    AutoGL_AddContourColorOfGrade(1.0, 0.0, 0.5, 1.0);

    // ドラッグ有効
    // Enable drug
    AutoGL_EnableDragInMode3D();

    // 描画関数を登録する
    // Register drawing functions
    AutoGL_SetDefaultCallbackInMode3D(NULL);
    AutoGL_SetViewRedrawCallback(viewRedrawCallback);
    AutoGL_SetBatchProcessCallback(viewRenderCallback);

    // アイドルイベント処理を有効にする
    // Enable idle event handling
    AutoGL_EnableIdleEvent();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // パネルの設定
    // Set panels
    makePanel();
}

//======================================================================
void Visualizer::renewFlags()
{
    _checkComponents();

    _simulationControl->renewFlags();
    _mapView->renewFlags();
    _vehicleView->renewFlags();
#ifdef INCLUDE_PEDESTRIANS
    _pedestrianView->renewFlags();
#endif //INCLUDE_PEDESTRIANS
    _routingTest->renewFlags();
}

//======================================================================
void Visualizer::clearTemporaryFlags()
{
    _checkComponents();

    _routingTest->clearTemporaryFlags();
}

//======================================================================
void Visualizer::makePanel()
{
    _checkComponents();

    _viewControl->makePanel();
    _simulationControl->makePanel();
    _mapView->makePanel();
    _vehicleView->makePanel();
#ifdef INCLUDE_PEDESTRIANS
    _pedestrianView->makePanel();
#endif //INCLUDE_PEDESTRIANS
    _backgroundView->makePanel();
    _vehicleGeneration->makePanel();
    _routingTest->makePanel();
    _objectSearch->makePanel();
}

//======================================================================
void Visualizer::viewRedrawCallback()
{
    _checkComponents();

    _backgroundView->redrawView();
    _mapView->redrawView();
    _vehicleView->redrawView();
#ifdef INCLUDE_PEDESTRIANS
    _pedestrianView->redrawView();
#endif //INCLUDE_PEDESTRIANS
    _routingTest->redrawView();

    // 現在時刻の表示
    // Display current time
    if (AppMates::getGVManager().getFlag("VIS_SHOW_ANALOG_CLOCK"))
    {
        // drawAnalogClock();
    }
    else
    {
        AutoGL_SetColor(0, 0, 0);
        ulint  presentTime = AppMates::getTimeManager().time() / 1000;
        string timeString
            = formatId(to_string(presentTime), NUM_FIGURE_FOR_DRAW_TIME)
              + "[sec]";
        AutoGL_DrawStringAtScaledPosition(
            0.01, 0.97, timeString.c_str());
    }
}

//======================================================================
void Visualizer::viewRenderCallback() {}

//======================================================================
void Visualizer::drawButtonCallback()
{
    renewFlags();
    AutoGL_DrawView();
}

//======================================================================
void Visualizer::quitButtonCallback()
{
    exit(EXIT_SUCCESS);
}
