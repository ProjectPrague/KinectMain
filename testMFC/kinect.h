//header file for the kinect.cpp file.
#pragma once 

#include "stdafx.h"
#include <afxwin.h>			//MFC core and standard components
#include "NuiApi.h"
#include <list>
#include "ImageDraw.h"
#include "resource.h"
#include "faceTracking.h"



class Kinect
{

public:
	int getKinectAngle();
	void setKinectAngle(int angle);
	HRESULT initialize();
	Kinect(INuiSensor * globalNui, HWND hwnd);
	~Kinect();

	HWND hWnd;

private:

	HRESULT EnsureDirect2DResources();
	void discardDirect2DResources();
	void unInit();

	// Global NuiSensor.
	INuiSensor * globalNui;
	
	//Handles for the specific data.
	bool gotColorAlert();
	bool gotDepthAlert(); 
	bool gotSkeletonAlert();

	// Blanks the skeletonscreen when there is no skeleton found.
	void blankSkeletonScreen( );

	//Draws the skeleton.. (Dôh.)
	void getClosestHint();
	void DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight);
	
	HRESULT D2DResources();

	void UpdateDepthFlag( DWORD flag, bool value);

	D2D1_POINT_2F Kinect::SkeletonScreen( Vector4 skeletonPoint, int width, int height );
	void UpdateSkelly( const NUI_SKELETON_FRAME &skelly );

	void DrawBone( const NUI_SKELETON_DATA & skelly, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);

	// Thread to handle Kinect processing, calls class instance thread processor.
	static DWORD WINAPI ProcessThread( LPVOID pParam );

	// Thread to handle the kinect processing.
	DWORD WINAPI ProcessThread( );

	// Skeletal drawing
	ID2D1HwndRenderTarget *   renderTarget;
	ID2D1SolidColorBrush  *   brushJointTracked;
	ID2D1SolidColorBrush  *   brushJointInferred;
	ID2D1SolidColorBrush  *   brushBoneTracked;
	ID2D1SolidColorBrush  *   brushBoneInferred;
	D2D1_POINT_2F             points[NUI_SKELETON_POSITION_COUNT];

	// Draw devices
	ImageDraw *				drawDepth;
	//ImageDraw *				drawColor;
	ID2D1Factory *			d2DFactory;

	//Facetracker
	FaceTracking*			faceTracker;
	//buffers used by the facetreacker
	IFTImage*				videoBuffer;

	// VGA bitmap
	ID2D1Bitmap *            bitmap;

	// Tread Handles
	HANDLE        treadNuiProcess;
	HANDLE        treadNuiProcessStop;

	HANDLE        nextDepthFrameEvent;
	HANDLE        nextColorFrameEvent;
	HANDLE        nextSkeletonEvent;
	HANDLE        depthStreamHandle;
	HANDLE        videoStreamHandle;

	FT_VECTOR3D m_NeckPoint[NUI_SKELETON_COUNT];
    FT_VECTOR3D m_HeadPoint[NUI_SKELETON_COUNT];
    bool        m_SkeletonTracked[NUI_SKELETON_COUNT];

	//the mutex
	HANDLE			mutex;

	BYTE		depthRGBX[640*480*4];
	DWORD       lastSkeletonFoundTime;
	bool        screenBlanked;
	//  int         depthFramesTotal;
	//  int			lastDepthFramesTotal;
	//  int			trackedSkeletons;
	DWORD		skeletonTrackingFlags;
	DWORD		depthStreamFlags;

	 DWORD		stickySkeletonId[NUI_SKELETON_MAX_TRACKED_COUNT];
};


// In control of / takes care of.
class KinectManager
{
public:
	KinectManager();
	~KinectManager();
	std::list<INuiSensor*> DiscoverList();
	std::list<INuiSensor*> getGlobalNuiList();
	HRESULT initialize(HWND hWnd);
	Kinect * selectKinect(CString selected);

private:
	static void CALLBACK OnSensorStatusChanged( HRESULT hr, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* userData);
	HWND hwnd;
	std::list<INuiSensor*> nuiList;
};