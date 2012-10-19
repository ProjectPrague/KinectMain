//file for kinect managing.

#include <iostream>
#include "kinect.h"
#include <mmsystem.h>
#include <sstream>		//Needed for the conversion from int to String
#include <assert.h>
#include <comdef.h>

//lookups for color tinting based on player index. (? Nog onbekend voor mij.)
static const int intensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int intensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int intensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

const int bytesPerPixel = 4;


KinectManager::KinectManager()
{

}

std::list<INuiSensor*> KinectManager::getGlobalNuiList()
{
	return nuiList;
}

HRESULT KinectManager::initialize(HWND hWnd)
{
	INuiSensor * nui;
	int nuiCount = 0;
	HRESULT hr, test;
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
		test = S_OK;
		_com_error error(test);
		OutputDebugString(L"------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		OutputDebugString(error.ErrorMessage());
		OutputDebugString(L"\n");
		OutputDebugString(L"------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
		

		if (S_OK == hr)
		{
			nuiList.push_front(nui);
			break;
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

//std::list<INuiSensor*> KinectManager::DiscoverList()
//{
//	INuiSensor * nui;
//	int nuiCount = 0;
//	HRESULT hr;
//	std::list<INuiSensor *> kinectList;
//
//	hr = NuiGetSensorCount(&nuiCount);
//	if ( FAILED(hr))
//	{
//		return kinectList;
//	}
//
//	// Look at each kinect sensor
//	for (int i = 0; i < nuiCount; i++)
//	{
//		// Create the sensor so we can check status, if we can't create it, move on.
//		hr = NuiCreateSensorByIndex(i, &nui);
//		if (FAILED(hr))
//		{
//			continue;
//		}
//
//		// Get the status of the sensor, and if connected, then we can initialize it.
//		hr = nui->NuiStatus();
//		if (S_OK == hr)
//		{
//			nuiList.push_front(nui);
//			break;
//		}
//
//		// This sensor was not okay, so we release it (into the wild!) since we're not using it.
//		nui->Release();
//	}
//
//	return;
//}

Kinect::Kinect(INuiSensor * globalNui, HWND hwnd)
{
	// da creator.
	this->hWnd = hwnd;
	this->globalNui = globalNui;
}

Kinect::~Kinect()
{
	//da destructor
	globalNui->NuiShutdown();

	globalNui->Release();
	globalNui = NULL;
}

HRESULT Kinect::initialize()
{
	HRESULT hr;
	bool result;

	nextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	nextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	nextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// recource ensurement? D2D resources.

	//init Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2DFactory);

	drawDepth = new ImageDraw();
	result = drawDepth->Initialize( GetDlgItem( hWnd, 1011), d2DFactory, 320, 240, 320 * 4);
	if (!result )
	{
		// Display error regarding the depth.
	}

	drawColor = new ImageDraw();
	result = drawColor->Initialize( GetDlgItem( hWnd, 1010 ), d2DFactory, 640, 480, 640 * 4);
	if ( !result )
	{
		// Display Error regarding the color.
	}

	//Flags for the kinect, usage is one line under the code.
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR;

	hr = globalNui->NuiInitialize(nuiFlags);
	if ( E_NUI_SKELETAL_ENGINE_BUSY == hr)
	{
		nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_COLOR;
		hr = globalNui->NuiInitialize(nuiFlags);
	}

	if ( FAILED(hr))
	{
		// error toevoegen device is in gebruik.
		// of nui init error.
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

	//Initialize the depth stream.
	hr = globalNui->NuiImageStreamOpen( 
		HasSkeletalEngine(globalNui) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_320x240,
		depthStreamFlags,
		2,
		nextDepthFrameEvent,
		&depthStreamHandle );

	// error toevoegen voor de depth stream


	// Start the processing thread
	treadNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	treadNuiProcess = CreateThread ( NULL, 0, ProcessThread, this, 0, NULL);

	return hr;
}

// Thread to handle Kinect processing, calls class instance thread processor.
DWORD WINAPI Kinect::ProcessThread( LPVOID param )		// LPVOID is a VOID LONG POINTER -> a pointer to an unspecified type and you
{														// cast your own parameter (which should be a pointer, in this case (Kinect *)param)
	Kinect *pThis = (Kinect *)param;
	return pThis->ProcessThread();
}



DWORD WINAPI Kinect::ProcessThread()
{
	//numEvents is the number of events, handleEvents is an Array of all the events being handled.
	const int numEvents = 3;
	HANDLE handleEvents[numEvents] = { treadNuiProcessStop, nextColorFrameEvent, nextDepthFrameEvent };
	std::stringstream ss;
	int eventIdx, colorFrameFPS = 0, depthFrameFPS = 0;
	DWORD t, lastColorFPSTime, lastDepthFPSTime;
	CString TextFPS;

	// Initializes the static text fields for FPS text.
	CStatic * MFC_ecFPSCOLOR, * MFC_ecFPSDEPTH;
	CWnd cWnd;
	cWnd.m_hWnd = hWnd;
	MFC_ecFPSCOLOR = (CStatic *) cWnd.GetDlgItem(1015);
	MFC_ecFPSDEPTH = (CStatic *) cWnd.GetDlgItem(1016);

	//Erachter komen en uitleggen waarom bovenstaande wel werkt. Uitleg, Jacko?

	lastColorFPSTime = timeGetTime( );
	lastDepthFPSTime = timeGetTime( );
	// blank the skeleton display on startup

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

		if ( WAIT_OBJECT_0 == WaitForSingleObject( nextDepthFrameEvent, 0) )
		{
			if( gotDepthAlert() )
			{
				++depthFrameFPS;
			}
		}

		if ( WAIT_OBJECT_0 == WaitForSingleObject( nextColorFrameEvent, 0) )
		{
			if( gotColorAlert() )
			{
				++colorFrameFPS; 
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
		}
	}
	return 0;
}

// gotColorAlert() handle new color data

bool Kinect::gotColorAlert()
{
	NUI_IMAGE_FRAME frame;
	bool processedFrame = true;

	HRESULT hr = globalNui->NuiImageStreamGetNextFrame( videoStreamHandle, 0, &frame);

	if (FAILED( hr ) )
	{
		return false;
	}

	INuiFrameTexture * texture = frame.pFrameTexture;
	NUI_LOCKED_RECT lockedRect;

	texture->LockRect( 0, &lockedRect, NULL, 0);
	if( lockedRect.Pitch != 0)
	{
		drawColor->GDP( static_cast<BYTE *>(lockedRect.pBits), lockedRect.size);
	}
	else
	{
		OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
		processedFrame = false;
	}

	texture->UnlockRect(0);

	globalNui->NuiImageStreamReleaseFrame( videoStreamHandle, &frame);

	return processedFrame;
}

// Handle new depth data.
bool Kinect::gotDepthAlert()
{
	NUI_IMAGE_FRAME frame;
	bool processedFrame = true;

	HRESULT hr = globalNui->NuiImageStreamGetNextFrame(
		depthStreamHandle,
		0,
		&frame);

	if ( FAILED(hr) )
	{
		return false;
	}

	INuiFrameTexture * texture = frame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	texture->LockRect(0, &LockedRect, NULL, 0);
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

		while( bufferRun < bufferEnd)
		{
			USHORT depth = *bufferRun;
			USHORT realdepth = NuiDepthPixelToDepth(depth);
			USHORT player = NuiDepthPixelToPlayerIndex(depth);

			// transform 13-bit depth information into an 8-bit intensity appropriate
			// for display (we disregard information in most significant bit)
			BYTE intensity = static_cast<BYTE>(~(realdepth >> 4));					// inverteren?

			// tint the intensity by dividing per-player values.
			*(rgbrun++) = intensity >> intensityShiftByPlayerB[player];				// mee gaan kloten om te begrijpen.
			*(rgbrun++) = intensity >> intensityShiftByPlayerG[player];
			*(rgbrun++) = intensity >> intensityShiftByPlayerR[player];

			// No alpha information, skip the last byte.
			++rgbrun;

			++bufferRun;
		}

		drawDepth->GDP( depthRGBX, fWidth * fHeight * bytesPerPixel);
	}
	else
	{
		processedFrame = false;
		OutputDebugString(L"Buffer length of received texture is BOGUS.\r\n Motherfucker");
	}

	texture->UnlockRect(0);

	globalNui->NuiImageStreamReleaseFrame( depthStreamHandle, &frame);

	return processedFrame;

}

//Get the angle of the current chosen Kinect.
int Kinect::getKinectAngle()
{
	LONG longAngle = -28;
	HRESULT hr;

	hr = globalNui->NuiCameraElevationGetAngle(&longAngle);

	if(FAILED(hr))
	{
		return hr;
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

	}

}