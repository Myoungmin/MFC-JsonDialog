#include "pch.h"
#include "framework.h"
#include "MFC-JsonDialog.h"
#include "MFC-JsonDialogDlg.h"
#include "afxdialogex.h"
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int margin = 15;   // 바깥·안쪽 여백
const int titleH = 20;   // 그룹 타이틀 높이
const int rowH = 24;   // 컨트롤 높이
const int vSpacing = 5;    // 행 간격
const int hSpacing = 5;    // 열 간격

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
	m_namedHandlers[_T("onAction1")] = [this](UINT id) { OnAction(id); };
	m_namedHandlers[_T("onAction2")] = [this](UINT id) { OnAction(id); };
	m_namedHandlers[_T("onAction3")] = [this](UINT id) { OnAction(id); };

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
			it->second(id);
			return TRUE;
		}
	}
	return CDialogEx::OnCommand(wParam, lParam);
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
	

	// 첫 그룹 시작 X 좌표에 바깥 여백 추가
	int xOff = margin;

	for (auto& grp : j["groups"]) {
		int grpW = int((rc.Width() - margin * 3) * grp["widthSpec"]["value"].get<double>()) - margin * 2;
		int yOff = margin;  // 위쪽 여백

		int grpH = CalcGroupHeight(grp, rowH, titleH, vSpacing);

		// 그룹 박스
		CString cTitle(grp["title"].get<std::string>().c_str());
		CButton* box = new CButton;
		box->Create(
			cTitle,
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			CRect(xOff, yOff, xOff + grpW, yOff + grpH),
			this,
			GetNextID()
		);

		// 내부 컨트롤 배치
		int y = yOff + titleH + vSpacing;
		for (auto& row : grp["rows"]) {
			int x = xOff + margin;
			int maxRowH = rowH;
			for (auto& ctrl : row["controls"]) {
				int ctrlH = rowH;
				if (ctrl["type"] == "Group")
					ctrlH = CalcGroupHeight(ctrl, rowH, titleH, vSpacing);
				if (ctrlH > maxRowH)
					maxRowH = ctrlH;
			}
			std::vector<UINT> rowIDs;
			for (auto& ctrl : row["controls"]) {
				double wRatio = ctrl["widthSpec"]["value"].get<double>();
				int cw = int((grpW - margin * 2) * wRatio);
				UINT id = GetNextID();
				rowIDs.push_back(id);
				CreateControl(ctrl, CRect(x, y, x + cw, y + maxRowH), id, rowIDs);
				x += cw + hSpacing;
			}
			y += maxRowH + vSpacing;

			// Row 내 Button에 대해 Row 전체 ID 저장
			for (size_t i = 0; i < row["controls"].size(); ++i) {
				const auto& ctrl = row["controls"][i];
				if (ctrl["type"] == "Button") {
					UINT btnID = rowIDs[i];
					m_rowMap[btnID] = rowIDs;
				}
			}
		}

		xOff += grpW + hSpacing;
	}
}

