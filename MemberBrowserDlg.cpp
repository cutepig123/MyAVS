﻿// MemberBrowserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MemberBrowserDlg.h"
#include "afxdialogex.h"
#include "TypeComposition.h"

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
	AddMeAndAllChildRecursive(pTreeCtrl, TVI_ROOT, inTypeName_, "");

	ExpandTreeItem(pTreeCtrl, TVI_ROOT);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
