//header file for the kinect.cpp file.
#include <afxwin.h>			//MFC core and standard components
#include "windows.h"
#include "NuiApi.h"
#include <list>

class Kinect{
public:
	int getKinectAngle();
	void setKinectAngle(int angle);
	HRESULT initialize();
	Kinect(INuiSensor * globalNui);
	~Kinect();

private:

	INuiSensor* globalNui;

    HANDLE        nextDepthFrameEvent;
    HANDLE        nextColorFrameEvent;
    HANDLE        nextSkeletonEvent;

};

class KinectManager{
public:
	KinectManager();
	~KinectManager();
	std::list<INuiSensor*> getNuiList();
	HRESULT initialize();
	Kinect * selectKinect(CString selected);

private:
	std::list<INuiSensor*> nuiList;
};