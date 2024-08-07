#include "pch.h"
#include "framework.h"

#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"
#include "MemberBrowserDlg.h"
#include "InpuParaDlg.h"
#include "mainfrm.h"
#include "userfilter.h"
#include "userfilterimpl.h"
#include <vector>
#include <deque>
#include <iterator>
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


CRect GetPortRect(int x, int top, int btm, size_t numPorts, size_t currentPortIndex)
{
	std::map<CString, CRect> ret;
	size_t i = currentPortIndex;
	
	double pitch = 1.0 * (btm - top) / (numPorts + 1);
	double portSize = Mymin(pitch / 4, 10.0);
	double centerX = x;
	double centerY = top + pitch + i * pitch;
	return CRect(centerX - portSize, centerY - portSize, centerX + portSize, centerY + portSize);
	
}

std::map<CString, CRect> GetPortRects(int x, int top, int btm, std::vector<CString> const& ports)
{
	size_t num = ports.size();
	std::map<CString, CRect> ret;
	for (size_t i = 0; i < num; i++)
	{
		ret[ports.at(i)] = GetPortRect(x, top, btm, num, i);
	}
	return ret;
}

struct HitTestResult
{
	CString blockName_;
	enum Type { NA, Block, InPort, OutPort } type_ = NA;
	CString portIndx_;
	bool IsHit() const
	{
		return type_ != NA;
	}

	HitTestResult() {}

	HitTestResult(CString const& blockName, Type type, CString const& portIndx)
		:blockName_(blockName), type_(type), portIndx_(portIndx)
	{}
};

class Block
{
	CRect rc_;
	std::unique_ptr<UserFilter> userFilter_;
	std::vector<CString> ui_ins_;
	std::vector<CString> ui_outs_;
public:
	Block(){}

	Block(CRect const& rect, std::unique_ptr<UserFilter>&& userFilter)
		:rc_(rect), userFilter_(std::move(userFilter))
	{
		auto& impl = getRawObj();
		
		std::transform(impl.ins_.ports_.begin(), impl.ins_.ports_.end(), std::back_inserter(ui_ins_),
			[](UserFilterImpl::Port const& p)->CString {return p.name.c_str(); });
		std::transform(impl.outs_.ports_.begin(), impl.outs_.ports_.end(), std::back_inserter(ui_outs_),
			[](UserFilterImpl::Port const& p)->CString {return p.name.c_str(); });
	}

	void AddInput(const char* name)
	{
		if (std::find(ui_ins_.begin(), ui_ins_.end(), name) == ui_ins_.end())
			ui_ins_.push_back(name);
	}

	void AddOutput(const char* name)
	{
		if (std::find(ui_outs_.begin(), ui_outs_.end(), name) == ui_outs_.end())
			ui_outs_.push_back(name);
	}

	void DeleteInport(const char* name)
	{
		auto it = std::find(ui_ins_.begin(), ui_ins_.end(), name);
		if (it!= ui_ins_.end())
		{
			ui_ins_.erase(it);
		}
	}
	void DeleteOutport(const char* name)
	{
		auto it = std::find(ui_outs_.begin(), ui_outs_.end(), name);
		if (it != ui_outs_.end())
		{
			ui_outs_.erase(it);
		}
	}

	void Invoke()
	{
		userFilter_->Invoke();
	}

