//file for kinect managing.

#include <iostream>
#include "kinect.h"



KinectManager::KinectManager()
{

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
			globalNui = nui;
			break;
		}

		// This sensor was not okay, so we release it (into the wild!) since we're not using it.
		nui->Release();
	}

	if (NULL != globalNui)
	{

		hr = globalNui->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
		if(SUCCEEDED(hr))
		{

		}
	}

	return hr;


}

int KinectManager::getKinectAngle()
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

void KinectManager::setKinectAngle(int angle)
{
	HRESULT hr;

	hr = globalNui->NuiCameraElevationSetAngle(angle);

	if(FAILED(hr))
	{

	}



}