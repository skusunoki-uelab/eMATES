/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file Config.hpp
 */
#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__
#include <cstdint>

//======================================================================
/**
 * @~japanese @name 円周率の定義
 * @~english  @name Define PI
 */
///@{
#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846; ///< \f$ \pi \f$
#endif
#ifndef M_PI_2
constexpr double M_PI_2 = 1.57079632679489661923; ///< \f$ \pi/2 \f$
#endif
#ifndef M_PI_4
constexpr double M_PI_4 = 0.78539816339744830962; ///< \f$ \pi/4 \f$
#endif
#ifndef M_1_PI
constexpr double M_1_PI = 0.31830988618379067154; ///< \f$ 1/\pi \f$
#endif
#ifndef M_2_PI
constexpr double M_2_PI = 0.63661977236758134308; ///< \f$ 2/\pi \f$
#endif
///@}

/** @name 電力消費に関わる物理量の定義 */
//@{
#ifndef GRA
constexpr double GRA = 9.80665; //< g
#endif
#ifndef RHO
constexpr double RHO = 1.205; //< Rho 20℃のとき
#endif
//@}

//======================================================================
/**
 * @~japanese @name 識別番号の桁数
 * @~english  @name Number Digits of ID number
 */
///@{

/**
 * @~japanese 交差点 (Intersection) の識別番号
 * @~english  Intersection ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_INTERSECTION = 6;

/**
 * @~japanese 単路部 (Section) の識別番号
 * @~english  Section ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_SECTION =
    (NUM_FIGURE_FOR_INTERSECTION * 2);

/**
 * @~japanese サブセクション (SubSection) の識別番号
 * @~english  SubSection ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_SUBSECTION = 2;

/**
 * @~japanese レーン (Lane) の識別番号
 * @~english  Lane ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_LANE = 8;

/**
 * @~japanese
 * コネクタ (Connector) のグローバル識別番号 (表示用)
 *
 * @note
 * 描画時のみ用いられる．実際の識別番号はコネクタが生成されるたびに
 * "コネクタ総数 + 1" で与えられる．
 *
 * @~english
 * Connector (global ID number - for drawing)
 *
 * @note
 * Used only when drawing. The actual ID number is given
 * "total num. of generated connectors + 1" each time it is generated.
 */
constexpr unsigned int NUM_FIGURE_FOR_CONNECTOR_GLOBAL = 2;

/**
 * @~japanese コネクタのローカル識別番号
 * @~english  Connector (local ID number)
 */
constexpr unsigned int NUM_FIGURE_FOR_CONNECTOR_LOCAL = 4;

/**
 * @~japanese ODノードグループの識別番号
 * @~english  Group ID number of OD nodes
 */
constexpr unsigned int NUM_FIGURE_FOR_OD_GROUP = 6;

/**
 * @~japanese 経路探索用ネットワーク階層数
 * @~english  Number of network layers for routing
 */
constexpr unsigned int NUM_FIGURE_FOR_ROUTING_LAYER = 2;

/**
 * @~japanese 経路探索用ノードの識別番号
 * @~english  ID number of the node for routing
 */
constexpr unsigned int NUM_FIGURE_FOR_ROUTING_NODE =
    (NUM_FIGURE_FOR_ROUTING_LAYER + NUM_FIGURE_FOR_SECTION);

/**
 * @~japanese 経路探索用リンクの識別番号
 * @~english  ID number of the link for routing
 */
constexpr unsigned int NUM_FIGURE_FOR_ROUTING_LINK =
    (NUM_FIGURE_FOR_ROUTING_LAYER + NUM_FIGURE_FOR_SECTION);

/**
 * @~japanese 経路探索用リンクの重複識別用番号
 * @~english  A number to distinguish duplication for the link for routing
 */
constexpr unsigned int NUM_FIGURE_FOR_SAME_SUBID = 2;

/**
 * @~japanese 感知器の識別番号
 * @~english  Monitor ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_MONITOR = 6;

#ifdef INCLUDE_VEHICLES
/**
 * @~japanese 車両の識別番号
 * @~english  Vehicle ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_VEHICLE = 6;
#endif // INCLUDE_VEHICLES

#ifdef INCLUDE_PEDESTRIANS
/**
 * @~japanese 歩行者の識別番号
 * @~english  Pedestrian ID number
 */
