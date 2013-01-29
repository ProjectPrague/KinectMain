//header file for the kinect.cpp file.
#pragma once 

#include "stdafx.h"
#include <afxwin.h>			//MFC core and standard components
#include "NuiApi.h"
#include <list>
#include "ImageDraw.h"
#include "resource.h"
#include "faceTracking.h"

/// \class	Kinect
///
/// \brief Kinect is the object build around the kinect. It contains information about what kinect is selected and it processes the data from this kinect

class Kinect
{
public:

	/// \fn	int Kinect::getKinectAngle();
	///
	/// \brief	Gets kinect angle.
	///
	/// \return	The kinect angle.

	int getKinectAngle();

	/// \fn	void Kinect::setKinectAngle(int angle);
	///
	/// \brief	Sets kinect angle.
	///
	/// \param	angle	The angle you wish that the selected kinect takes.

	void setKinectAngle(int angle);

	/// \fn	HRESULT Kinect::initialize();
	///
	/// \brief	Initializes the kinect object.
	///
	/// \return	.

	HRESULT initialize();

	/// \fn	Kinect::Kinect(INuiSensor * globalNui, HWND hwnd);
	///
	/// \brief	Constructor.
	///
	/// \param [in,out]	globalNui	Global nui.
	/// \param	hwnd			 	Handle of the.

	Kinect(INuiSensor * globalNui, HWND hwnd);

	/// \fn	Kinect::~Kinect();
	///
	/// \brief	Destructor for the kinect class.

	~Kinect();
	
	HWND hWnd;

private:

	/// \fn	HRESULT Kinect::EnsureDirect2DResources();
	///
	/// \brief	Ensures that the Direct2D resources exist and if necessary initializes these resources.
	///
	/// \return	Returns HRESULT = S_OK if the operation succeeded otherwise returns error.

	HRESULT EnsureDirect2DResources();

	/// \fn	void Kinect::discardDirect2DResources();
	///
	/// \brief	Discard Direct2D resources.

	void discardDirect2DResources();

	// Global NuiSensor.
	INuiSensor * globalNui;
	
	//Handles for the specific data.
	bool gotColorAlert();
	bool gotDepthAlert();
	bool gotSkeletonAlert();

	/// \fn	void Kinect::blankSkeletonScreen( );
	///
	/// \brief	Paints the skeleton screen white.

	void blankSkeletonScreen( );

	/// \fn	void Kinect::getClosestHint();
	///
	/// \brief	Gets closest hint for a face.

	void getClosestHint();

	/// \fn	void Kinect::DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth,
	/// 	int windowHeight);
	///
	/// \brief	Draws a skeleton on the rendertarget.
	///
	/// \param	skel			The skeleton data.
	/// \param	windowWidth 	Width of the window.
	/// \param	windowHeight	Height of the window.

