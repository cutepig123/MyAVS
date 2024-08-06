// InpuParaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "InpuParaDlg.h"
#include "afxdialogex.h"


// CInpuParaDlg dialog

IMPLEMENT_DYNAMIC(CInpuParaDlg, CDialogEx)

CInpuParaDlg::CInpuParaDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

CInpuParaDlg::~CInpuParaDlg()
{
}

void CInpuParaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInpuParaDlg, CDialogEx)
END_MESSAGE_MAP()


// CInpuParaDlg message handlers
CString CInpuParaDlg::GetInput()
{
	CString ret;
	GetDlgItemTextA(IDC_EDIT1, ret);
	return ret;
}
