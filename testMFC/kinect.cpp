//file for kinect managing.

#include <iostream>
#include "kinect.h"
#include <mmsystem.h>
#include <sstream>		//Needed for the conversion from int to String
#include <assert.h>
#include <comdef.h>
#include <stdio.h>
//lookups for color tinting based on player index.
static const int intensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int intensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int intensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

static const float jointThickness = 3.0f;
static const float trackedBoneThickness = 6.0f;
static const float inferredBoneThickness = 1.0f;

const int bytesPerPixel = 4;
const int screenWidth = 320;
const int screenHeight = 240;

KinectManager::KinectManager()
{

}

KinectManager::~KinectManager(){
}

std::list<INuiSensor*> KinectManager::getGlobalNuiList()
{
	return nuiList;
}

HRESULT KinectManager::initialize(HWND hWnd)
{
	INuiSensor * nui;
	int nuiCount = 0;
	HRESULT hr;
	this->hwnd = hWnd;


	NuiSetDeviceStatusCallback(OnSensorStatusChanged, NULL);

	hr = NuiGetSensorCount(&nuiCount);
	if ( FAILED(hr))
	{
		return hr;
	}

	// Look at each kinect sensor
	for (int i = 0; i < nuiCount; i++)
	{
		// Create the sensor so we can check status, if we can't create it, move on.
		hr = NuiCreateSensorByIndex(i, &nui);
		if (FAILED(hr))
		{
			continue;
		}

		// Get the status of the sensor, and if connected, then we can initialize it.
		hr = nui->NuiStatus();

		if (S_OK == hr)
		{
			nuiList.push_front(nui);
		}

		// This sensor was not okay, so we release it (into the wild!) since we're not using it.
		nui->Release();
	}

	return hr;


}

Kinect * KinectManager::selectKinect(CString selected)
{
	int i = 0;

	// create a list of available Nui objects.
	for (std::list<INuiSensor*>::const_iterator it = nuiList.begin();it != nuiList.end();++it)
	{
		// converts the NuiUnique Id.
		CString convert = (LPCTSTR) (*it)->NuiUniqueId();
		//if the unique ID is the same as the selected kinect, initialize and return it.
		if(convert.Compare(selected) != 0)
		{
			Kinect * kinect = new Kinect((*it),hwnd);
			kinect->initialize();
			return kinect;
		}
	}
	i++;
	return NULL;
}

void CALLBACK KinectManager::OnSensorStatusChanged( HRESULT hr, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* userData)
{
	OutputDebugString( L"KEIN KINECT.\r\n" );
}

//---------------------------END OF KINECTMANAGER, START OF KINECT ----------
Kinect::Kinect(INuiSensor * globalNui, HWND hwnd)
{
	unInit();
	videoBuffer = NULL;
	faceTracker = NULL;
	this->hWnd = hwnd;
	this->globalNui = globalNui;

}

Kinect::~Kinect()
{
	unInit();

	delete faceTracker;
	faceTracker = NULL;

	globalNui->NuiShutdown();

	globalNui->Release();
	globalNui = NULL;

	//Cleaning up pointers, to prevent memory leaking
	delete drawDepth;
	drawDepth = NULL;

}

