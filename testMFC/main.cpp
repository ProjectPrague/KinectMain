
//-----------------------------------------------------------------------------------------
#include "stdafx.h"
#include "resource.h"    //main symbols
#include <afxcmn.h>		//needed for the slider
#include <sstream>		//Needed for the conversion from int to String
#include "kinect.h"		//for kinect stuff
#include <map>			//used to map kinect ID's to the dropdown ID's.
#include <iostream>		// for debugging purposes
#define new DEBUG_NEW
#ifdef _DEBUG	
CMemoryState oldMemState, newMemState, diffMemState;
#endif

//Global variables
//graphical things
CEdit * MFC_ecCURVAL; //MFC_ prefix for easy recognizing of visual items
CEdit * MFC_ecNEWVAL;
CButton * MFC_bSETVAL;
CSliderCtrl * MFC_scKINECTANGLE;
CStatic * MFC_stCURVAL;
CStatic * MFC_stNEWVAL;
CComboBox * MFC_cbKinectList;
CStatic * MFC_ecFPSCOLOR, * MFC_ecDEPTHCOLOR, * MFC_pcSKELETON;
CMenu TopMenu;

CFont * cf;
CBrush * bClear;

//other things
KinectManager * kinectManager;
Kinect * kinect;
std::list<INuiSensor*> nuiList;
int sliderAngle, kinectAngle, currentSelection;

class MAINFORM: public CDialog
{
private:
	std::map<int, BSTR> kinectMap;

	void stopProgram(){
		//delete the kinect
		delete kinect;
		kinect = NULL;
		//delete the kinectManager
		delete kinectManager;
		kinectManager = NULL;
		delete cf;
		DestroyWindow();
		//this->EndDialog(0);
	}

public:
	MAINFORM(CWnd* pParent = NULL): CDialog(MAINFORM::IDD, pParent)
	{    }
	// Dialog Data, name of dialog form
	enum{IDD = MainWindow};
protected:
	virtual void DoDataExchange(CDataExchange* pDX) { CDialog::DoDataExchange(pDX); }
	//Called right after constructor. Initialize things here.
	virtual BOOL OnInitDialog() 
	{      
#ifdef _DEBUG
		oldMemState.Checkpoint();
#endif
		initializePointers();
		initializeKinect();
		initializeInterface();
		CDialog::OnInitDialog();
		return true; 
	}

	void initializePointers()
	{
		//initialize all MFC pointers: map them to the objects in the resource.
		//Both the numeral ID and the "real" ID may be used here. 1001 refers to EC_input, so GetDlgItem(EC_input) is also possible
		//All the IDs can be found inside the resource.h file.
		MFC_ecCURVAL = (CEdit *) GetDlgItem(1003); 
		MFC_ecNEWVAL = (CEdit *) GetDlgItem(1004);
		MFC_bSETVAL = (CButton *) GetDlgItem(1002);	
		MFC_scKINECTANGLE = (CSliderCtrl * ) GetDlgItem(1005);
		MFC_stCURVAL = (CStatic *) GetDlgItem(1006);
		MFC_stNEWVAL = (CStatic *) GetDlgItem(1007);
		MFC_cbKinectList = (CComboBox *) GetDlgItem(1008);
		MFC_ecFPSCOLOR = (CStatic *) GetDlgItem(1015);
		MFC_ecDEPTHCOLOR = (CStatic *) GetDlgItem(1016);
		MFC_pcSKELETON = (CStatic *) GetDlgItem(1012);
	}

	void initializeInterface()
	{
		//this method is used to put the GUI in its starting position.
		// Stringstream usage to turn the int that is returned by getKinectAngle into a string to use it om SetWindowText.
		std::stringstream ss;
		// integer used for counting in for loop for the kinectlist.
		int i = 0;

		TopMenu.LoadMenu(MFC_TOPMENU);
		SetMenu(&TopMenu);

		//Some pre-kinect-check Interface initialisation to make the GUI look nice, even if there is no Kinect
		MFC_scKINECTANGLE->SetRange(-27, 27, TRUE);
		//You need an LPCTSTR for a SetWindowText. For this kind of string use, just prefix an L. For normal strings: convert to CString: CString s(str.c_str())
		MFC_stCURVAL->SetWindowText(L"Current Kinect Angle");
		MFC_stNEWVAL->SetWindowText(L"New Kinect Angle");

		//The check if there is a Kinect
		if (nuiList.size() > 0){
			for (std::list<INuiSensor*>::const_iterator it = nuiList.begin();it != nuiList.end();++it,i++)
			{
				ss.str(std::string());
				ss.clear();
				ss << i;
				CString text = ss.str().c_str();
				MFC_cbKinectList->AddString(L"Kinect "+text);
				kinectMap[i] = (*it)->NuiUniqueId();
				CString textJeMoeder = (LPCTSTR) (*it)->NuiUniqueId();
				CString textJeVader;
				textJeVader.Format(_T("%d"), i);
				OutputDebugString(textJeMoeder);
				OutputDebugString(L"----");
				OutputDebugString(textJeVader);
				OutputDebugString(L"\n");
			}
			MFC_cbKinectList->SetCurSel(0);
			currentSelection = 0;
		} else {
			//if there is no kinect
			//disable usable GUI elements
			MFC_bSETVAL->EnableWindow(false);
			MFC_scKINECTANGLE->EnableWindow(false);
			MFC_cbKinectList ->EnableWindow(false);
			//Fill the two EditControls with a 0
			MFC_ecCURVAL->SetWindowText(L"0");
			MFC_ecNEWVAL->SetWindowText(L"0");
			return;
		}	

		kinectAngle = kinect->getKinectAngle();
		ss.str(std::string());
		ss.clear();
		ss << kinectAngle;
		CString text = ss.str().c_str();
		//Set the interface as you want it on your first run
		MFC_ecCURVAL->SetWindowText(text);
		MFC_ecNEWVAL->SetWindowText(text);
		MFC_scKINECTANGLE->SetPos(kinectAngle*-1);
		MFC_scKINECTANGLE->SetRange(-27, 27, TRUE);

		cf = new CFont();
		cf->CreatePointFont(300,L"Starcraft");
		MFC_ecFPSCOLOR->SetFont(cf);
		MFC_ecDEPTHCOLOR->SetFont(cf);

	} 
	void initializeKinect()
	{
		//The kinect initializer. Makes sure every thing is ready to be able to work with the kinect.
		kinectManager = new KinectManager;
		kinectManager->initialize();
		nuiList = kinectManager->getGlobalNuiList();

		if (nuiList.size() > 0){
			HRESULT kinectResult;		
			kinectResult = kinectManager->selectKinect((LPCTSTR) nuiList.front()->NuiUniqueId(), kinect, GetSafeHwnd() );
			int i = 1+1;
		}

	}

