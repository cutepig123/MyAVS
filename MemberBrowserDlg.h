#pragma once
#include "userfilterimpl.h"

// CMemberBrowserDlg dialog

class CMemberBrowserDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMemberBrowserDlg)

public:
	CMemberBrowserDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMemberBrowserDlg();

	UserFilterImpl *UserFilter_ = nullptr;

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
	afx_msg void OnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	void GetSelectedPath();
	afx_msg void OnBnClickedButtonUpdate();
};