constexpr unsigned int NUM_FIGURE_FOR_PEDESTRIAN = 6;
#endif // INCLUDE_PEDESTRIANS

/**
 * @~japanese
 * 時系列データのファイル名に用いる時刻の桁数
 *
 * @~english
 * Number of digits for time used in file names of historical data
 */
constexpr unsigned int NUM_FIGURE_FOR_TIMELINE_FILENAME = 10;

/**
 * @~japanese 時刻ステップ描画用桁数
 * @~english  Number of digits for displaying time step
 */
constexpr unsigned int NUM_FIGURE_FOR_DRAW_TIME = 6;

///@}

//======================================================================
/**
 * @~japanese @name 道路ネットワークに関する定数
 * @~english  @name Constants related to road network
 */
///@{

/**
 * @~japanese 横のレーンを探索するときの線分の長さ [m]
 * @~english  Length [m] of line segment when searching for side lanes
 */
constexpr double SEARCH_SIDE_LANE_LINE_LENGTH = 10.0;

///@}

//======================================================================
/**
 * @~japanese @name 信号に関する定数
 * @~english  @name Constants related to traffic lights
 */
///@{

/**
 * @~japanese スプリットの最大数
 * @~english  Max number of split
 */
constexpr unsigned int NUM_MAX_SPLIT = 20;

///@}

//======================================================================
/**
 * @~japanese @name エージェントに関する定数
 * @~english  @name Constants related to agents
 */
///@{

/**
 * @~japanese 経路探索パラメータの数
 * @~english  Number of parameters for routing
 */
// [eMATES] 2025/5/30 by abe 5->7に変更。CS出力・CS充電料金分。
constexpr unsigned int VEHICLE_ROUTING_PARAMETER_SIZE = 7;

/**
 * @~japanese
 * 経路探索用選好パラメータのインデックス
 *
 * @note
 * 順に，旅行距離，予想旅行時間，直進回数，左折回数，右折回数を表す
 *
 * @~english
 * Index for routing preferences
 *
 * @note
 * Trip distance, expected trip time, number of straight driving,
 * number of left turn, and number of right turn, in order.
 */
enum class RoutingParamIndex : unsigned int {
  DISTANCE = 0,
  TIME = 1,
  STRAIGHT = 2,
  LEFT_TURN = 3,
  RIGHT_TURN = 4,
  CS_TIME = 5, // [eMATES] CS充電時間
  CS_YEN = 6   // [eMATES] CS充電料金
};

/**
 * @~japanese リンク旅行時間算出用の車両数
 * @~english  Number of vehicles for link travel time calculation
 */
constexpr unsigned int VEHICLE_PASS_TIME_INTERSECTION = 10;

// [eMATES] CS進入時の経路コストのペナルティ
// constexpr double CS_ENTRY_PENALTY = 1e4;
// 251119　routeのコスト評価のため0にする Kusunoki
constexpr double CS_ENTRY_PENALTY = 0.0;

// [eMATES] 追加251226：フィーダー別配電網ペナルティのデフォルト値（楠木）
// F21-F28の7つのフィーダーに対するデフォルト係数
// Optunaで最適化する際の初期値として使用
constexpr double DEFAULT_FEEDER_PENALTY_F21 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F22 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F23 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F24 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F25 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F26 = 1.0;
constexpr double DEFAULT_FEEDER_PENALTY_F28 = 1.0;  // 高負荷フィーダーは高めに設定

// グリッドペナルティのスケール係数（route/waiting等のコストとバランスを取るため）
// 他のコスト項が600-1000程度なので、ペナルティを同等の影響力にする
constexpr double GRID_PENALTY_SCALE = 100.0;
// グリッドコストを考慮しない（NE時計算のため）
//constexpr double GRID_PENALTY_SCALE = 0.0;

///@}

//======================================================================
/**
 * @~japanese @name エイリアス
 * @~english  @name Aliases
 */
///@{

/**
 * @~japanese
 * 9桁を保証するunsigned longのエイリアス
 *
 * @attention
 * 互換性のため宣言を残している
 *
 * @~english
 * Alias of unsigned long guaranteeing 9 digits
 *
 * @attention
 * Leave it for compatibility.
 */
using ulint = uint32_t;

///@}

#endif //__CONFIG_HPP__
