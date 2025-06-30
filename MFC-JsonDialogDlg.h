#pragma once
#include <map>
#include <functional>
#include "json.hpp"
using json = nlohmann::json;


// CMFCJsonDialogDlg dialog
class CMFCJsonDialogDlg : public CDialogEx
{
// Construction
public:
	CMFCJsonDialogDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCJSONDIALOG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	std::map<CString, std::function<void()>> m_namedHandlers;
	std::map<UINT, std::function<void()>> m_evtMap;

	void LoadAndCreateUI();
	void CreateControl(const json& ctrl, const CRect& rc);
	UINT GetNextID();

	// 실제 핸들러
	void OnAction1() { AfxMessageBox(_T("Action1 호출")); }
	void OnAction2() { AfxMessageBox(_T("Action2 호출")); }
};
