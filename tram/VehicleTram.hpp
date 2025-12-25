/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTram.hpp
 */
#ifdef INCLUDE_TRAMS
#ifndef __VEHICLE_TRAM_HPP__
#define __VEHICLE_TRAM_HPP__
#include "../Vehicle.hpp"

//######################################################################
/**
 * @~japanese 路面電車
 * @~english  Tram
 * @~ @ingroup TramSim Vehicle  
 */
class VehicleTram : public Vehicle
{
public:
    VehicleTram() {};
    virtual ~VehicleTram();

    /**
     * @~japanese 経路を設定する
     *
     * 経路探索は必要なく，規定された路面電車の路線から選択する
     *
     * @~english  Set route
     *
     * No need to search for a route, just choose from the defined
     * tram routes.  
     */
    void setRoute();

    //==================================================================
    /**
     * @~japanese @name 親クラスの関数のオーバーライド
     * @~english  @name Override functions of parent class
     */
    ///@{
    virtual void preperceive() override;

    virtual void perceive() override;

    virtual void determine() override;

    virtual void act() override;

    virtual void postact() override;

    ///@}
};

#endif //__VEHICLE_TRAM_HPP__
#endif //INCLUDE_TRAMS
