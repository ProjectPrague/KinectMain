//header file for the kinect.cpp file.

#include "windows.h"
#include "NuiApi.h"
#include <list>
class KinectManager{
public:
	KinectManager();
	HRESULT initialize();
	int getKinectAngle();
	void setKinectAngle(int angle);

private:

	INuiSensor* globalNui;

	std::list<INuiSensor*> nuiList;
};