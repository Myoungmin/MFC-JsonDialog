#include "pch.h"
#include "framework.h"
#include "MFC-JsonDialog.h"
#include "MFC-JsonDialogDlg.h"
#include "afxdialogex.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMFCJsonDialogDlg::CMFCJsonDialogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCJSONDIALOG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCJsonDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCJsonDialogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CMFCJsonDialogDlg message handlers

BOOL CMFCJsonDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 1) 문자열→함수 테이블 등록
	m_namedHandlers[_T("onAction1")] = [this]() { OnAction1(); };
	m_namedHandlers[_T("onAction2")] = [this]() { OnAction2(); };

	// 2) JSON 파싱 및 UI 생성
	LoadAndCreateUI();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CMFCJsonDialogDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT id = LOWORD(wParam);
	UINT code = HIWORD(wParam);
	if (code == BN_CLICKED) {
		auto it = m_evtMap.find(id);
		if (it != m_evtMap.end()) {
			it->second();
			return TRUE;
		}
	}
	return CDialogEx::OnCommand(wParam, 0);
}

void CMFCJsonDialogDlg::LoadAndCreateUI()
{
	std::ifstream ifs(_T("ui_definition.json"));
	json j; ifs >> j;

	CRect rc; GetClientRect(&rc);
	int xOff = 0;

	for (auto& grp : j["groups"]) {
		int grpW = int(rc.Width() * grp["widthSpec"]["value"].get<double>());
		int y = 10;

		// 그룹 타이틀
		CStatic* title = new CStatic;
		UINT tid = GetNextID();
		std::string groupTitleStr = grp["title"].get<std::string>();
		CString groupTitle(groupTitleStr.c_str());
		title->Create(groupTitle, WS_CHILD | WS_VISIBLE,
			CRect(xOff, y, xOff + grpW, y + 20), this, tid);
		y += 30;

		// 각 행(Row)
		for (auto& row : grp["rows"]) {
			int x = xOff;
			for (auto& ctrl : row["controls"]) {
				int cw = int(grpW * ctrl["widthSpec"]["value"].get<double>());
				CreateControl(ctrl, CRect(x, y, x + cw, y + 24));
				x += cw + 5;
			}
			y += 29;
		}
		xOff += grpW + 10;
	}
}

void CMFCJsonDialogDlg::CreateControl(const json& ctrl, const CRect& rc)
{
	CString type(ctrl["type"].get<std::string>().c_str());
	UINT    id = GetNextID();

	if (type == _T("Static")) {
		std::string labelStr = ctrl["label"].get<std::string>();
		CString label(labelStr.c_str());
		CStatic* s = new CStatic;
		s->Create(label, WS_CHILD | WS_VISIBLE, rc, this, id);
	}
	else if (type == _T("Edit")) {
		CEdit* e = new CEdit;
		e->Create(WS_CHILD | WS_VISIBLE | WS_BORDER, rc, this, id);
		std::string defStr = ctrl.value("default", std::string(""));
		CString def(defStr.c_str());
		e->SetWindowText(def);
	}
	else if (type == _T("Combo")) {
		CComboBox* cb = new CComboBox;
		cb->Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, rc, this, id);
		for (auto& opt : ctrl["options"]) {
			std::string optStr = opt.get<std::string>();
			CString optCStr(optStr.c_str());
			cb->AddString(optCStr);
		}
		std::string defStr = ctrl.value("default", std::string(""));
		CString def(defStr.c_str());
		cb->SetCurSel(cb->FindStringExact(-1, def));
	}
	else if (type == _T("Button")) {
		std::string labelStr = ctrl["label"].get<std::string>();
		CString label(labelStr.c_str());
		CButton* b = new CButton;
		b->Create(label, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, this, id);
		std::string evtStr = ctrl.value("onClick", std::string(""));
		CString evt(evtStr.c_str());
		auto it = m_namedHandlers.find(evt);
		if (it != m_namedHandlers.end())
			m_evtMap[id] = it->second;
	}

}

UINT CMFCJsonDialogDlg::GetNextID()
{
	// 보통 리소스 ID는 1~32767 범위를 씁니다.
	// 10000~65535 중에서 사용되지 않은 ID를 찾음
	for (UINT id = 10000; id < 65535; ++id) {
		if (GetDlgItem(id) == nullptr) {
			return id;
		}
	}
	// 만약 못 찾으면 예외처리 필요
	AfxMessageBox(_T("사용 가능한 ID가 없습니다."));
	return (UINT)-1;
}

