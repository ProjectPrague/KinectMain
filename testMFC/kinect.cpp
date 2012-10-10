//file for kinect managing.

#include <iostream>
#include "kinect.h"
#include <list>
#include "NuiApi.h"

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
			//nuiList->push_front(*nui);
		}

		// This sensor was not okay, so we release it (into the wild!) since we're not using it.
		nui->Release();
	}

	



	return hr;
}

