/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Route.hpp
 */
#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__
#include <cassert>
#include <iterator>
#include <vector>

class Intersection;

//######################################################################
/**
 * @~japanese 経路探索の結果として得られた経路
 * @~english  Route obtained as a result of a routing
 * @~ @ingroup Routing
 */
struct Route
{
public:
    Route()
    {
        _isValid = true;
        clearIntersections();
    }
    Route(const Route& another)
    {
        _isValid = another.isValid();
        _cost = another.cost(); // [eMATES]
        std::copy(
            another._intersections.begin(),
            another._intersections.end(),
            std::back_inserter(_intersections));
    }
    ~Route() {}

    /**
     * @~japanese コピー代入演算子
     * @~english  Copy assignment operator
     */
    Route& operator=(const Route& another)
    {
        _isValid = another.isValid();
        _cost = another.cost(); // [eMATES]
        std::copy(
            another._intersections.begin(),
            another._intersections.end(),
            std::back_inserter(_intersections));
        return *this;
    }

    //==================================================================
    /**
     * @~japanese @name 等価演算子
     * @~english  @name Equality operator
     */
public:
    ///@{
    bool operator==(const Route& another) const
    {
        if (_isValid != another.isValid())
        {
            return false;
        }
        if (_intersections.size() != another.intersections().size())
        {
            return false;
        }
        for (unsigned int i = 0; i < _intersections.size(); i++)
        {
            if (_intersections[i] != another.intersection(i))
            {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Route& another) const
    {
        return !(*this == another);
    }

    ///@}

    //==================================================================
    /**
     * @~japanese 経路をクリアする
     * @~english  Clear route
     */
    void clearIntersections()
    {
        _intersections.clear();
        _cost = 0; // [eMATES]
    }

    /**
     * @~japanese vectorの最後の要素を削除する
     * @~english  remove last element of vector
     */
    void removeLastIntersection()
    {
        _intersections.pop_back();
    }

    /**
     * @~japanese 交差点リスト @p intersections を追加する
     * @~english  Add intersection list @p intersections
     */
    // [eMATES] cost追加
    void addIntersections(
        const std::vector<const Intersection*>& intersections,
        double cost = 0)
    {
        _intersections.insert(
            _intersections.end(), intersections.begin(),
            intersections.end());
        _cost += cost;
    }

    /**
     * @~japanese 交差点 @p intersection を追加する
     * @~english  Add intersection @p intersection
     */
    // [eMATES] cost追加
    void addIntersection(const Intersection* intersection, double cost = 0)
    {
        _intersections.emplace_back(intersection);
        _cost += cost;
    }

    /**
     * @~japanese @p intersection の次の交差点を戻す
     *
     * @attention
     * 同じ交差点を複数回通る場合，この関数では一意に決まらないので
     * できるだけnext(const Intersection*, const Intersection*)を使う
     *
     * @~english  Return the next intersection of intersection @p inter
     *
     * Use next(const Intersection*, const Intersection*) if possible
     * because this function cannot uniquely determine when passing
     * the same intersection multiple times.
     *
     * @~ @see next(const Intersection*, const Intersection*)
     */
    const Intersection* next(const Intersection* inter) const;

    /**
     * @~japanese
     * インデックス @p offset 以降で @p intersection の次の交差点を返す
     *
     * @~english
     * Returns the next intersection of intersection @p inter at index
     * @p offset and higher
     *
     * @~ @see next(const Intersection*)
     */
    const Intersection* next(
        const Intersection* inter, int offset) const;

    /**
     * @~japanese
     * @p rear から @p front への単路の次の交差点を戻す
     *
     * @note
     * 同じ単路を複数回通ることを想定しないが，先頭から順に探索すれば
     * 所望の機能を果たすと考えられる．
     *
     * @~english
     * Return the next intersection of section from @p rear to @p front
     *
     * Not assumed that the same section will be passed multiple times,
     * but it is thought that searching from the beginning will achieve
     * the desired function.
     */
    const Intersection* next(
        const Intersection* rear, const Intersection* front) const;

    /**
     * @~japanese
     * インデックス @p offset 以降で @p rear から @p front への単路の
     * 次の交差点を戻す
     *
     * @~english
     * Returns the next intersection of section from @p rear to @p front
     * at index @p offset and higher
     *
     * @~ @see next(const Intersection*, const Intersection*)
     */
    const Intersection* next(
        const Intersection* rear, const Intersection* front,
        int offset) const;

    /**
     * @~japanese 状態を @p out に出力する
     * @~english  Output status to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 経路本体
     *
     * 経由する交差点のvectorとして格納する
     *
     * @~english  Route
     *
     * Store as a vector of intersections to pass
     */
    std::vector<const Intersection*> _intersections;

    /**
     * @~japanese 有効な経路かどうか
     *
     * @note
     * 経路探索に失敗した場合には達成不可能な交差点列が格納されている．
     *
     * @~english  Whether it is a valid route
     *
     * @note
     * If the routing fails, an unattainable intersection sequence may
     * be stored.
     */
    bool _isValid;

    // [eMATES] by abe 2025/05/20 経路のコスト
    double _cost;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::vector<const Intersection*>& intersections() const
    {
        return _intersections;
    }

    const Intersection* intersection(unsigned int i) const
    {
        assert(i < _intersections.size());
        return _intersections[i];
    }

    bool isValid() const
    {
        return _isValid;
    }

    void setIsValid(bool isValid)
    {
        _isValid = isValid;
    }

    double cost() const // [eMATES] by abe 2025/05/20
    {
        return _cost;
    }


    ///@}
};

#endif //__ROUTE_HPP__
