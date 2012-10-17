//file for kinect managing.

#include <iostream>
#include "kinect.h"
#include <mmsystem.h>
#include <sstream>		//Needed for the conversion from int to String

KinectManager::KinectManager()
{

}

std::list<INuiSensor*> KinectManager::getNuiList()
{
	return nuiList;
}

HRESULT KinectManager::initialize(HWND hWnd)
{
	INuiSensor * nui;
	int nuiCount = 0;
	HRESULT hr;
	this->hwnd = hWnd;

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
	for (std::list<INuiSensor*>::const_iterator it = nuiList.begin();it != nuiList.end();++it)
	{
		CString convert = (LPCTSTR) (*it)->NuiUniqueId();

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

Kinect::Kinect(INuiSensor * globalNui, HWND hwnd)
{
	// da creator.
	this->hWnd = hwnd;
	this->globalNui = globalNui;
}

Kinect::~Kinect()
{
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

    // reset the tracked skeletons, range, and tracking mode
    // SendDlgItemMessage(m_hWnd, IDC_TRACKEDSKELETONS, CB_SETCURSEL, 0, 0);
    // SendDlgItemMessage(m_hWnd, IDC_TRACKINGMODE, CB_SETCURSEL, 0, 0);
    // SendDlgItemMessage(m_hWnd, IDC_RANGE, CB_SETCURSEL, 0, 0);

	// recource ensurement? D2D resources.

	//init Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2DFactory);

	drawColor = new ImageDraw();
	result = drawColor->Initialize( GetDlgItem( hWnd, 1010 ), d2DFactory, 640, 480, 640 * 4);
	if ( !result )
	{
		// geef een error message!
	}

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

	hr = globalNui->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_COLOR,
		NUI_IMAGE_RESOLUTION_640x480,
		0,
		2,
		nextColorFrameEvent,
		&videoStreamHandle);

	// error toevoegen voor de image stream

	// steam voor depth toevoegen en de goede errors.

	// start the processing thread
	treadNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	treadNuiProcess = CreateThread ( NULL, 0, ProcessThread, this, 0, NULL);

	return hr;
}

DWORD WINAPI Kinect::ProcessThread( LPVOID param )
{
	Kinect *pThis = (Kinect *)param;
	return pThis->ProcessThread();
}

DWORD WINAPI Kinect::ProcessThread()
{
	const int numEvents = 2;
	HANDLE handleEvents[numEvents] = { treadNuiProcessStop, nextColorFrameEvent };
	int eventIdx, ColorFrameFPS = 0;
	DWORD t, lastColorFPStime;
	CStatic * MFC_ecFPSCOLOR;
	
	std::stringstream ss;

	lastColorFPStime = timeGetTime( );
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
        // which has been updated the longest ago

		if ( WAIT_OBJECT_0 == WaitForSingleObject( nextColorFrameEvent, 0) )
		{
			if( gotColorAlert() )
			{
				++ColorFrameFPS; 
			}
		}

		t = timeGetTime();
		if((t - lastColorFPStime) > 1000)
	{
		ss<<ColorFrameFPS;
		CString TextFPS= ss.str().c_str();
		MFC_ecFPSCOLOR->SetWindowTextW(L"TEST");
		ColorFrameFPS = 0;
		lastColorFPStime = timeGetTime();
	}
	}
	return 0;
}

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

void Kinect::setKinectAngle(int angle)
{
	HRESULT hr;

	hr = globalNui->NuiCameraElevationSetAngle(angle);

	if(FAILED(hr))
	{

	}

}