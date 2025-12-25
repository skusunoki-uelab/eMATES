/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file GuiRoutingTest.hpp
 */
#ifndef __GUI_ROUTING_TEST_HPP__
#define __GUI_ROUTING_TEST_HPP__

class RoutingRecorder;

//######################################################################
/**
 * @~japanese 独立した経路探索をおこない結果を表示する
 *
 * Visualizer の部分クラス
 *
 * @~english  Execute independent routing and draw result
 *
 * A part class of Visualizer
 *
 * @~ @ingroup Visualization
 */
class GuiRoutingTest
{
public:
    GuiRoutingTest();
    ~GuiRoutingTest();

    /**
     * @~japanese フラグを更新する
     * @~english  Update flags
     */
    static void renewFlags();

    /**
     * @~japanese 一時フラグをクリアする
     * @~english  Clear temporary flags
     */
    static void clearTemporaryFlags();

    /**
     * @~japanese パネルを作成する
     * @~english  Create panel
     */
    static void makePanel();

    /**
     * @~japanese ビューを再描画する
     * @~english  Redraw the view
     */
    static void redrawView();

    /**
     * @~japanese
     * 指定した出発地・目的地間の経路を探索する
     *
     * Search Route ボタンが押されたときの動作．
     *
     * @~english
     * Search route between the specified origin and destination
     *
     * The action when Search Route button is pressed.
     */
    static void searchRouteButtonCallback();

    /**
     * @~japanese 経路探索結果レコードをインクリメントして表示する
     * @~english  Increment and draw routing record
     */
    static void incrementRecordButtonCallback();

    /**
     * @~japanese 経路探索結果レコードをデクリメントして表示する
     * @~english  Decrement and draw routing record
     */
    static void decrementRecordButtonCallback();

    /**
     * @~japanese 経路探索結果レコードをリセットする
     * @~english  Reset routing record
     */
    static void resetRecordButtonCallback();

private:
    /**
     * @~japanese
     * ランクが @p rank である経路探索用ネットワークを描画する
     *
     * @~english
     * Draw a network for routing with rank @p rank
     */
    static void _drawRoutingNetwork(int rank);

    /**
     * @~japanese 経路探索レコードを描画する
     * @~english  Draw routing record
     */
    static void _showRecord();

private:
    /**
     * @~japanese 経路探索用ネットワークを表示するかどうか
     * @~english  Whether to draw network for routing
     */
    static int _showsRoutingNetwork;

    /**
     * @~japanese 表示する経路探索用ネットワークのランク
     * @~english  Rank of the network for routing to be drawn
     */
    static int _routingNetworkRank;

public:
    /**
     * @~japanese 経路探索用リンク表示モードの種類
     * @~english  Type of node for routing drawing mode
     */
    enum DrawingTypeForRoutingNode
    {
        ROUTENODE_NONE  = 0,
        ROUTENODE_ID    = 1,
        ROUTENODE_RANK  = 2,
        ROUTENODE_ALLOW = 3,
        ROUTENODE_DENY  = 4,
    };

private:
    /**
     * @~japanese 経路探索用ノードの表示モード
     * @~english  Drawing mode for node for routing
     */
    static int _drawingModeForRoutingNode;

public:
    /**
     * @~japanese 経路探索用リンク表示モードの種類
     * @~english  Type of link for routing drawing mode
     */
    enum DrawingTypeForRoutingLink
    {
        ROUTELINK_NONE     = 0,
        ROUTELINK_ID       = 1,
        ROUTELINK_LENGTH   = 2,
        ROUTELINK_TIME     = 3,
        ROUTELINK_STRAIGHT = 4,
        ROUTELINK_LEFT     = 5,
        ROUTELINK_RIGHT    = 6,
        ROUTELINK_ALLOW    = 7,
        ROUTELINK_DENY     = 8,
    };

private:
    /**
     * @~japanese 経路探索用リンクの表示モード
     * @~english  Drawing mode for link for routing
     */
    static int _drawingModeForRoutingLink;

    /**
     * @~japanese 出発地の識別番号
     * @~english  Origin ID number
     */
    static char _startId[16];

    /**
     * @~japanese 出発地の次の交差点の識別番号
     * @~english  ID number of the next intersection of the origin
     */
    static char _nextId[16];

    /**
     * @~japanese 目的地の識別番号
     * @~english  Destination ID number
     */
    static char _goalId[16];

    /**
     * @~japanese 経由地の識別番号
     * @~english  ID numbers of intersections of pass
     */
    static char _gateId[256];

    /**
     * @~japanese 経路探索で選好するネットワークランク
     * @~english  Preferred network rank for routing
     */
    static char _prefRank[16];

    /**
     * @~japanese 経路探索過程のレコーダ
     * @~english  Recorder for routing process
     */
    static RoutingRecorder* _routingRecorder;

    /// 経路探索のログを可視化するか
    /**
     * @~japanese 経路探索結果を可視化するかどうか
     * @~english  Whether to draw the routing result
     */
    static int _showsRoutingRecord;

    /**
     * @~japanese 描画される経路探索レコードのサイズ
     * @~english  Size of routing record to be drawn
     */
    static unsigned long _recordCount;
};

#endif //__ROUTING_VIEW_HPP__
