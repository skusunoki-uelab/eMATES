/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file SubIntersection.hpp
 */
#ifndef __SUB_INTERSECTION_HPP__
#define __SUB_INTERSECTION_HPP__
#include "SubLaneBundle.hpp"

//######################################################################
/**
 * @~japanese 交差点サブセクション
 *
 * 交差点を細分化するためのクラス．SubLaneBundleを継承している．
 *
 * @~english  Subsection in intersection
 *
 * A class for subdividing an intersection. Inherits SubLaneBundle.
 *
 * @~ @ingroup RoadNetwork 
 */
class SubIntersection : public SubLaneBundle
{
public:
    SubIntersection(const std::string& id, SubsectionType type);
    virtual ~SubIntersection() {};
};

#endif //__SUB_INTERSECTION_HPP__
