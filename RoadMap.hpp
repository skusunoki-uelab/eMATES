/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file RoadMap.hpp
 */
#ifndef __ROAD_MAP_HPP__
#define __ROAD_MAP_HPP__
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class CSNodeBase;
class CSNodeFast;
class Intersection;
class Lane;
class LaneBundle;
class ODNode;
class Section;
class Signal;
class SubLaneBundle;
class VehicleEV;

/**
 * @defgroup RoadEnvironment
 * @~japanese 道路環境
 * @~english  Road environment
 */

/**
 * @defgroup RoadNetwork
 * @ingroup RoadEnvironment
 * @~japanese 道路ネットワーク
 * @~english  Road network
 */

//######################################################################
/**
 * @~japanese 地図オブジェクト
 *
 * 交差点，単路部，信号を集約する
 *
 * @~english  Road map object
 *
 * Aggregate intersections, sections, and signals
 *
 * @~ @ingroup RoadNetwork
 */
class RoadMap
{
public:
    RoadMap();
    ~RoadMap();

    //==================================================================
    /**
     * @~japanese @name 交差点に関する関数群
     * @~english  @name Functions related to intersections
     */
    ///@{
public:
    /**
     * @~japanese 識別番号 @p id の交差点を戻す
     * @~english  Return the intersection with ID number @p id
     */
    Intersection* intersection(const std::string& id) const
    {
        auto itr = _intersections.find(id);
        if (itr != _intersections.end())
        {
            return (*itr).second;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * @~japanese 交差点 @p ptInter をコンテナに追加する
     * @~english  Add intersection @p ptInter to the container
     */
    void addIntersection(Intersection* ptInter);

    ///@}

    //==================================================================
    /**
     * @~japanese @name 単路部に関する関数群
     * @~english  @name Functions related to sections
     */
    ///@{
public:
    /**
     * @~japanese 識別番号 @p id の単路部を戻す
     * @~english  Return the section with ID number @p id
     */
    Section* section(const std::string& id) const
    {
        auto its = _sections.find(id);
        if (its != _sections.end())
        {
            return (*its).second;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * @~japanese
     * 両端の交差点の識別番号を指定して単路部を戻す
     *
     * beginId->endId が上り方向であるかどうかを @p result_isUp に
     * 格納する
     *
     * @~english
     * Return the section by specifying the intersection ID numbers at
     * both ends
     *
     * Store in @p result_isUp if beginId->endId is up direction.
     */
    Section* section(
        const std::string& beginId, const std::string& endId,
        bool& result_isUp) const
    {
        std::string sectionId;
        if (beginId < endId)
        {
            sectionId   = beginId + endId;
            result_isUp = true;
        }
        else
        {
            sectionId   = endId + beginId;
            result_isUp = false;
        }
        return (section(sectionId));
    }

    /**
     * @~japanese 単路部 @p ptSection をコンテナに追加する
     * @~english  Add section @p ptSection to the container
     */
    void addSection(Section* ptSection);

    ///@}

    //==================================================================
    /**
     * @~japanese @name 信号に関する関数群
     * @~english  @name Functions related to traffic lights
     */
    ///@{
public:
    /**
     * @~japanese 識別番号 @p id の信号を戻す
     * @~english  Return the signal with ID number @p id
     */
    Signal* signal(const std::string& id) const
    {
        auto itr = _signals.find(id);
        if (itr != _signals.end())
        {
            return (*itr).second;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * @~japanese 信号 @p signal をコンテナに追加する
     * @~english  Add traffic light @p ptSignal to the container
     */
    void addSignal(Signal* ptSignal);

    /**
     * @~japanese
     * 交差点 @p inter を無信号交差点のコンテナに追加する
     *
     * @~english
     * Add intersection @p inter to unsignalized intersection container
     */
    void addUnsignalizedIntersection(Intersection* ptInter);

    /**
     * @~japanese
     * 交差点 @p inter が無信号交差点として登録されているかどうかを戻す
     *
     * @~english
     * Return whether intersection @p inter is registered as an unsignalized
     * intersection
     */
    bool hasUnsignalizedIntersection(Intersection* ptInter) const;

    ///@}

    //==================================================================
    // CSの推定待ち時間を更新 [eMATES]
    void renewEstimatedWaitingTimeInCS();

    //==================================================================
    /**
     * @~japanese @name エージェントに関する関数群
     * @~english  @name Functions related to agents
     */
    ///@{
public:
    /**
     * @~japanese
     * レーンやサブセクションが保持するエージェント情報を更新する
     *
     * @~english
     * Update agent information retained by lanes and subsections
     */
    void renewRetainedAgentInformation();

    /**
     * @~japanese 目的地に到達したエージェントを消去する
     * @~english  Delete agents that have reached its destination
     */
    void deleteArrivedAgents();

    /**
     * @~japanese 充電切れのエージェントを追加する
     * @~english Add stranded EVs into container
     */
    void addStrandedAgent(VehicleEV* Vehicle);

    /**
     * @~japanese 充電切れのエージェントの情報を出力し、消去する
     * @~english Output stranded EVs data in container and delete those EVs
     */
    void deleteStrandedAgents();

    ///@}

    //====================================================================
    /**
     * @~japanese レーンの整合性をチェックする
     * @~english  Check lane consistency
     */
    bool checkLaneConnectivity() const;

    /**
     * @~japanese
     * 地図オブジェクトに含まれる交差点のx座標，y座標の最大値と最小値を
     * 求める
     *
     * @note 地面の描画に用いる
     *
     * @~english
     * Find the maximum and minimum values of x- and y-coordinates of
     * intersections included in the road map object
     *
     * @note Used to draw the ground
     */
    void getRegion(
        double& result_xmin, double& result_xmax, double& result_ymin,
        double& result_ymax) const;

    /**
     * @~japanese
     * 交差点数，単路部数を @p out に出力する
     *
     * @~english
     * Output the number of intersections and the number of sections
     * to @p out
     */
    void printMapSimple(std::ostream& out) const;

    /**
     * @~japanese
     * 交差点の座標，種別と隣接交差点を @p out に出力する
     *
     * @~english
     * Output intersection coordinates, type and adjacent intersections
     * to @p out
     */
    void printMapDetail(std::ostream& out) const;

    //==================================================================
private:
    /**
     * @~japanese 交差点のメインコンテナ
     * @~english  Main container for intersections
     */
    std::unordered_map<std::string, Intersection*> _intersections;

    /**
     * @~japanese 単路部のメインコンテナ
     * @~english  Main container for sections
     */
    std::unordered_map<std::string, Section*> _sections;

    /**
     * @~japanese ODノードのサブコンテナ
     * @~english  Sub container for ODNodes
     */
    std::vector<ODNode*> _odNodes;

    // [eMATES]
    // CSのサブコンテナ
    std::vector<CSNodeBase*> _csNodes;
    std::vector<CSNodeBase*> _csNodesFast;

    /**
     * @~japanese レーン束オブジェクトのサブコンテナ
     *
     * 交差点と単路部をまとめて格納する
     *
     * @~english  Sub container for lane bundle object
     *
     * Storing intersections and sections together
     */
    std::vector<LaneBundle*> _laneBundles;

    /**
     * @~japanese 使用中のレーン束オブジェクトのコンテナ
     *
     * 使用中のレーン束オブジェクトにおいてのみエージェントの処理を
     * 実行する． renewAgentOrder において更新する．
     *
     * @~english  Container for lane bundle objects in use
     *
     * Run agent processing only in lane bundle objects that are
     * in use. Update in renewAgentOrder .
     */
    std::vector<LaneBundle*> _usedLaneBundles;

    /**
     * @~japanese 使用中のODノードのコンテナ
     *
     * 使用中のODノードにおいてのみ deleteArrivedAgents を実行する．
     * deleteArrivedAgetns の最後にクリアする．
     *
     * @~english  Container for ODNodes in use
     *
     * Run deleteArrivedAgents only in ODNodes that are in use. Clear
     * at the end of deleteArrivedAgents
     */
    std::vector<ODNode*> _usedODNodes;

    /**
     * @~japanese 充電切れのエージェントのコンテナ [eMATES]
     * @~english  Container for stranded EVs
     */
    std::vector<VehicleEV*> _strandedAgents;

    /**
     * @~japanese 信号のメインコンテナ
     * @~english  Main container for traffic lights
     */
    std::unordered_map<std::string, Signal*> _signals;

    /**
     * @~japanese 無信号交差点のサブコンテナ
     * @~english  Subcontainer of unsignalized intersections
     */
    std::vector<Intersection*> _unsignalizedIntersections;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variables
     */
    omp_lock_t _lock;
#endif //_OPENMP

    //==================================================================
    /**
     * @~japanese @name アクセッサ
     * @~english  @name Accessor
     */
    ///@{
public:
    const std::unordered_map<std::string, Intersection*>& intersections() const
    {
        return _intersections;
    }

    const std::unordered_map<std::string, Section*>& sections() const
    {
        return _sections;
    }

    const std::vector<ODNode*>& odNodes() const
    {
        return _odNodes;
    }

    const std::vector<CSNodeBase*>& csNodes() const // [eMATES]
    {
        return _csNodes;
    }

    const std::vector<CSNodeBase*>& csNodesFast() const // [eMATES]
    {
        return _csNodesFast;
    }

    const std::vector<LaneBundle*>& laneBundles() const
    {
        return _laneBundles;
    }

    const std::vector<LaneBundle*>& usedLaneBundles() const
    {
        return _usedLaneBundles;
    }

    const std::unordered_map<std::string, Signal*>& signals() const
    {
        return _signals;
    }

    const std::vector<Intersection*>& unsignalizedIntersections() const
    {
        return _unsignalizedIntersections;
    }

    ///@}
};

#endif //__ROAD_MAP_HPP__
