#pragma once
#include "stdafx.h"
#include <d2d1.h>

/// \class	ImageDraw
///
/// \brief	Image draw draws the depthImage.

class ImageDraw
{
public:

	/// \fn	ImageDraw::ImageDraw();
	///
	/// \brief	Default constructor.

	ImageDraw();

	/// \fn	ImageDraw::~ImageDraw();
	///
	/// \brief	Destructor.

	~ImageDraw();

	/// \fn	bool ImageDraw::Initialize( HWND hWnd, ID2D1Factory *& pD2DFactory, int sourceWidth,
	/// 	int sourceHeight, int sourceStride );
	///
	/// \brief	Initializes this object.
	///
	/// \param	hWnd			   	Handle of the window.
	/// \param [in,out]	pD2DFactory	[in,out] If non-null, the D2Dfactory.
	/// \param	sourceWidth		   	Width of the source.
	/// \param	sourceHeight	   	Height of the source.
	/// \param	sourceStride	   	Source stride.
	///
	/// \return	true if it succeeds, false if it fails.

	bool Initialize( HWND hWnd, ID2D1Factory *& pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride );

	/// \fn	bool ImageDraw::GDP ( BYTE * image, unsigned long cbImage);
	///
	/// \brief	Draws the image to the screen after checking if the image is ok.
	///
	/// \param [in,out]	image	If non-null, the image.
	/// \param	cbImage		 	The image.
	///
	/// \return	true if it succeeds, false if it fails.

	bool GDP ( BYTE * image, unsigned long cbImage);

private:

	/// \fn	void ImageDraw::blankImageDraw( );
	///
	/// \brief	Empties the screen.

	void					blankImageDraw( );

	HWND					iDisplay;

	UINT                     sourceHeight;
    UINT                     sourceWidth;
    LONG                     sourceStride;

    // Direct2D 
    ID2D1Factory *           d2DFactory;
    ID2D1HwndRenderTarget *  renderTarget;
    ID2D1Bitmap *            bitmap;

	/// \fn	HRESULT ImageDraw::CreateResources();
	///
	/// \brief  Ensure necessary Direct2D resources are created
	///
	/// \return	The new resources.

	HRESULT CreateResources();

	/// \fn	void ImageDraw::discardResources();
	///
	/// \brief	Discard resources.

	void discardResources();

};