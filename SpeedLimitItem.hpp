/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file SpeedLimitItem.hpp
 */
#ifndef __SPEED_LIMIT_ITEM_HPP__
#define __SPEED_LIMIT_ITEM_HPP__
#include "Config.hpp"
#include "ScheduleItemBase.hpp"

class Section;

//##############################################################################
/**
 * @~japanese 制限速度の設定を格納する構造体
 * @~english  Struct storing speed limit setting
 * @~
 * @ingroup RoadNetwork
 */
struct SpeedLimitItem : public ScheduleItemBase
{
public:
    SpeedLimitItem(ulint time, Section* section, bool isUp, double speedLimit);
    virtual ~SpeedLimitItem() {}

    /**
     * @~japanese 親クラスの関数のオーバーライド
     * @~english  Override functions of parent class
     */
    void activate() override;

private:
    /**
     * @~japanese 適用開始時刻 [ms]
     * @~english  Start time of application [ms]
     */
    const ulint _time;

    /**
     * @~japanese 対象となる単路部
     * @~english  Target section
     */
    Section* _section;

    /**
     * @~japanese
     * 対象の方向が単路部の上りかどうか
     *
     * @~english
     * Whether the target direction is the ascending one of the section
     */
    const bool _isUp;

    /**
     * @~japanese 制限速度 [km/h]
     * @~english  Speed limit [km/h]
     */
    const double _speedLimit;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    ulint time() const
    {
        return _time;
    }

    const Section* section() const
    {
        return _section;
    }

    bool isUp() const
    {
        return _isUp;
    }

    double speedLimit() const
    {
        return _speedLimit;
    }

    ///@}
};

#endif //__SPEED_LIMIT_ITEM_HPP__
