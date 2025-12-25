/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneInIntersection.hpp
 */
#ifndef __LANE_IN_INTERSECTION_HPP__
#define __LANE_IN_INTERSECTION_HPP__
#include "Lane.hpp"
#include <cstdlib>
#include <map>
#include <string>

//##############################################################################
/**
 * @~japanese 交差点内の仮想走行レーン
 * @~english  Virtual driving lane in intersection
 * @~ @ingroup RoadNetwork
 */
class LaneInIntersection : public Lane
{
public:
    LaneInIntersection(
        const std::string& id, const Connector* ptBegin, const Connector* ptEnd,
        amu::geometry::AmuLineSegment* ptLineSegment, LaneBundle* parent);
    virtual ~LaneInIntersection() {};

    /**
     * @~japanese 始点の交差点境界番号を戻す
     *
     * 始点の境界番号は識別番号の1 000 000の位
     *
     * @~english  Return the intersection boundary number of the start point
     *
     * The boundary number of the start point is the 1 000 000s place of the ID
     * number.
     */
    virtual int beginDirection() const override
    {
        return (stoi(_id) % 10000000) / 1000000;
    }

    /**
     * @~japanese 終点の交差点境界番号を戻す
     *
     * 終点の境界番号は識別番号の100の位
     *
     * @~english  Return the intersection boundary number of the end point
     *
     * The boundary number of the end point is the 100s place
     * of the ID number.
     */
    virtual int endDirection() const override
    {
        return (stoi(_id) % 1000) / 100;
    }

    /**
     * @~japanese
     * 所属する交差点における交錯レーン @p lane を追加する
     *
     * @~english
     * Add crossing lane @p lane at the intersection to which this lane belongs
     */
    void addCollisionLanesInIntersection(const Lane* lane) override
    {
        _collisionLanesInIntersection.emplace_back(lane);
    }

    /**
     * @~japanese
     * 所属する交差点における交錯レーンの上流にある単路部内レーン @p lane を
     * 追加する
     *
     * @~english
     * Add @p lane in a section upstream of a crossing lane in the intersection
     * to which this lane belongs
     */
    void addCollisionLanesInSection(const Lane* lane) override
    {
        _collisionLanesInSection.push_back(lane);
    }

    /**
     * @~japanese レーンの属性を @p out に出力する
     * @~english  Output lane attributes to @p out 
     */
    virtual void print(std::ostream& out) const override;

    //==========================================================================
protected:
    /**
     * @~japanese
     * 所属する交差点内における交錯レーンの集合
     *
     * @note これを「交差点内交錯レーン」と呼ぶ．
     *
     * @~english
     * Set of crossing lanes in the intersection to which this lane belongs
     *
     * @note They are called "crossing lanes in intersection."
     */
    std::vector<const Lane*> _collisionLanesInIntersection;

    /**
     * @~japanese
     * _collisionLanesInIntersection の上流にある単路内レーンの集合
     *
     * @note これを「単路部内交錯レーン」と呼ぶ．
     *
     * @~english
     * Set of section lanes upstream of _collisionLanesInIntersection
     *
     * @note They are called "crossing lanes in section."
     */
    std::vector<const Lane*> _collisionLanesInSection;

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    std::vector<const Lane*>& collisionLanesInIntersection()
    {
        return _collisionLanesInIntersection;
    }

    std::vector<const Lane*>& collisionLanesInSection()
    {
        return _collisionLanesInSection;
    }

    ///@}
};

#endif //__LANE_IN_SECTION_HPP__