HRESULT Kinect::initialize()
{
	HRESULT hr;
	bool result;

	//init Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2DFactory);

	//init faceTracker
	faceTracker = new FaceTracking(GetDlgItem(hWnd, 1010), d2DFactory);

	//the three events that the kinect will throw
	nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	nextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	videoBuffer = FTCreateImage();
	// depthBuffer = FTCreateImage();                                             <------------------------------ ?

	if (!videoBuffer)
	{
		return E_OUTOFMEMORY;
	}
	drawDepth = new ImageDraw();
	result = drawDepth->Initialize( GetDlgItem( hWnd, 1011), d2DFactory, 320, 240, 320 * 4);
	if (!result )
	{
		// Display error regarding the depth.
	}

	//drawColor = new ImageDraw();
	//result = drawColor->Initialize( GetDlgItem( hWnd, 1010 ), d2DFactory, 640, 480, 640 * 4);
	if ( !result )
	{
		// Display Error regarding the color.
	}

	// recource ensurement
	EnsureDirect2DResources();

	//Flags for the kinect, usage is one line under the code.
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;

	hr = globalNui->NuiInitialize(nuiFlags);
	//If the skeletal part cannot be initialized, just initialize everything else.
	if ( E_NUI_SKELETAL_ENGINE_BUSY == hr)
	{
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR;
		hr = globalNui->NuiInitialize(nuiFlags);
	}

	if ( FAILED(hr))
	{
		// Some error which result in the software being unable to properly initialize the kinect.
		// of nui init error.
	}

	if ( HasSkeletalEngine( globalNui ) )
	{
		hr = globalNui->NuiSkeletonTrackingEnable( nextSkeletonEvent, skeletonTrackingFlags);
		if( FAILED(hr))
		{
			// Error about skeleton tracking.
		}
	}

	// skeletal viewer error

	//Initialize the color stream
	hr = globalNui->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		nextColorFrameEvent,
		&videoStreamHandle);

	// error toevoegen voor de image stream
	if ( FAILED(hr) )
	{

	}

	//Initialize the depth stream.
	hr = globalNui->NuiImageStreamOpen( 
		HasSkeletalEngine(globalNui) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_320x240,
		depthStreamFlags,
		2,
		nextDepthFrameEvent,
		&depthStreamHandle );

	// error toevoegen voor de depth stream 
	if ( FAILED(hr) )
	{

	}
	//Init the Mutex system ( to prevent read and write on the same object simultaniously )
	mutex = CreateMutex(NULL, FALSE,L"D2DBitMapProtector");

	//FaceTracker Init
	faceTracker->init(mutex);
	// Start the processing thread
	treadNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	treadNuiProcess = CreateThread ( NULL, 0, ProcessThread, this, 0, NULL);
	//start the facetracker thread
	faceTracker->startThread();
	return hr;
}

void Kinect::unInit()
{
	//SafeRelease( globalNui );

	renderTarget = NULL;
	brushJointTracked = NULL;
	brushJointInferred = NULL;
	brushBoneTracked = NULL;
	brushBoneInferred = NULL;
	ZeroMemory(points,sizeof(points));

	nextDepthFrameEvent = NULL;
	nextColorFrameEvent = NULL;
	nextSkeletonEvent = NULL;
	depthStreamHandle = NULL;
	videoStreamHandle = NULL;
	//treadNuiProcess = NULL;
	//treadNuiProcessStop = NULL;
	lastSkeletonFoundTime = 0;
	screenBlanked = false;
	drawDepth = NULL;
	//drawColor = NULL;
	//trackedSkeletons = 0;
	skeletonTrackingFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE | NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;
	depthStreamFlags = NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE;
	ZeroMemory(stickySkeletonId,sizeof(stickySkeletonId));
}

// Thread to handle Kinect processing, calls class instance thread processor.
DWORD WINAPI Kinect::ProcessThread( LPVOID param )		// LPVOID is a VOID LONG POINTER -> a pointer to an unspecified type and you
{														// cast your own parameter (which should be a pointer, in this case (Kinect *)param)
	Kinect *pThis = (Kinect *)param;
	return pThis->ProcessThread();
}