void CMFCJsonDialogDlg::CreateControl(const json& ctrl, const CRect& rc, UINT id, std::vector<UINT>& rowIDs)
{
	CString jsonID(ctrl["id"].get<std::string>().c_str());
	m_idMap[jsonID] = id;

	CString type(ctrl["type"].get<std::string>().c_str());

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
	else if (type == _T("Check")) {
		std::string labelStr = ctrl["label"].get<std::string>();
		CString label(labelStr.c_str());
		CButton* chk = new CButton;
		chk->Create(
			label,
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			rc, this, id
		);
		if (ctrl.contains("default") && ctrl["default"].get<bool>())
			chk->SetCheck(BST_CHECKED);
	}
	else if (type == _T("Group")) {
		// 중첩 그룹의 실제 높이 계산
		int groupH = CalcGroupHeight(ctrl, rowH, titleH, vSpacing);
		int groupW = rc.Width();

		// 그룹 박스 그리기
		CString gTitle(ctrl["title"].get<std::string>().c_str());
		CButton* box = new CButton;
		box->Create(
			gTitle,
			WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			rc,
			this,
			id
		);

		int innerW = rc.Width() - margin * 2;
		int y = rc.top + titleH + vSpacing;

		for (auto& row : ctrl["rows"]) {
			int maxRowH = rowH;
			for (auto& c : row["controls"]) {
				int ctrlH = rowH;
				if (c["type"] == "Group")
					ctrlH = CalcGroupHeight(c, rowH, titleH, vSpacing);
				if (ctrlH > maxRowH)
					maxRowH = ctrlH;
			}
			int x = rc.left + margin;
			std::vector<UINT> rowIDs;
			for (auto& c : row["controls"]) {
				double wRatio = c["widthSpec"]["value"].get<double>();
				int cw = int(innerW * wRatio);
				UINT childID = GetNextID();
				rowIDs.push_back(childID);
				CreateControl(c, CRect(x, y, x + cw, y + maxRowH), childID, rowIDs);
				x += cw + hSpacing;
			}
			// Row 내 Button에 대해 Row 전체 ID 저장
			for (size_t i = 0; i < row["controls"].size(); ++i) {
				const auto& c = row["controls"][i];
				if (c["type"] == "Button") {
					UINT btnID = rowIDs[i];
					m_rowMap[btnID] = rowIDs;
				}
			}
			y += maxRowH + vSpacing;
		}
	}
}

int CMFCJsonDialogDlg::CalcGroupHeight(const json& group, int rowH, int titleH, int vSpacing)
{
	int totalH = titleH + vSpacing;
	for (auto& row : group["rows"]) {
		int maxRowH = rowH;
		for (auto& ctrl : row["controls"]) {
			if (ctrl["type"] == "Group") {
				int groupH = CalcGroupHeight(ctrl, rowH, titleH, vSpacing);
				if (groupH > maxRowH)
					maxRowH = groupH;
			}
		}
		totalH += maxRowH + vSpacing;
	}
	return totalH;
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

void CMFCJsonDialogDlg::OnAction(UINT btnID)
{
	CString msg;
	auto it = m_rowMap.find(btnID);
	if (it != m_rowMap.end()) {
		for (UINT cid : it->second) {
			CWnd* pWnd = GetDlgItem(cid);
			if (!pWnd) continue;

			if (auto pEdit = dynamic_cast<CEdit*>(pWnd)) {
				CString text; pEdit->GetWindowText(text);
				msg += _T("Edit: ") + text + _T("\n");
			}
			else if (auto pCombo = dynamic_cast<CComboBox*>(pWnd)) {
				int sel = pCombo->GetCurSel();
				if (sel != CB_ERR) {
					CString selText;
					pCombo->GetLBText(sel, selText);
					msg += _T("Combo: ") + selText + _T("\n");
				}
			}
			else if (auto pBtn = dynamic_cast<CButton*>(pWnd)) {
				// 체크박스만
				if ((pBtn->GetStyle() & BS_CHECKBOX) == BS_CHECKBOX) {
					int state = pBtn->GetCheck();
					msg += _T("Check: ") + CString(state ? _T("Checked") : _T("Unchecked")) + _T("\n");
				}
			}
		}
	}
	if (msg.IsEmpty())
		msg = _T("Row 내에 값이 없습니다.");

	for (const auto& pair : m_idMap) {
		if (pair.second == btnID) {
			CString jsonID = pair.first;
			if (jsonID == _T("IDC_BUTTON_ACTION1"))
				msg += _T("\n[Action1] 버튼이 클릭되었습니다.");
			else if (jsonID == _T("IDC_BUTTON_ACTION2"))
				msg += _T("\n[Action2] 버튼이 클릭되었습니다.");
			else if (jsonID == _T("IDC_BUTTON_ACTION3"))
				msg += _T("\n[Action3] 버튼이 클릭되었습니다.");
			break;
		}
	}

	AfxMessageBox(msg);
}