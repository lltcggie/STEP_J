#pragma once


// CDlgSetupOther

class CDlgSetupOther : public CPropertyPage
{
    DECLARE_DYNAMIC(CDlgSetupOther)

public:
    CDlgSetupOther();
    virtual ~CDlgSetupOther();

    enum { IDD = IDD_DLG_SETUP_OTHER};
    CComboBox m_cbId3v2EncodeNew;
    CComboBox m_cbId3v2VersionNew;
    CComboBox m_cbId3v2Encode;
    CComboBox m_cbId3v2Version;
    int m_nId3v2Encode;
    int m_nId3v2Version;
    int m_nId3v2VersionNew;
    int m_nId3v2EncodeNew;

protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
    afx_msg void OnSelchangeComboId3v2Version();
    afx_msg void OnSelchangeComboId3v2VersionNew();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtResetPage();
};


