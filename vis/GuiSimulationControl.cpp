/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GuiSimulationControl.cpp
 */
#include "GuiSimulationControl.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../RoadMap.hpp"
#include "../Simulator.hpp"
#include <AmuConverter.hpp>
#include <autogl.h>

using namespace std;
using namespace amu::converter;

int GuiSimulationControl::_isIdleEventOn = 0;
int GuiSimulationControl::_poseTime;
int GuiSimulationControl::_unit;
int GuiSimulationControl::_unitForDrawing;
int GuiSimulationControl::_thinsOutDrawingStep;
int GuiSimulationControl::_capturesView;
int GuiSimulationControl::_frameNumber;
int GuiSimulationControl::_outputsTimeSeriesAggregated;
int GuiSimulationControl::_outputsVehicleTimeSeriesDetailed;
#ifdef INCLUDE_PEDESTRIANS
int GuiSimulationControl::_outputsPedestrianTimeSeriesDetailed;
#endif //INCLUDE_PEDESTRIANS
int GuiSimulationControl::_outputsSignalTimeSeriesDetailed;
int GuiSimulationControl::_outputsTrafficCounterAggregated;
int GuiSimulationControl::_outputsTrafficCounterDetailed;
int GuiSimulationControl::_outputsLinkFlowMonitorAggregated;
int GuiSimulationControl::_outputsLinkFlowMonitorDetailed;
int GuiSimulationControl::_outputsInflowMonitor;
int GuiSimulationControl::_outputsConvoyMonitor;
int GuiSimulationControl::_outputsVehicleTrip;

//==============================================================================
GuiSimulationControl::GuiSimulationControl()
{
    if (AppMates::getGVManager().getMaxTime() < INT_MAX)
    {
        _poseTime = AppMates::getGVManager().getMaxTime();
    }
    else
    {
        _poseTime = INT_MAX;
    }

    _unit           = AppMates::getTimeManager().unit();
    _unitForDrawing = _unit;
    _frameNumber    = 1;

    renewFlags();
}

//==============================================================================
void GuiSimulationControl::renewFlags()
{
    GVManager& gv = AppMates::getGVManager();

    gv.resetFlag("FLAG_OUTPUT_TIMELINE_S", (_outputsTimeSeriesAggregated == 1));

    gv.resetFlag(
        "FLAG_OUTPUT_TIMELINE_VEHICLE_D",
        (_outputsVehicleTimeSeriesDetailed == 1));
#ifdef INCLUDE_PEDESTRIANS
    gv.resetFlag(
        "FLAG_OUTPUT_TIMELINE_PEDESTRIAN_D",
        (_outputsPedestrianTimeSeriesDetailed == 1));
#endif //INCLUDE_PEDESTRIANS
    gv.resetFlag(
        "FLAG_OUTPUT_TIMELINE_SIGNAL_D",
        (_outputsSignalTimeSeriesDetailed == 1));

    gv.resetFlag(
        "FLAG_OUTPUT_TRAFFIC_COUNTER_S",
        (_outputsTrafficCounterAggregated == 1));
    gv.resetFlag(
        "FLAG_OUTPUT_TRAFFIC_COUNTER_D", (_outputsTrafficCounterDetailed == 1));

    gv.resetFlag(
        "FLAG_OUTPUT_LINK_FLOW_MONITOR_S",
        (_outputsLinkFlowMonitorAggregated == 1));
    gv.resetFlag(
        "FLAG_OUTPUT_LINK_FLOW_MONITOR_D",
        (_outputsLinkFlowMonitorDetailed == 1));

    gv.resetFlag("FLAG_OUTPUT_INFLOW_MONITOR", (_outputsInflowMonitor == 1));

    gv.resetFlag("FLAG_OUTPUT_CONVOY_MONITOR", (_outputsConvoyMonitor == 1));

    gv.resetFlag("FLAG_OUTPUT_TRIP_INFO", (_outputsVehicleTrip == 1));
}

