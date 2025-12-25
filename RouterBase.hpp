/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RouterBase.hpp
 */
#ifndef __ROUTER_BASE_HPP__
#define __ROUTER_BASE_HPP__
#include "Config.hpp"
#include "RandomNumberGenerator.hpp"
#include "Route.hpp"
#include "RouteKeyBase.hpp"
#include "RoutingNetwork.hpp"
#include "RoutingNode.hpp"
#include "VehicleTypeManager.hpp"
#include <cassert>
#include <cfloat>
#include <string>

class Intersection;
class RoadMap;
class RouteCacheContainer;
class RouterManager;
class RoutingRecorder;
class VehicleGlobalRoute;

//######################################################################
/**
 * @~japanese 大域的経路探索器の基底クラス
 * @~english  Base class for global routers
 * @~ @ingroup Routing
 */
class RouterBase
{
public:
    //==================================================================
    /**
     * @~japanese 探索中のノードの状態を記録する基底クラス
     * @~english  Base class to record the note status being searched
     */
    struct NodeStatusBase
    {
    public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese 探索状況をあらわすラベル
         * @~english  Label indicating search status
         */
        enum class NodeLabel : unsigned int
        {
            /**
             * @~japanese 未探索
             * @~english  Not yet scanned
             */
            UNREACHED,

            /**
             * @~japanese 1度でも探索済み
             * @~english  Scanned at least once
             */
            LABELED,

            /**
             * @~japanese 上位ノード探索済み
             * @~english  Its upper node scanned
             */
            UPPERLABELED,

            /**
             * @~japanese 探索が完了しコスト確定済み
             * @~english  Finished Scanning, and cost decided
             */
            SCANNED,
        };

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        NodeStatusBase() {}
        explicit NodeStatusBase(RoutingNode* node)
            : _id(node->id()), _routingNode(node), _prevId("-1")
        {
            _distance = DBL_MAX;
            _cost     = DBL_MAX;
            _label    = NodeLabel::UNREACHED;
        }

        virtual ~NodeStatusBase() {}

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    protected:
        /**
         * @~japanese 識別番号
         * @~english  ID number
         */
        std::string _id;

        /**
         * @~japanese 経路探索用ネットワークにおいて対応するノード
         * @~english  Corresponding node in network for routing
         */
        RoutingNode* _routingNode;

        /**
         * @~japanese 上流ノードの識別番号
         * @~english  ID number of the upstream node
         */
        std::string _prevId;

        /**
         * @~japanese 探索開始点からの距離
         * @~english  Distance from the start node of routing
         */
        double _distance;

        /**
         * @~japanese 探索開始点からの実コスト
         * @~english  Actual cost from the start node of routing
         */
        double _cost;

        /**
         * @~japanese ノードの探索状況
         * @~english  Routing status of the node
         */
        NodeLabel _label;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
    public:
        std::string id() const
        {
            return _id;
        }

        void setId(const std::string& id)
        {
            _id = id;
        }

        RoutingNode* routingNode() const
        {
            return _routingNode;
        }

        void setRoutingNode(RoutingNode* node)
        {
            _routingNode = node;
        }

        std::string prevId() const
        {
            return _prevId;
        }

        void setPrevId(const std::string& id)
        {
            _prevId = id;
        }

        double distance() const
        {
            return _distance;
        }

        void setDistance(double distance)
        {
            _distance = distance;
        }

        double cost() const
        {
            return _cost;
        }

        void setCost(double cost)
        {
            _cost = cost;
        }

        NodeLabel label() const
        {
            return _label;
        }

        void setLabel(NodeLabel label)
        {
            _label = label;
        }

        ///@}
    };

    //==================================================================
public:
    RouterBase();
    virtual ~RouterBase();

    /**
     * @~japanese
     * 経路探索用ネットワーク @p networkを与え経路探索器を初期化する
     *
     * @note
     * コンストラクタの直後に呼び出されることを想定している．親クラスと
     * 別の初期化処理がなされる可能性があるため，コンストラクタとは
     * 分離した．
     *
     * @~english
     * Give network for routing @p network and initialize router
     *
     * @note
     * Assumed to be called immediately after the constructor. Since
     * there is a possibility that the initialization process is
     * different from that of the parent class, it is separated from
     * the constructor.
     */
    virtual void initialize(const RoutingNetwork* network) = 0;

    /**
     * @~japanese
     * トークン @p tokens から経路キャッシュを生成して登録する
     *
     * @~english
     * Generate and register route cache from tokens @p tokens
     */
    virtual bool generateRouteCache(
        RoadMap* roadMap, std::vector<std::string>& tokens) const;

    /**
     * @~japanese
     * @p fromから@p nextを経由し，@p gates で指定された経路を探索する
     *
     * @p gates の最後が目的地を表す
     *
     * @~english
     * Search the route specified by @p gates via @p next from @p from
     *
     * The end of @p gates represents the destination.
     */
    virtual Route search(
        const Intersection* from, const Intersection* next,
        const std::vector<const Intersection*>& gates
        );

    //==================================================================
protected:
    /**
     * @~japanese
     * @p n 回目の探索において，探索の始点に至る交差点を戻す
     *
     * @~english
     * Return intersection leading to the start of the @p n -th search
     */
    const Intersection* _getLeadIntersection(
        unsigned int n, const Intersection* inter, Route& route);