//This method gets called by its static counterpart, and is the actual thread logic
DWORD WINAPI Kinect::ProcessThread()
{
	//numEvents is the number of events, handleEvents is an Array of all the events being handled.
	const int numEvents = 4;
	HANDLE handleEvents[numEvents] = { treadNuiProcessStop, nextColorFrameEvent, nextDepthFrameEvent, nextSkeletonEvent };
	std::stringstream ss;
	int eventIdx, colorFrameFPS = 0, depthFrameFPS = 0;
	DWORD t, lastColorFPSTime, lastDepthFPSTime;
	CString TextFPS;


	// Initializes the static text fields for FPS text.
	CStatic * MFC_ecFPSCOLOR, * MFC_ecFPSDEPTH;
	CWnd cWnd;
	cWnd.m_hWnd = hWnd;
	//Initialize the Image vieuwer on the GUI. Because this class does not inherit anything relatied to MFC, we need CWnd::GetDlgItem instead of just GetDlgItem.
	// (By the way: because the main is a CWnd and 'GetDlgItem()' means the same thing as 'this->GetDlgItem()', main.cpp actually uses the same method.)
	MFC_ecFPSCOLOR = (CStatic *) cWnd.GetDlgItem(1015);
	MFC_ecFPSDEPTH = (CStatic *) cWnd.GetDlgItem(1016);	

	lastColorFPSTime	= timeGetTime( );
	lastDepthFPSTime	= timeGetTime( );

	//blank the skeleton display when started.
	lastSkeletonFoundTime = 0;

	bool continueProcess = true;
	while ( continueProcess )
	{
		// wait for any of the events
		eventIdx = WaitForMultipleObjects( numEvents, handleEvents, FALSE, 100);

		// timed out, continue
		if ( eventIdx == WAIT_TIMEOUT)
		{
			continue;
		}

		// stop event was signalled
		if (WAIT_OBJECT_0 == eventIdx )
		{
			continueProcess = false;
			break;
		}

		// Wait for each object individually with a 0 timeout to make sure to
		// process all signalled objects if multiple objects were signalled
		// this loop iteration

		// In situations where perfect correspondance between color/depth/skeleton
		// is essential, a priority queue should be used to service the item
		// which has been updated the longest ago Copyright Microsoft.


		if ( WAIT_OBJECT_0 == WaitForSingleObject( nextColorFrameEvent, 0) )
		{
			if( gotColorAlert() )
			{
				++colorFrameFPS; 
			}
		}

		if ( WAIT_OBJECT_0 == WaitForSingleObject( nextDepthFrameEvent, 0) )
		{
			if( gotDepthAlert() )
			{
				++depthFrameFPS;
			}
		}

		if (WAIT_OBJECT_0 == WaitForSingleObject( nextSkeletonEvent, 0))
		{
			if (gotSkeletonAlert() )
			{
			}
		}


		// fps counter for the color stream.
		// compare first frametime with the current time, if more then 1000 passed,
		// one second passed.
		t = timeGetTime();
		if((t - lastColorFPSTime) > 1000)
		{
			ss<<colorFrameFPS;
			TextFPS= ss.str().c_str();
			MFC_ecFPSCOLOR->SetWindowText(TextFPS);
			colorFrameFPS = 0;
			lastColorFPSTime = timeGetTime();

			// Reset both the CString text and the stringstream, so you don't get any crazy value's.
			TextFPS.Empty();
			ss.str("");

			ss<<depthFrameFPS;
			TextFPS = ss.str().c_str();
			MFC_ecFPSDEPTH->SetWindowText(TextFPS);
			depthFrameFPS = 0;

			// Reset both the CString text and the stringstream, so you don't get any crazy value's.
			TextFPS.Empty();
			ss.str("");

			if( (t - lastSkeletonFoundTime) > 300)
			{
				if (!screenBlanked)
				{
					blankSkeletonScreen();
					screenBlanked = true;
				}
			}
		}
	}
	return 0;
}

//Handles color data
bool Kinect::gotColorAlert()
{
	NUI_IMAGE_FRAME frame;
	bool processedFrame = true;
	//get the next frame from the Kinect
	HRESULT hr = globalNui->NuiImageStreamGetNextFrame( videoStreamHandle, 0, &frame);

	if (FAILED( hr ) )
	{
		return false;
	}
	//get the data we need: frame now also contains information about the kinect it came from, etc. We do not need that.
	INuiFrameTexture * texture = frame.pFrameTexture;
	NUI_LOCKED_RECT lockedRect;
	//lock the data we are going to use, so that other threads cant change it while we are using it.
	texture->LockRect( 0, &lockedRect, NULL, 0);
	if( lockedRect.Pitch != 0)
	{
		//draw it to the screen.
		//memcpy(faceTrackingColorData->GetBuffer(), PBYTE(lockedRect.pBits), min(faceTrackingColorData->GetBufferSize, UINT(texture->BufferLen()))); // <----------------------------------------------------------*********
		//drawColor->GDP( static_cast<BYTE *>(lockedRect.pBits), lockedRect.size);
		//mutex waiting, 5 ms timeout
		bitmap->CopyFromMemory(NULL, static_cast<BYTE *>(lockedRect.pBits), 640 * 4);
		memcpy(videoBuffer->GetBuffer(), PBYTE(lockedRect.pBits), min(videoBuffer->GetBufferSize(),UINT(texture->BufferLen())));
		DWORD result = WaitForSingleObject(mutex,5);
		if (result == WAIT_OBJECT_0){
			__try {
				faceTracker->setColorVars(lockedRect, texture);
			}
			__finally {
				ReleaseMutex(mutex);
			}
		}
	}
	else
	{
		OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
		processedFrame = false;
	}
	//unlock the just used data
	texture->UnlockRect(0);
	//tell the kinect to remove the frame from its buffer.
	globalNui->NuiImageStreamReleaseFrame( videoStreamHandle, &frame);

	return processedFrame;
}

