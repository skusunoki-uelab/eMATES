/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file GuiRoutingTest.cpp
 */
#include "GuiRoutingTest.hpp"
#include "DrawerForRouting.hpp"
#include "Visualizer.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Route.hpp"
#include "../RouterAStarHierarchy.hpp"
#include "../RouterBase.hpp"
#include "../RouterManager.hpp"
#include "../RoutingRecorder.hpp"
#include "../VehicleGlobalRoute.hpp"
#include "../VehicleType.hpp"
#include <AmuConverter.hpp>
#include <AmuStringOperator.hpp>
#include <autogl.h>
#include <cstring>

using namespace std;
using namespace amu::converter;
using namespace amu::string_operator;

int GuiRoutingTest::_showsRoutingNetwork = 0;
int GuiRoutingTest::_routingNetworkRank  = 0;

int GuiRoutingTest::_drawingModeForRoutingNode = 0;
int GuiRoutingTest::_drawingModeForRoutingLink = 0;

char GuiRoutingTest::_startId[16];
char GuiRoutingTest::_nextId[16];
char GuiRoutingTest::_goalId[16];
char GuiRoutingTest::_gateId[256];
char GuiRoutingTest::_prefRank[16];

RoutingRecorder* GuiRoutingTest::_routingRecorder;
int              GuiRoutingTest::_showsRoutingRecord;
unsigned long    GuiRoutingTest::_recordCount = 0;

//==============================================================================
GuiRoutingTest::GuiRoutingTest()
{ //=
    _routingRecorder = new RoutingRecorder();
    renewFlags();
}

//==============================================================================
GuiRoutingTest::~GuiRoutingTest()
{
    if (_routingRecorder)
    {
        delete _routingRecorder;
    }
}

//==============================================================================
void GuiRoutingTest::renewFlags()
{
    GVManager& gv = AppMates::getGVManager();

    gv.resetFlag("VIS_ROUTING_NETWORK", (_showsRoutingNetwork == 1));
    gv.resetNumeric(
        "VIS_ROUTING_NODE_PROP_MODE", (double)_drawingModeForRoutingNode);
    gv.resetNumeric(
        "VIS_ROUTING_LINK_PROP_MODE", (double)_drawingModeForRoutingLink);
}

//==============================================================================
void GuiRoutingTest::clearTemporaryFlags()
{
    _showsRoutingRecord = false;
}

