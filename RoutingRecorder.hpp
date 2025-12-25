/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoutingRecorder.hpp
 */
#ifndef __ROUTING_RECORDER_HPP__
#define __ROUTING_RECORDER_HPP__
#include "RouterBase.hpp"
#include <iostream>
#include <map>
#include <string>
#include <vector>

class RoutingNode;

//######################################################################
/**
 * @~japanese 経路探索のプロセスを保存するレコーダ
 * @~english  Recorder saving the routing process
 * @~ @ingroup Routing
 */
class RoutingRecorder
{
public:
    RoutingRecorder()
    {
        _log.clear();
        _nodeStatusId2RoutingNode.clear();
    }
    ~RoutingRecorder()
    {
        for (auto itr : _log)
        {
            delete itr;
        }
        _log.clear();
    }

    /**
     * @~japanese NodeStatus の識別番号と RoutingNode を紐づける
     * @~english  Link the ID number of NodeStatus and RoutingNode
     */
    void addRoutingNode(
        const std::string& nodeStatusId, const RoutingNode* rnode)
    {
        _nodeStatusId2RoutingNode.insert(
            make_pair(nodeStatusId, rnode));
    }

    /**
     * @~japanese
     * NodeStatus の識別番号と RoutingNode の変換テーブルをクリアする
     * 
     * @note
     * 経路探索器ごとにIDの付け方が異なる可能性があるため，毎回登録し
     * 毎回クリアする．
     *
     * @~english
     * Clear translation table for NodeStatus ID number and RoutingNode
     *
     * @note
     * Because each router may have a different ID number assignment
     * rule, record and clear them each time. 
     */
    void clearNodeStatusId2RoutingNode()
    {
        _nodeStatusId2RoutingNode.clear();
    };

    /**
     * @~japanese 探索プロセスが記録中であるかどうかを戻す
     * @~english  Return whether the routing process is being recorded
     */
    bool isRecorded() const
    {
        return !(_log.empty());
    }

    /**
     * @~japanese ログをリセットする
     * @~english  Reset log
     */
    void resetLog()
    {
        _log.clear();
        _nodeStatusId2RoutingNode.clear();
    }

    /**
     * @~japanese @p snode をログに記録する
     * @~english  Log @p snode
     */
    void addLog(RouterBase::NodeStatusBase* snode);

    /**
     * @~japanese ログを @p out に出力する
     * @~english  Output log to @p out
     */
    void print(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 探索過程を記録するコンテナ
     * @~english  Container to record the routing process
     */
    std::vector<RouterBase::NodeStatusBase*> _log;

    /**
     * @~japanese
     * NodeStatus の識別番号と RoutingNode の変換テーブル
     *
     * @~english
     * Translation table for NodeStatus ID number to RoutingNode
     */
    std::map<const std::string, const RoutingNode*>
        _nodeStatusId2RoutingNode;

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::vector<RouterBase::NodeStatusBase*>& log() const
    {
        return _log;
    }

    const std::map<const std::string, const RoutingNode*>&
    nodeStatusId2RoutingNode() const
    {
        return _nodeStatusId2RoutingNode;
    }

    ///@}
};

#endif //__ROUTING_RECORDER_HPP__
