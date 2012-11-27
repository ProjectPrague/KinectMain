#pragma once 
#include "stdafx.h"
#include "NuiApi.h"
#include <FaceTrackLib.h>
#include <d2d1.h>
#include "resource.h"
#include <math.h>

class FaceTracking
{
public:
	FaceTracking(HWND hwnd, ID2D1Factory * d2DFactory);
	~FaceTracking();

	HRESULT init(HANDLE mutex);
	void setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	void setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	void setFaceTrackingVars(FT_VECTOR3D hint[2]);
	void setTrackBool(bool b);
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

	void blankFT( );

	// for the video processing.
	void FaceTracking::faceTrackProcessing();
	
	//STRUCT edgeHashTable, to save converted 3d points
	struct EdgeHashTable
	{
		UINT32* pEdges;
		UINT edgesAlloc;

		void Insert(int a, int b) 
		{
			UINT32 v = (min(a, b) << 16) | max(a, b);
			UINT32 index = (v + (v << 8)) * 49157, i;
			for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
			{
			}
			pEdges[(index + i) & (edgesAlloc - 1)] = v;
		}
	};
	//For building a collection of coordinates that will represent lines on a face
	HRESULT createFTCCollection(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
		FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, EdgeHashTable *& eht, POINT *& point);


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

	// buffers for the FaceTracking color & depth data
	IFTImage * DepthBuffer;
	IFTImage * ColorBuffer;
	ID2D1Bitmap * intD2DcolorData;
	//Other variables necessary for FaceTracking


	//Variables about the current SkeletonTracked state
	bool						isTracked;
	FT_VECTOR3D                 hint3D[2];
	FT_VECTOR3D					hint3DBuffer[2];
	bool						lastFTSuccess;
	//checks if someone is writing the variables. There may only be one thread reading or writing
	bool locked;

	// Direct2D
	//vars
	ID2D1Factory *           d2DFactory;
	ID2D1HwndRenderTarget *  renderTarget;
	ID2D1SolidColorBrush  *   brushFaceRect;
	ID2D1SolidColorBrush *	brushFaceLines;
	//methods
	HRESULT ensureDirect2DResources();
	void discardDirect2DResources();
};