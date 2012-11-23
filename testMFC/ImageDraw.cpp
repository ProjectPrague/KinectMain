//this class is for making the image from a byte array.

#include "ImageDraw.h"

// Constructor
ImageDraw::ImageDraw() :
	iDisplay(0),
	sourceWidth(0),
	sourceHeight(0),
	sourceStride(0),
	d2DFactory(NULL), 
	renderTarget(NULL),
	bitmap(0)
{

}

// Destructor
ImageDraw::~ImageDraw()
{
	discardResources();
}

HRESULT ImageDraw::CreateResources()
{
	HRESULT hr = S_OK;

	if( !renderTarget )
	{
		D2D1_SIZE_U size = D2D1::SizeU( sourceWidth, sourceHeight);

		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		hr = d2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(iDisplay, size),
			&renderTarget
			);

		if ( FAILED(hr) )
		{
			return hr;
		}

		// Create a bitmap that we can copy image date into and then render to the target.
		hr = renderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&bitmap
			);

		if ( FAILED(hr) )
		{
			SafeRelease( renderTarget );
			return hr;
		}


	}

	return hr;
}

//Safely discard the Direct2d resources.
void ImageDraw::discardResources()
{
	SafeRelease(renderTarget);
	SafeRelease(bitmap);
}


bool ImageDraw::Initialize( HWND hWnd, ID2D1Factory * pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride )
{

	if ( NULL == pD2DFactory)
	{
		return false;
	}

	iDisplay = hWnd;

	// One factory for the entire application, so save a pointer!
	d2DFactory = pD2DFactory;

	d2DFactory->AddRef( );

	// Get the frame size.
	this->sourceWidth = sourceWidth;
	this->sourceHeight = sourceHeight;
	this->sourceStride = sourceStride;

	return true;
}

// GDP = Graphical Data Processor.
bool ImageDraw::GDP ( BYTE * image, unsigned long cbImage)
{
	// Check for inforrectly sized image data.
	if ( cbImage < ((sourceHeight - 1) * sourceStride) + (sourceWidth * 4) )
	{
		return false;
	}

	// create the resources for this device
	// they will be recreated if previously lost
	HRESULT hr = CreateResources();

	if ( FAILED(hr) )
	{
		return false;
	}

	// copy the image that was passed in into the direct2d bitmap
	hr = bitmap->CopyFromMemory( NULL, image, sourceStride);

	if ( FAILED(hr) )
	{
		return false;
	}

	renderTarget->BeginDraw();

	// Draw the bitmap streched to the size of the window.
	renderTarget->DrawBitmap( bitmap );

	hr = renderTarget->EndDraw();

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing.
	if ( hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		discardResources();
	}
	return SUCCEEDED( hr );
}	

ID2D1HwndRenderTarget * ImageDraw::getRenderTarget()
{
	if (renderTarget == NULL)
	{
		CreateResources();
	}

	return renderTarget;
}