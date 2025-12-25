/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingProbability.hpp
 */
#ifndef __ROUTING_PROBABILITY_HPP__
#define __ROUTING_PROBABILITY_HPP__
#include "VehicleType.hpp"
#include <map>

//######################################################################
/**
 * @~japanese 車種別の経路利用確率を格納する構造体
 *
 * 探索の展開時にリンクを確定的でなく確率的にたどらせることで，
 * 同一の探索クエリの結果が1つの経路に集中しないようにする．
 * ただしキャッシュが適用される際にはここで設定された確率が無視される．
 *
 * @~english  Struct storing route usage probability by vehicle type
 *
 * By following links stochastically rather than deterministically
 * when search expands, results of the same search query are not
 * concentrated in one route. However, the probabilities set here are
 * ignored when route caches are applied.
 * 
 * @~ @ingroup Routing 
 */
struct RoutingProbability
{
public:
    RoutingProbability()
    {
        _probabilities.clear();
    }
    ~RoutingProbability() {};

    /**
     * @~japanese
     * 車種 @p type の車両の経路選択確率を戻す
     *
     * @~english
     * Return routing probability for vehicles of type @p type
     */
    double probability(const VehicleType& type) const;

    /**
     * @~japanese
     * 車種 @p type の探索確率を @p prob に設定する
     *
     * @~english
     * Set search probability of vehicle type @p type to @ prob
     */
    void addProbability(const VehicleType& type, double prob)
    {
        _probabilities[type] = prob;
    }

    /**
     * @~japanese 他の探索確率設定をコピーする
     * @~english  Copy other probability property
     */
    void addProbability(const RoutingProbability& prob);

    /**
     * @~japanese 探索確率を @p out に出力する
     * @~english  Output search probabilities to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 経路探索時の探索確率テーブル
     * @~english  Search probability table for routing
     */
    std::map<VehicleType, double> _probabilities;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::map<VehicleType, double>& probabilities() const
    {
        return _probabilities;
    }

    ///@}
};

#endif //__ROUTING_PROBABILITY_HPP__