	static DWORD WINAPI setKinectAngle(LPVOID args){
		MAINFORM *pthis = (MAINFORM *) args;
		return pthis->setKinectAngle();
	}

	DWORD WINAPI setKinectAngle(){
		// When the buttons has been clicked, set the angle of the kinect.
		kinect->setKinectAngle(sliderAngle);

		// After setting the kinect angle, update the current value from the kinect.

		std::stringstream ss;
		kinectAngle = kinect->getKinectAngle();
		ss << kinectAngle; // Value comes from the kinect.
		CString text = ss.str().c_str();
		//set the textfield
		MFC_ecCURVAL->SetWindowText(text);
		//set the slider
		MFC_scKINECTANGLE->SetPos(kinectAngle * -1);
		//set the range to force an update
		MFC_scKINECTANGLE->SetRange(-27, 27, TRUE);
		return 0;
	}

	//-----------------------------------------------------------------------------------------
	// Event definition. These are the methods accessed when the main

public:

	void OnNMReleasedcapturekinectangle(NMHDR *pNMHDR, LRESULT *pResult)
	{
		std::stringstream ss;

		sliderAngle = MFC_scKINECTANGLE->GetPos() * -1;
		ss << sliderAngle;
		CString text = ss.str().c_str();
		MFC_ecNEWVAL->SetWindowText(text);
		*pResult = 0;

	}

	void OnBnClickedsetval()
	{
		if (kinectAngle == sliderAngle){
			return;
		}

		//If the value from this slider differs from the current kinect value, set it by starting a Thread for doing this.
		DWORD threadID;
		HANDLE thread = CreateThread(NULL, 0, setKinectAngle, this, 0, &threadID);

	}

	void OnCbnSelchangeKinectlist()
	{
		int changedSelection = MFC_cbKinectList->GetCurSel();
		CString text;
		text.Format(_T("%d"), changedSelection);
		OutputDebugString(text);
		//if(changedSelection != currentSelection)
		{

			OutputDebugString(L"Selection changed!\n");
			delete kinect;
			kinect = NULL;
#ifdef _DEBUG		
			//if( diffMemState.Difference( oldMemState, newMemState ) )
			{
		_CrtDumpMemoryLeaks();
				
			}
#endif			
			if (SUCCEEDED(kinectManager->selectKinect((LPCTSTR) kinectMap[changedSelection], kinect, GetSafeHwnd()))){
				currentSelection = changedSelection;
				int i = 0;
			}

		}

	}

	void OnFileQuit()
	{
		stopProgram();
	}

	void OnClose()
	{
		stopProgram();
	}

	// declares the message map
	DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------------------
class AppStart : public CWinApp
{
public:
	AppStart() {  }
public:
	virtual BOOL InitInstance()
	{
		afxMemDF = allocMemDF | checkAlwaysMemDF;
		long lBreakAlloc = 0;
		if ( lBreakAlloc > 0 )
		{
			_CrtSetBreakAlloc( lBreakAlloc );
		}
		oldMemState.Checkpoint();
		CWinApp::InitInstance();
		MAINFORM dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		return FALSE;
	} //close function
};
//-----------------------------------------------------------------------------------------
//Need a Message Map Macro for both CDialog and CWinApp
BEGIN_MESSAGE_MAP(MAINFORM, CDialog)	
	ON_NOTIFY(NM_RELEASEDCAPTURE, SC_kinectAngle, &MAINFORM::OnNMReleasedcapturekinectangle)	
	ON_BN_CLICKED(B_setVal, &MAINFORM::OnBnClickedsetval)
	ON_CBN_SELCHANGE(CB_KinectList, &MAINFORM::OnCbnSelchangeKinectlist)
	ON_COMMAND(ID_FILE_QUIT, &MAINFORM::OnFileQuit)
	ON_WM_CLOSE()
END_MESSAGE_MAP()
//-----------------------------------------------------------------------------------------

AppStart theApp;  //Starts the Application
