// DlgSetup.cpp : 実装ファイル
//

#include "stdafx.h"
#include "STEP_dsf.h"
#include "DlgSetup.h"


// CDlgSetup

IMPLEMENT_DYNAMIC(CDlgSetup, CPropertyPage)

CDlgSetup::CDlgSetup()
    : CPropertyPage(CDlgSetup::IDD)
    ,m_bGenreListSelect(FALSE)
    , m_ddxnFormatType(0)
{
}

CDlgSetup::~CDlgSetup()
{
}

void CDlgSetup::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CH_GENRE_LIST_SELECT, m_bGenreListSelect);
    DDX_Radio(pDX, IDC_RADIO_FORMATTYPE1, m_ddxnFormatType);
}

BEGIN_MESSAGE_MAP(CDlgSetup, CPropertyPage)
    ON_BN_CLICKED(IDC_BT_RESET_PAGE, &CDlgSetup::OnBnClickedBtResetPage)
END_MESSAGE_MAP()



// CDlgSetup メッセージ ハンドラー
void CDlgSetup::OnBnClickedBtResetPage()
{
    // TODO: ここにコントロール通知ハンドラー コードを追加します。
    extern const int nAudioFormatDispType_default;
    extern const bool bOptID3v2GenreListSelect_default;

    ((CButton *)GetDlgItem(IDC_RADIO_FORMATTYPE1))->SetCheck(nAudioFormatDispType_default);
    ((CButton *)GetDlgItem(IDC_CH_GENRE_LIST_SELECT))->SetCheck(bOptID3v2GenreListSelect_default);

    CheckRadioButton(IDC_RADIO_FORMATTYPE1, IDC_RADIO_FORMATTYPE4, IDC_RADIO_FORMATTYPE1);

    UpdateData();
}
