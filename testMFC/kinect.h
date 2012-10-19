//header file for the kinect.cpp file.
#include <afxwin.h>			//MFC core and standard components
#include "windows.h"
#include "NuiApi.h"
#include <list>
#include "ImageDraw.h"
#include "resource.h"


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
	//Handles for the specific data.
	bool gotColorAlert();
	bool gotDepthAlert();

	// Global NuiSensor.
	INuiSensor* globalNui;

	HRESULT D2DResources();

	void UpdateDepthFlag( DWORD flag, bool value);

	// Thread to handle Kinect processing, calls class instance thread processor.
	static DWORD WINAPI ProcessThread( LPVOID pParam );

	// Thread to handle the kinect processing.
	DWORD WINAPI ProcessThread( );

	// Skeletal drawing
	ID2D1HwndRenderTarget *  renderTarget;
	ID2D1SolidColorBrush *   brushJointTracked;
	ID2D1SolidColorBrush *   brushJointInferred;
	ID2D1SolidColorBrush *   brushBoneTracked;
	ID2D1SolidColorBrush *   brushBoneInferred;
	D2D1_POINT_2F            points[NUI_SKELETON_POSITION_COUNT];

	// Draw devices
	ImageDraw *            drawDepth;
	ImageDraw *            drawColor;
	ID2D1Factory *         d2DFactory;

	// Tread Handles
	HANDLE        treadNuiProcess;
	HANDLE        treadNuiProcessStop;

	HANDLE        nextDepthFrameEvent;
	HANDLE        nextColorFrameEvent;
	HANDLE        nextSkeletonEvent;
	HANDLE        depthStreamHandle;
	HANDLE        videoStreamHandle;

	BYTE		depthRGBX[640*480*4];
	//  DWORD       lastSkeletonFoundTime;
	//  bool        screenBlanked;
	//  int         depthFramesTotal;
	//  int			lastDepthFramesTotal;
	//  int			trackedSkeletons;
	//  DWORD		skeletonTrackingFlags;
	DWORD		depthStreamFlags;



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