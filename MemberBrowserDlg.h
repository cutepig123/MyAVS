#pragma once
#include "userfilterimpl.h"

// CMemberBrowserDlg dialog

class CMemberBrowserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMemberBrowserDlg)

public:
	CMemberBrowserDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMemberBrowserDlg();

	UserFilterImpl const *UserFilter_ = nullptr;

	bool retIsInput_ = true;
	CString retPath_;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MEMBER_BROWSER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