// Handles depth data.
bool Kinect::gotDepthAlert()
{
	NUI_IMAGE_FRAME frame;
	bool processedFrame = true;
	//get the next depthFrame from the kinect
	HRESULT hr = globalNui->NuiImageStreamGetNextFrame(
		depthStreamHandle,
		0,
		&frame);

	if ( FAILED(hr) )
	{
		return false;
	}
	//get the data we need: frame now also contains information about the kinect it came from, etc. We do not need that.
	INuiFrameTexture * texture = frame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	//lock the data we are going to use, so that other threads cant change it while we are using it.
	texture->LockRect(0, &LockedRect, NULL, 0);
	//give the data to the face tracker
	DWORD result = WaitForSingleObject(mutex,5);
	if (result == WAIT_OBJECT_0){
		__try {
			faceTracker->setDepthVars(LockedRect, texture);
		}
		__finally {
			ReleaseMutex(mutex);
		}
	}
	if( 0 != LockedRect.Pitch)
	{
		DWORD fWidth, fHeight;

		NuiImageResolutionToSize( frame.eResolution, fWidth, fHeight);

		// draw the bits to the bitmap
		BYTE * rgbrun = depthRGBX;
		const USHORT * bufferRun = (const USHORT *)LockedRect.pBits;

		// The end pixel is start + width*height. (-1 ?)
		const USHORT * bufferEnd = bufferRun + (fWidth * fHeight);

		//If the following statement returns a 0 as result, there will be an assertion error that will terminate the program.
		assert( fWidth * fHeight * bytesPerPixel <= ARRAYSIZE(depthRGBX) );
		//all the checks are now done and everything seems okay. Now we are going to convert distance to color.
		while( bufferRun < bufferEnd)
		{
			USHORT depth = *bufferRun;
			USHORT realdepth = NuiDepthPixelToDepth(depth);
			USHORT player = NuiDepthPixelToPlayerIndex(depth);

			// transform 13-bit depth information into an 8-bit intensity appropriate
			// for display (we disregard information in most significant bit)
			BYTE intensity = static_cast<BYTE>(~(realdepth >> 4));					// inverteren?

			// tint the intensity by dividing per-player values.
			*(rgbrun++) = intensity >> intensityShiftByPlayerB[player];				
			*(rgbrun++) = intensity >> intensityShiftByPlayerG[player];
			*(rgbrun++) = intensity >> intensityShiftByPlayerR[player];

			// No alpha information, skip the last byte.
			++rgbrun;

			++bufferRun;
		}
		//the distance has been recalculated to color and now the ImageDraw Class can make a picture from it.
		drawDepth->GDP( depthRGBX, fWidth * fHeight * bytesPerPixel);
	}
	else
	{
		processedFrame = false;
		OutputDebugString(L"Buffer length of received texture is BOGUS.\r\n Motherfucker");
	}
	//unlock the just used data
	texture->UnlockRect(0);
	//tell the kinect to remove the frame from its buffer.
	globalNui->NuiImageStreamReleaseFrame( depthStreamHandle, &frame);

	return processedFrame;

}

