/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterBase.cpp
 */
#include "RouterBase.hpp"
#include "AppMates.hpp"
#include "CSNodeFast.hpp" // [eMATES]
#include "RoadMap.hpp"
#include "RouteCacheContainer.hpp"
#include "RouteKeyBase.hpp"
#include "RoutingRecorder.hpp"
#include "Section.hpp"
#include "Intersection.hpp"
#include "Route.hpp"
#include "RouterManager.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RoutingLink.hpp"
#include "VehicleGlobalRoute.hpp"
#include "VehicleTypeManager.hpp"
#include "GVManager.hpp"
#include <vector>
#include <cassert>

using namespace std;

//======================================================================
RouterBase::RouterBase()
{
    _startNode = nullptr;
    _goalNodes.clear();

    _resetStatus();
    _isInUse  = false;
    _recorder = nullptr;

    _id          = "0";
    _vehicleType = nullptr;
    _rng         = nullptr;
}

//======================================================================
RouterBase::~RouterBase()
{
    if (_recorder)
    {
        delete _recorder;
    }
}

//======================================================================
bool RouterBase::generateRouteCache(
    RoadMap* roadMap, vector<string>& tokens) const
{
    assert(tokens.size() > 5);

    bool         isValidLine = true;
    unsigned int index       = 0;

    // 第1カラムは使用回数
    // First column is number of use
    int count = stoi(tokens[index]);
    index++;

    // 第2カラムは車種ID
    // Second column is vehicle type
    VehicleType type(tokens[index]);
    index++;

    // 第3カラムからは経路探索パラメータ
    // Routing parameters are from the third column
    double weights[VEHICLE_ROUTING_PARAMETER_SIZE];
    for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        weights[i] = stod(tokens[index]);
        index++;
    }

    /*
     * 次のカラムは出発地に至る交差点の識別番号
     *
     * The next column is the Intersection ID number leading to the
     * start point
     */
    const Intersection* past = roadMap->intersection(tokens[index]);
    index++;

    // 次のカラムは出発交差点の識別番号
    // The next column is the start Intersection ID number
    const Intersection* start = roadMap->intersection(tokens[index]);
    index++;

    // 次のカラムは目的交差点の識別番号
    // The next column is the goal Intersection ID number
    const Intersection* goal = roadMap->intersection(tokens[index]);
    index++;

    // 以降のカラムは経路
    // Subsequent columns are route
    Route route;
    for (; index < tokens.size(); index++)
    {
        const Intersection* inter
            = roadMap->intersection(tokens[index]);
        if (!inter)
        {
            isValidLine = false;
            break;
        }
        route.addIntersection(inter);
    }

    if (!past || !start || !goal || !isValidLine)
    {
        return false;
    }

    RouteKeyBase* key
        = new RouteKeyBase(type, weights, past, start, goal);

    const_cast<Intersection*>(past)
        ->routeCacheContainer()
        ->addRouteCacheDirectly(key, route, count);

    return true;
}

