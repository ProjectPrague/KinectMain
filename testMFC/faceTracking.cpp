#include "faceTracking.h"

FaceTracking::FaceTracking(HWND hwnd, ID2D1Factory *d2DFactory)
{
	this -> d2DFactory = d2DFactory;
	this->hWnd = hwnd;
	locked = false;
	// All variables must be NULL before the beginning.
	faceTrackingResult = NULL;
	nuiPresent = NULL;
	ColorBuffer = NULL;
	DepthBuffer = NULL;
	renderTarget = NULL;
}

FaceTracking::~FaceTracking()
{
	SafeRelease(renderTarget);
	// destruct all variables to NULL before shutting down.
}

void FaceTracking::setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture){
	d2DcolorData->CopyFromMemory(NULL, static_cast<BYTE *>(lockedRect.pBits), 640 * 4);
	memcpy(faceTrackingColorData->GetBuffer(), PBYTE(lockedRect.pBits), min(faceTrackingColorData->GetBufferSize(),UINT(texture->BufferLen())));
}

void FaceTracking::setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture){
	memcpy(faceTrackingDepthData->GetBuffer(), PBYTE(lockedRect.pBits), min(faceTrackingColorData->GetBufferSize(),UINT(texture->BufferLen())));
}

HRESULT FaceTracking::init(HANDLE mutex)
{
	this->mutex = mutex;
	HRESULT hr;
	FT_CAMERA_CONFIG colorConfig = {640, 480, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS};
	FT_CAMERA_CONFIG depthConfig = {320, 240, NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS};
	faceTracker = FTCreateFaceTracker(NULL);
	DWORD width = 0, height = 0;

	//VideoConfig(&colorConfig);
	//DepthVideoConfig(&depthConfig);	

	//initializes the face tracker.
	hr = faceTracker->Initialize(&colorConfig, &depthConfig, NULL, NULL);
	if (FAILED(hr))
	{

	}

	// create instance of the face tracker.
	faceTracker = FTCreateFaceTracker();
	if(!faceTracker)
	{
		// add error for face tracker.
	}

	// Initalize the face tracker.
	hr = faceTracker->Initialize(&colorConfig, &depthConfig, NULL, NULL);
	if( FAILED(hr))
	{
		OutputDebugString(L"Test, niggah.");
		// error for initializing of faceTracker.
	}

	hr = faceTracker->CreateFTResult(&faceTrackingResult);
	if( FAILED(hr)) 
	{
		// error for interface.
	}

	faceTrackingColorData = FTCreateImage();
	if(!faceTrackingColorData || FAILED(hr = faceTrackingColorData->Allocate(colorConfig.Height, colorConfig.Width, FTIMAGEFORMAT_UINT8_X8R8G8B8)))
	{
		// return an ERRORWOZOZZZ.
	}
	faceTrackingDepthData = FTCreateImage();
	if(!faceTrackingDepthData || FAILED(hr = faceTrackingDepthData->Allocate(depthConfig.Height, depthConfig.Width, FTIMAGEFORMAT_UINT16_D13P3)))
	{

	}

	ColorBuffer = FTCreateImage();
	if(!ColorBuffer || FAILED(hr = ColorBuffer->Allocate(colorConfig.Height, colorConfig.Width, FTIMAGEFORMAT_UINT8_X8R8G8B8)))
	{
		// return an ERRORWOZOZZZ.
	}

	//NuiImageResolutionToSize( NUI_IMAGE_RESOLUTION_640x480, width, height);

	DepthBuffer = FTCreateImage();
	if(!DepthBuffer || FAILED(hr = DepthBuffer->Allocate(depthConfig.Height, depthConfig.Width, FTIMAGEFORMAT_UINT16_D13P3)))
	{
		// return an error
	}

	//Direct2D
	ensureDirect2DResources();

	lastTrackingSuccess = false;
	return 0;
}

void FaceTracking::startThread(){
	thread = CreateThread(0,NULL,faceTrackingThread, this, 0,&threadId);
}

