/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RelativeDirectionTable.hpp
 */
#ifndef __RELATIVE_DIRECTION_TABLE_HPP__
#define __RELATIVE_DIRECTION_TABLE_HPP__
#include "RelativeDirection.hpp"
#include <cstdint>
#include <vector>

//######################################################################
/**
 * @~japanese 相対方向テーブルの抽象基底クラス
 *
 * 境界 @c i から境界 @c j に向かう方向をあらわす2次元配列を持つ．
 * RelativeDirectionTable rdt に対し，rdt(i, j) でアクセスする．
 * たとえば i==j のとき， rdt(i, j) は RD::BACK を戻す．
 *
 * @~english  Abstract base class for relative direction table
 *
 * The table has a two-dimensional array representing the direction from
 * border @c i to border @c j . Access RelativeDirectionTable rdt with
 * rdt(i, j). For example, rdt(i, j) returns RD::BACK when i==j .
 *
 * @~ @ingroup RoadNetwork
 */
class RelativeDirectionTable
{
public:
    virtual ~RelativeDirectionTable() = 0;

    /**
     * @~japanese テーブルのサイズを取得する
     * @~english  Get table size
     */
    virtual int size() const = 0;

    /**
     * @~japanese
     * ( @p i , @p j ) 成分の相対方向を戻す
     *
     * @~english
     *  Return the relative direction of the ( @p i , @p j ) component
     */
    virtual const RelativeDirection& operator()(int i, int j) const = 0;
};

//######################################################################
/**
 * @~japanese ODノード用相対方向テーブル
 * @~english  Relative direction table for ODNode
 */
class RelativeDirectionTableTerminal : public RelativeDirectionTable
{
public:
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual int size() const override
    {
        return 1;
    }

    virtual const RelativeDirection& operator()(
        int i, int j) const override;
    ///@}

private:
    /**
     * @~japanese 共通テーブル
     * @~english  Common table
     */
    static const RelativeDirection _table[2];
};

//######################################################################
/**
 * @~japanese 次数2の交差点用相対方向テーブル
 * @~english  Relative direction table for intersection of degree 2
 */
class RelativeDirectionTable2Way : public RelativeDirectionTable
{
public:
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual int size() const override
    {
        return 2;
    }

    virtual const RelativeDirection& operator()(
        int i, int j) const override;
    ///@}

private:
    /**
     * @~japanese 共通テーブル
     * @~english  Common table
     */
    static const RelativeDirection _table[2];
};

//######################################################################
/**
 * @~japanese T字路用相対方向テーブル
 * @~english  Relative direction table for T-type intersection
 */

class RelativeDirectionTableTJunction : public RelativeDirectionTable
{
public:
    explicit RelativeDirectionTableTJunction(const int start);

    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual int size() const override
    {
        return 3;
    }

    virtual const RelativeDirection& operator()(
        int i, int j) const override;
    ///@}

private:
    /**
     * @~japanese 基点（T字の左肩部分）
     * @~english  Base point. Left-shoulder of "T". 
     */
    const int _start;

    /**
     * @~japanese 共通テーブル
     * @~english  Common table
     */
    static const RelativeDirection _table[3][3];
};

//######################################################################
/**
 * @~japanese 十字路用相対方向テーブル
 * @~english  Relative direction table for cross-type intersection
 */
class RelativeDirectionTableCross : public RelativeDirectionTable
{
public:
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual int size() const override
    {
        return 4;
    }

    virtual const RelativeDirection& operator()(
        int i, int j) const override;
    ///@}

private:
    /**
     * @~japanese 共通テーブル
     * @~english  Common table
     */
    static const RelativeDirection _table[4];
};

//######################################################################
/**
 * @~japanese カスタム相対方向テーブル
 *
 * 交差点独自の相対方向テーブルを設定するためのクラス．
 * setItem(i, j, RD::RIGHT) のような形式で任意の値を設定できる．
 *
 * @~english  Custom relative direction table
 *
 * A class for setting intersection-specific relative direction table.
 * Any values can be set in the form of setItem(i, j, RD::RIGHT) .
 */
class RelativeDirectionTableCustom : public RelativeDirectionTable
{
public:
    explicit RelativeDirectionTableCustom(int size);

    /**
     * @~japanese ( @p i , @p j ) 成分に方向 @p dir を設定する 
     * @~english  Set direction @p dir to ( @p i , @p j ) component
     */
    virtual void setItem(int i, int j, RD_t dir);

    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual int size() const override
    {
        return _size;
    }

    virtual const RelativeDirection& operator()(
        int i, int j) const override;
    ///@}

private:
    /**
     * @~japanese テーブルのサイズ
     * @~english  Table size
     */
    const int _size;

    /**
     * @~japanese 交差点専用の相対方向テーブル
     * @~english  Intersection-specific relative direction table
     */
    std::vector<RelativeDirection> _table;
};

#endif //__RELATIVE_DIRECTION_TABLE_HPP__
