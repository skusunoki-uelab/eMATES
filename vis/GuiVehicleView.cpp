/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiVehicleView.cpp
 */
#include "GuiVehicleView.hpp"
#include "DrawerForVehicle.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../Vehicle.hpp"
#include <autogl.h>

using namespace std;

int    GuiVehicleView::_vehicleColorShowingMode  = 0;
int    GuiVehicleView::_velocityColorShowingMode = 0;
double GuiVehicleView::_velocityRateThreshold    = 0.5;
int    GuiVehicleView::_showsVehicleIds          = 0;
int    GuiVehicleView::_showsSOC                 = 0;

//======================================================================
GuiVehicleView::GuiVehicleView()
{
    renewFlags();
}

//======================================================================
void GuiVehicleView::renewFlags()
{
    GVManager& gv = AppMates::getGVManager();

    gv.resetFlag("VIS_VEHICLE_ID", (_showsVehicleIds == 1));
    gv.resetFlag("VIS_VEHICLE_SOC", (_showsSOC == 1)); // [eMATES]
    gv.resetNumeric(
        "VIS_VEHICLE_COLOR_MODE", (double)_vehicleColorShowingMode);
    gv.resetNumeric(
        "VIS_VELOCITY_COLOR_MODE", (double)_velocityColorShowingMode);
    gv.resetNumeric(
        "VIS_VELOCITY_RATE_THRESHOLD", _velocityRateThreshold);
}

//======================================================================
void GuiVehicleView::makePanel()
{
    AutoGL_AddGroup(" Vehicle View ");

    AutoGL_AddComment();
    AutoGL_SetLabel("Vehicle");
    AutoGL_AddBoolean(&_showsVehicleIds, "_showsVehicleIds");
    AutoGL_SetLabel("Show ID");
    // 2025/03/27 eMATESモードのみコントロールを表示させる [eMATES]
    GVManager& gv = AppMates::getGVManager();
    if (gv.getFlag("FLAG_GEN_EV"))
    {
      AutoGL_AddBoolean(&_showsSOC, "_showsSOC");
      AutoGL_SetLabel("Show SOC");
    }
    AutoGL_AddInteger(
        &_vehicleColorShowingMode, "_vehicleColorShowingMode");
    AutoGL_SetLabel("Vehicle Color Mode");
    AutoGL_AddIntegerItem("vehicle family");
    AutoGL_AddIntegerItem("average speed");
    AutoGL_AddIntegerItem("hybrid");
    AutoGL_AddInteger(
        &_velocityColorShowingMode, "_velocityColorShowingMode");
    AutoGL_SetLabel("Speed-based Coloring");
    AutoGL_AddIntegerItem("binary");
    AutoGL_AddIntegerItem("gradation");
    AutoGL_AddReal(&_velocityRateThreshold, "_velocityRateThreshold");
    AutoGL_SetLabel("Speed Rate Threshold");

    AutoGL_AddComment();
    AutoGL_AddCallback(
        Visualizer::drawButtonCallback,
        "Visualizer::drawButtonCallback");
    AutoGL_SetLabel("Draw");
    AutoGL_AddCallback(
        Visualizer::quitButtonCallback,
        "Visualizer::quitButtonCallback");
    AutoGL_SetLabel("Quit");
}

//======================================================================
void GuiVehicleView::redrawView()
{
    // drawSimple 用の図形サイズを決定
    // Determine shape size for drawSimple
    double size = 0.02 * AutoGL_GetViewSize();

    DrawerForVehicle drawer;
    for (auto itr : AppMates::getObjectManager().vehicles())
    {
        if (!(itr->isAwayFromOriginNode()))
        {
            continue;
        }
        if (AppMates::getGVManager().getFlag("VIS_SIMPLE_MAP")
            || AppMates::getGVManager().getFlag("VIS_NO_MAP"))
        {
            drawer.drawSimple(*itr, size);
        }
        else
        {
            drawer.draw(*itr);
        }
    }
}
