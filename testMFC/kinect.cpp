//file for kinect managing.

#include <iostream>
#include "kinect.h"


KinectManager::KinectManager()
{

}

std::list<INuiSensor*> KinectManager::getNuiList()
{
	return nuiList;
}

HRESULT KinectManager::initialize()
{
	INuiSensor * nui;
	int nuiCount = 0;
	HRESULT hr;

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
			Kinect * kinect = new Kinect((*it));
			kinect->initialize();
			return kinect;
		}
	}
	i++;
}

Kinect::Kinect(INuiSensor * globalNui)
{
	// da creator.
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

		if (NULL != globalNui)
	{
		hr = globalNui->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
		if(SUCCEEDED(hr))
		{
			
		}
	}
		return hr;
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