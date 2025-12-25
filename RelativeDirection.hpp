/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RelativeDirection.hpp
 */
#ifndef __RELATIVE_DIRECTION_HPP__
#define __RELATIVE_DIRECTION_HPP__
#include <cstdint>
#include <iostream>

using RD_t = uint32_t;

//######################################################################
/**
 * @~japanese 交差点における相対方向
 *
 * @note
 * ビット演算を利用してフラグとして扱えるように値を定めた．複数の値を
 * 同時に設定する場合は RelativeDirection rd = (RD::RIGHT | RD::LEFT) 
 * のように記述できる．また特定のフラグが立っているかどうか調べるには
 * bool contains = (rd & RD::STRAIGHT) のように記述できる．
 *
 * @attention
 * 交差点固有の拡張方向を定義できる余地を残しているが，これで十分かは
 * わからない．
 * 
 * @~english  Relative direction at intersection
 *
 * @note
 * Assign values so as to be used as flags through bit operations.
 * When setting multiple values at the same time, it can be described
 * as  * "RelativeDirection rd = (RD_RIGHT | RD_LEFT)." Also, when
 * checking  whether a specific flag is set, it can be described as
 * "bool contains = (rd & RD_STRAIGHT)".
 *
 * @attention
 * It leaves room to define intersection-specific extra directions,
 * but not sure if this is enough.
 *
 * @~ @ingroup RoadNetwork
 */
struct RelativeDirection
{
private:
    /**
     * @~japanese 保持する相対方向
     * @~english  Relative direction it holds
     */
    RD_t _value;

    //==================================================================
    /**
     * @~japanese @name フラグ定数
     * @~english  @name Flag constants
     */
    ///@{
public:
    /**
     * @~japanese 値なし
     * @~english  No value
     */
    static constexpr RD_t NONE = 0;

    /**
     * @~japanese 転回
     * @~english  Turning back
     */
    static constexpr RD_t BACK = 1 << 0;

    /**
     * @~japanese 右折
     * @~english  Turning right
     */
    static constexpr RD_t RIGHT = 1 << 1;

    /**
     * @~japanese 直進
     * @~english  Going straight
     */
    static constexpr RD_t STRAIGHT = 1 << 2;

    /**
     * @~japanese 左折
     * @~english  Turning left
     */
    static constexpr RD_t LEFT = 1 << 3;

    /**
     * @~japanese 拡張方向 1
     * @~english  Extra direction 1
     */
    static constexpr RD_t EXT1 = 1 << 4;

    /**
     * @~japanese 拡張方向 2
     * @~english  Extra direction 2
     */
    static constexpr RD_t EXT2 = 1 << 5;

    /**
     * @~japanese 拡張方向 3
     * @~english  Extra direction 3
     */
    static constexpr RD_t EXT3 = 1 << 6;

    /**
     * @~japanese 拡張方向 4
     * @~english  Extra direction 4
     */
    static constexpr RD_t EXT4 = 1 << 7;

    /**
     * @~japanese すべてのビットが立っている状態
     * @~english  All bits set
     */
    static constexpr RD_t ANY = 0xffffffff;

    ///@}

    //==================================================================
public:
    RelativeDirection()
    {
        _value = NONE;
    }

    explicit RelativeDirection(RD_t dir)
    {
        _value = dir;
    }

    RelativeDirection(const RelativeDirection& rd)
    {
        _value = rd._value;
    }

    ~RelativeDirection() {}

    /**
     * @~japanese コピー代入演算子
     * @~english  Copy assignment operator
     */
    RelativeDirection& operator=(const RelativeDirection& rd)
    {
        _value = rd._value;
        return *this;
    }

    /**
     * @~japanese 方向 @p dir を代入する
     * @~english  Assign direction @p dir
     */
    RelativeDirection& operator=(RD_t dir)
    {
        _value = dir;
        return *this;
    }

    /**
     * @~japanese uint32_t への明示的な型変換演算子
     * @~english  Explicit cast operator to uint32_t
     */
    explicit operator uint32_t() const
    {
        return _value;
    }

public:
    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
    RD_t value() const
    {
        return _value;
    }

    void setValue(RD_t dir)
    {
        _value = dir;
    }

    ///@}

    //==================================================================
    /**
     * @~japanese @name フレンド関数
     * @~english  @name Friend functions
     */
    ///@{

    /**
     * @~japanese 等価演算子
     * @~english  Equality operator
     */
    friend bool operator==(
        const RelativeDirection& lhs, const RelativeDirection& rhs)
    {
        return lhs._value == rhs._value;
    }

    /**
     * @~japanese 非等価演算子
     * @~english  Inequality operator
     */
    friend bool operator!=(
        const RelativeDirection& lhs, const RelativeDirection& rhs)
    {
        return lhs._value != rhs._value;
    }

    /**
     * @~japanese 等価演算子
     * @~english  Equality operator
     */
    friend bool operator==(const RelativeDirection& rd, RD_t dir)
    {
        return rd._value == dir;
    }

    /**
     * @~japanese 非等価演算子
     * @~english  Inequality operator
     */
    friend bool operator!=(const RelativeDirection& rd, RD_t dir)
    {
        return rd._value != dir;
    }

    /**
     * @~japanese 出力ストリーム挿入演算子
     * @~english  Output stream insertion operator
     */
    friend std::ostream& operator<<(
        std::ostream& out, const RelativeDirection& rd)
    {
        std::string str("");
        if (rd._value == NONE)
            str += "n";
        if (rd._value & BACK)
            str += "t";
        if (rd._value & RIGHT)
            str += "r";
        if (rd._value & LEFT)
            str += "l";
        if (rd._value & STRAIGHT)
            str += "s";
        if (rd._value & EXT1)
            str += "1";
        if (rd._value & EXT2)
            str += "2";
        if (rd._value & EXT3)
            str += "3";
        if (rd._value & EXT4)
            str += "4";
        if (rd._value == ANY)
            str += "a";
        return out << str;
    }

    ///@}
};

using RD = RelativeDirection;

#endif //__RELATIVE_DIRECTION_HPP__
