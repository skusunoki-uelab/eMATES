/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionTramExt.cpp
 */
#ifdef INCLUDE_TRAMS
#include "IntersectionTramExt.hpp"
#include "BorderTram.hpp"
#include "TramLaneInIntersection.hpp"
#include "TramRouteManager.hpp"
#include "../AppMates.hpp"
#include "../Config.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../Section.hpp"
#include <AmuConverter.hpp>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;
using namespace amu::converter;

//======================================================================
IntersectionTramExt::IntersectionTramExt(Intersection* inter)
    : _inter(inter){}

//======================================================================
void IntersectionTramExt::setNumTramConnectors()
{
    assert(_inter);

    // vectorの要素の初期化
    // Initializing vector components
    _numInTramLanes.resize(_inter->numNexts());
    _numOutTramLanes.resize(_inter->numNexts());
    for (int dir=0; dir<_inter->numNexts(); dir++)
    {
        fill(_numInTramLanes[dir].begin(),
             _numInTramLanes[dir].end(),
             0);
        fill(_numOutTramLanes[dir].begin(),
             _numOutTramLanes[dir].end(),
             0);
    }

    // 設定の反映
    // Apply settings
    TramRouteManager& tramRouteManager
        = AppMates::getTramRouteManager();
    for (int dir=0; dir<_inter->numNexts(); dir++)
    {
        tramRouteManager.getNumTramConnectors(
            _inter, dir, _numInTramLanes[dir], _numOutTramLanes[dir]);
    }
}

//======================================================================
int IntersectionTramExt::tramDirection(const Connector* connector) const
{
    int dir = -1;
    for (int i=0; i<_inter->numNexts(); i++)
    {
        BorderTram* borderTram
            = dynamic_cast<BorderTram*>
            (const_cast<Border*>(_inter->border(i)));
        if (!borderTram)
        {
            continue;
        }
        if (borderTram->inPointTram() == connector
            || borderTram->outPointTram() == connector)
        {
            dir = i;
            break;
        }
    }
    return dir;
}

//======================================================================
const Connector* IntersectionTramExt::edgeTramConnector(int idInt)
{
    // 境界方向
    // Border direction
    int dir = idInt/100;

    if (_inter->crosswalkWidth(dir) != 0)
    {
        /*
         * 境界に横断歩道が設置されている場合は内部コネクタを戻す
         *
         * Return internal connector if a crosswalk is installed at the
         * border
         */
        return _internalTramConnectors[formatId
                                       (to_string(idInt),
                                        NUM_FIGURE_FOR_CONNECTOR_LOCAL)];
    }
    else
    {
        // 横断歩道が設置されていない場合
        // In the case that a crosswalk is not installed
        BorderTram* borderTram
            = dynamic_cast<BorderTram*>(
                const_cast<Border*>(_inter->border(dir)));
        if (!borderTram)
        {
            return nullptr;
        }
        else
        {
            int idInt2 = idInt%100
                - (_inter->numIn(dir) + _inter->numOut(dir));
            return borderTram->tramConnector(idInt2);
        }
    }
}

#endif //INCLUDE_TRAMS
