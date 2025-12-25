/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouteCache.cpp
 */
#include "RouteCache.hpp"
#include "RouteKeyBase.hpp"
#include "Intersection.hpp"
#include <AmuConverter.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <typeinfo>

using namespace std;
using namespace amu::converter;

//======================================================================
RouteCache::RouteCache()
{
    _count = 0;
    _key   = NULL;
    _route.clearIntersections();
    _isStatic = false;
}

//======================================================================
RouteCache::~RouteCache()
{
    if (_key)
    {
        delete _key;
    }
}

//======================================================================
void RouteCache::makeCache(const RouteKeyBase* key, const Route& route)
{
    _key = key;
    if (_key->weight(toUnderlying(RoutingParamIndex::TIME)) < 1.0e-6)
    {
        _isStatic = true;
    }
    _route = route;
}

//======================================================================
bool RouteCache::hasEqualKey(RouteKeyBase* key) const
{
    if (typeid(*_key) != typeid(*key))
    {
        return false;
    }
    return _key->equals(key);
}

//======================================================================
bool RouteCache::equals(RouteKeyBase* key, const Route& route) const
{
    if (!hasEqualKey(key))
    {
        return false;
    }
    return (_route == route);
}

//======================================================================
void RouteCache::print(ostream& out) const
{
    out << _count << ",";
    _key->print(out);
    for (auto itr : _route.intersections())
    {
        out << "," << itr->id();
    }
    out << endl;
}
