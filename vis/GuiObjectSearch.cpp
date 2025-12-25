/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiObjectSearch.cpp
 */
#include "GuiObjectSearch.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>
#include <autogl.h>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;

char GuiObjectSearch::_vehicleId[16];
char GuiObjectSearch::_intersectionId[16];
char GuiObjectSearch::_laneIdInIntersection[16];

char GuiObjectSearch::_intersectionIdOfSection1[16];
char GuiObjectSearch::_intersectionIdOfSection2[16];
char GuiObjectSearch::_laneIdInSection[16];

//======================================================================
GuiObjectSearch::GuiObjectSearch() {}

//======================================================================
void GuiObjectSearch::makePanel()
{
    AutoGL_AddGroup(" Object Search ");
    AutoGL_AddComment();
    AutoGL_SetLabel("Vehicle");
    AutoGL_AddString(_vehicleId, "_vehicleId", 16);
    AutoGL_SetLabel("Vehicle ID");
    AutoGL_AddCallback(
        searchVehicleButtonCallback, "searchVehicleButtonCallback");
    AutoGL_SetLabel("Search Vehicle");
    AutoGL_AddCallback(
        showVehicleButtonCallback, "showVehicleButtonCallback");
    AutoGL_SetLabel("Show Vehicle Status");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Intersection");
    AutoGL_AddString(_intersectionId, "_intersectionId", 16);
    AutoGL_SetLabel("Intersection ID");
    AutoGL_AddString(
        _laneIdInIntersection, "_laneIdInIntersection", 16);
    AutoGL_SetLabel("Lane ID");
    AutoGL_AddCallback(
        searchIntersectionButtonCallback,
        "searchIntersectionButtonCallback");
    AutoGL_SetLabel("Search Intersection");
    AutoGL_AddCallback(
        showIntersectionButtonCallback,
        "showIntersectionButtonCallback");
    AutoGL_SetLabel("Show Intersection Status");
    AutoGL_AddCallback(
        showIntersectionLaneButtonCallback,
        "showIntersectionLaneButtonCallback");
    AutoGL_SetLabel("Show Intersection Lane Status");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Section");
    AutoGL_AddString(
        _intersectionIdOfSection1, "_intersectionIdOfSection1", 16);
    AutoGL_SetLabel("Intersection ID 1");
    AutoGL_AddString(
        _intersectionIdOfSection2, "_intersectionIdOfSection2", 16);
    AutoGL_SetLabel("Intersection ID 2");
    AutoGL_AddString(_laneIdInSection, "_laneIdInSection", 16);
    AutoGL_SetLabel("Lane ID");
    AutoGL_AddCallback(
        searchSectionButtonCallback, "searchSectionButtonCallback");
    AutoGL_SetLabel("Search Section");
    AutoGL_AddCallback(
        showSectionButtonCallback, "showSectionButtonCallback");
    AutoGL_SetLabel("Show Section Status");
    AutoGL_AddCallback(
        showSectionLaneButtonCallback, "showSectionLaneButtonCallback");
    AutoGL_SetLabel("Show Section Lane Status");
}

//======================================================================
void GuiObjectSearch::searchVehicleButtonCallback()
{
    Vehicle* tmpVehicle = _searchVehicle();
    if (!tmpVehicle)
    {
        return;
    }

    // ビューの中心を着目車両に移動する
    // Move the view center to the vehicle of interest
    AmuPoint pv = tmpVehicle->location()->position();
    AutoGL_SetViewCenter(pv.x(), pv.y(), pv.z());

    Visualizer::renewFlags();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
}

//======================================================================
void GuiObjectSearch::showVehicleButtonCallback()
{
    Vehicle* tmpVehicle = _searchVehicle();
    if (!tmpVehicle)
    {
        return;
    }

    tmpVehicle->print(cout);
}