/* 
HRESULT FaceTracking::VideoConfig(FT_CAMERA_CONFIG* config)
{
	if(!config)
	{
		return E_POINTER;
	}

	UINT width = ColorBuffer ? ColorBuffer->GetWidth() : 0;
	UINT height = ColorBuffer ? ColorBuffer->GetHeight() : 0;
	FLOAT focus = 0.f;

	if(width == 640 && height == 480)
	{
		focus = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	}
	else if( width == 1280 && height == 960)
	{
		focus = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS* 2.f;
	}

	if ( focus == 0.f)
	{
		return E_UNEXPECTED;
	}

	config->FocalLength = focus;
	config->Height = height;
	config->Width = width;
	return(S_OK);
}

HRESULT FaceTracking::DepthVideoConfig(FT_CAMERA_CONFIG* dConfig)
{
	if(!dConfig)
	{
		return E_POINTER;
	}

	UINT width = DepthBuffer ? DepthBuffer->GetWidth() : 0;
	UINT height = DepthBuffer ? DepthBuffer->GetHeight() : 0;
	FLOAT focus = 0.f;

	if(width == 320 && height == 240)
	{
		focus = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
	}
	else if(width == 640 && height == 480)
	{
		focus = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
	}

	if(focus == 0.f)
	{
		return E_UNEXPECTED;
	}

	dConfig->FocalLength = focus;
	dConfig->Width = width;
	dConfig->Height = height;

	return S_OK;
} */ // Commented config out.

void FaceTracking::faceTrackProcessing()
{
	int i = 0;
	ensureDirect2DResources();
	HRESULT hrFT = E_FAIL;
	HRESULT hrCopy = E_FAIL;
	DWORD result = WaitForSingleObject(mutex,INFINITE);
	//create local copies of the objects to prevent a lock that takes extremely long
	if (result == WAIT_OBJECT_0){
		__try {
			hrCopy = faceTrackingColorData->CopyTo(ColorBuffer, NULL, 0, 0);
			if (SUCCEEDED(hrCopy) && DepthBuffer)
			{
				hrCopy = faceTrackingDepthData->CopyTo(DepthBuffer, NULL, 0, 0);
				if (FAILED(hrCopy))
				{
					//OutputDebugString(L"DepthData copy error");
				}
			}

			intD2DcolorData->CopyFromBitmap(NULL,d2DcolorData,NULL);
		}
		__finally {
			ReleaseMutex(mutex);
		}
		//start with the face tracking
		if (SUCCEEDED(hrCopy)){ //If this one is true, both the DepthBuffer and the ColorBuffer are filled whithout errors
			POINT ptt = {0,0};
			FT_SENSOR_DATA sensorData(ColorBuffer,DepthBuffer,1.0f,&ptt);
		}
		renderTarget->BeginDraw();
		renderTarget->DrawBitmap(intD2DcolorData);
		renderTarget->EndDraw();

	}	
}
DWORD WINAPI FaceTracking::faceTrackingThread(PVOID lpParam){
	FaceTracking * faceTracking = static_cast<FaceTracking*> (lpParam);
	if (faceTracking){
		return faceTracking->faceTrackingThread();
	}
	return 1;
}

DWORD WINAPI FaceTracking::faceTrackingThread()
{
	//thread loop
	while(applicationRunning)
	{
		faceTrackProcessing();
		Sleep(16);
	}

	return 0;
}
//Should delete a pointer and set it to NULL
void FaceTracking::Release()
{
	faceTrackingResult = NULL;
	nuiPresent = NULL;
	ColorBuffer = NULL;
	DepthBuffer = NULL;
}

//------Direct2D

const int sourceWidth=640;
const int sourceHeight = 480;

HRESULT FaceTracking::ensureDirect2DResources(){
	HRESULT hr = S_OK;

	if( !renderTarget )
	{
		D2D1_SIZE_U size = D2D1::SizeU( sourceWidth, sourceHeight);

		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
		hr = d2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(hWnd, size),
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
			&d2DcolorData
			);
		hr = renderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&intD2DcolorData
			);

		if ( FAILED(hr) )
		{
			SafeRelease( renderTarget );
			return hr;
		}


	}
	return hr;
}