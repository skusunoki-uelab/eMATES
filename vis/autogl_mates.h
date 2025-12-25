/* **************************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ************************************************************************** */
/**
 * @file autogl_mates.h
 */
#ifndef __AUTOGL_MATES_H__
#define __AUTOGL_MATES_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/**
 * @ingroup Drawing
 */
/** @{ */

/**
 * @~japanese 2次元の幅のある線分を描画する
 * @~english  Draw a 2D wide line segment
 */
void AutoGL_DrawBoldLine2D(
    double x0, double y0, double z0, double x1, double y1, double z1,
    double width);

/**
 * @~japanese 3次元の幅のある線分を描画する
 * @~english  Draw a 3D wide line segment
 */
void AutoGL_DrawBoldLine(
    double x0, double y0, double z0, double x1, double y1, double z1,
    double width);

/**
 * @~japanese 2次元の矢印を描画する
 * @~english  Draw a 2D arrow
 */
void AutoGL_DrawArrow2D(
    double x0, double y0, double z0, double x1, double y1, double z1);

/**
 * @~japanese 2次元の幅のある矢印を描画する
 * @~english  Draw a 2D wide arrow
 */
void AutoGL_DrawBoldArrow2D(
    double x0, double y0, double z0, double x1, double y1, double z1,
    double width);

/**
 * @~japanese 3次元の幅のある矢印を描画する
 * @~english  Draw a 3D wide arrow
 */
void AutoGL_DrawBoldArrow(
    double x0, double y0, double z0, double x1, double y1, double z1,
    double width);

/**
 * @~japanese 赤から青への順でコンターマップを設定する
 * @~english  Set the contour map in order from red to blue
 */
void AutoGL_SetContourMap_RB();

/**
 * @~japanese 地面を描画する
 * @~english  Draw the ground
 */
void AutoGL_DrawBackground2D();

/** @} */
#ifdef __cplusplus
}
#endif
#endif /*__AUTOGL_MATES_H__*/
