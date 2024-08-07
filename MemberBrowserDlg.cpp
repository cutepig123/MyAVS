// MemberBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MemberBrowserDlg.h"
#include "afxdialogex.h"
#include "TypeComposition.h"
#include "userfilterimpl.h"
// CMemberBrowserDlg dialog

IMPLEMENT_DYNAMIC(CMemberBrowserDlg, CDialogEx)

CMemberBrowserDlg::CMemberBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_MEMBER_BROWSER, pParent)
{

}

CMemberBrowserDlg::~CMemberBrowserDlg()
{
}

void CMemberBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMemberBrowserDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMemberBrowserDlg::OnBnClickedOk)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CMemberBrowserDlg::OnSelchangedTree1)
	ON_BN_CLICKED(ID_BUTTON_UPDATE, &CMemberBrowserDlg::OnBnClickedButtonUpdate)
END_MESSAGE_MAP()


// CMemberBrowserDlg message handlers

HTREEITEM AddTreeItem(CTreeCtrl* pTreeCtrl, HTREEITEM parent, const char* name)
{
	TVITEM tvItem;
	tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvItem.pszText = (char*)name;
	tvItem.iImage = 0;
	tvItem.iSelectedImage = 1;
	TVINSERTSTRUCT tvInsert;
	tvInsert.item = tvItem;
	tvInsert.hParent = parent; // TVI_ROOT; // 将新元素插入到根节点
	tvInsert.hInsertAfter = TVI_LAST; // 将新元素插入到最后
	HTREEITEM hItem = pTreeCtrl->InsertItem(&tvInsert);
	return hItem;
}

void AddMeAndAllChildRecursive(CTreeCtrl* pTreeCtrl, HTREEITEM parent, const char* fieldType, const char* fieldName)
{
	auto rootHandle = AddTreeItem(pTreeCtrl, parent, CString(fieldName) + ":" + CString(fieldType));
	for (const auto& subitem: GetTypeItems(fieldType))
	{
		AddMeAndAllChildRecursive(pTreeCtrl, rootHandle, subitem.type.c_str(), subitem.name.c_str());
	}
}

void AddUserFilter(CTreeCtrl* pTreeCtrl, HTREEITEM parent, UserFilterImpl const* UserFilter_)
{
	auto rootHandle = AddTreeItem(pTreeCtrl, parent, "Input");
	for (const auto& port : UserFilter_->ins_.ports_)
	{
		AddMeAndAllChildRecursive(pTreeCtrl, rootHandle, port.value->type.c_str(), port.name.c_str());
	}

	rootHandle = AddTreeItem(pTreeCtrl, parent, "Output");
	for (const auto& port : UserFilter_->outs_.ports_)
	{
		AddMeAndAllChildRecursive(pTreeCtrl, rootHandle, port.value->type.c_str(), port.name.c_str());
	}
}

void ExpandTreeItem(CTreeCtrl* pTreeCtrl, HTREEITEM hItem)
{
	// 展开当前节点
	pTreeCtrl->Expand(hItem, TVE_EXPAND);

	// 递归展开当前节点的子节点
	HTREEITEM hChild = pTreeCtrl->GetChildItem(hItem);
	while (hChild != NULL)
	{
		ExpandTreeItem(pTreeCtrl, hChild);
		hChild = pTreeCtrl->GetNextItem(hChild, TVGN_NEXT);
	}
}

BOOL CMemberBrowserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CTreeCtrl* pTreeCtrl = (CTreeCtrl*)GetDlgItem(IDC_TREE1);
	AddUserFilter(pTreeCtrl, TVI_ROOT, UserFilter_);

	ExpandTreeItem(pTreeCtrl, TVI_ROOT);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// "a:int" --> a
CString GetName(CString const& NameAndType)
{
	int p = NameAndType.Find(":");
	if (p < 0) return NameAndType;
	return NameAndType.Mid(0, p);
}

void CMemberBrowserDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetSelectedPath();
	CDialogEx::OnOK();
}


void CMemberBrowserDlg::OnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	GetSelectedPath();
	std::string sValue = retIsInput_?
		UserFilter_->ins_.Read(retPath_)
		:UserFilter_->outs_.Read(retPath_);
	SetDlgItemText(IDC_EDIT1, sValue.c_str());
}


void CMemberBrowserDlg::GetSelectedPath()
{
	CTreeCtrl* pTreeCtrl = (CTreeCtrl*)GetDlgItem(IDC_TREE1);
	HTREEITEM hSelectedItem = pTreeCtrl->GetSelectedItem();
	CString ret;
	for (HTREEITEM pThisItem = hSelectedItem; pThisItem; pThisItem = pTreeCtrl->GetParentItem(pThisItem))
	{
		CString text = pTreeCtrl->GetItemText(pThisItem);
		CString name = GetName(text);

		if (ret.IsEmpty())
			ret = name;
		else
			ret = name + "." + ret;
	}

	retIsInput_ = ret.Find("Input.") == 0;
	if (retIsInput_)
	{
		retPath_ = ret.Mid(strlen("Input."));
	}
	else if (ret.Find("Output.") == 0)
	{
		retPath_ = ret.Mid(strlen("Output."));
	}
}


void CMemberBrowserDlg::OnBnClickedButtonUpdate()
{
	// TODO: Add your control notification handler code here
	CString sValue;
	GetDlgItemText(IDC_EDIT1, sValue);
	if (retIsInput_)
		UserFilter_->ins_.Write(retPath_, (const char*)sValue);
	else
		UserFilter_->outs_.Write(retPath_, (const char*)sValue);
	
}
