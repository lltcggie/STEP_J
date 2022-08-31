#pragma once


// CDlgSetup

class CDlgSetup : public CPropertyPage
{
	DECLARE_DYNAMIC(CDlgSetup)

public:
	CDlgSetup();
	virtual ~CDlgSetup();

	enum { IDD = IDD_DLG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

protected:
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bGenreListSelect;
	int m_ddxnFormatType;
	afx_msg void OnBnClickedBtResetPage();
};