	CString title() const
	{
		return getRawObj().name_.c_str();
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
		for (const auto&i: ui_ins_)
		{
			const auto rc = GetInPortRect(i);
			const auto port = getRawObj().ins_.GetObjByName(i);
			CPen pen(PS_SOLID, 1, GetColorByType(port->type.c_str()));
			auto old = pDC->SelectObject(pen);
			pDC->Rectangle(rc);
			pDC->TextOutA(rc.right, rc.top, (i + ":" /*+ port.value + ":"*/ + port->type.c_str()));
			pDC->SelectObject(old);
		}

		// Outs
		for (const auto& i : ui_outs_)
		{
			const auto& rc = GetOutPortRect(i);
			const auto port = getRawObj().outs_.GetObjByName(i);
			CPen pen(PS_SOLID, 1, GetColorByType(port->type.c_str()));
			auto old = pDC->SelectObject(pen);
			pDC->Rectangle(rc);
			pDC->TextOutA(rc.right, rc.top, (i + ":" /*+ port.value + ":"*/ + port->type.c_str()));
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
	}

	void Offset(CPoint const& offset)
	{
		rc_ += offset;
		
		
	}

	std::shared_ptr<MyObject> GetInPort(const char* i) const
	{
		return getRawObj().ins_.GetObjByName(i);
	}

	UserFilterImpl& getRawObj()
	{
		return Access::GetUserFilterImpl(*userFilter_);
	}

	UserFilterImpl& getRawObj() const
	{
		return Access::GetUserFilterImpl(*userFilter_);
	}
	
	std::map<CString, CRect> GetInPortRects() const
	{
		auto insRect_ = GetPortRects(rc_.left, rc_.top, rc_.bottom, ui_ins_);
		return insRect_;
	}
	
	CRect GetInPortRect(CString const& name) const
	{
		auto index = std::find(ui_ins_.begin(), ui_ins_.end(), name) - ui_ins_.begin();
		auto insRect_ = GetPortRect(rc_.left, rc_.top, rc_.bottom, ui_ins_.size(), index);
		return insRect_;
	}

	std::map<CString, CRect> GetOutPortRects() const
	{
		auto outsRect_ = GetPortRects(rc_.right, rc_.top, rc_.bottom, ui_outs_);
		return outsRect_;
	}

	CRect GetOutPortRect(CString const& name) const
	{
		auto index = std::find(ui_outs_.begin(), ui_outs_.end(), name) - ui_outs_.begin();
		auto insRect_ = GetPortRect(rc_.right, rc_.top, rc_.bottom, ui_outs_.size(), index);
		return insRect_;
	}

	CPoint GetInPortCenter(CString const& i) const
	{
		return GetInPortRect(i).CenterPoint();
	}

	CPoint GetOutPortCenter(CString const& i) const
	{
		return GetOutPortRect(i).CenterPoint();
	}

	HitTestResult HitTest(CPoint pt) const
	{
		const auto& b = *this;

		{
			for (const auto& b: b.GetInPortRects())
			{
				const auto& rc = b.second;

				if (PtInRect(rc, pt))
				{
					return HitTestResult( 0, HitTestResult::Type::InPort, b.first );
				}
			}
		}

		{
			for (const auto& b : b.GetOutPortRects())
			{
				const auto& rc = b.second;
				if (PtInRect(rc, pt))
				{
					return HitTestResult( 0, HitTestResult::Type::OutPort, b.first);
				}
			}
		}

		{
			if (PtInRect(b.GetRect(), pt))
			{
				return HitTestResult( 0, HitTestResult::Type::Block, CString() );
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

CString MyFormat(const char* fmt, ...)
{
	va_list l;
	va_start(l, fmt);
	CString s;
	s.FormatV(fmt, l);
	return s;
}

CPoint TrackLine(CWnd* pWnd, CPoint point)
{
	pWnd->SetCapture();

	auto pDrawDC = pWnd->GetDC();
	CPoint newPt = point;

	for (;;)
	{
		MSG msg;
		VERIFY(::GetMessage(&msg, NULL, 0, 0));

		if (CWnd::GetCapture() != pWnd)
			break;

		switch (msg.message)
		{
			// handle movement/accept messages
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			newPt = CPoint(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));

			pWnd->RedrawWindow();
			pDrawDC->MoveTo(point);
			pDrawDC->LineTo(newPt);

			if (msg.message == WM_LBUTTONUP)
			{
				goto ExitLoop;

			}
			break;

			// handle cancel messages
		case WM_KEYDOWN:
			if (msg.wParam != VK_ESCAPE)
				break;

		default:
			DispatchMessage(&msg);
			break;
		}
	}

ExitLoop:
	pWnd->ReleaseDC(pDrawDC);
	ReleaseCapture();
	pWnd->RedrawWindow();

	return newPt;
}

// -------------------------------------------------------------------
// --- Reversed iterable

template <typename T>
struct reversion_wrapper { T& iterable; };

template <typename T>
auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T>
auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }

template <typename T>
reversion_wrapper<T> reverse(T&& iterable) { return{ iterable }; }

long PopupMenu(CWnd* pWnd, long ID, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(ID);

	CMenu *pPopup = menu.GetSubMenu(0);

	long command = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, pWnd);
	return command;
}
struct Test
{
	std::map<CString, Block> blocks;

	struct Connection
	{
		CString srcBlockName;
		CString srcBlockPortIdx;

		CString dstBlockName;
		CString dstBlockPortIdx;

		Connection() {}
		Connection(CString const& srcBlockName_, CString const& srcBlockPortIdx_, CString const& dstBlockName_, CString const& dstBlockPortIdx_)
		:srcBlockName(srcBlockName_), srcBlockPortIdx(srcBlockPortIdx_)
			, dstBlockName(dstBlockName_), dstBlockPortIdx(dstBlockPortIdx_)
		{}
	};
	std::vector<Connection> conns_;

	CString NewBlockName(const char* prefix)
    {
        for(int i=0; ; i++)
        {
            CString name;
            name.Format("%s %d", prefix, i);
            if(blocks.find(name)==blocks.end())
                return name;
        }
    }

	Test()
	{
		// TODO
		for (int i = 0; i < 2; i++)
		{
			int x = 100 + 300 * i, y = 100 + 300 * i;
			Block block1(CRect(x, y, x + 200, y + 200), CreateFilter("Add"));
			blocks[NewBlockName("Add")] = (std::move(block1));
		}

		//for (int i = 0; i < 3; i++)
		//	conns_.emplace_back(Connection{ 0,i,1,i });
	}
	
	void AddBlock(const char* name)
	{
		int i = 0;
		int x = 200 + 300 * i, y = 100 + 300 * i;
		Block block1(CRect(x, y, x + 200, y + 200), CreateFilter(name));
		blocks[NewBlockName(name)] = (std::move(block1));
	}

	void Invoke()
	{
		std::map<CString, int> outstandingInputs;	// Num of "Not ready" inputs for each block
        for (const auto& b : blocks)
		{
			outstandingInputs[b.first] =0;
		}

		for (const auto& conn : conns_)
		{
			outstandingInputs[conn.dstBlockName]++;
		}

		std::deque<CString> readyBlocks;
		// Find a node/UserFilter whose inputs are all ready (ie no connection destination is it)
		for (const auto& b : blocks)
		{
			bool bIsReady = outstandingInputs[b.first] == 0;
			if (bIsReady)
			{
				readyBlocks.push_back(b.first);
			}
		}

		while (!readyBlocks.empty())
		{
			// Invoke
			CString currentName = readyBlocks.front();
			Block& current = blocks.at(currentName);
			current.Invoke();
			readyBlocks.pop_front();

			// Update successor Inputs
			for (const auto& conn : conns_)
			{
				if (conn.srcBlockName == currentName)
				{
					// Update successor Inputs
					blocks.at(conn.dstBlockName).getRawObj().ins_.Write(conn.dstBlockPortIdx,
						blocks.at(conn.srcBlockName).getRawObj().outs_.Read(conn.srcBlockPortIdx));
					// Update successor outstandingInputs
					outstandingInputs.at(conn.dstBlockName)--;
					// add to queue if it is ready
					if (outstandingInputs.at(conn.dstBlockName)==0)
					{
						readyBlocks.push_back(conn.dstBlockName);
					}
				}
			}
		}
	}

	std::set<CString> GetAllSuccessorNodes(CString const& blocksName) const
	{
		std::set<CString> successors;
		
		while (true)
		{
			bool bcontinue = false;
			for (const auto& conn : conns_)
			{
                if (conn.srcBlockName == blocksName 
                || std::find(successors.begin(), successors.end(), conn.srcBlockName) != successors.end())
				{
					if (std::find(successors.begin(), successors.end(), conn.dstBlockName) == successors.end())
					{
						successors.insert(conn.dstBlockName);
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

		ConnectResult(bool succ, CString const& err)
			:succeed(succ), error(err)
		{}
	};

	ConnectResult Connect(const CString& srcBlockName, const CString& srcBlockPortIdx, const CString& dstBlockName, const CString& dstBlockPortIdx)
	{
		if (srcBlockName == dstBlockName)
		{
			return ConnectResult( false, "Same block" );
		}
		
		auto type1 = blocks.at(srcBlockName).getRawObj().outs_.GetTypeByName(srcBlockPortIdx);
		auto type2 = blocks.at(dstBlockName).getRawObj().ins_.GetTypeByName(dstBlockPortIdx);
		if (type1 != type2)
		{
			return ConnectResult( false, "Diff type" );
		}
			
		// a destination cannot have more than one source
		//RemoveConnetionWithDestination(dstblockName, dstBlockPortIdx);
		auto isSame = [dstBlockName, dstBlockPortIdx](Connection const& c)->bool {
			return c.dstBlockName == dstBlockName && c.dstBlockPortIdx == dstBlockPortIdx;
			};
		auto it = std::find_if(conns_.begin(), conns_.end(), isSame);
		if (it != conns_.end())
			return ConnectResult( false, "Already have source connected" );

		// block֮�g������ѭ�h朽�
		auto successors = GetAllSuccessorNodes(dstBlockName);
		if (std::find(successors.begin(), successors.end(), srcBlockName) != successors.end())
		{
			return ConnectResult( false, "Circular Dependence" );
		}
		
		// insert connection
		conns_.emplace_back(Connection{ srcBlockName , srcBlockPortIdx , dstBlockName , dstBlockPortIdx });
		return ConnectResult( true, "" );
			
	}

	void Draw(CDC* pDC)
	{
		// blocks
		for (const auto& b:blocks)
		{
			b.second.Draw(pDC);
            auto center = b.second.CenterPoint();
            pDC->TextOut(center.x-5, center.y-5, b.first);
		}
		
		// connections
		for (int i=0; i<conns_.size(); i++)
		{
			const auto& conn = conns_[i];
			auto pt1 = blocks[conn.srcBlockName].GetOutPortCenter(conn.srcBlockPortIdx);
			auto pt2 = blocks[conn.dstBlockName].GetInPortCenter(conn.dstBlockPortIdx);

			auto type = blocks[conn.srcBlockName].getRawObj().outs_.GetTypeByName(conn.srcBlockPortIdx);
			auto numConn = MyMax(6, (int)conns_.size());
			double pitch = 0.5 / numConn;
			Line(pDC, type.c_str(), pt1, pt2, 0.5 + pitch * (-i + numConn * 0.5));
		}

	}


	HitTestResult HitTest(CPoint pt) const
	{
		for (const auto&b: reverse(blocks))
		{
			auto ret = b.second.HitTest(pt);

			if (ret.IsHit())
			{
				ret.blockName_ = b.first;
				return ret;
			}
		}
		return HitTestResult();
	}
	void DeleteInPort(CString const& blockName_, const CString& portIndx_)
	{
		RemoveConnetionWithDestination(blockName_, portIndx_);
		blocks.at(blockName_).DeleteInport(portIndx_);
	}

	void DeleteOutPort(CString const& blockName_, const CString& portIndx_)
	{
		RemoveConnetionWithSource(blockName_, portIndx_);
		blocks.at(blockName_).DeleteOutport(portIndx_);
	}

	void RemoveConnetionWithDestination(CString const& blockName_, const CString& portIndx_)
	{
		auto isSame = [blockName_, portIndx_](Connection const& c)->bool {
			return c.dstBlockName == blockName_ && c.dstBlockPortIdx == portIndx_;
			};
		for (auto it = conns_.begin(); it != conns_.end();)
		{
			if (isSame(*it))
				it = conns_.erase(it);
			else
				++it;
		}

	}

	void RemoveConnetionWithSource(CString const& blockName_, const CString& portIndx_)
	{
		auto isSame = [blockName_, portIndx_](Connection const& c)->bool {
			return c.srcBlockName == blockName_ && c.srcBlockPortIdx == portIndx_;
		};

		for (auto it=conns_.begin(); it!= conns_.end();)
		{
			if (isSame(*it))
				it = conns_.erase(it);
			else
				++it;
		}
		
	}

	void DeleteBlock(CString const& blockName)
	{
		// Delete related connections
		for (int i = conns_.size() - 1; i >= 0; i--)
		{
			if (conns_[i].dstBlockName == blockName || conns_[i].srcBlockName == blockName)
			{
				conns_.erase(conns_.begin() + i);
			}
		}
		// Delete related blocks
		blocks.erase(blockName);
	}

	void OnLButtonDblClk(CWnd* pWnd, CPoint pt)
	{
		HitTestResult hitTest = HitTest(pt);
		switch (hitTest.type_)
		{
		case HitTestResult::Type::InPort:
		{
			RemoveConnetionWithDestination(hitTest.blockName_, hitTest.portIndx_);
			pWnd->RedrawWindow();
			break;
		}
		case HitTestResult::Type::Block:
		{
			DeleteBlock(hitTest.blockName_);
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
			auto& b = blocks[hitTest.blockName_];
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
			
			auto& b = blocks[hitTest.blockName_];
			CPoint newPt = TrackLine(pWnd, pt);
			HitTestResult hitTest2 = HitTest(newPt);
			if (hitTest2.type_== HitTestResult::Type::InPort)
			{
				auto connSts = Connect(hitTest.blockName_, hitTest.portIndx_, hitTest2.blockName_, hitTest2.portIndx_);
				if (connSts.succeed)
				{
					pWnd->RedrawWindow();
				}
				else
				{
					CPoint ptScreen = newPt;
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
			break;
		}
		case HitTestResult::Type::OutPort:
		{
			auto const& b = blocks[hitTest.blockName_];
			const auto ps = b.getRawObj().outs_.GetByName(hitTest.portIndx_);
			text = ps.c_str();
			break;
		}
		case HitTestResult::Type::InPort:
		{
			auto const& b = blocks[hitTest.blockName_];
			const auto ps = b.getRawObj().ins_.GetByName(hitTest.portIndx_);
			text = ps.c_str();
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
	
	void ContextMenu(CWnd* pWnd, CPoint const& point)
	{
		CPoint ptScreen = point;
		pWnd->ClientToScreen(&ptScreen);
		HitTestResult hitTest = HitTest(point);
		
		switch (hitTest.type_)
		{
		case HitTestResult::Type::Block:
		{
			long id = PopupMenu(pWnd, IDR_MENU_BLOCK, ptScreen);
			switch(id)
			{
				case ID_BLOCK_DELETE:
				DeleteBlock(hitTest.blockName_);
				break;
				case ID_BLOCK_ADDTERMINAL:
					CMemberBrowserDlg dlg;
					dlg.UserFilter_ = &blocks.at(hitTest.blockName_).getRawObj();
					if (IDOK == dlg.DoModal())
					{
						switch (dlg.retType_)
						{
							case CMemberBrowserDlg::Input:
								blocks.at(hitTest.blockName_).AddInput(dlg.retPath_);
								break;
							case CMemberBrowserDlg::Output:
								blocks.at(hitTest.blockName_).AddOutput(dlg.retPath_);
								break;
						}
					}
				break;
			}
			break;
		}
		case HitTestResult::Type::InPort:
		{
			long id = PopupMenu(pWnd, IDR_MENU_PIN, ptScreen);
			switch(id)
			{
				case ID_PIN_DELETE:
				DeleteInPort(hitTest.blockName_, hitTest.portIndx_);
				break;
				case ID_PIN_DELETECONNECTION:
				RemoveConnetionWithDestination(hitTest.blockName_, hitTest.portIndx_);
				break;
			}

			break;
		}
		case HitTestResult::Type::OutPort:
		{
			long id = PopupMenu(pWnd, IDR_MENU_PIN, ptScreen);
			switch(id)
			{
				case ID_PIN_DELETE:
					DeleteOutPort(hitTest.blockName_, hitTest.portIndx_);
				break;
				case ID_PIN_DELETECONNECTION:
				RemoveConnetionWithSource(hitTest.blockName_, hitTest.portIndx_);
				break;
			}

			break;
		}
		
		default:
			break;
		}

		pWnd->RedrawWindow();
	}
};

std::unique_ptr<Test> g_test;

void Eng_Create()
{
	g_test.reset(new Test);
	g_MyToolTip.Create();
}

void Eng_Draw(CDC *pDC)
{
	g_test->Draw(pDC);
}

void Eng_OnLButtonDown(CWnd* pWnd, CPoint point)
{
	g_test->LBtnDown(g_MyToolTip, pWnd, point);
}

void Eng_OnLButtonDblClk(CWnd* pWnd, CPoint point)
{
	g_test->OnLButtonDblClk(pWnd, point);
}

void Eng_OnButtonRun()
{
	// TODO: �ڴ�����������������
	g_test->Invoke();
}

void Eng_AddBlock(const char* name)
{
	g_test->AddBlock(name);
}

void Eng_ContextMenu(CWnd* pWnd, CPoint const& point)
{
	g_test->ContextMenu(pWnd, point);
}

void Eng_OnMouseMove(CWnd* pWnd, CPoint point)
{
	g_test->ShowToolTip(g_MyToolTip, pWnd, point);
}
