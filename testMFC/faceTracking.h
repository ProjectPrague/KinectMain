#include "kinect.h"
#include <FaceTrackLib.h>

class FaceTracking
{
public:
	FaceTracking();
	~FaceTracking();

	HRESULT init();

	bool lastTrackingSuccess;
	bool applicationRunning;

	DWORD WINAPI faceTrackingThread();

private:
	// Safe release method.
	void Release();

	//video configurator
	HRESULT VideoConfig(FT_CAMERA_CONFIG* config);

	//depth config
	HRESULT DepthVideoConfig(FT_CAMERA_CONFIG* config);

	// Is the kinect there? :D
	BOOL nuiPresent;

	// for the video processing.
	void FaceTracking::faceTrackProcessing();

	// Global instance of the face tracker.
	IFTFaceTracker * faceTracker;

	// Result instance for face tracking.
	IFTResult * faceTrackingResult;

	// Sensor data for Face tracking.
	FT_SENSOR_DATA sensorData;

	// Images interfaces that hold the RGB and depth data for the facetracking.
	IFTImage * faceTrackingDepthData;
	IFTImage * faceTrackingColorData;

	// buffers for the data
	IFTImage * DepthBuffer;
	IFTImage * ColorBuffer;
};