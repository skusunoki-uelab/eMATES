/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file IntersectionPedExt.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "IntersectionPedExt.hpp"
#include "../Intersection.hpp"

using namespace std;

//======================================================================
Zebra* IntersectionPedExt::zebra(int dir) const
{
    for (auto itr : _zebras)
    {
        if (itr.second->directionInIntersection() == dir)
        {
            return itr.second;
        }
    }
    return nullptr;
}

#endif //INCLUDE_PEDESTRIANS