//==============================================================================
void GuiRoutingTest::makePanel()
{
    AutoGL_AddGroup(" Routing View ");
    AutoGL_AddComment();
    AutoGL_SetLabel("Network");
    AutoGL_AddBoolean(&_showsRoutingNetwork, "_showsRoutingNetwork");
    AutoGL_SetLabel("Show Network for Routing");
    AutoGL_AddInteger(&_routingNetworkRank, "_routingNetworkRank");
    AutoGL_SetLabel("Network Rank to be Shown");
    AutoGL_AddInteger(
        &_drawingModeForRoutingNode, "_drawingModeForRoutingNode");
    AutoGL_SetLabel("Draw Node Property");
    AutoGL_AddIntegerItem("Disable");
    AutoGL_AddIntegerItem("ID");
    AutoGL_AddIntegerItem("Rank");
    AutoGL_AddIntegerItem("AllowList");
    AutoGL_AddIntegerItem("DenyList");
    AutoGL_AddInteger(
        &_drawingModeForRoutingLink, "_drawingModeForRoutingLink");
    AutoGL_SetLabel("Draw Link Property");
    AutoGL_AddIntegerItem("Disable");
    AutoGL_AddIntegerItem("ID");
    AutoGL_AddIntegerItem("Length");
    AutoGL_AddIntegerItem("Time");
    AutoGL_AddIntegerItem("Straight");
    AutoGL_AddIntegerItem("Left");
    AutoGL_AddIntegerItem("Right");
    AutoGL_AddIntegerItem("AllowList");
    AutoGL_AddIntegerItem("DenyList");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Routing Test");
    AutoGL_AddString(_startId, "_startId", 16);
    AutoGL_SetLabel("Origin");
    AutoGL_AddString(_nextId, "_nextId", 16);
    AutoGL_SetLabel("Next");
    AutoGL_AddString(_goalId, "_goalId", 16);
    AutoGL_SetLabel("Destination");
    AutoGL_AddString(_gateId, "_gateId", 256);
    AutoGL_SetLabel("Intersections to Pass");
    AutoGL_AddString(_prefRank, "_prefRank", 16);
    AutoGL_SetLabel("Preferred Network Rank");

    AutoGL_AddCallback(searchRouteButtonCallback, "searchRouteButtonCallback");
    AutoGL_SetLabel("Search Route");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_SetLabel("Routing Record View");
    AutoGL_AddCallback(
        incrementRecordButtonCallback, "incrementRecordButtonCallback");
    AutoGL_SetLabel("Increment Routing Record");
    AutoGL_AddCallback(
        decrementRecordButtonCallback, "decrementRecordButtonCallback");
    AutoGL_SetLabel("Decrement Routing Record");
    AutoGL_AddCallback(resetRecordButtonCallback, "resetRecordButtonCallback");
    AutoGL_SetLabel("Reset Routing Record View");

    AutoGL_AddComment();
    AutoGL_AddComment();
    AutoGL_AddCallback(
        Visualizer::drawButtonCallback, "Visualizer::drawButtonCallback");
    AutoGL_SetLabel("Draw");
    AutoGL_AddCallback(
        Visualizer::quitButtonCallback, "Visualizer::quitButtonCallback");
    AutoGL_SetLabel("Quit");
}