//======================================================================
void GuiObjectSearch::searchIntersectionButtonCallback()
{
    Intersection* tmpInter = _searchIntersection();
    if (!tmpInter)
    {
        return;
    }

    // ビューの中心を着目交差点に移動する
    // Move the view center to the intersection of interest
    AutoGL_SetViewCenter(
        tmpInter->center().x(), tmpInter->center().y(),
        tmpInter->center().z());

    Visualizer::renewFlags();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
}

//======================================================================
void GuiObjectSearch::showIntersectionButtonCallback()
{
    Intersection* tmpInter = _searchIntersection();
    if (!tmpInter)
    {
        return;
    }

    tmpInter->print(cout, dynamic_cast<ODNode*>(tmpInter) != nullptr);
}

//======================================================================
void GuiObjectSearch::showIntersectionLaneButtonCallback()
{
    Intersection* tmpInter = _searchIntersection();
    if (!tmpInter)
    {
        return;
    }

    string laneId
        = formatId(_laneIdInIntersection, NUM_FIGURE_FOR_LANE);
    auto itr = tmpInter->lanes().find(laneId);
    if (itr == tmpInter->lanes().end())
    {
        cerr << "WARNING(" << __FUNCTION__ << "): cannot find lane["
             << laneId << "](@intersection[" << tmpInter->id() << "])"
             << endl;
        return;
    }
    (*itr).second->print(cout);
}

//======================================================================
void GuiObjectSearch::searchSectionButtonCallback()
{
    Section* tmpSec = _searchSection();
    if (!tmpSec)
    {
        return;
    }

    // ビューの中心を着目単路部に移動する
    // Move the view center to the section of interest
    AutoGL_SetViewCenter(
        tmpSec->center().x(), tmpSec->center().y(),
        tmpSec->center().z());

    Visualizer::renewFlags();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
}

//======================================================================
void GuiObjectSearch::showSectionButtonCallback()
{
    Section* tmpSec = _searchSection();
    if (!tmpSec)
    {
        return;
    }

    tmpSec->print(cout);
}

//======================================================================
void GuiObjectSearch::showSectionLaneButtonCallback()
{
    Section* tmpSec = _searchSection();
    if (!tmpSec)
    {
        return;
    }

    string laneId = formatId(_laneIdInSection, NUM_FIGURE_FOR_LANE);
    auto   itr    = tmpSec->lanes().find(laneId);
    if (itr == tmpSec->lanes().end())
    {
        cerr << "WARNING(" << __FUNCTION__ << "): cannot find lane["
             << laneId << "](@section[" << tmpSec->id() << "])" << endl;
        return;
    }
    (*itr).second->print(cout);
}

//======================================================================
Vehicle* GuiObjectSearch::_searchVehicle()
{
    string   vehicleId = formatId(_vehicleId, NUM_FIGURE_FOR_VEHICLE);
    Vehicle* tmpVehicle
        = AppMates::getObjectManager().vehicle(vehicleId);
    if (!tmpVehicle)
    {
        cerr << "WARNING(" << __FUNCTION__ << "): cannot find vehicle["
             << vehicleId << "]" << endl;
        return nullptr;
    }
    return tmpVehicle;
}

//======================================================================
Intersection* GuiObjectSearch::_searchIntersection()
{
    string intersectionId
        = formatId(_intersectionId, NUM_FIGURE_FOR_INTERSECTION);
    Intersection* tmpInter
        = Visualizer::simulator()->roadMap()->intersection(
            intersectionId);
    if (!tmpInter)
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find intersection[" << intersectionId << "]"
             << endl;
        return nullptr;
    }
    return tmpInter;
}

//======================================================================
Section* GuiObjectSearch::_searchSection()
{
    string sectionId = concatIdsInAscendingOrder(
        _intersectionIdOfSection1, _intersectionIdOfSection2,
        NUM_FIGURE_FOR_INTERSECTION);
    Section* tmpSec
        = Visualizer::simulator()->roadMap()->section(sectionId);
    if (!tmpSec)
    {
        cerr << "WARNING(" << __FUNCTION__ << "): cannot find section["
             << sectionId << "]" << endl;
        return nullptr;
    }
    return tmpSec;
}
