/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiMapView.cpp
 */
#include "GuiMapView.hpp"
#include "DrawerForIntersection.hpp"
#include "DrawerForMonitor.hpp"
#include "DrawerForSection.hpp"
#include "GLColor.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../RoadMap.hpp"
#include <autogl.h>

using namespace std;

int GuiMapView::_drawingModeForRoadMap     = GuiMapView::RM_DETAIL;
int GuiMapView::_showsInterIds             = 0;
int GuiMapView::_showsCSIds                = 0;
int GuiMapView::_showsCSValues             = 0;
int GuiMapView::_showsLanesInter           = 1;
int GuiMapView::_showsLanesSection         = 1;
int GuiMapView::_showsLaneIds              = 0;
int GuiMapView::_drawingModeForConnectorId = GuiMapView::CN_NONE;
int GuiMapView::_showsSignals              = 1;
int GuiMapView::_showsMonitors             = 0;

//======================================================================
GuiMapView::GuiMapView()
{
    renewFlags();
}

//======================================================================
void GuiMapView::renewFlags()
{
    GVManager& gv = AppMates::getGVManager();

    gv.resetFlag(
        "VIS_SIMPLE_MAP",
        (_drawingModeForRoadMap == GuiMapView::RM_SIMPLE));
    gv.resetFlag(
        "VIS_NO_MAP", (_drawingModeForRoadMap == GuiMapView::RM_NONE));

    gv.resetFlag("VIS_INTER_ID", (_showsInterIds == 1));
    gv.resetFlag("VIS_CS_ID", (_showsCSIds == 1));
    gv.resetFlag("VIS_CS_VALUE", (_showsCSValues == 1));
    gv.resetFlag("VIS_LANE_INTER", (_showsLanesInter == 1));
    gv.resetFlag("VIS_LANE_SECTION", (_showsLanesSection == 1));
    gv.resetFlag("VIS_LANE_ID", (_showsLaneIds == 1));

    gv.resetNumeric(
        "VIS_CONNECTOR_ID_MODE", (double) _drawingModeForConnectorId);

    gv.resetFlag("VIS_SIGNAL", (_showsSignals == 1));
    gv.resetFlag("VIS_ROADSIDE_UNIT", (_showsMonitors == 1));
}

//======================================================================
void GuiMapView::makePanel()
{
    AutoGL_AddGroup(" Map View ");

    AutoGL_AddComment();
    AutoGL_SetLabel("Road Map");
    AutoGL_AddInteger(
        &_drawingModeForRoadMap, "_drawingModeForRoadMap");
    AutoGL_SetLabel("Draw Road Map");
    AutoGL_AddIntegerItem("Detail");
    AutoGL_AddIntegerItem("Simple");
    AutoGL_AddIntegerItem("Disable");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Intersection");
    AutoGL_AddBoolean(&_showsInterIds, "_showsInterIds");
    AutoGL_SetLabel("Show ID");

    // 2025/03/27 eMATESモードのみコントロールを表示させる [eMATES]
    GVManager& gv = AppMates::getGVManager();
    if (gv.getFlag("FLAG_GEN_EV"))
    {
      AutoGL_AddBoolean(&_showsCSIds, "_showsCSIds");
      AutoGL_SetLabel("Show CS ID");
      AutoGL_AddBoolean(&_showsCSValues, "_showsCSValues");
      AutoGL_SetLabel("Show CS value");
    }

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Lane");
    AutoGL_AddBoolean(&_showsLanesInter, "_showsLanesInter");
    AutoGL_SetLabel("Show Intersection Lane");
    AutoGL_AddBoolean(&_showsLanesSection, "_showsLanesSection");
    AutoGL_SetLabel("Show Section Lane");
    AutoGL_AddBoolean(&_showsLaneIds, "_showsLaneIds");
    AutoGL_SetLabel("Show ID");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Connector");
    AutoGL_AddInteger(
        &_drawingModeForConnectorId, "_drawingModeForConnectorId");
    AutoGL_SetLabel("Show Connector ID");
    AutoGL_AddIntegerItem("Disable");
    AutoGL_AddIntegerItem("Global ID");
    AutoGL_AddIntegerItem("Local ID");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Signal");
    AutoGL_AddBoolean(&_showsSignals, "_showsSignals");
    AutoGL_SetLabel("Show");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Monitor");
    AutoGL_AddBoolean(&_showsMonitors, "_showsMonitors");
    AutoGL_SetLabel("Show");

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
void GuiMapView::redrawView()
{
    // 地面は GuiBackgroundView で描画する
    // Draw the ground in GuiBackgroundView.

    // 道路地図の描画
    // Draw road map
    if (_drawingModeForRoadMap != GuiMapView::RM_NONE)
    {
        _drawRoadMap();
    }

    // 観測器の描画
    // Draw monitors
    if (_showsMonitors)
    {
        _drawMonitors();
    }
}

//======================================================================
void GuiMapView::_drawGround()
{
    double xmin, xmax, ymin, ymax;
    Visualizer::getRegion(xmin, xmax, ymin, ymax);

    GLColor::setGround();
    AutoGL_DrawQuadrangle(
        xmin, ymin, -2.0, xmax, ymin, -2.0, xmax, ymax, -2.0, xmin,
        ymax, -2.0);
}

//======================================================================
void GuiMapView::_drawRoadMap()
{
    RoadMap*              roadMap = Visualizer::simulator()->roadMap();
    DrawerForIntersection iDrawer;
    DrawerForSection      sDrawer;

    // 通常の地図描画
    // Normal map drawing
    if (!AppMates::getGVManager().getFlag("VIS_SIMPLE_MAP"))
    {
        for (auto itr : roadMap->intersections())
        {
            iDrawer.draw(*(itr.second));
        }

        for (auto itr : roadMap->sections())
        {
            sDrawer.draw(*(itr.second));
        }
    }
    // シンプルな地図描画
    // Simple map drawing
    else
    {
        double linkWidth = 0.002 * AutoGL_GetViewSize();

        for (auto itr : roadMap->intersections())
        {
            iDrawer.drawSimple(*(itr.second), linkWidth);
        }

        for (auto itr : roadMap->sections())
        {
            sDrawer.drawSimple(*(itr.second), linkWidth);
        }
    }
}

//======================================================================
void GuiMapView::_drawMonitors()
{
    DrawerForMonitor drawer;
    for (auto itr : AppMates::getObjectManager().trafficCounters())
    {
        drawer.drawTrafficCounter(*itr);
    }
}
