/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiPedestrianView.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "GuiPedestrianView.hpp"
#include "DrawerForPedestrian.hpp"
#include "Pedestrian.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../ObjectManager.hpp"
#include "../Vehicle.hpp"
#include "../vis/Visualizer.hpp"
#include <autogl.h>
#include <AmuConverter.hpp>
#include <AmuPoint.hpp>

using namespace std;
using namespace amu::converter;
using namespace amu::geometry;

int  GuiPedestrianView::_showsPedestrianIds      = 0;
int  GuiPedestrianView::_showsPedestrianVelocity = 0;
char GuiPedestrianView::_pedestrianId[16];

int  GuiPedestrianView::_showsSubsections          = 1;
int  GuiPedestrianView::_showsSubnetworks          = 0;
int  GuiPedestrianView::_showsSubsectionAttributes = 0;
int  GuiPedestrianView::_showsSubsectionIds        = 0;
char GuiPedestrianView::_intersectionIdToShowSubIntersection[16];
char GuiPedestrianView::_subIntersectionId[16];
char GuiPedestrianView::_sectionIdToShowSubSection[16];
char GuiPedestrianView::_intersectionIdOfSectionToShowSubSection1[16];
char GuiPedestrianView::_intersectionIdOfSectionToShowSubSection2[16];
char GuiPedestrianView::_subSectionId[16];

//======================================================================
GuiPedestrianView::GuiPedestrianView()
{
    renewFlags();
}

//======================================================================
void ::GuiPedestrianView::renewFlags()
{
    GVManager& gv = AppMates::getGVManager();

    gv.resetFlag("VIS_PEDESTRIAN_ID", (_showsPedestrianIds == 1));
    gv.resetFlag(
        "VIS_PEDESTRIAN_VELOCITY", (_showsPedestrianVelocity == 1));

    gv.resetFlag("VIS_SUBSECTION_SHAPE", (_showsSubsections == 1));
    gv.resetFlag("VIS_SUBNETWORK", (_showsSubnetworks == 1));
    gv.resetFlag(
        "VIS_SUBSECTION_ATTRIBUTE", (_showsSubsectionAttributes == 1));

    gv.resetFlag("VIS_SUBSECTION_ID", (_showsSubsectionIds == 1));
}

//======================================================================
void GuiPedestrianView::makePanel()
{
    AutoGL_AddGroup(" Pedestrian & Subsection ");

    AutoGL_AddComment();
    AutoGL_SetLabel("Pedestrian");
    AutoGL_AddBoolean(&_showsPedestrianIds, "_showsPedestrianIds");
    AutoGL_SetLabel("Show ID");
    AutoGL_AddBoolean(
        &_showsPedestrianVelocity, "_showsPedestrianVelocity");
    AutoGL_SetLabel("Show Velocity");
    AutoGL_AddString(_pedestrianId, "_pedestrianId", 16);
    AutoGL_SetLabel("Pedestrian ID");
    AutoGL_AddCallback(
        searchPedestrianButtonCallback,
        "searchPedestrianButtonCallback");
    AutoGL_SetLabel("Search Pedestrian");
    AutoGL_AddCallback(
        showPedestrianButtonCallback, "showPedestrianButtonCallback");
    AutoGL_SetLabel("Show Pedestrian Status");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Subsection");
    AutoGL_AddBoolean(&_showsSubsections, "_showsSubsections");
    AutoGL_SetLabel("Show Shape");
    AutoGL_AddBoolean(&_showsSubnetworks, "_showsSubnetworks");
    AutoGL_SetLabel("Show Subnetwork");
    AutoGL_AddBoolean(&_showsSubsectionIds, "_showsSubsectionIds");
    AutoGL_SetLabel("Show ID");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("SubIntersection");
    AutoGL_AddString(
        _intersectionIdToShowSubIntersection,
        "_intersectionIdToShowSubIntersection", 16);
    AutoGL_SetLabel("Intersection ID");
    AutoGL_AddString(_subIntersectionId, "_subIntersectionId", 16);
    AutoGL_SetLabel("SubIntersection ID");
    AutoGL_AddCallback(
        showSubIntersectionButtonCallback,
        "showSubIntersectionButtonCallback");
    AutoGL_SetLabel("Show SubIntersection Status");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("SubSection");
    AutoGL_AddString(
        _intersectionIdOfSectionToShowSubSection1,
        "_intersectionIdOfSectionToShowSubSection1", 16);
    AutoGL_SetLabel("Intersection ID 1");
    AutoGL_AddString(
        _intersectionIdOfSectionToShowSubSection2,
        "_intersectionIdOfSectionToShowSubSection2", 16);
    AutoGL_SetLabel("Intersection ID 2");
    AutoGL_AddString(_subSectionId, "_subSectionId", 16);
    AutoGL_SetLabel("SubSection ID");
    AutoGL_AddCallback(
        showSubSectionButtonCallback, "showSubSectionButtonCallback");
    AutoGL_SetLabel("Show SubSection Status");

    AutoGL_AddComment();
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
void GuiPedestrianView::redrawView()
{
    DrawerForPedestrian drawer;
    for (auto itr : AppMates::getObjectManager().pedestrians())
    {
        drawer.draw(*itr);
    }
}

//======================================================================
void GuiPedestrianView::searchPedestrianButtonCallback()
{
    string pedestrianId
        = formatId(_pedestrianId, NUM_FIGURE_FOR_PEDESTRIAN);
    Pedestrian* tmpPedestrian
        = AppMates::getObjectManager().pedestrian(pedestrianId);
    if (!tmpPedestrian)
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find pedestrian[" << pedestrianId << "]"
             << endl;
        return;
    }

    // ビューの中心を着目歩行者に移動する
    // Move the view center to the pedestrian of interest
    AmuPoint pv = tmpPedestrian->location()->position();
    AutoGL_SetViewCenter(pv.x(), pv.y(), pv.z());

    Visualizer::renewFlags();
    Visualizer::clearTemporaryFlags();
    AutoGL_DrawView();
}

