#pragma once
#include "resource.h"


// CMyDlgBar

class CMyDlgBar : public CDialogBar
{
	DECLARE_DYNAMIC(CMyDlgBar)

public:
	CMyDlgBar();
	virtual ~CMyDlgBar();

	// Dialog Data
	//{{AFX_DATA(CMyDlgBar)
	enum { IDD = IDD_DIALOGBAR };
	//}}AFX_DATA


	CEdit m_edit;
	CListCtrl m_listCtrl;
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnBnClickedButtonFilter();
	LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickListBlocks(NMHDR * /* pNMHDR */, LRESULT *pResult);
};


