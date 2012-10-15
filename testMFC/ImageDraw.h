#pragma once

#include <d2d1.h>

class ImageDraw
{
public:
	// Constructor
	ImageDraw();

	// Destructor
	~ImageDraw();

	//Initialize
	bool Initialize( HWND hWnd, ID2D1Factory * pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride );

	// Set the window to draw to as well as the video format
	// implied biets per pixel is 32.
	bool GDP ( BYTE * image, unsigned long cbImage );

private:

	HWND					iDisplay;

	UINT                     sourceHeight;
    UINT                     sourceWidth;
    LONG                     sourceStride;

    // Direct2D 
    ID2D1Factory *           d2DFactory;
    ID2D1HwndRenderTarget *  renderTarget;
    ID2D1Bitmap *            bitmap;

	// Ensure necessary Direct2d resources are created
	HRESULT CreateResources();

	// Dispose of Direct2d resources.
	void Discard();

};