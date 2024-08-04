
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
#include "userfilter.h"
#include "userfilterimpl.h"
#include <vector>
#include <optional>
#include <deque>
#include <map>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


COLORREF colors[] = {
	RGB(0, 0, 0)	// Black
	RGB(255, 0, 0),   // Red
	RGB(0, 255, 0),   // Green
	RGB(0, 0, 255),   // Blue
};

size_t NumColors = sizeof(colors) / sizeof(colors[0]);
static std::map < CString, int> g_dict;

COLORREF GetColorByType(CString const& type)
{
	auto it = g_dict.find(type);
	if (it != g_dict.end())
	{
		return colors[it->second];
	}
	int index = (int)g_dict.size() % NumColors;
	g_dict[type] = index;
	return colors[index];
}


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
		ti.lpszText = LPSTR("");
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

template<class T>
const T& Mymin(const T& a, const T& b)
{
	return (b < a) ? b : a;
}

template<class T>
const T& MyMax(const T& a, const T& b)
{
	return (b > a) ? b : a;
}

std::vector<CRect> GetPortRects(int x, int top, int btm, int num)
{
	std::vector<CRect> ret;
	for (size_t i = 0; i < num; i++)
	{
		double pitch = 1.0 * (btm-top) / (num + 1);
		double portSize = Mymin(pitch / 4, 10.0);
		double centerX = x;
		double centerY = top + pitch + i * pitch;
		ret.emplace_back(centerX - portSize, centerY - portSize, centerX + portSize, centerY + portSize);
	}
	return ret;
}

struct HitTestResult
{
	int blockIdx_ = -1;
	enum Type { NA, Block, InPort, OutPort } type_ = NA;
	int portIndx_ = -1;
	bool IsHit() const
	{
		return type_ != NA;
	}
};

class Block
{
	CRect rc_;
	std::unique_ptr<UserFilter> userFilter_;
	std::vector< CRect> insRect_;
	std::vector< CRect> outsRect_;
public:
	Block(){}

