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
	~CMFCJsonDialogDlg();

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
	std::vector<CWnd*> m_dynCtrls;

	std::map<CString, std::function<void(UINT)>> m_namedHandlers;
	std::map<UINT, std::function<void(UINT)>> m_evtMap;

	std::map<UINT, std::vector<UINT>> m_rowMap;
	std::map<CString, UINT> m_idMap;

	void LoadAndCreateUI();
	void CreateControl(const json& ctrl, const CRect& rc, UINT id, std::vector<UINT>& rowIDs);
	int CalcGroupHeight(const json& group, int rowH, int titleH, int vSpacing);
	UINT GetNextID();

	// 실제 핸들러
	void OnAction(UINT btnID);
	void OnCalc(UINT btnID);

	std::wstring UTF8ToWide(const std::string& utf8);
};
