/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file BgImage.cpp
 */
#ifdef USE_OPENCV

#include "BgImage.hpp"
#include <cmath>
#include <iostream>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <autogl.h>

using namespace std;

//#define DEBUG_BG_TEXTURE

//======================================================================
BgImage::BgImage()
{
    _sizeSlicedImage = 512;
    _sizeOverlapped  = 5;
    _slicedImages.clear();
}

//======================================================================
BgImage::~BgImage()
{
    if (_textureIds)
    {
        free(_textureIds);
    }
}

//======================================================================
bool BgImage::loadImage(const char* fname, bool grayscaleIsOn)
{
    // ファイル読み込み
    // File read
    _srcImage = cv::imread(fname);
    if (_srcImage.empty())
    {
        cerr << "WARNING: image file not found. - " << fname << endl;
        return false;
    }

    if (grayscaleIsOn)
    {
        cv::Mat grayImage;
#if (CV_VERSION_MAJOR >= 4)
        cv::cvtColor(_srcImage, grayImage, cv::COLOR_RGB2GRAY);
        cv::cvtColor(grayImage, _srcImage, cv::COLOR_GRAY2RGB);
#else  //if CV_VERSION_MAJOR >= 4 is not satisfied
        cvtColor(_srcImage, grayImage, CV_RGB2GRAY);
        cvtColor(grayImage, _srcImage, CV_GRAY2RGB);
#endif //CV_VERSION_MAJOR >= 4
        grayImage.release();
    }

    // テクスチャとシミュレーション世界の軸を合わせるため反転
    // Flip to align the texture and simulated world axes
    cv::flip(_srcImage, _srcImage, 0);
    _orgWidth  = _srcImage.cols;
    _orgHeight = _srcImage.rows;

    return true;
}

//======================================================================
bool BgImage::prepareTexture()
{
    if (_srcImage.empty())
    {
        cerr << "WARNING: image file has not been set." << endl;
        return false;
    }

    // 画像のリサイズ
    // Resizing image
    _resizeImage();

    // テクスチャIDの生成
    // Generating texture ID number
    if (_textureIds)
    {
        delete _textureIds;
    }
    _textureIds = (GLuint*) malloc(
        _numImagesInRow * _numImagesInCol * sizeof(GLuint));
    glGenTextures(_numImagesInRow * _numImagesInCol, _textureIds);

    // 画像の分割とテクスチャIDとのひも付け
    // Slice image and associate them with texture ID numbers
    _sliceImage();

    // 画像とテクスチャのバインド
    // Bind image and texture
    _bindTextures();

    return true;
}

//======================================================================
void BgImage::_resizeImage()
{
#ifdef DEBUG_BG_TEXTURE
    cout << "image size: " << _srcImage.cols << "x" << _srcImage.rows
         << endl;
#endif //DEBUG_BG_TEXTURE

    /*
     * 分割後の画像が _sizeSlicedImage x _sizeSlicedImage になるよう
     * 余白を加える．このとき，各分割画像は_sizeOverlapped の袖領域を
     * もつので注意する．
     *
     * Add margins so that the image after division is _sizeSlicedImage
     * x _sizeSlicedImage. Note that each sliced image has a sleeve area
     * of _sizeOverlapped.
     */
    _numImagesInRow
        = ceil(_orgWidth / static_cast<double>(_sizeSlicedImage));
    _numImagesInCol
        = ceil(_orgHeight / static_cast<double>(_sizeSlicedImage));

    int resizedWidth = _sizeSlicedImage * _numImagesInRow
                       - _sizeOverlapped * (_numImagesInRow - 1);
    int resizedHeight = _sizeSlicedImage * _numImagesInCol
                        - _sizeOverlapped * (_numImagesInCol - 1);

    _xMargin = (resizedWidth - _srcImage.cols) / 2;
    _yMargin = (resizedHeight - _srcImage.rows) / 2;

#ifdef DEBUG_BG_TEXTURE
    cout << "new size: " << resizedWidth << "x" << resizedHeight
         << endl;
#endif //DEBUG_BG_TEXTURE

    cv::Mat resizedImage(
        cv::Size(resizedWidth, resizedHeight), CV_8UC3,
        CV_RGB(255, 255, 255));
    cv::Mat srcROI(
        resizedImage,
        cv::Rect(_xMargin, _yMargin, _orgWidth, _orgHeight));
    _srcImage.copyTo(srcROI);

    cv::swap(_srcImage, resizedImage);
}

//======================================================================
void BgImage::_sliceImage()
{
    _slicedImages.clear();

    int index = 0;
    for (unsigned int j = 0; j < _numImagesInCol; j++)
    {
        for (unsigned int i = 0; i < _numImagesInRow; i++)
        {
            SlicedImage newSlicedImage(*(_textureIds + index), i, j);

            /*
             * 深いコピー (clone) が必要
             *   分割画像のサイズは _sizeSlicedImage x _sizeSlicedImage
             *   になるが，袖領域に注意．
             *
             * Requires deep copy (clone)
             *   The size of the split image will be _sizeSlicedImage x
             *   _sizeSlicedImage, but note the sleeve area.
             */
            newSlicedImage.setImage(
                _srcImage(cv::Rect(
                              (_sizeSlicedImage - _sizeOverlapped) * i,
                              (_sizeSlicedImage - _sizeOverlapped) * j,
                              _sizeSlicedImage, _sizeSlicedImage))
                    .clone());

            _slicedImages.push_back(newSlicedImage);
            index++;
        }
    }
}