//======================================================================
Route RouterBase::search(
    const Intersection* from, const Intersection* next,
    const vector<const Intersection*>& gates)
{
    /*
     * 最初の探索で用いる startNode は from から next への Section
     * である．Section の下流側 (next) から経路探索を開始する．
     * 厳密には startNode から出る RoutingLink から探索を開始し，
     * 最後に RoutingLink に含まれる Intersection を戻すことになる．
     * 戻される Intersection のvectorの先頭は next であり from でない
     * ので，まずは from のみ最初に route に格納しておく．
     *
     * The startNode used in the first search is Section from "from"
     * to "next". Start routing from the downstream side (next) of
     * Section. To be exact, the routing starts from the RoutingLink
     * that exits from startNode, and finally the Intersection included
     * in the RoutingLink is returned. Since the head of the returned
     * Intersection vector is "next" and not "from", only "from" is
     * first stored in "route".
     */
    Route route;
    route.addIntersection(from);

    // gatesによって複数回の探索に分割される
    // Divided int multiple searches by gates
    for (int i = 0; i < static_cast<int>(gates.size()) - 1; i++)
    {
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 探索の始点に至る交差点
        // Intersection leading to the start of the routing
        const Intersection* leadIntersection
            = _getLeadIntersection(i, from, route);

        // 探索の始点
        // Start intersection of the routing
        const Intersection* startIntersection
            = _getStartIntersection(i, next, route, gates);

        // [eMATES] 非通過型CSでの方向転換処理
        // 高速化のため、最初にtypeidで弾く
        // NOTE 将来的に、通過型CSでは、方向転換あり・なしの両方を探索したい
        if (typeid(*startIntersection) == typeid(CSNodeFast))
        {
            const CSNodeFast* cs = static_cast<const CSNodeFast*>(startIntersection);
            if (cs->deadend())
            {
                leadIntersection = startIntersection->nextAnother(leadIntersection);
            }
        }

        // 探索の終点
        // Goal intersection of the routing
        const Intersection* goalIntersection = gates[i + 1];

        // スタートノードの決定と経路キャッシュコンテナの取得
        // Determine start node and route cache container
        _findStartNode(leadIntersection, startIntersection);
        assert(_startNode);
        RouteCacheContainer* routeCacheContainer
            = const_cast<Intersection*>(leadIntersection)
                  ->routeCacheContainer();

        // キャッシュの利用
        // Use cache
        if (_rng->uniform() < AppMates::getGVManager().getNumeric(
                "VEHICLE_CACHE_ROUTING_PROBABILITY"))
        {
            Route cachedRoute;
            bool  isCacheFound = _loadCache(
                routeCacheContainer, leadIntersection,
                startIntersection, goalIntersection, cachedRoute);

            // キャッシュを利用できた場合は探索を行う必要はない
            // No need to routing if any route caches are available
            if (isCacheFound)
            {
                route.addIntersections(cachedRoute.intersections(), cachedRoute.cost()); // [eMATES] cost追加
                continue;
            }
        }

        // ゴールノードの決定
        // Determine goal nodes
        _findGoalNodes(goalIntersection, gates, i);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // 経路探索の実行
        // Process routing
        vector<RoutingNode*> results;
        double gateCost = _search(results);

        // 探索に失敗した場合
        // The case that routing failed
        if (results.empty())
        {
            cerr << "search route(from:" << gates[i]->id()
                 << ", to:" << gates[i + 1]->id() << ") failed."
                 << endl;
            /*
             * ひとまず手前の交差点とゴールを代入しておく．もちろん
             * このような経路は存在しない．
             *
             * Substitute the intersection "from" and one of goal
             * intersections for now. Of course such a route does not
             * exist.
             */
            route.clearIntersections();
            route.addIntersection(from);
            route.addIntersection(*(gates.rbegin()), DBL_MAX); // [eMATES] cost追加
            route.setIsValid(false);
            // route->print(cout);

            // 属性を元に戻す
            // Reset internal state
            _resetStatus();
            return route;
        }

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // routeに結果を格納する
        // Store the routing result in "route"
        route.setIsValid(true);
        const vector<const Intersection*> intersections
            = _convertNodesToIntersections(results);
        route.addIntersections(intersections, gateCost); // [eMATES] cost追加

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /*
         * この回の探索結果を保存する
         *   route にはこれまでの探索結果がすべて保存されているので注意
         *
         * Save this search result
         *   Note that route stores all search results so far
         */
        Route subRoute;
        subRoute.addIntersections(intersections, gateCost); // [eMATES] cost追加
        _saveCache(
            routeCacheContainer, leadIntersection, startIntersection,
            *(intersections.rbegin()), subRoute);
    }

    // 属性を元に戻す
    // Reset internal state
    _resetStatus();
    return route;
}

//======================================================================
const Intersection* RouterBase::_getLeadIntersection(
    unsigned int n, const Intersection* inter, Route& route)
{
    if (n == 0)
    {
        return inter;
    }
    else
    {
        /*
         * route.intersections() の最後から2番目の要素は，
         * startIntersection に至る直前の Intersection
         *
         * The second element from the end of route.intersections() is
         * the intersection just before startIntersection.
         */
        return route.intersection(route.intersections().size() - 2);
    }
}

//======================================================================
const Intersection* RouterBase::_getStartIntersection(
    unsigned int n, const Intersection* inter, Route& route,
    const std::vector<const Intersection*>& gates)
{
    if (n == 0)
    {
        return inter;
    }
    else
    {
        /*
         * leadIntersection->startIntersection の探索は前回の探索と
         * 今回の探索で重複しているため，前回の結果の末尾を削除
         *
         * Since the search of leadIntersection->startIntersection
         * overlaps between the previous and this searches, delete
         * the end of the previous result.
         */
        if (route.intersections().size() >= 1
            && route.intersection(route.intersections().size() - 1)
                   == gates[n])
        {
            route.removeLastIntersection();
        }

        return gates[n];
    }
}