	void DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight);

	/// \fn	void Kinect::UpdateDepthFlag( DWORD flag, bool value);
	///
	/// \brief	Updates the depth flags for different modes considering the kinect options.
	///
	/// \param	flag 	The flag.
	/// \param	value	if the value is false the flag, parameter, will be removed, if it is true flag will be added.

	void UpdateDepthFlag( DWORD flag, bool value);

	/// \fn	D2D1_POINT_2F Kinect::SkeletonScreen( Vector4 skeletonPoint, int width, int height );
	///
	/// \brief	Calculates the 'point' of the body and translates these into Direct2D Point data that is used furtheron to paint the skeleton on the screen.
	///
	/// \param	skeletonPoint	The skeleton point vector data.
	/// \param	width		 	The width of the imagescreen.
	/// \param	height		 	The height of the image screen.
	///
	/// \return	.

	D2D1_POINT_2F Kinect::SkeletonScreen( Vector4 skeletonPoint, int width, int height );

	/// \fn	void Kinect::DrawBone( const NUI_SKELETON_DATA & skelly,
	/// 	NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);
	///
	/// \brief	Draws the connections between the different joints, creating the bones.
	///
	/// \param	skelly	The skeleton data.
	/// \param	bone0 	The index number of the first joint.
	/// \param	bone1 	The index number of the second joint.

	void DrawBone( const NUI_SKELETON_DATA & skelly, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);

	/// \fn	static DWORD WINAPI Kinect::ProcessThread( LPVOID pParam );
	///
	/// \brief	Thread to handle Kinect processing, calls class instance thread processor.
	///
	/// \param	pParam	Pointer to thiss.
	///
	/// \return	Returns the return of the process thread, indication of the status if the tread has run.

	static DWORD WINAPI ProcessThread( LPVOID pParam );

	/// \fn	DWORD WINAPI Kinect::ProcessThread( );
	///
	/// \brief	Thread to handle the kinect processing.
	///
	/// \return	Returns the status of the tread.

	DWORD WINAPI ProcessThread( );

	//Direct2D
	ID2D1Factory		  *   d2DFactory;
	ID2D1HwndRenderTarget *   renderTarget;
	ID2D1SolidColorBrush  *   brushJointTracked;
	ID2D1SolidColorBrush  *   brushJointInferred;
	ID2D1SolidColorBrush  *   brushBoneTracked;
	ID2D1SolidColorBrush  *   brushBoneInferred;
	ID2D1Bitmap			  *	  bitmap;
	D2D1_POINT_2F             points[NUI_SKELETON_POSITION_COUNT];

	// Used for drawing the depth image
	ImageDraw *				drawDepth;

	//Facetracker
	FaceTracking*			faceTracker;

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
	DWORD		skeletonTrackingFlags;
	DWORD		depthStreamFlags;

	 DWORD		stickySkeletonId[NUI_SKELETON_MAX_TRACKED_COUNT];
	 CWnd cWnd;
};

/// \class	KinectManager
///
/// \brief	KinectManager manages the list of kinects. It contains methods to create this list and can return the list. 

class KinectManager
{
public:

	/// \fn	KinectManager::KinectManager();
	///
	/// \brief	Default constructor.

	KinectManager();

	/// \fn	KinectManager::~KinectManager();
	///
	/// \brief	Destructor.

	~KinectManager();

	/// \fn	std::list<INuiSensor*> KinectManager::DiscoverList();
	///
	/// \brief	Gets the list of kinects that are available.
	///
	/// \return	null if it fails, else a list of kinects is returned.

	std::list<INuiSensor*> DiscoverList();

	/// \fn	std::list<INuiSensor*> KinectManager::getGlobalNuiList();
	///
	/// \brief	Gets the list of kinects.
	///
	/// \return	null if it fails, else the list of kinects is returned.

	std::list<INuiSensor*> getGlobalNuiList();

	/// \fn	HRESULT KinectManager::initialize();
	///
	/// \brief	Initializes this object.
	///
	/// \return	.

	HRESULT initialize();

	/// \fn	HRESULT KinectManager::selectKinect(CString selected, Kinect *& kinect, HWND hwdn);
	///
	/// \brief	Select kinect.
	///
	/// \param	selected	  	The selected kinect from the dropdown menu.
	/// \param [in,out]	kinect	[in,out] If non-null, the kinect object.
	/// \param	hwdn		  	Handle of the hwdn.
	///
	/// \return	HRESULT status, if succesful S_OK.

	HRESULT selectKinect(CString selected, Kinect *& kinect, HWND hwdn);

private:

	/// \fn	static void CALLBACK KinectManager::OnSensorStatusChanged( HRESULT hr,
	/// 	const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* userData);
	///
	/// \brief	Code gets executed when there is a change on the sensors, for example if a new sensor is connected.
	///
	/// \param	hr					The hr.
	/// \param	instanceName		Name of the instance.
	/// \param	uniqueDeviceName	Name of the unique device.
	/// \param [in,out]	userData	If non-null, information describing the user.
	///
	/// \return	.

	static void CALLBACK OnSensorStatusChanged( HRESULT hr, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* userData);
	std::list<INuiSensor*> nuiList;
};