#include "faceTracking.h"

//TODO: make sure the configs are not hardcoded

FaceTracking::FaceTracking(HWND hwnd, ID2D1Factory *d2DFactory)
{
	this -> d2DFactory = d2DFactory;
	this->hWnd = hwnd;
	locked = false;
	// All variables must be NULL before the beginning.
	isTracked = false;
	faceTrackingResult = NULL;
	ColorBuffer = NULL;
	DepthBuffer = NULL;
	renderTarget = NULL;
	lastFTSuccess = false;
	redCheck = 0;
	blueCheck = 0;
	greenCheck = 0;
}

FaceTracking::~FaceTracking()
{
	applicationRunning = false;
	if(thread)
	{
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);
	}
	thread = NULL;
	hWnd = NULL;

	blankFT();

	//saferelease all facetracking pointers
	SafeRelease(faceTracker);
	SafeRelease(faceTrackingResult);
	SafeRelease(faceTrackingColorData);
	SafeRelease(faceTrackingDepthData);
	SafeRelease(ColorBuffer);
	SafeRelease(DepthBuffer);
	ZeroMemory(&hint3D,sizeof(hint3D));
	discardDirect2DResources();
	int i  = 2+2;
}

HRESULT FaceTracking::setMaskColor(int red, int green, int blue)
{
	HRESULT hr = E_FAIL;

	if( redCheck == red && greenCheck == green && blueCheck == blue)
	{
		hr = S_OK;
		return hr;
	}
	else
	{
		SafeRelease(brushFaceLines);
		hr = renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(red, green, blue, 1.0F),
			&brushFaceLines
			);
	}
}

void FaceTracking::setColorVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture){
	d2DcolorData->CopyFromMemory(NULL, static_cast<BYTE *>(lockedRect.pBits), 640 * 4);
	memcpy(faceTrackingColorData->GetBuffer(), PBYTE(lockedRect.pBits), min(faceTrackingColorData->GetBufferSize(),UINT(texture->BufferLen())));
}

void FaceTracking::setDepthVars(NUI_LOCKED_RECT lockedRect, INuiFrameTexture * texture){
	memcpy(faceTrackingDepthData->GetBuffer(), PBYTE(lockedRect.pBits), min(faceTrackingColorData->GetBufferSize(),UINT(texture->BufferLen())));
}
void FaceTracking::setTrackBool(bool b){
	isTracked = b;
}

void FaceTracking::setFaceTrackingVars(FT_VECTOR3D hint[2]){
	hint3D[0] = hint[0];
	hint3D[1] = hint[1];
	isTracked = true;
}

