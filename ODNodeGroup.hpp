/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file ODNodeGroup.hpp
 */
#ifndef __OD_NODE_GROUP_HPP__
#define __OD_NODE_GROUP_HPP__
#include "RandomNumberGenerator.hpp"
#include <iostream>
#include <string>
#include <vector>

class ODNode;

//######################################################################
/**
 * @~japanese グループ化された ODNode をまとめたコンテナ
 * @~english  Container for grouped ODNodes
 * @~ @ingroup RoadNetwork
 */
struct ODNodeGroup
{
    //==================================================================
    /**
     * @~japanese
     * 単独の ODNode と選択ウェイトを格納する構造体
     *
     * @~english
     * Struct storing single ODNode and weight for selection
     */
    struct WeightedODSolo
    {
    private:
        /**
         * @~japanese 格納されている ODNode
         * @~english  Stored ODNode
         */
        ODNode* _node;

        /**
         * @~japanese 選択ウェイト
         * @~english  Weight for selection
         */
        double _weight = 1.0;

    public:
        WeightedODSolo(ODNode* node, double weight)
            : _node(node), _weight(weight) {};
        WeightedODSolo() {};

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        ODNode* node() const
        {
            return _node;
        }

        double weight() const
        {
            return _weight;
        }

        ///@}
    };

    //==================================================================
    /**
     * @~japanese
     * 出発地と目的地のペアと選択ウェイトを格納する構造体
     *
     * @~english
     * Struct storing origin-destination pair and weight for selection
     */
    struct WeightedODPair
    {
    private:
        /**
         * @~japanese 格納されている出発地
         * @~english  Stored origin
         */
        ODNode* _start;

        /**
         * @~japanese 格納されている目的地
         * @~english  Stored destination
         */
        ODNode* _goal;

        /**
         * @~japanese 格納されている選択ウェイト
         * @~english  Stored weight for selection
         */
        double _weight = 1.0;

    public:
        WeightedODPair(ODNode* start, ODNode* goal, double weight)
            : _start(start), _goal(goal), _weight(weight)
        {
        }
        ~WeightedODPair() {}

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        ODNode* start() const
        {
            return _start;
        }

        ODNode* goal() const
        {
            return _goal;
        }

        double weight() const
        {
            return _weight;
        }

        ///@}
    };

    //==================================================================
    /**
     * @~japanese ODNode のグループ化の種類
     * @~english  Type of grouping ODNodes
     */
    enum class GroupType : unsigned int
    {
        UNDEFINED,
        SOLO,
        PAIR
    };

    //==================================================================
public:
    ODNodeGroup()
    {
        _type        = GroupType::UNDEFINED;
        _totalWeight = 0.0;
        _odSolos.clear();
        _odPairs.clear();
        _rng.reset();
    }

    ODNodeGroup(const std::string& id)
    {
        _id          = id;
        _type        = GroupType::UNDEFINED;
        _totalWeight = 0.0;
        _odSolos.clear();
        _odPairs.clear();
        _rng.reset();
    }

    ~ODNodeGroup()
    {
        for (auto itr : _odSolos)
        {
            delete itr;
        }
        _odSolos.clear();
        for (auto itr : _odPairs)
        {
            delete itr;
        }
        _odPairs.clear();
    }

    /**
     * @~japanese 単独の ODNode @p node をコンテナに追加する
     * @~english  Add a single ODNode @p node to the container
     */
    void addODSolo(ODNode* node, double weight);

    /**
     * @~japanese
     * コンテナから単独の ODNode をランダムに選択し，@p result_node に
     * 格納する
     *
     * @~english
     * Select a single ODNode from the container randomly, and store
     * it in @p result_node
     */
    void getODSolo(ODNode** result_node);

    /**
     * @~japanese
     * 出発地 @p start と目的地 @p goal のペアを追加する
     *
     * @~english
     * Add a pair of origin @p start and destination @p goal to the
     * container
     */
    void addODPair(ODNode* start, ODNode* goal, double weight);

    /**
     * @~japanese
     * コンテナから ODNode のペアをランダムに選択し，出発地を
     * @p result_start に，目的地を @p result_goal に格納する
     *
     * @~english
     * Select a pair of ODNodes form the container randomly, and store
     * the origin and destination in @p result_start and @p result_goal,
     * respectively
     */
    void getODPair(ODNode** result_start, ODNode** result_goal);

    /**
     * @~japanese 格納している ODNode を @p out に出力する
     * @~english  Output the stored ODNodes to @p out
     */
    void print(std::ostream& out) const;

private:
    /**
     * @~japanese 識別番号
     * @~english  ID number
     */
    std::string _id;

    /**
     * @~japanese グループ化の種別
     * @~english  Grouping type
     */
    GroupType _type;

    /**
     * @~japanese 選択ウェイトの合計
     * @~english  Sum of weight for selection
     */
    double _totalWeight;

    /**
     * @~japanese 単独のODNodeのコンテナ
     * @~english  Container for single ODNodes
     */
    std::vector<WeightedODSolo*> _odSolos;

    /**
     * @~japanese ODNodeのペアのコンテナ
     * @~english  Container for pairs of ODNodes
     */
    std::vector<WeightedODPair*> _odPairs;

    /**
     * @~japanese 乱数生成器
     * @~english  Random number generator
     */
    RandomNumberGenerator _rng;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::string& id() const
    {
        return _id;
    }

    ///@}
};

#endif //__OD_NODE_GROUP_HPP__