    /**
     * @~japanese
     * @p n 回目の探索における始点交差点を戻す
     *
     * @~english
     * Return start intersection of the @p n -th search
     */
    const Intersection* _getStartIntersection(
        unsigned int n, const Intersection* inter, Route& route,
        const std::vector<const Intersection*>& gates);

    /**
     * @~japanese 経路探索用ネットワークにおける始点を決定する．
     *
     * @p from と @p next とを結ぶ単路に相当する RoutingNode ．
     *
     * @~english  Determine the start point in the routing network.
     *
     * RoutingNode corresponding to a section connecting @p from and
     * @p next.
     */
    virtual void _findStartNode(
        const Intersection* from, const Intersection* next);

    /**
     * @~japanese
     * 経路探索用ネットワークにおける探索の終点を決定する．
     *
     * 本来の goalIntersection に接続する RoutingNode (RoadMap における
     * Section )は複数存在する場合がある
     *
     * @~english Determine the end points in the routing network.
     *
     * There may be multiple RoutingNodes (Sections in RoadMap) that
     * connect to the original goalIntersection .
     */
    virtual void _findGoalNodes(
        const Intersection* goal, const std::vector<const Intersection*>& gates,
        int gateIndex);

    /**
     * @~japanese
     * ノード @p node が探索の終点に含まれるかどうかを戻す．
     *
     * @~english
     * Return whether the node @p node is included in the end point of
     * the search.
     */
    virtual bool _isNodeIdIncluded(
        const RoutingNode*                     node,
        const std::vector<const RoutingNode*>& container) const;

    /**
     * @~japanese
     * 探索結果 @p routingNodes を変換した Intersection のvectorを返す
     *
     * @~english
     * Return Intersection vector converted from search result
     * @p routingNodes
     */
    std::vector<const Intersection*> _convertNodesToIntersections(
        std::vector<RoutingNode*>& routingNodes);

    /**
     * @~japanese
     * 経路を構成する RoutingNode の集合を @p result_nodes に格納する
     *
     * @~english
     * Store the set of RoutingNodes for the route in @p result_nodes
     */
    // [eMATES] by abe 2025/05/20 経路コストを返すようにした
    virtual double _search(std::vector<RoutingNode*>& result_nodes) = 0;

    /**
     * @~japanese 経路キャッシュのコンテナから経路をロードする
     * @~english  Load route from the route cache container.
     */
    virtual bool _loadCache(
        RouteCacheContainer* container, const Intersection* from,
        const Intersection* next, const Intersection* goal,
        Route& result_route) const;

    /**
     * @~japanese 経路キャッシュをコンテナに保存する
     * @~english  Store route cache in container
     */
    virtual void _saveCache(
        RouteCacheContainer* container, const Intersection* from,
        const Intersection* next, const Intersection* goal,
        const Route& route) const;

    /**
     * @~japanese 内部状態を元に戻す
     * @~english  Reset internal state
     */
    virtual void _resetStatus();

    //==================================================================
protected:
    /**
     * @~japanese 探索の開始ノード
     * @~english  Start node of routing
     */
    const RoutingNode* _startNode;

    /**
     * @~japanese 探索の終了ノードの集合
     * @~english  Set of goal nodes of routing
     */
    std::vector<const RoutingNode*> _goalNodes;

    /**
     * @~japanese コスト計算用の重み
     * @~english  Weight for calculating cost
     */
    double _weights[VEHICLE_ROUTING_PARAMETER_SIZE];

    /**
     * @~japanese 使用中かどうか
     *
     * 処理が重複しないようにするためのフラグ
     *
     * @~english  Whether being in use
     *
     * Flag to avoid duplicate processing
     */
    bool _isInUse;

    /**
     * @~japanese 経路探索の手順を保存するレコーダ
     * @~english  Recorder saving routing procedures
     */
    RoutingRecorder* _recorder;

    //==================================================================
    /**
     * @~japanese @name 使用者の属性
     * @note 経路探索器が割り当てられる際にポインタをコピーする．
     *
     * @~english  @name Use's property
     * @note Copy pointer when the router is assigned.
     */
    ///@{
protected:
    /**
     * @~japanese 使用者の識別番号
     * @~english  User's ID number
     */
    std::string _id;

    /**
     * @~japanese 使用主の車種
     * @~english  User's vehicle type
     */
    const VehicleType* _vehicleType;

    /**
     * @~japanese 使用者の乱数生成器
     * @~english  User's random number generator
     */
    RandomNumberGenerator* _rng;

    ///@}

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    void setWeights(const double weights[])
    {
        assert(_isInUse);
        for (unsigned int i = 0; i < VEHICLE_ROUTING_PARAMETER_SIZE; i++)
        {
            _weights[i] = weights[i];
        }
    }

    bool isInUse() const
    {
        return _isInUse;
    }

    void setInUseOn()
    {
        _isInUse = true;
    }

    void setInUseOff()
    {
        _isInUse = false;
    }

    RoutingRecorder* recorder() const
    {
        return _recorder;
    }

    void setRecorder(RoutingRecorder* recorder)
    {
        _recorder = recorder;
    }

    void setId(const std::string& id)
    {
        assert(_isInUse);
        _id = id;
    }

    void setVehicleType(const VehicleType* type)
    {
        assert(_isInUse);
        _vehicleType = type;
    }

    void setRandomNumberGenerator(RandomNumberGenerator* rng)
    {
        assert(_isInUse);
        _rng = rng;
    }

    ///@}
};

#endif //__ROUTER_BASE_HPP__