//==============================================================================
void GuiSimulationControl::makePanel()
{
    AutoGL_AddGroup(" Simulator Control ");

    AutoGL_AddComment();
    AutoGL_SetLabel(" Time Flow Control ");
    AutoGL_AddCallback(incrementButtonCallback, "incrementButtonCallback");
    AutoGL_SetLabel(" Time Increment ");
    AutoGL_AddCallback(
        autoIncrementButtonCallback, "autoIncrementButtonCallback");
    AutoGL_SetLabel("Auto Time Increment");

    AutoGL_AddCallback(runButtonCallback, "runButtonCallback");
    AutoGL_SetLabel("Continuous Run");
    AutoGL_AddInteger(&_poseTime, "_poseTime");
    AutoGL_SetLabel("Time To Pose");

    AutoGL_AddInteger(&_unitForDrawing, "_unitForDrawing");
    AutoGL_SetLabel("Skip Time Duration");
    AutoGL_AddBoolean(&_thinsOutDrawingStep, "thinsOutDrawingStep");
    AutoGL_SetLabel("Enable Skip Drawing");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Output File");

    AutoGL_AddBoolean(
        &_outputsTimeSeriesAggregated, "_outputsTimeSeriesAggregated");
    AutoGL_SetLabel("Aggregated Time Series Record");
    AutoGL_AddBoolean(
        &_outputsVehicleTimeSeriesDetailed,
        "_outputsVehicleTimeSeriesDetailed");
    AutoGL_SetLabel("Detailed Vehicle Time Series Record");
#ifdef INCLUDE_PEDESTRIANS
    AutoGL_AddBoolean(
        &_outputsPedestrianTimeSeriesDetailed,
        "_outputsPedestrianTimeSeriesDetailed");
    AutoGL_SetLabel("Detailed Pedestrian Time Series Record");
#endif //INCLUDE_PEDESTRIANS
    AutoGL_AddBoolean(
        &_outputsSignalTimeSeriesDetailed, "_outputsSignalTimeSeriesDetailed");
    AutoGL_SetLabel("Detailed Signal Time Series Record");

    AutoGL_AddBoolean(
        &_outputsTrafficCounterAggregated, "_outputsTrafficCounterAggregated");
    AutoGL_SetLabel("Aggregated Traffic Counter Record");
    AutoGL_AddBoolean(
        &_outputsTrafficCounterDetailed, "_outputsTrafficCounterDetailed");
    AutoGL_SetLabel("Detailed Traffic Counter Record");

    AutoGL_AddBoolean(
        &_outputsLinkFlowMonitorAggregated,
        "_outputsLinkFlowMonitorAggregated");
    AutoGL_SetLabel("Aggregated Link Traffic Flow Record");
    AutoGL_AddBoolean(
        &_outputsLinkFlowMonitorDetailed, "_outputsLinkFlowMonitorDetailed");
    AutoGL_SetLabel("Detailed Link Traffic Flow Record");

    AutoGL_AddBoolean(&_outputsInflowMonitor, "_outputsInflowMonitor");
    AutoGL_SetLabel("Inflow Record");

    AutoGL_AddBoolean(&_outputsConvoyMonitor, "_outputsConvoyMonitor");
    AutoGL_SetLabel("Vehicle Convoy Record");

    AutoGL_AddBoolean(&_outputsVehicleTrip, "_outputsVehicleTrip");
    AutoGL_SetLabel("Trip Record");

    AutoGL_AddBoolean(&_capturesView, "_capturesView");
    AutoGL_SetLabel("Captured Image");

    AutoGL_AddComment();
    AutoGL_SetPanelForSave();

    AutoGL_AddComment();
    AutoGL_AddCallback(
        Visualizer::drawButtonCallback, "Visualizer::drawButtonCallback");
    AutoGL_SetLabel("Draw");
    AutoGL_AddCallback(
        Visualizer::quitButtonCallback, "Visualizer::quitButtonCallback");
    AutoGL_SetLabel("Quit");
}

//==============================================================================
void GuiSimulationControl::incrementButtonCallback()
{
    if (Visualizer::simulator()->failsLaneCheck())
    {
        return;
    }

    Visualizer::renewFlags();
    AppMates::getTimeManager().setUnit(_unit);

    Visualizer::simulator()->incrementStep();

    AutoGL_DrawView();
    _saveImage();
}


//==============================================================================
void GuiSimulationControl::runButtonCallback()
{
    if (Visualizer::simulator()->failsLaneCheck())
    {
        return;
    }

    Visualizer::renewFlags();
    AppMates::getTimeManager().setUnit(_unit);

    while (AppMates::getTimeManager().time() < static_cast<ulint>(_poseTime))
    {
        _incrementStepInside();
    }
}

//==============================================================================
void GuiSimulationControl::autoIncrementButtonCallback()
{
    if (_isIdleEventOn)
    {
        AutoGL_SetIdleEventCallback(0);
        _isIdleEventOn = false;
    }
    else
    {
        if (Visualizer::simulator()->failsLaneCheck())
        {
            return;
        }

        Visualizer::renewFlags();
        AppMates::getTimeManager().setUnit(_unit);

        AutoGL_SetIdleEventCallback(_incrementStepInside);
        _isIdleEventOn = true;
    }
}

//==============================================================================
void GuiSimulationControl::_incrementStepInside()
{
    if (!_thinsOutDrawingStep)
    {
        Visualizer::simulator()->incrementStep();
    }
    else
    {
        /*
         * 1ステップ進めるたびに描画の判定を行うのでなく， 次の描画が
         * 必要な時刻までシミュレーションを進めた後に描画する
         *
         * Instead of judging drawing each time advanced by one step,
         * draw after advancing the simulation to the time when the
         * next drawing is required.
         */
        ulint timeOfNextDrawing
            = AppMates::getTimeManager().time() + _unitForDrawing;
        while (AppMates::getTimeManager().time() < timeOfNextDrawing)
        {
            Visualizer::simulator()->incrementStep();
        }
    }

    AutoGL_DrawView();
    _saveImage();
}

//==============================================================================
void GuiSimulationControl::_saveImage()
{
    if (!_capturesView)
    {
        return;
    }

    string fname = AppMates::getGVManager().getString("RESULT_IMG_DIRECTORY");
    /// @todo "6"のマクロ化
    fname += formatId(to_string(_frameNumber), 6) + ".ppm";
    _frameNumber++;
    AutoGL_SaveViewImageToPPMFile(fname.c_str());
}
