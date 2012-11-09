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

	// create instance of the face tracker.
	faceTracker = FTCreateFaceTracker();
	if(!faceTracker)
	{
		// add error for face tracker.
	}

	// Video camera config for face tracking.
	FT_CAMERA_CONFIG videoCameraConfig = {640, 480, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS};

	// Depth camera config for face tracking.
	FT_CAMERA_CONFIG depthCameraConfig = {320, 240, NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS};

	// Initalize the face tracker.
	hr = faceTracker->Initialize(&videoCameraConfig, &depthCameraConfig, NULL, NULL);
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
	if(!faceTrackingColorData || FAILED(hr = faceTrackingColorData->Allocate(videoCameraConfig.Height, videoCameraConfig.Width, FTIMAGEFORMAT_UINT8_X8R8G8B8)))
	{
		// return an ERRORWOZOZZZ.
	}
	faceTrackingDepthData = FTCreateImage();
	if(!faceTrackingDepthData || FAILED(hr = faceTrackingDepthData->Allocate(depthCameraConfig.Width, depthCameraConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
	{
		// return an error
	}

	lastTrackingSuccess = false;
	return 0;
}

void FaceTracking::faceTrackProcessing()
{
	HRESULT hrFT = E_FAIL;


}

DWORD WINAPI FaceTracking::faceTrackingThread()
{
	FT_CAMERA_CONFIG videoConfig;
	FT_CAMERA_CONFIG depthConfig;
	HRESULT hr;

	// try to start the face tracker.
	hr = init();
	if (SUCCEEDED(hr))
	{
		nuiPresent = true;
	}
	else
	{
		nuiPresent = false;
	}

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
}