/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCacheReader.cpp
 */
#include "RouteCacheReader.hpp"
#include "../AppMates.hpp"
#include "../CustomMessage.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../RoadMap.hpp"
#include "../RouterBase.hpp"
#include "../RouterManager.hpp"
#include <AmuStringOperator.hpp>
#include <iostream>
#include <fstream>
#include <cassert>
#include <map>
#include <sstream>
#include <typeinfo>

using namespace std;
using amu::string_operator::getAdjustString;
using amu::string_operator::getTokens;

//======================================================================
void RouteCacheReader::readRouteCache(RoadMap* roadMap)
{
    GVManager&    gv    = AppMates::getGVManager();
    string        fname = gv.getString("CACHE_ROUTING_FILE");
    ostringstream ss;
    ss << "read route cache file (" << gv.stripDataDir(fname)
       << ") ... ";

    // ファイルを読み込む
    // Load file
    ifstream fin(fname.c_str(), ios::in);
    if (!fin)
    {
        ss << "not found";
        amu::msg::status(cout, ss.str());
        return;
    }
    amu::msg::status(cout, ss.str());

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 1行目に記述された経路探索器のタイプを読み整合性をチェックする
     *   入力情報は経路探索器のタイプに依存するため
     *
     * Check route searcher consistency by reading line 1.
     *   Because input data depends on the route searcher type.
     */
    RouterManager& rm     = AppMates::getRouterManager();
    RouterBase*    router = rm.assignRouter();

    // 1行目の処理
    // Processing line 1
    string str;
    getline(fin, str);
    getAdjustString(&str);
    if (str != typeid(*router).name())
    {
        amu::msg::warn("route cache is invalid for current router.");
        fin.close();
        rm.releaseRouter(router);
        return;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * 2行目以降の処理
     *   行の解釈は経路探索器に依存するため，実質的な処理は
     *   RouterBase::generateRouteComponent()に任せる
     *
     * Processing from the second line onwards
     *   Line interpretation is dependent on route searcher, so actual
     *   processing is left to RouterBase::generateRouteComponent()
     */
    int numFinishedLines = 1;
    while (fin.good())
    {
        getline(fin, str);
        numFinishedLines++;
        if (numFinishedLines % 1000 == 0
            && AppMates::getGVManager().getFlag("FLAG_VERBOSE"))
        {
            ostringstream ssm;
            ssm << "import route cache ... processed "
                << numFinishedLines << " lines";
            amu::msg::status(cout, ssm.str());
        }

        getAdjustString(&str);
        if (str.empty())
        {
            continue;
        }
        vector<string> tokens;
        getTokens(&tokens, str, ',');
        bool result = router->generateRouteCache(roadMap, tokens);

        if (!result)
        {
            ostringstream ssw;
            ssw << "invalid line - " << str;
            amu::msg::warn(ssw.str());
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    rm.releaseRouter(router);
    fin.close();

    ss << "done";
    amu::msg::status(cout, ss.str());
    return;
}