//Handles skeleton data
bool Kinect::gotSkeletonAlert()
{
	NUI_SKELETON_FRAME sFrame = {0};

	bool foundSkeleton = false;

	if ( SUCCEEDED(globalNui->NuiSkeletonGetNextFrame( 0, &sFrame)))
	{
		//find the closest skeleton and save its head and neck coordinates to facetracking
		for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++)
		{
			if( sFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
				NUI_SKELETON_POSITION_TRACKED == sFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] &&
				NUI_SKELETON_POSITION_TRACKED == sFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER])
			{
				foundSkeleton = true;
				m_SkeletonTracked[i] = true;
				m_HeadPoint[i].x = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
				m_HeadPoint[i].y = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
				m_HeadPoint[i].z = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;
				m_NeckPoint[i].x = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
				m_NeckPoint[i].y = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;
				m_NeckPoint[i].z = sFrame.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;
			}
			else
			{
				m_HeadPoint[i] = m_NeckPoint[i] = FT_VECTOR3D(0, 0, 0);
				m_SkeletonTracked[i] = false;
			}
		}
	}

	// no skeletons..
	if (!foundSkeleton)
	{
		return true;
	}
	getClosestHint();
	// smooth out the data (?)
	HRESULT hr = globalNui->NuiTransformSmooth(&sFrame, NULL); // change the parameters?
	if ( FAILED(hr) )
	{
		return false;
	}

	//We found a skeleton, restart the timer.
	screenBlanked = false;
	lastSkeletonFoundTime = timeGetTime( );

	// Ensure Direct2D is ready to go
	hr = EnsureDirect2DResources();

	renderTarget->BeginDraw( );
	renderTarget->DrawBitmap( bitmap );

	RECT rct;
	GetClientRect( GetDlgItem( hWnd, 1012 ), &rct);
	int width = 640; //rct.right;
	int height = 480;// rct.bottom;

	for ( int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		NUI_SKELETON_TRACKING_STATE trackState = sFrame.SkeletonData[i].eTrackingState;

		if ( trackState == NUI_SKELETON_TRACKED )
		{
			// We are tracking the skeleton, we need to draw it.
			DrawSkeleton( sFrame.SkeletonData[i], width, height);
		}
		else if ( trackState == NUI_SKELETON_POSITION_ONLY)
		{
			// We have only recieved the point that is the center of the skeleton
			// draw that point.
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				SkeletonScreen( sFrame.SkeletonData[i].Position, width, height ),
				jointThickness,
				jointThickness
				);
			renderTarget->DrawEllipse(ellipse, brushJointTracked);
		}
	}

	hr = renderTarget->EndDraw( );

	UpdateSkelly( sFrame );
	return false;
}

// -------- GotSkeletonAlert's Helper Classes ---

//Empties the skeleton screen
void Kinect::blankSkeletonScreen( )
{
	renderTarget->BeginDraw( );
	renderTarget->Clear( D2D1::ColorF( 0xFF3FFA, 0.5f ) );
	renderTarget->EndDraw( );
}

void Kinect::getClosestHint(){
	FT_VECTOR3D hint[2];
	int selectedSkeleton = -1;
	float smallestDistance = 0;


	// Get the skeleton closest to the camera
	for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
	{
		if (m_SkeletonTracked[i] && (smallestDistance == 0 || m_HeadPoint[i].z < smallestDistance))
		{
			smallestDistance = m_HeadPoint[i].z;
			selectedSkeleton = i;
		}
	}
	if (selectedSkeleton == -1)
	{
		DWORD result = WaitForSingleObject(mutex,10);
		if (result == WAIT_OBJECT_0){
			__try {
				faceTracker->setTrackBool(false);
			}
			__finally {
				ReleaseMutex(mutex);
			}
		}
	}

	hint[0] = m_NeckPoint[selectedSkeleton];
	hint[1] = m_HeadPoint[selectedSkeleton];
	//mutex lock for writing the data to faceTracking
	DWORD result = WaitForSingleObject(mutex,10);
		if (result == WAIT_OBJECT_0){
			__try {
				faceTracker->setFaceTrackingVars(hint);
			}
			__finally {
				ReleaseMutex(mutex);
			}
		}
}

//Draws a bone from points
void Kinect::DrawBone( const NUI_SKELETON_DATA & skelly, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1)
{
	NUI_SKELETON_POSITION_TRACKING_STATE bone0State = skelly.eSkeletonPositionTrackingState[bone0];
	NUI_SKELETON_POSITION_TRACKING_STATE bone1State = skelly.eSkeletonPositionTrackingState[bone1];

	// If we can't find either joints, exit!
	if (bone0State == NUI_SKELETON_POSITION_NOT_TRACKED || bone1State == NUI_SKELETON_POSITION_NOT_TRACKED )
	{
		return;
	}

	// if both points are inferred, exit.
	if ( bone0State == NUI_SKELETON_POSITION_INFERRED && bone1State == NUI_SKELETON_POSITION_INFERRED )
	{
		return;
	}

	// Assume all drawn bones are inferred unless both joints are tracked.
	if (bone0State == NUI_SKELETON_POSITION_TRACKED && bone1State == NUI_SKELETON_POSITION_TRACKED )
	{
		renderTarget->DrawLine( points[bone0], points[bone1], brushBoneTracked, trackedBoneThickness);
	}
	else
	{
		renderTarget->DrawLine( points[bone0], points[bone1], brushBoneInferred, inferredBoneThickness);
	}
}

