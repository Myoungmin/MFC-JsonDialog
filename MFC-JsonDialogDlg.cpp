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

	SetWindowPos(nullptr, 0, 0, 1000, 600, SWP_NOMOVE | SWP_NOZORDER);

	// 1) 함수 테이블 등록
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
	std::ifstream ifs("ui_definition.json");
	if (!ifs.is_open()) {
		AfxMessageBox(_T("ui_definition.json 파일을 열 수 없습니다."));
		return;
	}
	json j; ifs >> j;

	CRect rc; GetClientRect(&rc);
	const int margin = 15;   // 바깥·안쪽 여백
	const int titleH = 20;   // 그룹 타이틀 높이
	const int rowH = 24;   // 컨트롤 높이
	const int vSpacing = 5;    // 행 간격
	const int hSpacing = 5;    // 열 간격

	// 첫 그룹 시작 X 좌표에 바깥 여백 추가
	int xOff = margin;

	for (auto& grp : j["groups"]) {
		// 그룹 너비: 전체 너비 * ratio  – (margin*2) → 좌우 여백 확보
		int grpW = int(rc.Width() * grp["widthSpec"]["value"].get<double>()) - margin * 2;
		int yOff = margin;  // 위쪽 여백

		// 그룹 전체 높이: 타이틀 + 각 행 높이 + 간격 + 아래 여백
		int totalRows = grp["rows"].size();
		int grpH = titleH + vSpacing
			+ totalRows * (rowH + vSpacing)
			+ margin;

		// 그룹 박스
		CString cTitle(grp["title"].get<std::string>().c_str());
		CButton* box = new CButton;
		box->Create(
			cTitle,
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			CRect(xOff, yOff, xOff + grpW + margin * 2, yOff + grpH),  // 그룹 너비에 좌우 margin*2 보정
			this,
			GetNextID()
		);

		// 내부 컨트롤 배치
		int y = yOff + titleH + vSpacing;
		for (auto& row : grp["rows"]) {
			int x = xOff + margin;  // 그룹 박스 안쪽 왼쪽 여백
			for (auto& ctrl : row["controls"]) {
				double wRatio = ctrl["widthSpec"]["value"].get<double>();
				int    cw = int(grpW * wRatio);
				CreateControl(ctrl, CRect(x, y, x + cw, y + rowH));
				x += cw + hSpacing;
			}
			y += rowH + vSpacing;
		}

		// 다음 그룹은 (이 그룹 너비 + 가로 바깥 여백*2 + 그룹 간격) 만큼 떨어뜨려서 배치
		xOff += grpW + margin * 2 + hSpacing;
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

