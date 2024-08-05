#pragma once


// CInpuParaDlg dialog

class CInpuParaDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CInpuParaDlg)

public:
	CInpuParaDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInpuParaDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

	CString GetInput();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
