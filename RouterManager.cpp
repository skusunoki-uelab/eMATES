/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file RouterManager.cpp
 */
#include "RouterManager.hpp"
#include "AppMates.hpp"
#include "CustomMessage.hpp"
#include "GVManager.hpp"
#include "Intersection.hpp"
#include "RoadMap.hpp"
#include "RouteCacheContainer.hpp"
#include "RouterAStar.hpp"
#include "RouterAStarHierarchy.hpp"
#include "RouterBase.hpp"
#include "RouterDijkstra.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "RoutingLink.hpp"
#include "Section.hpp"
#include "io/RoutingNetworkBuilder.hpp"
#include <cassert>
#include <typeinfo>

using namespace std;

//==============================================================================
RouterManager::RouterManager()
{
    _routingNetworks.clear();
    _highestNetworkRank = 0;
    _updateRequiredLinks.clear();
    _routers.clear();
#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
    _className = typeid(this).name();
}

//==============================================================================
RouterManager::~RouterManager()
{
    deleteAll();
#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
bool RouterManager::getReadyRoutingNetworks()
{
    amu::msg::status(cout, "build network for routing:");

    RoutingNetworkBuilder builder(_roadMap);
    builder.generateRoutingNetworks();
    _highestNetworkRank = _routingNetworks[0]->highestNodeRank();

    return true;
}

//==============================================================================
void RouterManager::addUpdateRequiredLink(const RoutingLink* link)
{
    // 最上位リンクのみを登録する
    // Register only the top link
    if (link->upperLink())
    {
        cout << "Because routingLink[" << link->id() << "] has a upperLink, "
             << "it cannot be added to RouterManager::_updateRequiredLinks."
             << endl;
        exit(EXIT_FAILURE);
    }

    // duplication check
    if (_updateRequiredLinks.count(link->id()))
    {
        return;
    }

    /*
    auto itr
        = find(_updateRequiredLinks.begin(), _updateRequiredLinks.end(), link);
    if (itr != _updateRequiredLinks.end())
    {
        return;
    }
    */

    _updateRequiredLinks.insert(
        make_pair(link->id(), const_cast<RoutingLink*>(link)));
}

//==============================================================================
bool RouterManager::setInitialCosts()
{
    unsigned int highestNodeRank = _routingNetworks[0]->highestNodeRank();
    for (unsigned int i = 0; i <= highestNodeRank; i++)
    {
        _routingNetworks[i]->setInitialCosts();
    }
    return true;
}

//==============================================================================
bool RouterManager::renewCosts()
{
    for (auto itr : _updateRequiredLinks)
    {
        itr.second->renewDynamicCosts();
    }
    _updateRequiredLinks.clear();

    return true;
}

//==============================================================================
RouterBase* RouterManager::assignRouter()
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP
    RouterBase* router = nullptr;

    for (auto itr : _routers)
    {
        // 使用中でないn経路探索器を探す
        // Search router not in use
        if (!(itr->isInUse()))
        {
            router = itr;
        }
    }
    // 使用中でない経路探索器が見つからなかった場合は新規作成する
    // If router not in use is not found, create new one
    if (!router)
    {
        router = new RouterAStarHierarchy();
        router->initialize(_routingNetworks[0]);
        _routers.push_back(router);
    }
    router->setInUseOn();

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP

    return router;
}

//==============================================================================
void RouterManager::releaseRouter(RouterBase* router)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    router->setInUseOff();

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void RouterManager::printRoutingNetwork(ostream& out) const
{
    amu::msg::title(out, "Network for Routing");

    for (unsigned int i = 0; i <= _routingNetworks[0]->highestNodeRank(); i++)
    {
        if (!_routingNetworks[i])
        {
            continue;
        }
        ostringstream ss;
        ss << "RoutingNetwork[" << i << "]" << endl;
        ss << "  Node: " << _routingNetworks[i]->nodes().size() << endl;
        ss << "  Link: " << _routingNetworks[i]->links().size();
        amu::msg::message(out, ss.str());
    }
}
