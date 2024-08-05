#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1View.h"
#include "MainFrm.h"
#include "MyDlgBar.h"
#include "userfilter.h"
#include <algorithm>
#include <iterator>
// CMyDlgBar

IMPLEMENT_DYNAMIC(CMyDlgBar, CDialogBar)

CMyDlgBar::CMyDlgBar()
{

}

CMyDlgBar::~CMyDlgBar()
{
}


BEGIN_MESSAGE_MAP(CMyDlgBar, CDialogBar)
	ON_BN_CLICKED(IDC_BUTTON_FILTER, &CMyDlgBar::OnBnClickedButtonFilter)
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_BLOCKS, OnClickListBlocks)
END_MESSAGE_MAP()

// CMyDlgBar message handlers

void CMyDlgBar::DoDataExchange(CDataExchange* pDX)
{
	// TODO: Add your specialized code here and/or call the base class
	CDialogBar::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_FILTER, m_edit);
	DDX_Control(pDX, IDC_LIST_BLOCKS, m_listCtrl);
	
}

void UpdateList(CListCtrl& m_listCtrl, const char* filter)
{
	auto all = GetAllUserFilters();

	m_listCtrl.DeleteAllItems();
	int index = 0;
	std::for_each(all.begin(), all.end(), 
		[&m_listCtrl, filter, &index](std::string const& t) {
		if (!filter || t.find(filter)!=t.npos)
		{
			m_listCtrl.InsertItem(index++, t.c_str());
		}
	});
	
}

LRESULT CMyDlgBar::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = HandleInitDialog(wParam, lParam);

	if (!UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
	}

	UpdateList(m_listCtrl, nullptr);

	return bRet;
}

void CMyDlgBar::OnBnClickedButtonFilter()
{
	m_edit.SetWindowTextA("XXX");
}

void CMyDlgBar::OnClickListBlocks(NMHDR * /* pNMHDR */, LRESULT *pResult)
{
	UINT numSel = m_listCtrl.GetSelectedCount();
	if (numSel > 0)
	{
		POSITION pos = m_listCtrl.GetFirstSelectedItemPosition();
		int nSelId = m_listCtrl.GetNextSelectedItem(pos);
		ASSERT(nSelId >= 0);

		CString szBlockName = m_listCtrl.GetItemText(nSelId, 0);
		CMFCApplication1View* pActiveView = (CMFCApplication1View*)(((CFrameWnd*)AfxGetMainWnd())->GetActiveView());
		pActiveView->AddBlock(szBlockName);
	}

	*pResult = 0;
}