//======================================================================
void GuiPedestrianView::showPedestrianButtonCallback()
{
    string pedestrianId
        = formatId(_pedestrianId, NUM_FIGURE_FOR_PEDESTRIAN);
    Pedestrian* tmpPedestrian
        = AppMates::getObjectManager().pedestrian(pedestrianId);
    if (!tmpPedestrian)
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find pedestrian[" << pedestrianId << "]"
             << endl;
        return;
    }

    tmpPedestrian->print(cout);
}

//======================================================================
void GuiPedestrianView::showSubIntersectionButtonCallback()
{
    string intersectionId = formatId(
        _intersectionIdToShowSubIntersection,
        NUM_FIGURE_FOR_INTERSECTION);
    Intersection* tmpInter
        = Visualizer::simulator()->roadMap()->intersection(
            intersectionId);
    if (!tmpInter)
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find intersection[" << intersectionId << "]"
             << endl;
        return;
    }

    string subsecId
        = formatId(_subIntersectionId, NUM_FIGURE_FOR_SUBSECTION);
    auto itr = tmpInter->subLaneBundles().find(subsecId);
    if (itr == tmpInter->subLaneBundles().end())
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find subIntersection[" << subsecId
             << "](@intersection[" << intersectionId << "])" << endl;
        return;
    }

    (*itr).second->print(cout);
}

//======================================================================
void GuiPedestrianView::showSubSectionButtonCallback()
{
    string sectionId = concatIdsInAscendingOrder(
        _intersectionIdOfSectionToShowSubSection1,
        _intersectionIdOfSectionToShowSubSection2,
        NUM_FIGURE_FOR_INTERSECTION);
    Section* tmpSec
        = Visualizer::simulator()->roadMap()->section(sectionId);
    if (!tmpSec)
    {
        cerr << "WARNING(" << __FUNCTION__ << "): cannot find section["
             << sectionId << "]" << endl;
        return;
    }

    string subsecId
        = formatId(_subSectionId, NUM_FIGURE_FOR_SUBSECTION);
    auto itr = tmpSec->subLaneBundles().find(subsecId);
    if (itr == tmpSec->subLaneBundles().end())
    {
        cerr << "WARNING(" << __FUNCTION__
             << "): cannot find subSection[" << subsecId
             << "](@section[" << sectionId << "])" << endl;
        return;
    }

    (*itr).second->print(cout);
}

#endif //INCLUDE_PEDESTRIANS
