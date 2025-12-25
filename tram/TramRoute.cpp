/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file TramRoute.cpp
 */
#ifdef INCLUDE_TRAMS
#include "TramRoute.hpp"
#include "IntersectionTramExt.hpp"
#include "../RoadMap.hpp"
#include "../Intersection.hpp"
#include <algorithm>
#include <iostream>

using namespace std;
using TL = TramLaneSide;

//======================================================================
bool TramRoute::createTramRouteInter(
    Intersection* inter, Intersection* prev, TL::Type prevSide,
    Intersection* next, TL::Type nextSide)
{
    assert(inter);

    TramRouteInter* tramInter = new TramRouteInter(
        inter, (prev ? prev : nullptr),
        (prev ? inter->direction(prev) : -1),
        (prev ? prevSide : TL::None), (next ? next : nullptr),
        (next ? inter->direction(next) : -1),
        (next ? nextSide : TL::None));

    if ((prev && tramInter->prevDir() == -1)
        || (next && tramInter->nextDir() == -1))
    {
        cerr << "ERROR: tram route is invalid." << endl;
        delete tramInter;
        return false;
    }

    _inters.push_back(tramInter);
    return true;
}

//======================================================================
void TramRoute::getNumTramConnectors(
    Intersection* inter, int dir, array<int, 3>& result_numIn,
    array<int, 3>& result_numOut)
{
    for (auto itr : _inters)
    {
        if (itr->inter() != inter)
        {
            continue;
        }
        if (itr->prevDir() == dir)
        {
            switch (itr->prevSide())
            {
            case TL::Left:
                result_numIn[TL::Left] = 1;
                break;
            case TL::Center:
                result_numIn[TL::Center] = 1;
                break;
            case TL::Right:
                result_numIn[TL::Right] = 1;
                break;
            default:
                break;
            }
        }
        if (itr->nextDir() == dir)
        {
            switch (itr->nextSide())
            {
            case TL::Left:
                result_numOut[TL::Left] = 1;
                break;
            case TL::Center:
                result_numOut[TL::Center] = 1;
                break;
            case TL::Right:
                result_numOut[TL::Right] = 1;
                break;
            default:
                break;
            }
        }
    }
}

//======================================================================
bool TramRoute::connects(Intersection* inter, int from, int to)
{
    for (auto itr : _inters)
    {
        if (itr->inter() == inter && itr->prevDir() == from
            && itr->nextDir() == to)
        {
            return true;
        }
    }
    return false;
}

//======================================================================
void TramRoute::print(ostream& out) const
{
    for (auto itr : _inters)
    {
        switch (itr->prevSide())
        {
        case TL::None:
            out << "*/";
            break;
        case TL::Left:
            out << "L/";
            break;
        case TL::Center:
            out << "C/";
            break;
        case TL::Right:
            out << "R/";
            break;
        default:
            out << "?/";
            break;
        }
        out << itr->inter()->id();
        switch (itr->nextSide())
        {
            /*
             * 進行方向の左右に合わせるため，流出境界方向を逆転する
             *
             * Reverse the outflow border direction to mach the left
             * or right of the traveling direction.
             */
        case TL::None:
            out << "/* ";
            break;
        case TL::Left:
            out << "/R-";
            break;
        case TL::Center:
            out << "/C-";
            break;
        case TL::Right:
            out << "/L-";
            break;
        default:
            out << "/?-";
            break;
        }
    }
}

#endif //INCLUDE_TRAMS
