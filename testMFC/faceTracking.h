#pragma once 
#include "stdafx.h"
#include <afxwin.h>
#include "NuiApi.h"
#include <FaceTrackLib.h>
#include <d2d1.h>
#include "resource.h"
#include <math.h>

/// \class	FaceTracking
///
/// \brief	The FaceTracking class is responsible for everything related to facetracking. It is started by the Kinect object,

class FaceTracking
{
public:

	/// \fn	FaceTracking::FaceTracking(HWND hwnd, ID2D1Factory *& d2DFactory, CWnd & cWnd);
	///
	/// \brief	Constructor.
	///
	/// \param	hwnd			  	Handle of the.
	/// \param [in,out]	d2DFactory	[in,out] If non-null, the 2 d factory.
	/// \param [in,out]	cWnd	  	The window.

	FaceTracking(HWND hwnd, ID2D1Factory *& d2DFactory, CWnd & cWnd);

	/// \fn	FaceTracking::~FaceTracking();
	///
	/// \brief	Destructor.

	~FaceTracking();

	/// \fn	HRESULT FaceTracking::init(HANDLE mutex);
	///
	/// \brief	Initialises this object.
	///
	/// \param	mutex	Handle of the mutex required to lock data to prevent writing while another thread is reading it or vice versa.
	///
	/// \return	.

	HRESULT init(HANDLE mutex);

	/// \fn	void FaceTracking::setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	///
	/// \brief	Sets color variables.
	///
	/// \param	lockedRect	   	The nui image data.
	/// \param [in,out]	texture	If non-null, the texture.

	void setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);

	/// \fn	void FaceTracking::setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);
	///
	/// \brief	Sets depth variables.
	///
	/// \param	lockedRect	   	The nui depth data.
	/// \param [in,out]	texture	If non-null, the texture.

	void setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture);

	/// \fn	void FaceTracking::setFaceTrackingVars(FT_VECTOR3D hint[2]);
	///
	/// \brief	Sets face tracking variables.
	///
	/// \param	hint	A hint from the depthDataTracker where the closest face is (x,y).

	void setFaceTrackingVars(FT_VECTOR3D hint[2]);

	/// \fn	void FaceTracking::setTrackBool(bool b);
	///
	/// \brief	Sets if the skeletal tracker has a skeleton tracked. If not the color is reset.
	///
	/// \param	b	true to b.

	void setTrackBool(bool b);

	/// \fn	void FaceTracking::startThread();
	///
	/// \brief	Starts a thread.

	void startThread();

	/// \fn	HRESULT FaceTracking::setMaskColor(int red, int green, int blue);
	///
	/// \brief	Sets mask color. The color will be the same as the skeleton color shown in the depth video, it is done by giving the color of the skeleton the skeletondataprocessor hints to, to this function.
	///
	/// \param	red  	The red.
	/// \param	green	The green.
	/// \param	blue 	The blue.
	///
	/// \return	.

	HRESULT FaceTracking::setMaskColor(int red, int green, int blue);	
	
	bool lastTrackingSuccess;
	bool applicationRunning;

private:

	/// \fn	static DWORD WINAPI FaceTracking::faceTrackingThread(PVOID lpParam);
	///
	/// \brief	Launches the non-static method of this as a thread.
	///
	/// \param	lpParam	a pointer to this (the facetracker object).
	///
	/// \return	.

	static DWORD WINAPI faceTrackingThread(PVOID lpParam);

	/// \fn	DWORD WINAPI FaceTracking::fceTrackingThread();
	///
	/// \brief	Face tracking thread. Waits 16 milliseconds, then starts faceTrackProcessing, and repeats this until applicationRunning becomes false.
	///
	/// \return	.

	DWORD WINAPI faceTrackingThread();

	/// \fn	void FaceTracking::FTMeasuring();
	///
	/// \brief	This thread is supposed to put some details of the tracked face on the GUI.

	void FaceTracking::FTMeasuring();

	/// \fn	HRESULT FaceTracking::VideoConfig(FT_CAMERA_CONFIG* config);
	///
	/// \brief	Video configuration.
	///
	/// \param [in,out]	config	If non-null, the configuration.
	///
	/// \return	HRESULT indicating the status.

	HRESULT VideoConfig(FT_CAMERA_CONFIG* config);

	/// \fn	HRESULT FaceTracking::DepthVideoConfig(FT_CAMERA_CONFIG* config);
	///
	/// \brief	Creates the video configuration.
	///
	/// \param [in,out]	config	If non-null, the configuration.
	///
	/// \return		HRESULT indicating the status.

	HRESULT DepthVideoConfig(FT_CAMERA_CONFIG* config);

	/// \fn	void FaceTracking::blankFT();
	///
	/// \brief	clears the facetracking screen on destruction.
	
	void blankFT();

	void setFields();

	/// \fn	void FaceTracking::faceTrackProcessing();
	///
	/// \brief	Face track processing. This function calculates all the facetracking parts.

	void FaceTracking::faceTrackProcessing();

	/// \struct	EdgeHashTable
	///
	/// \brief	Edge hash table. This is used to calculate and store the 3d facemask.

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

	/// \fn	HRESULT FaceTracking::createFTCCollection(IFTImage* pColorImg, IFTModel* pModel,
	/// 	FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, FLOAT zoomFactor,
	/// 	POINT viewOffset, IFTResult* pAAMRlt, EdgeHashTable *& eht, POINT *& point);
	///
	/// \brief	Creates the 3d facemask, and stores it in the EdgeHashTable.
	///
	/// \param [in,out]	pColorImg	If non-null, the color image.
	/// \param [in,out]	pModel   	If non-null, the model.
	/// \param	pCameraConfig	 	The camera configuration.
	/// \param	pSUCoef			 	The su coef.
	/// \param	zoomFactor		 	The zoom factor.
	/// \param	viewOffset		 	The view offset.
	/// \param [in,out]	pAAMRlt  	If non-null, a m rlt.
	/// \param [in,out]	eht		 	[in,out] If non-null, the eht.
	/// \param [in,out]	point	 	[in,out] If non-null, the point.
	///
	/// \return	The new ftc collection.

	HRESULT createFTCCollection(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
	FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, EdgeHashTable *& eht, POINT *& point);

	/// \fn	HRESULT FaceTracking::ensureDirect2DResources();
	///
	/// \brief	Ensures that Direct2D resources exist and are properly initialized. If they are not initialized this method will initialize them.
	///
	/// \return	.

	HRESULT ensureDirect2DResources();

	/// \fn	void FaceTracking::discardDirect2DResources();
	///
	/// \brief	Discard Direct2D resources.

	void discardDirect2DResources();

	int redCheck, blueCheck, greenCheck;
	HWND hWnd;
	CWnd * cWnd;
	HANDLE thread;
	DWORD threadId;
	HANDLE mutex;
	FLOAT scale, rotation[3], translation[3];


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
		ID2D1Factory *           d2DFactory;
	ID2D1HwndRenderTarget *  renderTarget;
	ID2D1SolidColorBrush *	brushFaceLines;
	
};