/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file VehicleTripWriter.cpp
 */

#include "../AppMates.hpp"
#include "../FileManager.hpp"
#include "../Intersection.hpp"
#include "../ObjectManager.hpp"
#include "../TimeManager.hpp"
#include "../VehicleBodyProperty.hpp"
#include "../VehicleGlobalRoute.hpp"
#include "../VehicleLocation.hpp"
#include "VehicleTripWriter.hpp"
#include <cassert>
#include <cstdlib>
#include <vector>


using namespace std;

//=====================================================================
void WaitingLineWriter::writeWaitingLine(VehicleEV *vehicle) {
  ofstream *fout = AppMates::getFileManager().getOFStream(
      AppMates::getGVManager().getString("RESULT_WAITINGLINE_FILE"));
  if (!(fout->good())) {
    cout << "[ERROR] cannnot open file -"
         << AppMates::getGVManager().getString("RESULT_WAITINGLINE_FILE")
         << endl;
    exit(EXIT_FAILURE);
  }

  while (time > AppMates::getTimeManager().time()) {
    incrementStep();
    // 2022/11/29 by uchida
    // 2025/7/30 by komatsu
    // 待ち行列と利用時間(1分毎に出力)
    (AppMates::getTimeManager().time() % 60000 == 0) //%60000 == 0
    {
      // 元のベクトルポインタを取得
      vector<CSNodeBase *> fcsNodes = _roadMap->csNodesFast();
      vector<CSNodeBase *> csNodes = _roadMap->csNodes();
      // --- csNodes に含まれ、fcsNodes には含まれないポインタのリストを作成 ---
      vector<CSNodeBase *> ncsNodes;
      for (CSNodeBase *csNode_ptr : csNodes) {
        bool found = false;
        for (CSNodeBase *fcsNode_ptr : fcsNodes) {
          if (csNode_ptr == fcsNode_ptr) {
            found = true;
            break; // fcsNodes に見つかったので、内側のループを終了
          }
        }
        if (!found) {
          ncsNodes.push_back(csNode_ptr); // fcsNodes に見つからなかったので追加
        }
      }
      const int tMax = 97200;
      const int dt = 60;

      // const string filename = "waitingline_log.csv";
      {
        ofstream fout(filename, ios::trunc);
        fout << "time";
        for (int i = 0; i < fcsNodes.size(); i++) {
          fout << ", CS" << setw(2) << setfill('0') << (i + 1);
        }
        fout << endl;
      }

      // ====各タイムステップごとに1行追加====
      for (int t = 0; t < tMax; t += dt) {
        ofstream fout(filename, ios::app);
        fout << t;

        for (int i = 0; i < fcsNodes.size(); i++) {
          fout << ", " << (fcsNodes[i]->waitingVehicles()).size();
        }
        fout << endl;

        cout << "t=" << setw(6) << t << "書き込み完了" << endl;
      }

      writeEVComm(); // [eMATES]
    }
  }
