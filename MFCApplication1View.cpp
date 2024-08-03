
// MFCApplication1View.cpp: CMFCApplication1View 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#endif

#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"
#include <vector>
#include <deque>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCApplication1View

IMPLEMENT_DYNCREATE(CMFCApplication1View, CView)

BEGIN_MESSAGE_MAP(CMFCApplication1View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CMFCApplication1View 构造/析构

CMFCApplication1View::CMFCApplication1View() noexcept
{
	// TODO: 在此处添加构造代码

}

CMFCApplication1View::~CMFCApplication1View()
{
}

BOOL CMFCApplication1View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFCApplication1View 绘图

struct MyToolTip
{
	HWND hwndTT = 0;
	TOOLINFO ti;

	void Create()
	{
		unsigned int uid = 0;       // for ti initialization
		hwndTT = CreateWindowEx(WS_EX_TOPMOST,
			TOOLTIPS_CLASS,
			NULL,
			TTS_NOPREFIX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			NULL,
			NULL
		);

		// INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE
		memset(&ti, 0, sizeof(ti));
		//ti.cbSize = sizeof(TOOLINFO);
		ti.cbSize = TTTOOLINFO_V1_SIZE;
		ti.uFlags = TTF_TRACK;
		ti.hwnd = NULL;
		ti.hinst = NULL;
		ti.uId = uid;
		ti.lpszText = "";
		// ToolTip control will cover the whole window
		ti.rect.left = 0;
		ti.rect.top = 0;
		ti.rect.right = 0;
		ti.rect.bottom = 0;
		::SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
	}

	void Show(const char* strText, CPoint ptMousePosition)
	{
		ti.lpszText = (LPSTR)(LPCSTR)strText;
		::SendMessage(hwndTT, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
		::SendMessage(hwndTT, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD)MAKELONG(ptMousePosition.x, ptMousePosition.y));
		::SendMessage(hwndTT, TTM_TRACKACTIVATE, true, (LPARAM)(LPTOOLINFO)&ti);
	}

	void Hide()
	{
		ShowWindow(hwndTT, SW_HIDE);
	}
}g_MyToolTip;

std::vector<CRect> GetPortRects(int x, int top, int btm, int num)
{
	std::vector<CRect> ret;
	for (size_t i = 0; i < num; i++)
	{
		double pitch = 1.0 * (btm-top) / (num + 1);
		double portSize = pitch / 4;
		double centerX = x;
		double centerY = top + pitch + i * pitch;
		ret.emplace_back(centerX - portSize, centerY - portSize, centerX + portSize, centerY + portSize);
	}
	return ret;
}

struct Block
{
	CRect rc_;
	CString title_;
	size_t numIns_;
	size_t numOuts_;
public:
	Block():numIns_(0), numOuts_(0){}

	void Draw(CDC* pDC) const
	{
		pDC->Rectangle(rc_);
		pDC->TextOut(rc_.left+5, rc_.top+5, title_);

		//for (size_t i = 0; i < numIns_; i++)
		//{
		//	double pitch = 1.0 * rc_.Height() / (numIns_ + 1);
		//	double portSize = pitch / 4;
		//	double centerX = rc_.left;
		//	double centerY= rc_.top + pitch + i * pitch;
		//	pDC->Rectangle(centerX - portSize, centerY - portSize, centerX + portSize, centerY + portSize);
		//}

		for (const auto& rc : GetPortRects(rc_.left, rc_.top, rc_.bottom, numIns_))
		{
			pDC->Rectangle(rc);
		}
		for (const auto& rc : GetPortRects(rc_.right, rc_.top, rc_.bottom, numOuts_))
		{
			pDC->Rectangle(rc);
		}
	}

	std::vector<CRect> GetInPortRects() const
	{
		return GetPortRects(rc_.left, rc_.top, rc_.bottom, numIns_);
	}
	std::vector<CRect> GetOutPortRects() const
	{
		return GetPortRects(rc_.right, rc_.top, rc_.bottom, numOuts_);
	}

	CPoint GetInPortCenter(size_t i) const
	{
		auto rects = GetPortRects(rc_.left, rc_.top, rc_.bottom, numIns_);
		return rects[i].CenterPoint();
	}

	CPoint GetOutPortCenter(size_t i) const
	{
		auto rects = GetPortRects(rc_.right, rc_.top, rc_.bottom, numOuts_);
		return rects[i].CenterPoint();
	}
};

void Line(CDC* pDC, CPoint const& pt1, CPoint const& pt2, double ratio)
{
	double x = (pt2.x - pt1.x) * ratio + pt1.x;
	
	pDC->MoveTo(pt1);
	pDC->LineTo(CPoint(x, pt1.y));
	pDC->LineTo(CPoint(x, pt2.y));
	pDC->LineTo(pt2);
}

template <typename T, size_t N>
class CircularQueue {
public:
	
	void enqueue(const T& value) {
		t_.push_back(value);
		while (t_.size() > N)
			t_.pop_front();
	}

	bool isEmpty() const {
		return t_.empty();
	}

	size_t size() const {
		return t_.size();
	}

	template <class F>
	void ForEach(F f) const
	{
		for (const auto& t : t_)
			f(t);
	}
private:
	std::deque<T> t_;
};


struct Test
{
	std::vector<Block> blocks;

	struct Connection
	{
		int srcBlockIdx=0;
		int srcBlockPortIdx = 0;

		int dstBlockIdx = 0;
		int dstBlockPortIdx = 0;
	};
	std::vector<Connection> conns_;

	struct HitTestResult
	{
		int blockIdx_ = -1;
		enum Type { NA, Block, InPort, OutPort } type_ = NA;
		int portIndx_ = -1;
	};

	CircularQueue< HitTestResult, 2> selected_;

	Test()
	{
		for (int i = 0; i < 2; i++)
		{
			Block block1;
			int x = 100 + 300 * i, y = 100 + 300 * i;
			block1.rc_ = CRect(x, y, x+200, y+200);
			block1.title_.Format("block%d", i);
			block1.numIns_ = 3;
			block1.numOuts_ = 4;
			blocks.push_back(block1);
		}

		for (int i = 0; i < 3; i++)
			conns_.emplace_back(Connection{ 0,i,1,i });
	}

	void Draw(CDC* pDC)
	{
		for (const auto& b:blocks)
		{
			b.Draw(pDC);
		}
		
		for (int i=0; i<conns_.size(); i++)
		{
			const auto& conn = conns_[i];
			auto pt1 = blocks[conn.srcBlockIdx].GetOutPortCenter(conn.srcBlockPortIdx);
			auto pt2 = blocks[conn.dstBlockIdx].GetInPortCenter(conn.dstBlockPortIdx);
			//pDC->MoveTo(pt1);
			//pDC->LineTo(pt2);

			double pitch = 0.5 / conns_.size();
			Line(pDC, pt1, pt2, 0.5 + pitch * (-i + conns_.size() *0.5));
		}

		selected_.ForEach([pDC, this](HitTestResult const& hitTest) {
			auto& b = blocks[hitTest.blockIdx_];
			switch (hitTest.type_)
			{
			case HitTestResult::Type::OutPort:
			{
				auto pt = b.GetOutPortCenter(hitTest.portIndx_);
				pDC->Rectangle(CRect(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5));
				break;
			}
			case HitTestResult::Type::InPort:
			{
				auto pt = b.GetInPortCenter(hitTest.portIndx_);
				pDC->Rectangle(CRect(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5));
				break;
			}
			default:
				break;
			}
			});
	}


	HitTestResult HitTest(CPoint pt) const
	{
		for (int i=0; i<blocks.size(); i++)
		{
			const auto& b = blocks[i];

			{
				auto ins = b.GetInPortRects();
				for (int j = 0; j < ins.size(); j++)
				{
					const auto& rc = ins[j];
					
					if (PtInRect(rc, pt))
					{
						return HitTestResult{ i, HitTestResult::Type::InPort, j };
					}
				}
			}

			{
				auto outs = b.GetOutPortRects();
				for (int j = 0; j < outs.size(); j++)
				{
					const auto& rc = outs[j];
					if (PtInRect(rc, pt))
					{
						return HitTestResult{ i, HitTestResult::Type::OutPort, j };
					}
				}
			}

			{
				if (PtInRect(b.rc_, pt))
				{
					return HitTestResult{ i, HitTestResult::Type::Block, -1 };
				}
			}
		}
		return HitTestResult();
	}

	void LBtnDown(CWnd* pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		switch (hitTest.type_)
		{
		case HitTestResult::Type::Block:
		{
			auto& b = blocks[hitTest.blockIdx_];
			CRectTracker tracker(b.rc_, CRectTracker::dottedLine | CRectTracker::resizeInside);
			tracker.Track(pWnd, pt);
			tracker.GetTrueRect(b.rc_);
			pWnd->RedrawWindow();
			break;
		}
		case HitTestResult::Type::OutPort:
		case HitTestResult::Type::InPort:
		{
			selected_.enqueue(hitTest);
			pWnd->RedrawWindow();
			break;
		}
		default:
			break;
		}
	}

	void ShowToolTip(MyToolTip& tooltip, CWnd*pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		switch (hitTest.type_)
		{
		case HitTestResult::Type::Block:
		case HitTestResult::Type::OutPort:
		case HitTestResult::Type::InPort:
		{
			CString text;
			text.Format("%d %d %d", hitTest.type_, hitTest.blockIdx_, hitTest.portIndx_);
			CPoint ptScreen = pt;
			pWnd->ClientToScreen(&ptScreen);
			ptScreen.x -= 15;
			ptScreen.y -= 45;
			tooltip.Show(text, ptScreen);
			break;
		}
		default:
			tooltip.Hide();
			break;
		}

	}
}g_test;

void CMFCApplication1View::OnDraw(CDC* pDC)
{
	CMFCApplication1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	
	g_test.Draw(pDC);
	
}


// CMFCApplication1View 打印

BOOL CMFCApplication1View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFCApplication1View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFCApplication1View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMFCApplication1View 诊断

#ifdef _DEBUG
void CMFCApplication1View::AssertValid() const
{
	CView::AssertValid();
}

void CMFCApplication1View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCApplication1Doc* CMFCApplication1View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCApplication1Doc)));
	return (CMFCApplication1Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFCApplication1View 消息处理程序

void CMFCApplication1View::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	g_test.LBtnDown(this, point);

	CView::OnLButtonDown(nFlags, point);
}


BOOL CMFCApplication1View::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	
	return CView::PreTranslateMessage(pMsg);
}


int CMFCApplication1View::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	g_MyToolTip.Create();

	
	return 0;
}


void CMFCApplication1View::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	g_test.ShowToolTip(g_MyToolTip, this, point);

	CView::OnMouseMove(nFlags, point);
}
