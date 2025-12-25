/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Route.cpp
 */
#include "Route.hpp"
#include "Intersection.hpp"

using namespace std;

//======================================================================
const Intersection* Route::next(const Intersection* inter) const
{
    assert(inter);
    return next(inter, 0);
}

//======================================================================
const Intersection* Route::next(
    const Intersection* inter, int offset) const
{
    assert(inter);
    offset = max(offset, 0);

    for (unsigned int i = offset; i < _intersections.size() - 1; i++)
    {
        if (_intersections[i] == inter)
        {
            return _intersections[i + 1];
        }
    }
    return nullptr;
}

//======================================================================
const Intersection* Route::next(
    const Intersection* rear, const Intersection* front) const
{
    assert(rear && front);
    return next(rear, front, 0);
}

//======================================================================
const Intersection* Route::next(
    const Intersection* rear, const Intersection* front,
    int offset) const
{
    assert(rear && front);
    offset = max(offset, 0);

    for (unsigned int i = offset; i < _intersections.size() - 2; i++)
    {
        if (_intersections[i] == front && _intersections[i + 1] == rear)
        {
            return _intersections[i + 2];
        }
    }
    return nullptr;
}

//======================================================================
void Route::print(ostream& out) const
{
    for (auto itr : _intersections)
    {
        out << "\t" << itr->id() << endl;
    }
}
