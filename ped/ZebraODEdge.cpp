/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file ZebraODEdge.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "ZebraODEdge.hpp"
#include "InflowPedestrianMonitorWriter.hpp"
#include "Pedestrian.hpp"
#include "Zebra.hpp"
#include "../AppMates.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../TimeManager.hpp"
#ifdef _OPENMP
#include <omp.h>
#endif //_OPENMP

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
ZebraODEdge::ZebraODEdge() : _zebra(nullptr)
{
    _outputsGeneratedPedestrianRecord = false;
    _lastGenerationTime               = 0;
    _lastInflowTime                   = 0;

    _waitingPedestrians.clear();
    _finishedPedestrians.clear();

    // 歩行者発生量のデフォルト値
    // Default value for pedestrian generation volume
    setGenerationVolume(100.0);

    _inflowMonitor = nullptr;
    _rng.reset();

#ifdef _OPENMP
    omp_init_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
ZebraODEdge::~ZebraODEdge()
{
    for (auto itr : _waitingPedestrians)
    {
        delete itr;
    }
    _waitingPedestrians.clear();

#ifdef _OPENMP
    omp_destroy_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void ZebraODEdge::appendWaitingPedestrian(Pedestrian* ped)
{
    assert(ped);

    // duplication check
    if (find(_waitingPedestrians.begin(), _waitingPedestrians.end(), ped)
        != _waitingPedestrians.end())
    {
        cerr << "ERROR: zebra[" << _zebra->id() << "(@"
             << _zebra->parent()->id() << ")][" << _crossingDirection
             << "] already has pedestrian$" << ped << endl;
        for (unsigned int i = 0; i < _waitingPedestrians.size(); i++)
        {
            cerr << "  " << i << ": p$" << _waitingPedestrians[i] << endl;
        }
        exit(EXIT_FAILURE);
    }
    _waitingPedestrians.push_back(ped);
}

//==============================================================================
void ZebraODEdge::pushPedestriansToRoadMap()
{
    if (_waitingPedestrians.empty())
    {
        return;
    }

    Pedestrian* tmpPed = _waitingPedestrians.front();
    _waitingPedestrians.pop_front();
    assert(tmpPed);

#ifdef PDS_DEBUG
    cerr << "z[" << _zebra->_id() << "][" << _crossingDirection "] pop new p$"
         << tmpPed << endl;
#endif

    /*
     * 希望歩行方向
     *   ZebraODEdge の direction はその辺に向かう方向であるので，
     *   辺から向かう方向に変更するため反転させる
     *
     * Desired walking direction
     *   ZebraODEdge's direction is the direction towards the edge,
     *   so flip the direction to change it to the direction from
     *   the edge. 
     */
    AmuVector dv = -_direction;

    /*
     * 初期位置
     *   確実に横断歩道の内部に入るようにするため，dv方向に
     *   少しだけずらす
     *
     * Initial position
     *   To ensure that the pedestrian is inside the crosswalk,
     *   move it slightly in the direction of dv.
     */
    double   baselinePosition = _rng.uniform();
    AmuPoint ip               = _initialPosition(baselinePosition);
    ip += _direction * 0.001;
    const_cast<PedestrianLocation*>(tmpPed->location())->setPosition(ip);

    // 歩行開始時刻を保存
    // Save starting time of walk
    const_cast<PedestrianLocation*>(tmpPed->location())
        ->setStartingTime(AppMates::getTimeManager().time());

    // 歩行者を横断歩道に配置
    // Place a pedestrian on this crosswalk
    bool result = AppMates::getObjectManager().addPedestrianToRoadMap(tmpPed);
    assert(result);

    _zebra->putPedestrian(tmpPed);

    Intersection* inter = dynamic_cast<Intersection*>(_zebra->parent());
    assert(inter);
    tmpPed->addToZebra(inter, _zebra, _crossingDirection, ip, dv);

    if (AppMates::getGVManager().getFlag("FLAG_OUTPUT_INFLOW_MONITOR")
        && _inflowMonitor)
    {
        ulint headway     = AppMates::getTimeManager().time() - _lastInflowTime;
        ulint genTime     = tmpPed->location()->generationTime();
        ulint genInterval = genTime - _lastGenerationTime;
        _inflowMonitor->recordInflowPedestrian(
            tmpPed, headway, genTime, genInterval);
    }

    _lastInflowTime     = AppMates::getTimeManager().time();
    _lastGenerationTime = tmpPed->location()->generationTime();
}

//==============================================================================
void ZebraODEdge::appendFinishedPedestrian(Pedestrian* ped)
{
#ifdef _OPENMP
    omp_set_lock(&_lock);
#endif //_OPENMP

    _finishedPedestrians.emplace_back(ped);

#ifdef _OPENMP
    omp_unset_lock(&_lock);
#endif //_OPENMP
}

//==============================================================================
void ZebraODEdge::deleteFinishedPedestrians()
{
    for (auto itr : _finishedPedestrians)
    {
        AppMates::getObjectManager().deletePedestrian(itr);
    }
    _finishedPedestrians.clear();
}

//==============================================================================
AmuPoint ZebraODEdge::_initialPosition(double baselineRatio)
{
    return _lineSegment.createInteriorPoint(baselineRatio, 1.0 - baselineRatio);
}

//==============================================================================
void ZebraODEdge::setGenerationVolume(double volume)
{
    _generationVolume      = volume;
    _generationProbability = _generationVolume
        * AppMates::getTimeManager().unit() / 60.0 / 60.0
        / 1000.0; // [ped./h]->[ped./step]
}

#endif //INCLUDE_PEDESTRIANS