HRESULT FaceTracking::init(HANDLE mutex)
{
	this->mutex = mutex;
	HRESULT hr;
	FT_CAMERA_CONFIG colorConfig = {640, 480, NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS};
	FT_CAMERA_CONFIG depthConfig = {320, 240, NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS};
	faceTracker = FTCreateFaceTracker(NULL);
	DWORD width = 0, height = 0;

	//VideoConfig(&colorConfig);
	//DepthVideoConfig(&depthConfig);	

	//initializes the face tracker.
	hr = faceTracker->Initialize(&colorConfig, &depthConfig, NULL, NULL);
	if (FAILED(hr))
	{

	}

	// create instance of the face tracker.
	faceTracker = FTCreateFaceTracker(NULL);
	if(!faceTracker)
	{
		// add error for face tracker.
	}

	// Initalize the face tracker.
	hr = faceTracker->Initialize(&colorConfig, &depthConfig, NULL, NULL);
	if( FAILED(hr))
	{
		// error for initializing of faceTracker.
	}

	hr = faceTracker->CreateFTResult(&faceTrackingResult);
	if( FAILED(hr) || !faceTrackingResult) 
	{
		// error for interface.
	}

	faceTrackingColorData = FTCreateImage();
	if(!faceTrackingColorData || FAILED(hr = faceTrackingColorData->Allocate(colorConfig.Width, colorConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
	{
		// return an ERRORWOZOZZZ.
	}
	faceTrackingDepthData = FTCreateImage();
	if(!faceTrackingDepthData || FAILED(hr = faceTrackingDepthData->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
	{

	}

	ColorBuffer = FTCreateImage();
	if(!ColorBuffer || FAILED(hr = ColorBuffer->Allocate(colorConfig.Width, colorConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
	{
		// return an ERRORWOZOZZZ.
	}

	//NuiImageResolutionToSize( NUI_IMAGE_RESOLUTION_640x480, width, height);

	DepthBuffer = FTCreateImage();
	if(!DepthBuffer || FAILED(hr = DepthBuffer->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
	{
		// return an error
	}

	//Direct2D
	ensureDirect2DResources();

	lastTrackingSuccess = false;
	return 0;
}

void FaceTracking::startThread(){
	thread = CreateThread(0,NULL,faceTrackingThread, this, 0,&threadId);
}

/* 
HRESULT FaceTracking::VideoConfig(FT_CAMERA_CONFIG* config)
{
if(!config)
{
return E_POINTER;
}

UINT width = ColorBuffer ? ColorBuffer->GetWidth() : 0;
UINT height = ColorBuffer ? ColorBuffer->GetHeight() : 0;
FLOAT focus = 0.f;

if(width == 640 && height == 480)
{
focus = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
}
else if( width == 1280 && height == 960)
{
focus = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS* 2.f;
}

if ( focus == 0.f)
{
return E_UNEXPECTED;
}

config->FocalLength = focus;
config->Height = height;
config->Width = width;
return(S_OK);
}

HRESULT FaceTracking::DepthVideoConfig(FT_CAMERA_CONFIG* dConfig)
{
if(!dConfig)
{
return E_POINTER;
}

UINT width = DepthBuffer ? DepthBuffer->GetWidth() : 0;
UINT height = DepthBuffer ? DepthBuffer->GetHeight() : 0;
FLOAT focus = 0.f;

if(width == 320 && height == 240)
{
focus = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
}
else if(width == 640 && height == 480)
{
focus = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
}

if(focus == 0.f)
{
return E_UNEXPECTED;
}

dConfig->FocalLength = focus;
dConfig->Width = width;
dConfig->Height = height;

return S_OK;
} */ // Commented config out.

void FaceTracking::faceTrackProcessing()
{
	int i = 0;
	HRESULT hrCopy = E_FAIL;
	HRESULT hrFT = E_FAIL;
	HRESULT hrD2D = E_FAIL;
	DWORD result = WaitForSingleObject(mutex,INFINITE);
	bool skeletonTracked = false;
	//create local copies of the objects to prevent a lock that takes extremely long
	if (result == WAIT_OBJECT_0){
		__try {
			hrCopy = faceTrackingColorData->CopyTo(ColorBuffer, NULL, 0, 0);	//copy the colorbuffer
			if (SUCCEEDED(hrCopy) && DepthBuffer)
			{
				hrCopy = faceTrackingDepthData->CopyTo(DepthBuffer, NULL, 0, 0); //copy the depthbuffer
				if (SUCCEEDED(hrCopy))
				{
					if (isTracked){
						skeletonTracked = true;
						hint3DBuffer[0] = hint3D[0];
						hint3DBuffer[1] = hint3D[1];	//copy the head and neck coordinates
					}
				}
			}

			intD2DcolorData->CopyFromBitmap(NULL,d2DcolorData,NULL);
		}
		__finally {
			ReleaseMutex(mutex);
		}
		//start with the face tracking
		if (SUCCEEDED(hrCopy)){ //If this one is true, both the DepthBuffer and the ColorBuffer are filled whithout errors, the tracking boolean is true and the coordinates are copied

			POINT ptt = {0,0};
			FT_SENSOR_DATA sensorData(ColorBuffer,DepthBuffer,1.0f,&ptt);
			FT_VECTOR3D * hint = NULL;
			if (skeletonTracked)hint = hint3DBuffer;
			if (lastFTSuccess)
			{
				hrFT = faceTracker->ContinueTracking(&sensorData,hint,faceTrackingResult);
			}
			else 
			{
				hrFT = faceTracker->StartTracking(&sensorData,NULL, hint, faceTrackingResult);
			}
		}
		lastFTSuccess = SUCCEEDED(hrFT) && SUCCEEDED(faceTrackingResult->GetStatus());
		//make sure D2D is ready to go
		ensureDirect2DResources();

		EdgeHashTable * eht;
		POINT * pFTT;
		hrFT = E_FAIL;
		if (lastFTSuccess){
			//Fill rect with the data for the rect around the face
			RECT faceRect;
			//Create a collection containing all the coordinates for the lines that need to be drawn for the face tracking
			IFTModel * fTModel;
			hrFT = faceTracker->GetFaceModel(&fTModel);
			if (SUCCEEDED(hrFT)){
				FLOAT* pSU = NULL;
				UINT numSU;
				BOOL suConverged;
				hrFT = faceTracker->GetShapeUnits(NULL,&pSU,&numSU,&suConverged);
				//Should not be hardcoded
				FT_CAMERA_CONFIG camCon;
				camCon.Width = 640;
				camCon.Height = 480;
				camCon.FocalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
				POINT vieuwOffset = {0, 0};					
				eht = new EdgeHashTable();
				pFTT = new POINT();
				hrFT = createFTCCollection(ColorBuffer,fTModel,&camCon,pSU,1.0,vieuwOffset,faceTrackingResult, eht, pFTT);

				//The next part is to get direct 2d coordinates from the facetracker. Needs some experimenting. See: http://msdn.microsoft.com/en-us/library/jj130970.aspx
				/*FT_VECTOR2D * DPoints;
				UINT points;
				faceTrackingResult->Get2DShapePoints(&DPoints,&points);*/
			}
		}
		renderTarget->BeginDraw();
		renderTarget->DrawBitmap(intD2DcolorData);
		if (SUCCEEDED(hrFT)){
			if (eht->pEdges){
				for(UINT i = 0; i < eht->edgesAlloc;++i){
					if (eht->pEdges[i] != 0){
						D2D_POINT_2F d2DPointA, d2DPointB;
						d2DPointA.x = pFTT[eht->pEdges[i] >> 16].x;
						d2DPointA.y = pFTT[eht->pEdges[i] >> 16].y;
						d2DPointB.x = pFTT[eht->pEdges[i] & 0xFFFF].x;
						d2DPointB.y = pFTT[eht->pEdges[i] & 0xFFFF].y;

						renderTarget->DrawLine(d2DPointA,d2DPointB,brushFaceLines);
					}
				}
			}
			ZeroMemory(eht->pEdges, sizeof(UINT32) * eht->edgesAlloc);
			_freea(eht->pEdges);
			_freea(pFTT);			
			delete eht;
			//delete pFTT;
		}
		hrD2D = renderTarget->EndDraw();
		if (hrD2D == D2DERR_RECREATE_TARGET)
		{
			discardDirect2DResources();
		}

	}	
}
DWORD WINAPI FaceTracking::faceTrackingThread(PVOID lpParam){
	FaceTracking * faceTracking = static_cast<FaceTracking*> (lpParam);
	if (faceTracking){
		return faceTracking->faceTrackingThread();
	}
	return 1;
}

DWORD WINAPI FaceTracking::faceTrackingThread()
{
	//thread loop
	while(applicationRunning)
	{
		faceTrackProcessing();
		Sleep(16);
	}

	return 0;
}
//Should delete a pointer and set it to NULL

HRESULT FaceTracking::createFTCCollection(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, FLOAT zoomFactor, POINT viewOffset, IFTResult* pAAMRlt, EdgeHashTable *& eht, POINT *& point)
{
	HRESULT hr = S_OK;
	UINT vertexCount = pModel->GetVertexCount();
	FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));
	if (pPts2D)
	{
		FLOAT *pAUs;
		UINT auCount;
		hr = pAAMRlt->GetAUCoefficients(&pAUs, &auCount);
		if (SUCCEEDED(hr))
		{
			FLOAT scale, rotationXYZ[3], translationXYZ[3];
			hr = pAAMRlt->Get3DPose(&scale, rotationXYZ, translationXYZ);
			if (SUCCEEDED(hr))
			{
				hr = pModel->GetProjectedShape(pCameraConfig, zoomFactor, viewOffset, pSUCoef, pModel->GetSUCount(), pAUs, auCount, 
					scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);
				if (SUCCEEDED(hr))
				{
					point   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * vertexCount));
					if (point)
					{
						for (UINT i = 0; i < vertexCount; ++i)
						{
							point[i].x = LONG(pPts2D[i].x + 0.5f);
							point[i].y = LONG(pPts2D[i].y + 0.5f);
						}

						FT_TRIANGLE* pTriangles;
						UINT triangleCount;
						hr = pModel->GetTriangles(&pTriangles, &triangleCount);

						if (SUCCEEDED(hr))
						{
							eht->edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
							eht->pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht->edgesAlloc));
							if (eht->pEdges)
							{
								ZeroMemory(eht->pEdges, sizeof(UINT32) * eht->edgesAlloc);
								for (UINT i = 0; i < triangleCount; ++i)
								{ 
									eht->Insert(pTriangles[i].i, pTriangles[i].j);
									eht->Insert(pTriangles[i].j, pTriangles[i].k);
									eht->Insert(pTriangles[i].k, pTriangles[i].i);
								}
								//for (UINT i = 0; i < eht->edgesAlloc; ++i)
								//{
								//	if(eht->pEdges[i] != 0)
								//	{
								//		//pColorImg->DrawLine(point[eht->pEdges[i] >> 16], point[eht->pEdges[i] & 0xFFFF], color, 1);
								//	}
								//}
								//_freea(eht->pEdges);
							}

							// Render the face rect in magenta
							/*RECT rectFace;
							hr = pAAMRlt->GetFaceRect(&rectFace);
							if (SUCCEEDED(hr))
							{
							POINT leftTop = {rectFace.left, rectFace.top};
							POINT rightTop = {rectFace.right - 1, rectFace.top};
							POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
							POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
							UINT32 nColor = 0xff00ff;
							SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
							SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
							SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
							SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
							}*/
						}

						//_freea(point); 
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
				}
			}
		}
		_freea(pPts2D);
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}
	return hr;
}

//------Direct2D

const int sourceWidth = 640;
const int sourceHeight = 480;

HRESULT FaceTracking::ensureDirect2DResources(){
	HRESULT hr = S_OK;

	if( !renderTarget )
	{
		D2D1_SIZE_U size = D2D1::SizeU( sourceWidth, sourceHeight);

		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;
		hr = d2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&renderTarget
			);

		if ( FAILED(hr) )
		{
			return hr;
		}

		// Create a bitmap that we can copy image data into and then render to the target.
		hr = renderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&d2DcolorData
			);
		hr = renderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&intD2DcolorData
			);

		//brushes for drawing (:O)
		hr = renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::YellowGreen),
			&brushFaceLines
			);

		if ( FAILED(hr) )
		{
			discardDirect2DResources();
			return hr;
		}


	}
	return hr;
}

void FaceTracking::blankFT( )
{
	renderTarget->BeginDraw( );
	renderTarget->Clear( D2D1::ColorF( 0xFFFFFF, 0.0f ) );
	renderTarget->EndDraw( );
}

void FaceTracking::discardDirect2DResources(){
	SafeRelease(d2DcolorData);
	SafeRelease(intD2DcolorData);
	SafeRelease(brushFaceLines);
	SafeRelease(renderTarget);
}