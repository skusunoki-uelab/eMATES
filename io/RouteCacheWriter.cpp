/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheWriter.cpp
 */
#include "RouteCacheWriter.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RouteCacheContainer.hpp"
#include "../RouterBase.hpp"
#include "../RouterManager.hpp"
#include <iostream>
#include <fstream>
#include <typeinfo>

using namespace std;

//======================================================================
void RouteCacheWriter::writeRouteCache(RoadMap* roadMap)
{
    cout << "RouteIO::writeRouteCache" << endl;
    string fname
        = AppMates::getGVManager().getString("CACHE_ROUTING_FILE");
    ofstream fout(fname.c_str(), ios::out | ios::trunc);
    if (!fout)
    {
        cerr << "no route log file: " << endl << "  " << fname << endl;
        return;
    }

    /*
     * 出力情報は経路探索器のタイプに依存するため，1行目に記録する
     * 
     * Output data depends on the route searcher type, so record it
     * in line 1.
     */
    RouterManager& rm     = AppMates::getRouterManager();
    RouterBase*    router = rm.assignRouter();
    fout << typeid(*router).name() << endl;

    for (auto itr : roadMap->intersections())
    {
        itr.second->routeCacheContainer()->printStatic(fout);
    }

    rm.releaseRouter(router);
    fout.close();
}
