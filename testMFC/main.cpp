
//-----------------------------------------------------------------------------------------
#include <afxwin.h>      //MFC core and standard components
#include "resource.h"    //main symbols
#include <afxcmn.h>		//needed for the slider
//-----------------------------------------------------------------------------------------
//Globals
CEdit * MFC_ecINPUT; //MFC_ prefix for easy recognizing of visual items
CEdit * MFC_ecOUTPUT;
CButton * MFC_bGO;
CSliderCtrl * MFC_scKINECTANGLE;
class MAINFORM: public CDialog
{
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
            CDialog::OnInitDialog();
			initializePointers();
			initializeInterface();
            return true; 
    }

	void initializePointers()
	{
		//initialize all MFC pointers: map them to the objects in the resource.
		MFC_ecINPUT = (CEdit *) GetDlgItem(1001); //Both the numeral ID and the "real" ID may be used here. 1001 refers to EC_input, so GetDlgItem(EC_input) is also possible
		MFC_ecOUTPUT = (CEdit *) GetDlgItem(1003);
		MFC_bGO = (CButton *) GetDlgItem(1002);	
		MFC_scKINECTANGLE = (CSliderCtrl * ) GetDlgItem(SC_kinectAngle);
	}

	void initializeInterface()
	{
		//Set the interface as you want it on your first run
		MFC_ecINPUT->SetWindowText(L"Type Here"); 
		//You need an LPCTSTR here. For this kind of string use, just prefix an L. For normal strings: convert to CString: CString s(str.c_str())
		MFC_scKINECTANGLE->SetRangeMax(27);
		MFC_scKINECTANGLE->SetRangeMin(-27);
		MFC_scKINECTANGLE->SetPos(0);
	}

	
//-----------------------------------------------------------------------------------------
// Event definition.
public:
	
	afx_msg void button_go()
	{
		CString text;
		MFC_ecINPUT->GetWindowText(text);
		MFC_ecOUTPUT ->SetWindowText(text);
	}

DECLARE_MESSAGE_MAP()
};
//-----------------------------------------------------------------------------------------
class TheGame : public CWinApp
{
public:
TheGame() {  }
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
	ON_COMMAND(B_go, button_go)
END_MESSAGE_MAP()
//-----------------------------------------------------------------------------------------
TheGame theApp;  //Starts the Application