	Block(CRect const& rect, std::unique_ptr<UserFilter>&& userFilter)
		:rc_(rect), userFilter_(std::move(userFilter))
	{
		insRect_ = GetPortRects(rc_.left, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).ins_.size());
		outsRect_ = GetPortRects(rc_.right, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).outs_.size()); 
	}

	const CString& title() const
	{
		return Access::GetUserFilterImpl(*userFilter_).name_.c_str();
	}

	CPoint CenterPoint() const
	{
		return rc_.CenterPoint();
	}

	void Draw(CDC* pDC) const
	{
		// title
		pDC->Rectangle(rc_);
		pDC->TextOut(rc_.left+5, rc_.top+5, title());

		// Ins
		for (size_t i=0; i< insRect_.size(); i++)
		{
			const auto& rc = insRect_.at(i);
			const auto& port = Access::GetUserFilterImpl(*userFilter_).ins_.at(i);
			CPen pen(PS_SOLID, 1, GetColorByType(port.type.c_str()));
			auto old = pDC->SelectObject(pen);
			pDC->Rectangle(rc);
			pDC->TextOutA(rc.right, rc.top, (port.name + ":" + port.value + ":" + port.type).c_str());
			pDC->SelectObject(old);
		}

		// Outs
		for (size_t i = 0; i < outsRect_.size(); i++)
		{
			const auto& rc = outsRect_.at(i);
			const auto& port = Access::GetUserFilterImpl(*userFilter_).outs_.at(i);
			CPen pen(PS_SOLID, 1, GetColorByType(port.type.c_str()));
			auto old = pDC->SelectObject(pen);
			pDC->Rectangle(rc);
			pDC->TextOutA(rc.right, rc.top, (port.name + ":" + port.value + ":" + port.type).c_str());
			pDC->SelectObject(old);
		}
	}

	CRect GetRect() const
	{
		return rc_;
	}

	void SetRect(CRect const& rc)
	{
		rc_ = rc;
		insRect_ = GetPortRects(rc_.left, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).ins_.size());
		outsRect_ = GetPortRects(rc_.right, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).outs_.size());
	}

	void Offset(CPoint const& offset)
	{
		rc_ += offset;
		insRect_ = GetPortRects(rc_.left, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).ins_.size());
		outsRect_ = GetPortRects(rc_.right, rc_.top, rc_.bottom, (int)Access::GetUserFilterImpl(*userFilter_).outs_.size());
	}

	const UserFilterImpl::Port& GetInPort(size_t i) const
	{
		return Access::GetUserFilterImpl(*userFilter_).ins_.at(i);
	}

	const UserFilterImpl::Port& GetOutPort(size_t i) const
	{
		return Access::GetUserFilterImpl(*userFilter_).outs_.at(i);
	}

	std::vector<CRect> GetInPortRects() const
	{
		return GetPortRects(rc_.left, rc_.top, rc_.bottom, (int)insRect_.size());
	}
	std::vector<CRect> GetOutPortRects() const
	{
		return GetPortRects(rc_.right, rc_.top, rc_.bottom, (int)outsRect_.size());
	}

	CPoint GetInPortCenter(size_t i) const
	{
		auto rects = GetPortRects(rc_.left, rc_.top, rc_.bottom, (int)insRect_.size());
		return rects[i].CenterPoint();
	}

	CPoint GetOutPortCenter(size_t i) const
	{
		auto rects = GetPortRects(rc_.right, rc_.top, rc_.bottom, (int)outsRect_.size());
		return rects[i].CenterPoint();
	}

	HitTestResult HitTest(CPoint pt) const
	{
		const auto& b = *this;

		{
			auto ins = b.GetInPortRects();
			for (int j = 0; j < ins.size(); j++)
			{
				const auto& rc = ins[j];

				if (PtInRect(rc, pt))
				{
					return HitTestResult{ 0, HitTestResult::Type::InPort, j };
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
					return HitTestResult{ 0, HitTestResult::Type::OutPort, j };
				}
			}
		}

		{
			if (PtInRect(b.GetRect(), pt))
			{
				return HitTestResult{ 0, HitTestResult::Type::Block, -1 };
			}
		}
		
		return HitTestResult();
	}
};


void Line(CDC* pDC, CString const& type, CPoint const& pt1, CPoint const& pt2, double ratio)
{
	CPen pen(PS_SOLID, 1, GetColorByType(type));
	double x = (pt2.x - pt1.x) * ratio + pt1.x;
	
	auto old = pDC->SelectObject(pen);
	pDC->MoveTo(pt1);
	pDC->LineTo(CPoint((int)x, pt1.y));
	pDC->LineTo(CPoint((int)x, pt2.y));
	pDC->LineTo(pt2);
	pDC->SelectObject(old);
}

//std::vector<Port> MakePorts(const char* prefix, int n)
//{
//	std::vector<Port> ret;
//	for (int i = 0; i < n; i++)
//	{
//		CString name;
//		name.Format("%s %d", prefix, i);
//		CString type; type.Format("type%d", i%2);
//		ret.emplace_back(Port{ name , type });
//	}
//	return ret;
//}

