#pragma once 

#include "NuiApi.h"
#include <FaceTrackLib.h>
#include <d2d1.h>
#include "resource.h"
#include "Collection.h"

class FaceTracking
{
public:
	FaceTracking(HWND hwnd, ID2D1Factory * d2DFactory);
	~FaceTracking();

	HRESULT init(HANDLE mutex);
	void setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	void setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	bool lastTrackingSuccess;
	bool applicationRunning;
	void startThread();
	DWORD WINAPI faceTrackingThread();

private:
	//hwnd
	HWND hWnd;
	//Thread
	HANDLE thread;
	DWORD threadId;
	static DWORD WINAPI faceTrackingThread(PVOID lpParam);
	DWORD WINAPI fceTrackingThread();
	//mutex
	HANDLE mutex;
	// Safe release method.
	void Release();

	//video configurator
	HRESULT VideoConfig(FT_CAMERA_CONFIG* config);

	//depth config
	HRESULT DepthVideoConfig(FT_CAMERA_CONFIG* config);

	// Is the kinect there? :D
	BOOL nuiPresent;

	// for the video processing.
	void FaceTracking::faceTrackProcessing();

	// Global instance of the face tracker.
	IFTFaceTracker * faceTracker;

	// Result instance for face tracking.
	IFTResult * faceTrackingResult;

	// Sensor data for Face tracking.
	FT_SENSOR_DATA sensorData;

	// Image interfaces that hold the RGB and depth data for the facetracking.
	IFTImage * faceTrackingDepthData;
	IFTImage * faceTrackingColorData;
	ID2D1Bitmap * d2DcolorData;
	//internal image interfaces. These are not touched by other threads
	IFTImage * intFaceTrackingDepthData;
	IFTImage * intFaceTrackingColorData;
	ID2D1Bitmap * intD2DcolorData;
	// buffers for the data
	IFTImage * DepthBuffer;
	IFTImage * ColorBuffer;

	//checks if someone is writing the variables. There may only be one reading or writing
	bool locked;

	// Direct2D 
    ID2D1Factory *           d2DFactory;
    ID2D1HwndRenderTarget *  renderTarget;

	HRESULT ensureDirect2DResources();
};