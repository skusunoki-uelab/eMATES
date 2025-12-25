/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file Blinker.hpp
 */
#ifndef __BLINKER_HPP__
#define __BLINKER_HPP__
#include <cstdint>
#include "RelativeDirection.hpp"

using BD_t = RD_t;

//##############################################################################
/**
 * @~japanese ウインカー
 * @~english  Blinker
 * @~ @ingroup Vehicle
 */
class Blinker
{
private:
    /**
     * @~japanese ウインカーが指示する方向
     *
     * RelativeDirection と比較することがあるためRD_tと値を一致させる．
     * ハザードは考慮していない．
     *
     * @~english  Direction indicated by blinker
     *
     * Match the value with RD_t, as it may be compared with RelativeDirection.
     * Hazards are not considered.
     */
    BD_t _direction;

public:
    //==========================================================================
    /**
     * @~japanese @name フラグ定数
     * @~english  @name Flag constants
     */
    ///@{

    /**
     * @~japanese 点灯なし
     * @~english  No lighting
     */
    static constexpr BD_t NONE = RD::NONE;

    /**
     * @~japanese 右ウインカー
     * @~english  Right blinker
     */
    static constexpr BD_t RIGHT = RD::RIGHT;

    /**
     * @~japanese 左ウインカー
     * @~english  Left blinker
     */
    static constexpr BD_t LEFT = RD::LEFT;

    ///@}

    //==========================================================================

public:
    Blinker()
    {
        _direction = NONE;
    }

    ~Blinker() {};

    /**
     * @~japanese ウインカーを消す
     * @~english  Turn off blinker
     */
    void setNone()
    {
        _direction = NONE;
    }

    /**
     * @~japanese 左合図を出す
     * @~english  Turn on left blinker
     */
    void setLeft()
    {
        _direction = LEFT;
    }

    /**
     * @~japanese 右合図を出す
     * @~english  Turn on right blinker
     */
    void setRight()
    {
        _direction = RIGHT;
    }

    //==========================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    BD_t direction() const
    {
        return _direction;
    }

    ///@}
};

#endif //__BLINKER_HPP__
