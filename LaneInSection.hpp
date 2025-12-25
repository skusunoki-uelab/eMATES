/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file LaneInSection.hpp
 */
#ifndef __LANE_IN_SECTION_HPP__
#define __LANE_IN_SECTION_HPP__
#include "Lane.hpp"
#include <map>

//##############################################################################
/**
 * @~japanese 単路部内の仮想走行レーン
 * @~english  Virtual driving lane in section
 * @~ @ingroup RoadMap
 */
class LaneInSection : public Lane
{
public:
    LaneInSection(
        const std::string& id, const Connector* ptBegin, const Connector* ptEnd,
        amu::geometry::AmuLineSegment* ptLineSegment, LaneBundle* parent);
    virtual ~LaneInSection() {};

    /**
     * @~japanese
     * 始点の単路部境界番号を戻す
     *
     * 始点の境界番号は識別番号の1 000 000の位
     *
     * @~english
     * Return the section boundary number of the start point
     *
     * The boundary number of the start point is the 1 000 000s place of the ID
     * number.
     */
    virtual int beginDirection() const override
    {
        return (stoi(_id) % 10000000) / 1000000;
    }

    /**
     * @~japanese 終点の単路部境界番号を戻す
     *
     * 終点の境界番号は識別番号の100の位
     *
     * @~english  Return the section boundary number of the end point
     *
     * The boundary number of the start point is the 100s place of the ID
     * number.
     */
    virtual int endDirection() const override
    {
        return (stoi(_id) % 1000) / 100;
    }

    /**
     * @~japanese 区間 @p interval に 横のレーン @p lane を設定する
     *
     * @p direction により設定方向を指定する．
     *
     * @~english  Set side lane @p lane in @p interval
     *
     * Specify the setting direction by @p direction
     */
    void addSideLane(
        amu::math::AmuInterval interval, const Lane* lane,
        LanePosition::Type direction) override
    {
        if (direction == LanePosition::Left)
        {
            _leftLanes.insert(std::make_pair(interval, lane));
        }
        else if (direction == LanePosition::Right)
        {
            _rightLanes.insert(std::make_pair(interval, lane));
        }
    }

    /**
     * @~japanese @p lane が横のレーンとしてすでに登録されているかどうかを戻す
     * @~english  Return whether @p lane is already registered as a side lane
     */
    bool isSideLaneFound(
        const Lane* lane, LanePosition::Type direction) const override;

    /**
     * @~japanese 横のレーンにおける位置を戻す
     * @param originDistance 自分のレーンの始点からの距離
     * @param direction 探索方向
     * @param[out] result_sideLane 見つかった横のレーン
     * @param[out] result_distance 横のレーンに投影した時の始点からの距離
     *
     * @todo 2つの値を戻す関数を検討する
     *
     * @~english Return the position in side lane
     * @param originDistance Distance from the start point of this lane
     * @param direction Search direction
     * @param[out] result_sideLane Side lane found
     * @param[out] result_distance Distance form the start point when projected
     * onto the side lane
     */
    virtual void getSideLaneDistance(
        double originDistance, LanePosition::Type direction,
        const Lane** result_sideLane, double* result_distance) const override;

    /**
     * @~japanese 横のレーン @p sideLane における位置を戻す
     * @param originDistance 自分のレーンの始点からの距離
     * 
     * @~english  Return the position in @p sideLane
     * @param originDistance Distance from the start point of this lane
     */
    virtual double lengthOnSideLane(
        double originDistance, const Lane* sideLane) const override;

    /**
     * @~japanese 位置 @p distance における右レーンを戻す 
     * @~english  Return the right lane at position @p distance
     */
    virtual const Lane* rightLane(double distance) const override;

    /**
     * @~japanese 位置 @p distance における左レーンを戻す
     * @~english  Return the left lane at position @p distance
     */
    virtual const Lane* leftLane(double length) const override;

    /**
     * @~japanese 位置 @p distance における右か左のレーンを戻す
     *
     * @p direction により探索方向を指定する．
     *
     * @~english  Return side lane at position @p direction
     *
     * Specify the search direction by @p direction.
     */
    virtual const Lane* sideLane(
        LanePosition::Type direction, double distance) const override;

    /**
     * @~japanese レーンの属性を @p out に出力する
     * @~english  Output lane attributes to @p out 
     */
    virtual void print(std::ostream& out) const override;

    //==========================================================================
protected:
    /**
     * @~japanese このレーンのひとつ右のレーンの集合
     *
     * レーンが内部コネクタにより分割されている場合には，複数のレーンが含まれ
     * うる．
     *
     * @~english  Set of lanes just right of this lane
     *
     * Multiple lanes may be contained if a lane is separated by internal
     * connectors.
     */
    std::map<amu::math::AmuInterval, const Lane*> _rightLanes;

    /**
     * @~japanese このレーンのひとつ左のレーンの集合
     *
     * レーンが内部コネクタにより分割されている場合には，複数のレーンが含まれ
     * うる．
     *
     * @~english  Set of lanes just left of this lane
     *
     * Multiple lanes may be contained if a lane is separated by internal
     * connectors.
     */
    std::map<amu::math::AmuInterval, const Lane*> _leftLanes;
};

#endif //__LANE_IN_SECTION_HPP__
