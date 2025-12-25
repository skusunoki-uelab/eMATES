/* ******************************************************************
 * Copyright 2014-2024 ADVENTURE Project
 * All Rights Reserved
 * ****************************************************************** */
/**
 * @file BgImage.hpp
 */
#ifndef __BG_IMAGE_HPP__
#define __BG_IMAGE_HPP__

#ifdef USE_OPENCV

#include <vector>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>

//######################################################################
/**
 * @~japanese 背景画像を処理する
 * @~english  Process the background image
 * @~ @ingroup Visualization Drawing
 */
class BgImage
{
    //==================================================================
private:
    /**
     * @~japanese 分割された画像の情報を格納する構造体
     * @~english  Strict storing information about the split image 
     */
    struct SlicedImage
    {
    private:
        /**
         * @~japanese テクスチャID
         * @~english  Texture ID number
         */
        GLuint _id;

        /**
         * @~japanese 画像データ
         * @~english  Image data
         */
        cv::Mat _image;

        /**
         * @~japanese 幅方向 (x) の表示順序
         * @~english  Display order in the width (x) direction
         */
        unsigned int _orderX;

        /**
         * @~japanese 高さ方向 (y) の表示順序
         * @~english  Display order in the height (y) direction
         */
        unsigned int _orderY;

        /**
         * @~japanese @name 描画領域
         * @~english  @name Drawing area
         */
        ///@{
        double _x0;
        double _x1;
        double _y0;
        double _y1;
        ///@}

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        SlicedImage(GLuint id, unsigned int orderX, unsigned int orderY)
            : _id(id), _orderX(orderX), _orderY(orderY)
        {
        }
        ~SlicedImage() {};

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        /**
         * @~japanese @name アクセッサ
         * @~english  @name Accessor
         */
        ///@{
        GLuint id() const
        {
            return _id;
        }

        cv::Mat* image()
        {
            return &_image;
        }

        void setImage(cv::Mat image)
        {
            _image = image;
        }

        unsigned int orderX() const
        {
            return _orderX;
        }

        unsigned int orderY() const
        {
            return _orderY;
        }

        double x0() const
        {
            return _x0;
        }

        void setX0(double x)
        {
            _x0 = x;
        }

        double x1() const
        {
            return _x1;
        }

        void setX1(double x)
        {
            _x1 = x;
        }

        double y0() const
        {
            return _y0;
        }

        void setY0(double y)
        {
            _y0 = y;
        }

        double y1() const
        {
            return _y1;
        }

        void setY1(double y)
        {
            _y1 = y;
        }

        ///@}
    };

    //==================================================================
public:
    BgImage();
    ~BgImage();

    /**
     * @~japanese ファイル名が @p fname である背景画像を読み込む
     * @~english  Load a background image whose file name is @p fname
     */
    bool loadImage(const char* fname, bool showsGrayscale);

public:
    /**
     * @~japanese テクスチャを準備する
     * @~english  Prepare texture
     */
    bool prepareTexture();

private:
    /**
     * @~japanese 画像をリサイズする
     * @~english  Resize image
     */
    void _resizeImage();

    /**
     * @~japanese 大きなサイズの画像を分割する
     * @~english  Slice a large-size image
     */
    void _sliceImage();

    /**
     * @~japanese 画像とテクスチャをバインドする
     * @~english  Bind images and textures
     */
    void _bindTextures();

public:
    /**
     * @~japanese テクスチャを貼り付けるための領域を設定する
     * @~english  Set the area for drawing the texture
     */
    bool setRegion(
        double xminInput, double xmaxInput, double yminInput,
        double ymaxInput, bool keepsAspectRatio);

    /**
     * @~japanese テクスチャを描画する
     * @~english  Draw the textures
     */
    void putImage(double z);

    /**
     * @~japanese テクスチャ画像をクリアする
     * @~english  Clear the texture image
     */
    bool clearImage();

    //==================================================================
private:
    /**
     * @~japanese テクスチャにするためのソース画像
     * @~english  Source image for texture
     */
    cv::Mat _srcImage;

    /**
     * @~japanese @name ソース画像のサイズ
     *
     * _srcImageは読み込んだ後でリサイズするので別に記憶する
     * 
     * @~english  @name Source image size
     *
     * _srcImage is resized after loading, so record it separately.
     */
    ///@{
    unsigned int _orgWidth;
    unsigned int _orgHeight;
    ///@}

    /**
     * @~japanese
     * @name 分割前の画像に含まれる余白
     *
     * @~english
     * @name Margins included in the source image before splitting
     */
    ///@{
    unsigned int _xMargin;
    unsigned int _yMargin;
    ///@}

    /**
     * @~japanese 分割後の画像のサイズ
     * @name      2のn乗にすることが望ましい
     *
     * @~english  Image size after splitting
     * @note      Is it desirable to be the n-th power of 2?
     */
    unsigned int _sizeSlicedImage;

    /**
     * @~japanese 袖領域のサイズ
     * @~english  Sleeve area size
     */
    unsigned int _sizeOverlapped;

    /**
     * @~japanese 幅方向の分割画像数
     *
     * 1行 (row) あたりの画像数
     * 
     * @~english  Number of split images in width direction
     *
     * Number of images per row
     */
    unsigned int _numImagesInRow;

    /**
     * @~japanese 高さ方向の分割画像数
     *
     * 1列 (column) あたりの画像数
     * 
     * @~english  Number of split images in height direction
     *
     * Number of images per column
     */
    unsigned int _numImagesInCol;

    /**
     * @~japanese テクスチャID
     * @~english  Texture ID number
     */
    GLuint* _textureIds;

    /**
     * @~japanese 分割画像の集合
     * @~english  Set of split images
     */
    std::vector<SlicedImage> _slicedImages;
};

#endif //USE_OPENCV
#endif //__BG_IMAGE_HPP__