//Draws the entire skelton 
void Kinect::DrawSkeleton( const NUI_SKELETON_DATA & skelly, int windowWidth, int windowHeight)
{
	int i;

	for ( i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		points[i] = SkeletonScreen( skelly.SkeletonPositions[i], windowWidth, windowHeight );
	}

	// Rendering part
	// Torso
	DrawBone( skelly, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER );
	DrawBone( skelly, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
	DrawBone( skelly, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE );
	DrawBone( skelly, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER );
	DrawBone( skelly, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT );

	// Left Arm
	DrawBone( skelly, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT );

	// Right Arm
	DrawBone( skelly, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT );
	DrawBone( skelly, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT );
	DrawBone( skelly, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT );

	// Left Leg
	DrawBone( skelly, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT );
	DrawBone( skelly, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT );

	// Right Leg
	DrawBone( skelly, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT );
	DrawBone( skelly, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT );
	DrawBone( skelly, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT );

	// Draw in different colors 
	for( i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse( points[i], jointThickness, jointThickness);

		if( skelly.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED )
		{
			renderTarget->DrawEllipse(ellipse, brushJointInferred);
		}
		else if( skelly.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED )
		{
			renderTarget->DrawEllipse(ellipse, brushBoneTracked);
		}

	}
}

void Kinect::UpdateSkelly( const NUI_SKELETON_FRAME &skelly )
{
	DWORD nearestIDs[2] = { 0, 0};
	USHORT nearestDepth[2] = { NUI_IMAGE_DEPTH_MAXIMUM, NUI_IMAGE_DEPTH_MAXIMUM };

	// Clean old sticky skeleton IDs, if the user has left the frame, etc.
	bool sticky0Found = false;
	bool sticky1Found = false;
	for ( int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		NUI_SKELETON_TRACKING_STATE trackState = skelly.SkeletonData[i].eTrackingState;

		if (trackState == NUI_SKELETON_TRACKED || trackState == NUI_SKELETON_POSITION_ONLY )
		{
			if ( skelly.SkeletonData[i].dwTrackingID == stickySkeletonId[0] )
			{
				sticky0Found = true;
			}
			else if ( skelly.SkeletonData[i].dwTrackingID == stickySkeletonId[1] )
			{
				sticky1Found = true;
			}
		}
	}

	if (!sticky0Found && sticky1Found )
	{
		stickySkeletonId[0] = stickySkeletonId[1];
		stickySkeletonId[1] = 0;
	}
	else if (!sticky0Found )
	{
		stickySkeletonId[0] = 0;
	}
	else if (!sticky1Found )
	{
		stickySkeletonId[1] = 0;
	}

	// calculate the nearest and sticky skeletons.
	for( int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		NUI_SKELETON_TRACKING_STATE trackState = skelly.SkeletonData[i].eTrackingState;

		if( trackState == NUI_SKELETON_TRACKED || trackState == NUI_SKELETON_POSITION_ONLY )
		{
			// Save skeleton ID for sticky mode if there is none currently saved.
			if ( 0 == stickySkeletonId[0] && stickySkeletonId[1] != skelly.SkeletonData[i].dwTrackingID )
			{
				stickySkeletonId[0] = skelly.SkeletonData[i].dwTrackingID;
			}
			else if ( 0 == stickySkeletonId[1] && stickySkeletonId[0] != skelly.SkeletonData[i].dwTrackingID )
			{
				stickySkeletonId[1] = skelly.SkeletonData[i].dwTrackingID;
			}

			LONG x, y;
			USHORT depth;

			NuiTransformSkeletonToDepthImage ( skelly.SkeletonData[i].Position, &x, &y, &depth);

			if (depth < nearestDepth[0] )
			{
				nearestDepth[1] = nearestDepth[0];
				nearestIDs[1] = skelly.SkeletonData[i].dwTrackingID;

				nearestDepth[0] = depth;
				nearestIDs[0] = skelly.SkeletonData[i].dwTrackingID;
			}
			else if ( depth < nearestDepth[1] )
			{
				nearestDepth[1] = depth;
				nearestIDs[1] = skelly.SkeletonData[i].dwTrackingID;
			}
		}
	}

	/*  For sticky skeletons, could be nice later on.

	if ( SV_TRACKED_SKELETONS_NEAREST1 == m_TrackedSkeletons || SV_TRACKED_SKELETONS_NEAREST2 == m_TrackedSkeletons )
	{
	// Only track the closest single skeleton in nearest 1 mode
	if ( SV_TRACKED_SKELETONS_NEAREST1 == m_TrackedSkeletons )
	{
	nearestIDs[1] = 0;
	}
	m_pNuiSensor->NuiSkeletonSetTrackedSkeletons(nearestIDs);
	}

	if ( SV_TRACKED_SKELETONS_STICKY1 == m_TrackedSkeletons || SV_TRACKED_SKELETONS_STICKY2 == m_TrackedSkeletons )
	{
	DWORD stickyIDs[2] = { m_StickySkeletonIds[0], m_StickySkeletonIds[1] };

	// Only track a single skeleton in sticky 1 mode
	if ( SV_TRACKED_SKELETONS_STICKY1 == m_TrackedSkeletons )
	{
	stickyIDs[1] = 0;
	}
	m_pNuiSensor->NuiSkeletonSetTrackedSkeletons(stickyIDs);
	}
	*/

}

//Converts the Depthdata to usable pixel data
D2D1_POINT_2F Kinect::SkeletonScreen( Vector4 skeletonPoint, int width, int height )
{
	LONG x, y;
	USHORT depth;

	// calculate the skeleton's position on the screen
	// NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
	NuiTransformSkeletonToDepthImage( skeletonPoint, &x, &y, &depth);

	float screenPointX = static_cast<float>(x * width) / screenWidth;
	float screenPointY = static_cast<float>(y * height) / screenHeight;

	return D2D1::Point2F(screenPointX, screenPointY);
}

//Checks and builds the Hardware rendering part
HRESULT Kinect::EnsureDirect2DResources()
{
	HRESULT hr = S_OK;


	if (!renderTarget)
	{
		RECT rc;
		GetWindowRect( GetDlgItem( hWnd, 1012 ), &rc);

		int width = 640;// (rc.right - rc.left);
		int height = 480; //(rc.bottom - rc.top);
		D2D1_SIZE_U size = D2D1::SizeU(width, height);
		D2D1_RENDER_TARGET_PROPERTIES rtProp = D2D1::RenderTargetProperties();
		rtProp.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
		rtProp.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		hr = d2DFactory->CreateHwndRenderTarget(
			rtProp,
			D2D1::HwndRenderTargetProperties( GetDlgItem( hWnd, 1012 ), size),
			&renderTarget
			);
		if ( FAILED(hr))
		{
			// error code, yo.
		}

		hr = renderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&bitmap
			);

		//light green
		renderTarget->CreateSolidColorBrush( D2D1::ColorF( 68, 192, 68 ), &brushJointTracked );

		//yellow
		renderTarget->CreateSolidColorBrush( D2D1::ColorF( 255, 255, 0 ), &brushJointInferred );

		//green
		renderTarget->CreateSolidColorBrush( D2D1::ColorF( 0, 128, 0 ), &brushBoneTracked );

		//gray
		renderTarget->CreateSolidColorBrush( D2D1::ColorF( 128, 128, 128 ), &brushBoneInferred );
	}

	return hr;
}

//Get the angle of the current chosen Kinect.
int Kinect::getKinectAngle()
{
	LONG longAngle = -28;
	HRESULT hr;

	hr = globalNui->NuiCameraElevationGetAngle(&longAngle);

	if(FAILED(hr))
	{
		return hr; //this should be fixed, It should not return a errorcode as if it is the angle, It should state clear that there has been an error.
	}

	return (int)longAngle;
}

//Sets or clears the specified depth stream flag.
void Kinect::UpdateDepthFlag( DWORD flag, bool value)
{
	DWORD newFlag = depthStreamFlags;

	if (value)
	{
		newFlag |= flag;
	}
	else
	{
		newFlag &= ~flag;
	}

	if(NULL != globalNui && newFlag != depthStreamFlags)
	{
		depthStreamFlags = newFlag;
		globalNui->NuiImageStreamSetImageFrameFlags( depthStreamHandle, depthStreamFlags);
	}
}

//Sets the angle of the specified kinect. 
void Kinect::setKinectAngle(int angle)
{
	HRESULT hr;

	hr = globalNui->NuiCameraElevationSetAngle(angle);

	if(FAILED(hr))
	{
		//again, probebly should do something to report there has been an error.
	}

}