//==============================================================================
void GuiRoutingTest::searchRouteButtonCallback()
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // _startId，_nextId，_goalId は必須
    // _startId, _nextId, and _goalId are required.
    string        startId = formatId(_startId, NUM_FIGURE_FOR_INTERSECTION);
    Intersection* start
        = Visualizer::simulator()->roadMap()->intersection(startId);
    if (!start)
    {
        cerr << "ERROR(" << __FUNCTION__ << "): origin intersection[" << startId
             << "] is not found." << endl;
        return;
    }

    string        nextId = formatId(_nextId, NUM_FIGURE_FOR_INTERSECTION);
    Intersection* next
        = Visualizer::simulator()->roadMap()->intersection(nextId);
    if (!next)
    {
        cerr << "ERROR(" << __FUNCTION__ << "): next intersection[" << nextId
             << "] is not found." << endl;
        return;
    }

    string        goalId = formatId(_goalId, NUM_FIGURE_FOR_INTERSECTION);
    Intersection* goal
        = Visualizer::simulator()->roadMap()->intersection(goalId);
    if (!goal)
    {
        cerr << "ERROR(" << __FUNCTION__ << "): goal intersection[" << goalId
             << "] is not found." << endl;
        return;
    }

    // start->nextが接続しているかどうか確認
    // Check whether start->next are connected.
    const Section* section = start->nextSection(next);
    if (!section || (section->lanesFrom(start)).size() == 0)
    {
        cerr << "ERROR(" << __FUNCTION__ << "): section from intersection["
             << startId << "] to intersection[" << nextId << "] is not found."
             << endl;
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // start，経由地，goal をゲートとして登録
    // Register start, intersections to pass, and goal as gates
    VehicleGlobalRoute globalRoute;
    globalRoute.addGate(start);

    string         str = _gateId;
    vector<string> tokens;
    getAdjustString(&str);
    getTokens(&tokens, str, ',');
    for (auto itr : tokens)
    {
        // 空欄の処理を防ぐ
        // Prevent processing of blank field
        if (itr.size() == 0)
        {
            continue;
        }

        string        gateId = formatId(itr, NUM_FIGURE_FOR_INTERSECTION);
        Intersection* gate
            = Visualizer::simulator()->roadMap()->intersection(gateId);
        if (!gate)
        {
            cerr << "WARNING(" << __FUNCTION__ << "): intersection[" << gateId
                 << "] to be passed is not found." << endl;
            continue;
        }
        globalRoute.addGate(gate);
    }
    globalRoute.addGate(goal);
    tokens.clear();

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 経路選択パラメータ
    // Routing parameters
    double params[VEHICLE_ROUTING_PARAMETER_SIZE];
    params[0] = 1.0;
    for (unsigned int i = 1; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        params[i] = 0.0;
    }

    // 選好するネットワークランク
    // Preferred network rank
    int prefRank;
    if (strcmp(_prefRank, "") == 0)
    {
        prefRank = INT_MAX;
    }
    else
    {
        prefRank = max(1, atoi(_prefRank));
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 経路探索器の取得と設定
    // Get and set a router
    RouterManager&    rm     = AppMates::getRouterManager();
    RouterBase*       router = rm.assignRouter();
    const VehicleType type;
    router->setVehicleType(&type);
    router->setWeights(params);
    if (dynamic_cast<RouterAStarHierarchy*>(router))
    {
        dynamic_cast<RouterAStarHierarchy*>(router)->setPreferredNetworkRank(
            prefRank);
    }
    router->setRandomNumberGenerator(Visualizer::randomNumberGenerator());
    _routingRecorder->resetLog();
    router->setRecorder(_routingRecorder);

    // 経路探索の実行
    // Execute routing

    /// @todo globalRouteここまで不要では？
    vector<const Intersection*> gates;
    globalRoute.getGatesToPass(gates);
    AppMates::getClockerManager().startClock("ROUTING_USER");
    Route route = router->search(start, next, gates);
    AppMates::getClockerManager().stopClock("ROUTING_USER");
    _recordCount = 0;

    // 経路探索結果の表示
    // Show routing result
    globalRoute.setRoute(route);
    _routingRecorder->print(cout);
    if (!(globalRoute.route().isValid()))
    {
        cout << "route search failed." << endl;
        globalRoute.print(cout);
    }
    else
    {
        cout << "search result:" << endl;
        globalRoute.print(cout);
    }
    AppMates::getClockerManager().findClock("ROUTING_USER")->print();

    router->setRecorder(nullptr);
    rm.releaseRouter(router);
}

//==============================================================================
void GuiRoutingTest::redrawView()
{
    if (_showsRoutingNetwork)
    {
        _drawRoutingNetwork(_routingNetworkRank);
    }

    if (_showsRoutingRecord && _routingRecorder->isRecorded())
    {
        DrawerForRouting drawer;
        drawer.drawRecord(*_routingRecorder, _recordCount);
    }
}

//==============================================================================
void GuiRoutingTest::incrementRecordButtonCallback()
{
    if (_recordCount >= _routingRecorder->log().size() - 1)
    {
        return;
    }
    _recordCount++;
    _showRecord();
}

//==============================================================================
void GuiRoutingTest::decrementRecordButtonCallback()
{
    if (_recordCount < 1)
    {
        return;
    }
    _recordCount--;
    _showRecord();
}

//==============================================================================
void GuiRoutingTest::resetRecordButtonCallback()
{
    _recordCount = 0;
    _showRecord();
}

//==============================================================================
void GuiRoutingTest::_drawRoutingNetwork(int rank)
{
    DrawerForRouting drawer;
    RouterManager&   routerManager = AppMates::getRouterManager();

    if (rank >= 0)
    {
        const RoutingNetwork* routingNetwork
            = routerManager.routingNetwork(rank);
        if (!routingNetwork)
        {
            return;
        }
        drawer.drawNetwork(*routingNetwork);
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            const RoutingNetwork* routingNetwork
                = routerManager.routingNetwork(i);
            if (!routingNetwork)
            {
                continue;
            }
            drawer.drawNetwork(*routingNetwork);
        }
    }
}

//==============================================================================
void GuiRoutingTest::_showRecord()
{
    _showsRoutingRecord = 1;

    Visualizer::renewFlags();
    AutoGL_DrawView();
}
