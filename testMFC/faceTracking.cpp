#include "faceTracking.h"

FaceTracking::FaceTracking()
{
	// All variables must be NULL before the beginning.
	Release();
}

FaceTracking::~FaceTracking()
{
	// destruct all variables to NULL before shutting down.
}

HRESULT FaceTracking::init()
{
	HRESULT hr;
	FT_CAMERA_CONFIG colorConfig;
	FT_CAMERA_CONFIG depthConfig;

	// try to start the face tracker.
	hr = faceTracker->Initialize(&colorConfig, &depthConfig, NULL, NULL);
	if (SUCCEEDED(hr))
	{
		VideoConfig(&colorConfig);
		DepthVideoConfig(&depthConfig);
		nuiPresent = TRUE;
	}
	else
	{
		nuiPresent = false;
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
	if(!faceTrackingDepthData || FAILED(hr = faceTrackingDepthData->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
	{
		// return an error
	}

	lastTrackingSuccess = false;
	faceTrackingThread();
	return 0;
}

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
}

void FaceTracking::faceTrackProcessing()
{
	HRESULT hrFT = E_FAIL;

	if(false)
	{

	}
}

DWORD WINAPI FaceTracking::faceTrackingThread()
{
	//thread loop
	while(applicationRunning)
	{
		// check camera input ?
		faceTrackProcessing();
	}

	return 0;
}

void FaceTracking::Release()
{
	faceTrackingResult = NULL;
	nuiPresent = NULL;
	ColorBuffer = NULL;
}