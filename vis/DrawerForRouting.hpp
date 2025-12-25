/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file DrawerForRouting.hpp
 */
#ifndef __DRAWER_FOR_ROUTING_HPP__
#define __DRAWER_FOR_ROUTING_HPP__
#include "../RoutingLink.hpp"
#include "../RoutingNetwork.hpp"
#include "../RoutingNode.hpp"
#include "../RoutingRecorder.hpp"

//######################################################################
/**
 * @~japanese 経路探索用ネットワークと経路探索結果レコードを描画する
 * @~english  Draw a network for routing and a routing record
 * @~ @ingroup Drawing
 */
class DrawerForRouting
{
public:
    DrawerForRouting() {};
    ~DrawerForRouting() {};

    /**
     * @~japanese 経路探索用ネットワーク @p network を描画する
     * @~english  Draw a network for routing @p network
     */
    void drawNetwork(const RoutingNetwork& network) const;

private:
    /**
     * @~japanese 経路探索用リンク @p link を描画する
     * @~english  Draw a link for routing @p link
     */
    void _drawLink(const RoutingLink& link, double width) const;

    /**
     * @~japanese 経路探索用ノード @p node を 描画する
     * @~english  Draw a node for routing @p node
     */
    void _drawNode(const RoutingNode& node) const;

public:
    /**
     * @~japanese 経路探索結果レコード @p record を描画する
     * @~english  Draw a routing record @p record
     */
    void drawRecord(const RoutingRecorder& recorder, int maxStep) const;

private:
    /**
     * @~japanese z方向のマージン
     * @~english  Margin in z direction
     */
    static constexpr double _ZMARGIN = 30.0;
};

#endif //__DRAWER_FOR_ROUTING_HPP__