CString MyFormat(const char* fmt, ...)
{
	va_list l;
	va_start(l, fmt);
	CString s;
	s.FormatV(fmt, l);
	return s;
}

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

	std::optional<HitTestResult> selected_;

	Test()
	{
		// TODO
		for (int i = 0; i < 2; i++)
		{
			int x = 100 + 300 * i, y = 100 + 300 * i;
			Block block1(CRect(x, y, x + 200, y + 200), CreateFilter("Add"));
			blocks.push_back(std::move(block1));
		}

		//for (int i = 0; i < 3; i++)
		//	conns_.emplace_back(Connection{ 0,i,1,i });
	}
	std::set<int> GetAllSuccessorNodes(int blocksIdx) const
	{
		std::set<int> successors;
		
		while (true)
		{
			bool bcontinue = false;
			for (const auto& conn : conns_)
			{
				if (conn.srcBlockIdx == blocksIdx || std::find(successors.begin(), successors.end(), blocksIdx) != successors.end())
				{
					if (std::find(successors.begin(), successors.end(), conn.dstBlockIdx) == successors.end())
					{
						successors.insert(conn.dstBlockIdx);
						bcontinue = true;
					}
				}
			}

			if (!bcontinue)
			{
				break;
			}
		}

		return successors;
	}

	struct ConnectResult
	{
		bool succeed = false;
		CString error;
	};

	ConnectResult Connect(int srcBlockIdx, int srcBlockPortIdx, int dstBlockIdx, int dstBlockPortIdx)
	{
		if (srcBlockIdx == dstBlockIdx)
		{
			return ConnectResult{ false, "Same block" };
		}
		
		auto type1 = blocks.at(srcBlockIdx).GetOutPort(srcBlockPortIdx).type;
		auto type2 = blocks.at(dstBlockIdx).GetInPort(dstBlockPortIdx).type;
		if (type1 != type2)
		{
			return ConnectResult{ false, "Diff type" };
		}
			
		// a destination cannot have more than one source
		//RemoveConnetionWithDestination(dstBlockIdx, dstBlockPortIdx);
		auto isSame = [dstBlockIdx, dstBlockPortIdx](Connection const& c)->bool {
			return c.dstBlockIdx == dstBlockIdx && c.dstBlockPortIdx == dstBlockPortIdx;
			};
		auto it = std::find_if(conns_.begin(), conns_.end(), isSame);
		if (it != conns_.end())
			return ConnectResult{ false, "Already have source connected" };

		// block之間不能有循環鏈接
		auto successors = GetAllSuccessorNodes(dstBlockIdx);
		if (std::find(successors.begin(), successors.end(), srcBlockIdx) != successors.end())
		{
			return ConnectResult{ false, "Circular Dependence" };
		}
		
		// insert connection
		conns_.emplace_back(Connection{ srcBlockIdx , srcBlockPortIdx , dstBlockIdx , dstBlockPortIdx });
		return ConnectResult{ true, "" };;
			
	}

	void Draw(CDC* pDC)
	{
		// blocks
		for (const auto& b:blocks)
		{
			b.Draw(pDC);
		}
		
		// connections
		for (int i=0; i<conns_.size(); i++)
		{
			const auto& conn = conns_[i];
			auto pt1 = blocks[conn.srcBlockIdx].GetOutPortCenter(conn.srcBlockPortIdx);
			auto pt2 = blocks[conn.dstBlockIdx].GetInPortCenter(conn.dstBlockPortIdx);

			auto type = blocks[conn.srcBlockIdx].GetOutPort(conn.srcBlockPortIdx).type;
			auto numConn = MyMax(6, (int)conns_.size());
			double pitch = 0.5 / numConn;
			Line(pDC, type.c_str(), pt1, pt2, 0.5 + pitch * (-i + numConn * 0.5));
		}

		// selections
		if (selected_)
		{
			const auto& hitTest = *selected_;
			auto& b = blocks[hitTest.blockIdx_];
			switch (hitTest.type_)
			{
			case HitTestResult::Type::OutPort:
			{
				auto pt = b.GetOutPortCenter(hitTest.portIndx_);
				pDC->Rectangle(CRect(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5));
				break;
			}
			//case HitTestResult::Type::InPort:
			//{
			//	auto pt = b.GetInPortCenter(hitTest.portIndx_);
			//	pDC->Rectangle(CRect(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5));
			//	break;
			//}
			default:
				break;
			}
		}
	}


	HitTestResult HitTest(CPoint pt) const
	{
		for (int i=0; i<blocks.size(); i++)
		{
			const auto& b = blocks[i];

			auto ret = b.HitTest(pt);

			if (ret.IsHit())
			{
				ret.blockIdx_ = i;
				return ret;
			}
		}
		return HitTestResult();
	}

	void RemoveConnetionWithDestination(int blockIdx_, int portIndx_)
	{
		auto isSame = [blockIdx_, portIndx_](Connection const& c)->bool {
			return c.dstBlockIdx == blockIdx_ && c.dstBlockPortIdx == portIndx_;
			};
		auto it = std::find_if(conns_.begin(), conns_.end(), isSame);
		if (it!= conns_.end())
		{
			conns_.erase(it);
		}
	}

	void Delete(CWnd* pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		switch (hitTest.type_)
		{
		case HitTestResult::Type::InPort:
		{
			RemoveConnetionWithDestination(hitTest.blockIdx_, hitTest.portIndx_);
			pWnd->RedrawWindow();
			break;
		}
		}
	}
	void LBtnDown(MyToolTip& tooltip, CWnd* pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		switch (hitTest.type_)
		{
		case HitTestResult::Type::Block:
		{
			auto& b = blocks[hitTest.blockIdx_];
			CRectTracker tracker(b.GetRect(), CRectTracker::dottedLine | CRectTracker::resizeInside);
			if (tracker.Track(pWnd, pt))
			{
				CRect rc;
				tracker.GetTrueRect(rc);
				b.SetRect(rc);

				pWnd->RedrawWindow();
			}
			break;
		}
		case HitTestResult::Type::OutPort:
		{
			// If it is first click, it must be output type
			// If it is 2nd click, 
			// - if it is output type, replace first click
			// - if it is input type & diff block & connectable, make connection
			selected_ = hitTest;
			pWnd->RedrawWindow();
			break;
		}
		case HitTestResult::Type::InPort:
		{
			if (selected_)
			{
				auto connSts = Connect(selected_->blockIdx_, selected_->portIndx_, hitTest.blockIdx_, hitTest.portIndx_);
				if(connSts.succeed)
				{
					selected_.reset();
					pWnd->RedrawWindow();
				}
				else
				{
					CPoint ptScreen = pt;
					pWnd->ClientToScreen(&ptScreen);
					ptScreen.x -= 15;
					ptScreen.y -= 45;
					tooltip.Show(connSts.error, ptScreen);
				}
			}
			break;
		}
		default:
			break;
		}
	}

	void ShowToolTip(MyToolTip& tooltip, CWnd*pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		CString text;

		switch (hitTest.type_)
		{
		case HitTestResult::Type::Block:
		{
			auto const& b = blocks[hitTest.blockIdx_];
			text = b.title();
			break;
		}
		case HitTestResult::Type::OutPort:
		{
			auto const& b = blocks[hitTest.blockIdx_];
			const auto& p = b.GetOutPort(hitTest.portIndx_);
			text.Format("%s %s %s", b.title(), p.name, p.type);
			break;
		}
		case HitTestResult::Type::InPort:
		{
			auto const& b = blocks[hitTest.blockIdx_];
			const auto& p = b.GetInPort(hitTest.portIndx_);
			text.Format("%s %s %s", b.title(), p.name, p.type);
			break;
		}
		default:
			tooltip.Hide();
			break;
		}

		if (!text.IsEmpty())
		{
			CPoint ptScreen = pt;
			pWnd->ClientToScreen(&ptScreen);
			ptScreen.x -= 15;
			ptScreen.y -= 45;
			tooltip.Show(text, ptScreen);
		}
	}
};

std::unique_ptr<Test> g_test;

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
	ON_WM_LBUTTONDBLCLK()
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
	g_test.reset(new Test);
	return CView::PreCreateWindow(cs);
}

// CMFCApplication1View 绘图
void CMFCApplication1View::OnDraw(CDC* pDC)
{
	CMFCApplication1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	
	g_test->Draw(pDC);
	
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
	g_test->LBtnDown(g_MyToolTip, this, point);

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
	//g_test.ShowToolTip(g_MyToolTip, this, point);

	CView::OnMouseMove(nFlags, point);
}


void CMFCApplication1View::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	g_test->Delete(this, point);

	CView::OnLButtonDblClk(nFlags, point);
}