//======================================================================
void RouterBase::_findStartNode(
    const Intersection* from, const Intersection* next)
{

    const Section* section = from->nextSection(next);
    bool           isUp    = section->isUp(from, next);
    _startNode
        = AppMates::getRouterManager().routingNetwork(0)->convertS2N(
            section, isUp);
}

//======================================================================
void RouterBase::_findGoalNodes(
    const Intersection* goal, const vector<const Intersection*>& gates,
    int gateIndex)
{
    _goalNodes.clear();

    for (int i = 0; i < goal->numNexts(); i++)
    {
        const RoutingNode* goalNode = NULL;

        if (goal->numIn(i) == 0)
        {
            // goalへ進入不可の単路は無視
            // Ignore section that cannot enter the goal
            continue;
        }

        if (gateIndex < static_cast<int>(gates.size()) - 2
            && goal->next(i) == gates[gateIndex + 2])
        {
            /*
             * gates[gateIndex+1](=goal)とgates[gateIndex+2]が隣接する
             * 場合には，この探索の次の，gates[gateIndex+1]をstartとする
             * 探索において，startとgoalを直接接続する単路を選択したい．
             * そのためには，gates[i+1]をgoalとする探索でgates[i+2]->
             * gates[i+1]をゴールとしてはならない．
             *
             * ただし隣接する交差点A, BがA->B->Aのようにgateとして指定
             * されている場合はこの限りではない．先に指定されたA->Bを
             * 優先してそれらを直接接続する単路を選び，B->Aはそれ以外の
             * 単路となる．
             *
             * Uターンを許可する場合はこのif文に条件を加える必要がある
             * かもしれない．
             *
             * If gates[gateIndex+1](=goal) and gates[gateIndex+2] are
             * adjacent to,  want to select the section connecting them
             * directly in the search following this search, starting
             * with gates[gateIndex+1]. For that reason, the section
             * gates[i+2]->gates[i+1] should not be the goal in the
             * search with gates[i+1] as the goal.
             *
             * However, this does not apply if adjacent intersections
             * A and B are specified as gates like A->B->A. Prioritize
             * A->B specified earlier and choose the section connecting
             * them directly, and choose other sections for B->A.
             *
             * It may be necessary to add conditions to the if-statement
             * in the case U-turns are allowed.
             */

            if (goal->next(i) != gates[gateIndex])
            {
                continue;
            }
        }

        const Section* section = goal->nextSection(i);
        goalNode               = AppMates::getRouterManager()
                       .routingNetwork(0)
                       ->convertS2N(
                           section, section->isUp(goal->next(i), goal));
        assert(goalNode);
        _goalNodes.push_back(goalNode);
    }
}

//======================================================================
bool RouterBase::_isNodeIdIncluded(
    const RoutingNode*                node,
    const vector<const RoutingNode*>& container) const
{
    for (auto itr : container)
    {
        if (node->id() == itr->id())
        {
            return true;
        }
    }
    return false;
}

//======================================================================
vector<const Intersection*> RouterBase::_convertNodesToIntersections(
    vector<RoutingNode*>& routingNodes)
{
    vector<const Intersection*> result;
    for (auto itr : routingNodes)
    {
        result.emplace_back(itr->section()->intersection(itr->isUp()));
    }
    return result;
}

//======================================================================
bool RouterBase::_loadCache(
    RouteCacheContainer* container, const Intersection* from,
    const Intersection* next, const Intersection* goal,
    Route& result_route) const
{
    RouteKeyBase key(*_vehicleType, _weights, from, next, goal);
    return container->searchRouteCache(_id, &key, result_route);
}

//======================================================================
void RouterBase::_saveCache(
    RouteCacheContainer* container, const Intersection* from,
    const Intersection* next, const Intersection* goal,
    const Route& route) const
{
    RouteKeyBase* key
        = new RouteKeyBase(*_vehicleType, _weights, from, next, goal);
    container->addRouteCache(_id, key, route);
}

//======================================================================
void RouterBase::_resetStatus()
{
    _weights[0] = 1;
    for (unsigned int i = 1; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
    {
        _weights[i] = 0;
    }
    _id          = "0";
    _vehicleType = nullptr;
    _rng         = nullptr;
}
