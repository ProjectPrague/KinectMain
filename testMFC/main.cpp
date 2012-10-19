
//-----------------------------------------------------------------------------------------
#include "resource.h"    //main symbols
#include <afxcmn.h>		//needed for the slider
#include <sstream>		//Needed for the conversion from int to String
#include "kinect.h"		//for kinect stuff
#include <map>			//used to map kinect ID's to the dropdown ID's.
#include <iostream>		// for debugging purposes
//-----------------------------------------------------------------------------------------
// Using
//-------------------------------------------------------------------------------------------
	//using namespace System;
	//using namespace System::Collections::Generic;
//-------------------------------------------------------------------------------------------
//Globals
CEdit * MFC_ecCURVAL; //MFC_ prefix for easy recognizing of visual items
CEdit * MFC_ecNEWVAL;
CButton * MFC_bSETVAL;
CSliderCtrl * MFC_scKINECTANGLE;
CStatic * MFC_stCURVAL;
CStatic * MFC_stNEWVAL;
CComboBox * MFC_cbKinectList;
KinectManager * kinectManager;
Kinect * kinect;
std::list<INuiSensor*> nuiList;
CStatic * MFC_ecFPSCOLOR, * MFC_ecDEPTHCOLOR;

int sliderAngle;

class MAINFORM: public CDialog
{
private:
	std::map<int, BSTR> kinectMap;
	

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
			initializePointers();
			// maakt kinectmanager object aan.
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
	}

	void initializeInterface()
	{
		// Stringstream usage to turn the int that is returned by getKinectAngle into a string to use it om SetWindowText.
		std::stringstream ss;
		// integer used for counting in for loop for the kinectlist.
		int i = 0;

		// need to add comments here.
		for (std::list<INuiSensor*>::const_iterator it = nuiList.begin();it != nuiList.end();++it,i++)
		{
				ss.clear();
				ss << i;
				CString text = ss.str().c_str();
				MFC_cbKinectList->AddString(L"Kinect "+text);
				kinectMap[i] = (*it)->NuiDeviceConnectionId();
				CString textJeMoeder = (LPCTSTR) (*it)->NuiUniqueId();
		}

		if(kinectMap.size() > 0)
		{
			kinect = kinectManager->selectKinect((LPCTSTR) kinectMap[0]);
			MFC_cbKinectList->SetCurSel(0);
		}

		if(kinectMap.size() == 0)
		{
			// do shit.
		}

		ss << kinect->getKinectAngle();
		CString text = ss.str().c_str();

		//Set the interface as you want it on your first run
		//You need an LPCTSTR for a SetWindowText. For this kind of string use, just prefix an L. For normal strings: convert to CString: CString s(str.c_str())
		MFC_stCURVAL->SetWindowText(L"Current Kinect Angle");
		MFC_stNEWVAL->SetWindowText(L"New Kinect Angle");
		MFC_ecCURVAL->SetWindowText(text);
		MFC_ecNEWVAL->SetWindowText(L"0");
		MFC_scKINECTANGLE->SetPos(0);
		MFC_scKINECTANGLE->SetRange(-27, 27, TRUE);

		CFont * cf = new CFont();
		//GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT
		cf->CreatePointFont(300,L"Starcraft");
		MFC_ecFPSCOLOR->SetFont(cf);
		MFC_ecDEPTHCOLOR->SetFont(cf);
	} 
	void initializeKinect()
	{
		kinectManager = new KinectManager;
		kinectManager->initialize(this->GetSafeHwnd());
		nuiList = kinectManager->getGlobalNuiList();
	}
	
//-----------------------------------------------------------------------------------------
// Event definition.
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
		// When the buttons has been clicked, set the angle of the kinect.
		kinect->setKinectAngle(sliderAngle);

		// After setting the kinect angle, update the current value from the kinect.
		std::stringstream ss;
		ss << kinect->getKinectAngle(); // Value comes from the kinect.
		CString text = ss.str().c_str();
		MFC_ecCURVAL->SetWindowText(text);
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
END_MESSAGE_MAP()
//-----------------------------------------------------------------------------------------
AppStart theApp;  //Starts the Application
