/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Zebra.hpp
 */
#ifdef INCLUDE_PEDESTRIANS
#ifndef __ZEBRA_HPP__
#define __ZEBRA_HPP__
#include "ZebraODEdge.hpp"
#include "../SubIntersection.hpp"
#include "../RandomNumberGenerator.hpp"
#include <AmuLineSegment.hpp>
#include <AmuMatrix2D.hpp>
#include <AmuPoint.hpp>
#include <deque>
#include <iostream>
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

class Pedestrian;
class Vehicle;

//######################################################################
/**
 * @~japanese 横断歩道
 * @~english  Crosswalk
 * @~ @ingroup PedSim RoadNetwork
 */
class Zebra : public SubIntersection
{
public:
    explicit Zebra(const std::string& id);
    virtual ~Zebra();

    //==================================================================
    /**
     * @~japanese @name 横断歩道自身に関する関数群
     * @~english  @name Functions related to crosswalk itself
     */
    ///@{
public:
    /**
     * @~japanese メンバ変数を初期化する
     * @~english  Initialize member variables
     */
    void initialize(int dirInIntersection);

private:
    /**
     * @~japanese
     * ローカル座標系への変換行列を作成する
     *
     * @attention
     * 4頂点の座標を必要とするため，コンストラクタからの呼び出しは不可
     *
     * @~english
     * Create a transformation matrix to local coordinate system
     *
     * @attention
     * Cannot be called in the constructor because it requires the
     * coordinates of 4 vertexes.
     */
    void _calcTransposeMatrix();

public:
    /**
     * @~japanese
     * 横断方向 @p crossingDir の始境界を戻す
     *
     * _odEdges[dir] は終境界を表すため，添字の反転させる．
     *
     * @~english
     * Return start boundary of crossing direction @p crossingDir
     *
     * _odEdges[dir] represents the end boundary, so reverse the index.
     */
    ZebraODEdge* beginEdge(int crossingDir)
    {
        assert(crossingDir == 0 || crossingDir == 1);
        return &_odEdges[(crossingDir + 1) % 2];
    }

    /**
     * @~japanese
     * 横断方向 @p dir の終境界を戻す
     *
     * @~english
     * Return end boundary of crossing direction @p crossingDir
     */
    ZebraODEdge* endEdge(int crossingDir)
    {
        assert(crossingDir == 0 || crossingDir == 1);
        return &_odEdges[crossingDir];
    }

    ///@}

    //==================================================================
    /**
     * @~japanese
     * @name 横断歩道内のエージェントの操作に関する関数群
     *
     * @~english
     * @name Functions related to agent operations in crosswalk
     */
    ///@{
public:
    /**
     * @~japanese 歩行者の順序列を更新する
     * @~english  Update pedestrian sequence
     */
    void renewPedestrianOrder();

    /**
     * @~japanese 接近する自動車の順序列を更新する
     * @~english  Update the sequence of approaching cars
     */
    void renewApproachingVehicleOrder();

    /**
     * @~japanese レーンに歩行者の接近を通知する
     * @~english  Notify lanes of approaching pedestrians
     */
    void notifyLaneOfApproachingPedestrian();

    /**
     * @~japanese @p ped を横断歩道に所属させる
     * @~english  Make @p ped belong to the crosswalk
     */
    bool putPedestrian(Pedestrian* pds);

    ///@}

    //==================================================================
private:
    /**
     * @~japanese 歩行者の発生・消滅境界
     *
     * 0 or 1 で定義された歩行者の横断方向の先にある発生・消滅境界．
     * 0 は交差点を時計回りに渡る方向，1 が反時計方向に渡る方向である．
     * 結果的に，_odEdge[0] が辺3 ( _vertexes[3] , _vertexes[0] )，
     * _odEdge[1] が辺1 ( _vertexes[1] , _vertexes[2] ) に合致する．
     *
     * @attention
     * 始境界を取得するに横断方向の 0/1 を反転させる必要がある．
     * 特別な理由がない限り beginEdge(dir) を用いるべきである．
     *
     * @~english  Pedestrian generation/deletion boundary
     *
     * Generation/deletion edge ahead of the pedestrian's crossing
     * direction defined as 0 or 1. 0 indicates the direction to cross
     * the intersection clockwise, 1 indicates the direction to cross
     * the intersection counterclockwise. Consequently, _odEdge[0]
     * matches the edge 3 ( _vertexes[3] , _vertexes[0] ), _odEdge[1]
     * matches the edge 1 ( _vertexes[1] , _vertexes[2] ).
     *
     * @attention
     * To obtain the generation(start) boundary, necessary to invert
     * 0/1 in the crossing direction. beginEdge(dir) should be used
     * unless there is a particular reason.
     */
    ZebraODEdge _odEdges[2];

    /**
     * @~japanese
     * この横断歩道が設置されている交差点の境界方向
     *
     * @~english
     * Border direction of the intersection where this crosswalk is
     * installed
     */
    int _dirInIntersection;

    /**
     * @~japanese この横断歩道に配置されている歩行者の集合
     * @~english  Set of pedestrians placed at this crosswalk
     */
    std::vector<Pedestrian*> _pedestrians;

    /**
     * @~japanese
     * 次のステップでこの横断歩道に配置される歩行者のコンテナ
     *
     * @~english
     * A container for pedestrians that will be placed on this crosswalk
     * in the next step
     */
    std::vector<Pedestrian*> _tmpPedestrians;

    /**
     * @~japanese ローカル座標系への変換行列
     * @note 歩行者の集合のソートに用いる
     *
     * @~english  Transformation matrix to local coordinate system
     * @note Used to sort pedestrian container 
     */
    amu::math::AmuMatrix2D _transMat;

#ifdef _OPENMP
    /**
     * @~japanese ロック変数
     * @~english  Lock variable
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
    int directionInIntersection() const
    {
        return _dirInIntersection;
    }

    const std::vector<Pedestrian*>& pedestrians()
    {
        return _pedestrians;
    }

    ZebraODEdge* odEdge(unsigned int crossingDir)
    {
        assert(crossingDir == 0 || crossingDir == 1);
        return &_odEdges[crossingDir];
    }

    ///@}
};

#endif //__ZEBRA_HPP__
#endif //INCLUDE_PEDESTRIANS
