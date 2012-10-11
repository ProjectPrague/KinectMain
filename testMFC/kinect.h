//header file for the kinect.cpp file.

#include "windows.h"
#include "NuiApi.h"
#include <list>
class KinectManager{
public:
	KinectManager();
	~KinectManager();
	
	HRESULT initialize();

private:
	std::list<INuiSensor*> nuiList;
};

class Kinect{
public:
	int getKinectAngle();
	void setKinectAngle(int angle);
	HRESULT initialize();
	Kinect(INuiSensor * globalNui);
	~Kinect();

private:

	INuiSensor* globalNui;

};