//======================================================================
void BgImage::_bindTextures()
{
    for (auto itr : _slicedImages)
    {
        glBindTexture(GL_TEXTURE_2D, itr.id());
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        /*
             * glTextImage2D(
             *   type of texture,
             *   pyramid level for mip-mapping,
             *   internal color format to convert to,
             *   image width [px],
             *   image height [px],
             *   border width [px],
             *   input image format,
             *   image data type,
             *   actual image data)
             */
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, itr.image()->cols,
            itr.image()->rows, 0, GL_BGR, GL_UNSIGNED_BYTE,
            itr.image()->ptr());
        itr.image()->release();
    }
}

//======================================================================
bool BgImage::setRegion(
    double xminInput, double xmaxInput, double yminInput,
    double ymaxInput, bool keepsAspectRatio)
{
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /*
     * _resizeImage で設定したマージンをを考慮して実際の描画領域を決定
     *
     * Determine the actual drawing area considering the margin set by
     * _resizeImage.
     */
    double widthInput  = xmaxInput - xminInput;
    double heightInput = ymaxInput - yminInput;

    if (keepsAspectRatio)
    {
        // 高さ / 幅
        // Height / width
        double aspectRatio
            = _srcImage.rows / static_cast<double>(_srcImage.cols);

        // 描画領域内に収まるよう補正する
        // Correct to fit within the drawing region
        if (heightInput < widthInput * aspectRatio)
        {
            widthInput = heightInput / aspectRatio;
        }
        else
        {
            heightInput = widthInput * aspectRatio;
        }
    }

    double xmin = xminInput - _xMargin * widthInput / _orgWidth;
    double xmax
        = xminInput + widthInput + _xMargin * widthInput / _orgWidth;
    double ymin = yminInput - _yMargin * heightInput / _orgHeight;
    double ymax
        = yminInput + heightInput + _yMargin * heightInput / _orgHeight;

#ifdef DEBUG_BG_TEXTURE
    cout << "region: (" << xmin << ", " << ymin << "), (" << xmax
         << ", " << ymax << ")" << endl;
#endif //DEBUG_BG_TEXTURE

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 分割画像を描画する際の幅と高さを求める
    // Find the width and height when drawing split images
    double widthOverlapped = _sizeOverlapped * widthInput / _orgWidth;
    double heightOverlapped
        = _sizeOverlapped * heightInput / _orgHeight;

#ifdef DEBUG_BG_TEXTURE
    cout << "(widthOverlapped, heightOverlapped) = (" << widthOverlapped
         << ", " << heightOverlapped << ")" << endl;
#endif //DEBUG_BG_TEXTURE

    double widthPerSlicedImage
        = (xmax - xmin + (_numImagesInRow - 1) * widthOverlapped)
          / _numImagesInRow;
    double heightPerSlicedImage
        = (ymax - ymin + (_numImagesInCol - 1) * heightOverlapped)
          / _numImagesInCol;

#ifdef DEBUG_BG_TEXTURE
    cout << "(widthPerSlicedImage, heightPerSlicedImage) = ("
         << widthPerSlicedImage << ", " << heightPerSlicedImage << ")"
         << endl;
#endif //DEBUG_BG_TEXTURE

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // 各分割画像を描画する領域を求める
    // Find the area to draw each split image
    for (auto itr : _slicedImages)
    {
        itr.setX0(
            xmin
            + (widthPerSlicedImage - widthOverlapped) * itr.orderX());
        itr.setX1(itr.x0() + widthPerSlicedImage);
        itr.setY0(
            ymin
            + (heightPerSlicedImage - heightOverlapped) * itr.orderY());
        itr.setY1(itr.y0() + heightPerSlicedImage);
#ifdef DEBUG_BG_TEXTURE
        cout << "texture[" << itr.id() << "]: (" << itr.x0() << ", "
             << itr.y0() << "), (" << itr.x1() << ", " << itr.y1()
             << ")" << endl;
#endif //DEBUG_BG_TEXTURE
    }
    return true;
}

//======================================================================
void BgImage::putImage(double z)
{
    if (_srcImage.empty())
    {
        cerr << "ERROR: image file not set." << endl;
        return;
    }

    for (auto itr : _slicedImages)
    {
        AutoGL_BeginNativeCall();
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, itr.id());

            glBegin(GL_QUADS);
            {
                glTexCoord2d(0.0, 1.0);
                glVertex3d(itr.x0(), itr.y1(), z);

                glTexCoord2d(0.0, 0.0);
                glVertex3d(itr.x0(), itr.y0(), z);

                glTexCoord2d(1.0, 0.0);
                glVertex3d(itr.x1(), itr.y0(), z);

                glTexCoord2d(1, 1);
                glVertex3d(itr.x1(), itr.y1(), z);
            }
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        AutoGL_EndNativeCall();
    }
}

//======================================================================
bool BgImage::clearImage()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(_numImagesInCol * _numImagesInRow, _textureIds);
    _slicedImages.clear();
    return true;
}

#endif //USE_OPENCV
