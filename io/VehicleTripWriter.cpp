/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTripWriter.cpp
 */
#include "VehicleTripWriter.hpp"
#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../GVManager.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../TimeManager.hpp"
#include "../VehicleBodyProperty.hpp"
#include "../VehicleEV.hpp"
#include "../VehicleGlobalRoute.hpp"
#include "../VehicleLocation.hpp"
#include <cassert>
#include <cstdlib>
#include <vector>

using namespace std;

//======================================================================
void VehicleTripWriter::writeVehicleTrip(Vehicle *vehicle) {
  ofstream *fout = AppMates::getFileManager().getOFStream(
      AppMates::getGVManager().getString("RESULT_VEHICLE_TRIP_FILE"));

  *fout << vehicle->id() << "," << *(vehicle->body()->type()) << ","
        << vehicle->location()->startingTime() << ","
        << AppMates::getTimeManager().time() << ","
        << (AppMates::getTimeManager().time() -
            vehicle->location()->startingTime())
        << "," << vehicle->globalRoute()->start()->id() << ","
        << vehicle->globalRoute()->goal()->id() << ","
        << vehicle->location()->tripLength() << endl;

  fout->close();
}

//======================================================================
void VehicleTripWriter::writeVehicleTripEV(VehicleEV *vehicle) {
  ofstream *fout = AppMates::getFileManager().getOFStream(
      AppMates::getGVManager().getString("RESULT_VEHICLE_TRIP_FILE"));
  if (!(fout->good())) {
    cout << "[ERROR] cannot open file - "
         << AppMates::getGVManager().getString("RESULT_VEHICLE_TRIP_FILE")
         << endl;
    exit(EXIT_FAILURE);
  }

  *fout << vehicle->id() << "," << *(vehicle->body()->type()) << ","
        << vehicle->location()->startingTime() << ","
        << AppMates::getTimeManager().time() << ","
        << (AppMates::getTimeManager().time() -
            vehicle->location()->startingTime())
        << ","
        //<< vehicle->globalRoute()->start()->id() << ","
        //<< vehicle->globalRoute()->goal()->id() << ","
        //<< vehicle->location()->tripLength() << ","

        // by uchida 2017/2/8
        // CSでの充電時間を記録
        // まずCSの待ち行列に追加された時刻
        << vehicle->waitingLineEntryTime()
        << "," // 5
        // 充電開始時刻(上下の２つは同じ値になることもある)
        << vehicle->startChargingTime()
        << "," // 6
        // CSの出庫時刻
        << vehicle->restartTime()
        << "," // 7
        // それにあわせて全旅行時間から割り引く
        << (AppMates::getTimeManager().time() -
            vehicle->location()->startingTime()) -
               (vehicle->restartTime() - vehicle->waitingLineEntryTime())
        << "," // 8 充電除く移動時間
        << AppMates::getTimeManager().time() -
               vehicle->location()->startingTime()
        << "," // 9 総移動時間
        << vehicle->startChargingTime() - vehicle->waitingLineEntryTime()
        << "," // 10 充電待ち時間
        << vehicle->restartTime() - vehicle->startChargingTime()
        << "," // 11 充電時間
        << vehicle->restartTime() - vehicle->waitingLineEntryTime()
        << ","                                          // 12 CS滞在時間
        << vehicle->globalRoute()->start()->id() << "," // 13
        << vehicle->globalRoute()->goal()->id() << ","  // 14
        << vehicle->location()->tripLength()
        << "," // 15 移動距離
        // 充電が行われたCSのID // by obinata 2024/9/2
        << (vehicle->visitedCS() ? vehicle->visitedCS()->id() : "")
        << "," // 16
        // CS到着時のSoC //by obinata 2024/9/30
        << vehicle->SOC()
        << "," // 17
        // CS探索時のコストを記録
        << vehicle->chosenCSCost()
        // 相対ギャップを取得
        << "," << vehicle->relativeGap()
        << "," // 18
               //   << vehicle->evalByWaitingTimeSumCost(RouterBase, ) << ",";
               //   //18
               // EVが実際に消えた地点を記録(初期設定（GoalID）とは別) //by
               // obinata 2024/9/30
               //<< vehicle->location()->intersection()->id() << ","
               // by takusagawa 2018/11/10
               // 待ち時間情報を受け取れるかどうか.
               // 受け取れる場合は1,受け取れない場合は0
        << (vehicle->canReceiveWaitingInfo() ? "1" : "0") << endl;

  // fout->close()はFileManagerで行われる
  // fout->close() will be done in FileManager
}

//======================================================================
void VehicleTripWriter::writeAllVehiclesTrip() {
  ofstream *fout = AppMates::getFileManager().getOFStream(
      AppMates::getGVManager().getString("RESULT_VEHICLE_TRIP_FILE"));

  for (auto itr : AppMates::getObjectManager().vehicles()) {
    *fout << itr->id() << "," << *(itr->body()->type()) << ","
          << itr->location()->startingTime() << "," << "******"
          << ","
          << (AppMates::getTimeManager().time() -
              itr->location()->startingTime())
          << "," << itr->globalRoute()->start()->id() << ","
          << itr->globalRoute()->goal()->id() << ","
          << itr->location()->tripLength() << endl;
  }

  fout->close();
}
