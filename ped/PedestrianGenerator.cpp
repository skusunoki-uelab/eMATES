/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file PedestrianGenerator.cpp
 */
#ifdef INCLUDE_PEDESTRIANS
#include "PedestrianGenerator.hpp"
#include "IntersectionPedExt.hpp"
#include "Zebra.hpp"
#include "ZebraODEdge.hpp"
#include "Pedestrian.hpp"
#include "../AppMates.hpp"
#include "../RoadMap.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../GVManager.hpp"
#include "../RandomNumberGenerator.hpp"

using namespace std;
using namespace amu::geometry;
using namespace amu::math;

//==============================================================================
PedestrianGenerator::PedestrianGenerator(RoadMap* roadMap)
{
    _roadMap = roadMap;
    _rng.reset();
}

//==============================================================================
PedestrianGenerator::~PedestrianGenerator() {}

//==============================================================================
void PedestrianGenerator::generatePedestrians()
{
    // 現段階では歩行者は横断歩道上にしか存在させない
    // At present, pedestrians are only allowed to exist on crosswalks.
    for (auto itr_i : _roadMap->intersections())
    {
        Intersection* inter = itr_i.second;
        for (auto itr_z : inter->pedExt()->zebras())
        {
            Zebra* zebra = itr_z.second;
            if (!zebra)
            {
                continue;
            }

            // 横断歩道に待機歩行者を生成する
            // Generate waiting pedestrians at crosswalks
            _generateWaitingPedestrian(zebra);

            /*
             * 横断歩道の信号が青でない場合は歩行者をポップしない
             *
             * If the traffic light on the crosswalk is not green,
             * not pop pedestrians.
             */
            if (inter->signal()->walkerColor(inter->direction(zebra))
                != SignalColor::WalkerState::BLUE)
            {
                continue;
            }

            /*
             * 待機歩行者の集合から歩行者を取り出し横断歩道に登録する
             *
             * Pick a pedestrian from a set of waiting pedestrians and
             * register it at a crosswalk
             */
            for (unsigned int dir = 0; dir < 2; dir++)
            {
                zebra->odEdge(dir)->pushPedestriansToRoadMap();
            }
        }
    }
}

//==============================================================================
bool PedestrianGenerator::_canGeneratePedestrian(Zebra* zebra, int dir)
{
    double rnd = _rng.uniform();
    return (rnd < zebra->odEdge(dir)->generationProbability());
}

//==============================================================================
void PedestrianGenerator::_generateWaitingPedestrian(Zebra* zebra)
{
    for (int dir = 0; dir < 2; dir++)
    {
        if (_canGeneratePedestrian(zebra, dir))
        {
            Pedestrian* tmpPed
                = AppMates::getObjectManager().createPedestrian();

            // 横断歩道の _waitingPedestrians に入れる
            // Put in _waitingPedestrians on the crosswalk
            zebra->odEdge(dir)->appendWaitingPedestrian(tmpPed);

#ifdef PDS_DEBUG
            cerr << "z[" << zebra->id() << "][" << dir << "] generate new ped$"
                 << tmpPed << endl;
#endif
        }
    }
}

#endif //INCLUDE_PEDESTRIANS
