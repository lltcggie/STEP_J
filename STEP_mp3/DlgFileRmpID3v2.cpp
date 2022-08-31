// DlgFileRmpID3v2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "STEP_mp3.h"
#include "DlgFileRmpID3v2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFileRmpID3v2 プロパティ ページ

IMPLEMENT_DYNCREATE(CDlgFileRmpID3v2, CPropertyPage)

CDlgFileRmpID3v2::CDlgFileRmpID3v2() : CPropertyPage(CDlgFileRmpID3v2::IDD)
{
    //{{AFX_DATA_INIT(CDlgFileRmpID3v2)
    m_bRmpID3tagAutoWrite = FALSE;
    m_bAutoConvRMP = FALSE;
    m_nSIFieldConvertType = 0;
    m_bRmpGenreListSelect = FALSE;
    m_bChangeFileExt = FALSE;
    m_bAutoConvID3v2 = FALSE;
    m_bID3v2ID3tagAutoWrite = FALSE;
    m_bID3v2GenreListSelect = FALSE;
    m_bID3v2GenreAddNumber = FALSE;
    m_bID3v2Id3tagAutoDelete = FALSE;
    //}}AFX_DATA_INIT
}

CDlgFileRmpID3v2::~CDlgFileRmpID3v2()
{
}

void CDlgFileRmpID3v2::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgFileRmpID3v2)
    DDX_Check(pDX, IDC_CH_RMP_ID3TAG_AUTO_WRITE, m_bRmpID3tagAutoWrite);
    DDX_Check(pDX, IDC_CH_AUTO_CONV_RMP, m_bAutoConvRMP);
    DDX_Radio(pDX, IDC_RA_SIF_CONV_ALL, m_nSIFieldConvertType);
    DDX_Check(pDX, IDC_CH_RMP_GENRE_LIST_SELECT, m_bRmpGenreListSelect);
    DDX_Check(pDX, IDC_CH_CHANGE_FILE_EXT, m_bChangeFileExt);
    DDX_Check(pDX, IDC_CH_AUTO_CONV_ID3V2, m_bAutoConvID3v2);
    DDX_Check(pDX, IDC_CH_ID3V2_ID3TAG_AUTO_WRITE, m_bID3v2ID3tagAutoWrite);
    DDX_Check(pDX, IDC_CH_ID3V2_GENRE_LIST_SELECT, m_bID3v2GenreListSelect);
    DDX_Check(pDX, IDC_CH_ID3V2_GENRE_ADD_NUMBER, m_bID3v2GenreAddNumber);
    DDX_Check(pDX, IDC_CH_ID3V2_ID3TAG_AUTO_DELETE, m_bID3v2Id3tagAutoDelete);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFileRmpID3v2, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgFileRmpID3v2)
    ON_BN_CLICKED(IDC_BT_RESET_PAGE, OnBtResetPage)
    ON_BN_CLICKED(IDC_CH_ID3V2_ID3TAG_AUTO_DELETE, OnChId3v2Id3tagAutoDelete)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFileRmpID3v2 メッセージ ハンドラ
BOOL CDlgFileRmpID3v2::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    // TODO: この位置に初期化の補足処理を追加してください
    UpdateStatus();

    return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
                  // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDlgFileRmpID3v2::UpdateStatus(void)
{
    UpdateData();
    ((CButton *)GetDlgItem(IDC_CH_ID3V2_ID3TAG_AUTO_WRITE))->EnableWindow(!m_bID3v2Id3tagAutoDelete);
}

// 初期値に戻す
void CDlgFileRmpID3v2::OnBtResetPage()
{
    extern const bool bOptAutoConvID3v2_default;
    //extern const bool bOptAutoConvRMP_default;
    extern const int  nOptSIFieldConvType_default;
    extern const bool bOptID3v2ID3tagAutoWrite_default;
    extern const bool bOptRmpID3tagAutoWrite_default;
    extern const bool bOptID3v2GenreAddNumber_default;
    extern const bool bOptChangeFileExt_default;
    extern const bool bOptID3v2GenreListSelect_default;
    extern const bool bOptRmpGenreListSelect_default;
    extern const bool bOptID3v2ID3tagAutoDelete_default;
    extern const TCHAR strOptSoftwareteTag_default[];
    extern const bool bOptNotUnSyncAlways_default;
    extern const bool bOptUnSyncNew_default;
    extern const int  nId3v2Encode_default; 
    extern const int  nId3v2Version_default;
    extern const int  nId3v2EncodeNew_default;
    extern const int  nId3v2VersionNew_default;    
    // RIFF MP3
    ((CButton *)GetDlgItem(IDC_CH_CHANGE_FILE_EXT))->SetCheck(bOptChangeFileExt_default);
    //((CButton *)GetDlgItem(IDC_CH_AUTO_CONV_RMP))->SetCheck(bOptAutoConvRMP_default);
    ((CButton *)GetDlgItem(IDC_CH_RMP_ID3TAG_AUTO_WRITE))->SetCheck(bOptRmpID3tagAutoWrite_default);
    ((CButton *)GetDlgItem(IDC_CH_RMP_GENRE_LIST_SELECT))->SetCheck(bOptRmpGenreListSelect_default);
    ((CButton *)GetDlgItem(IDC_CH_AUTO_CONV_ID3V2))->SetCheck(bOptAutoConvID3v2_default);
    ((CButton *)GetDlgItem(IDC_CH_ID3V2_ID3TAG_AUTO_WRITE))->SetCheck(bOptID3v2ID3tagAutoWrite_default);
    ((CButton *)GetDlgItem(IDC_CH_ID3V2_ID3TAG_AUTO_DELETE))->SetCheck(bOptID3v2ID3tagAutoDelete_default);
    ((CButton *)GetDlgItem(IDC_CH_ID3V2_GENRE_LIST_SELECT))->SetCheck(bOptID3v2GenreListSelect_default);
    ((CButton *)GetDlgItem(IDC_CH_ID3V2_GENRE_ADD_NUMBER))->SetCheck(bOptID3v2GenreAddNumber_default);

    CheckRadioButton(IDC_RA_SIF_CONV_ALL, IDC_RA_SIF_CONV_LENGTH, IDC_RA_SIF_CONV_ALL);

    UpdateStatus();
}

void CDlgFileRmpID3v2::OnChId3v2Id3tagAutoDelete()
{
    // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
    //UpdateData();
    //if (m_bID3v2Id3tagAutoDelete) {
        //((CButton *)GetDlgItem(IDC_CH_ID3V2_ID3TAG_AUTO_WRITE))->SetCheck(FALSE);
    //}
    UpdateStatus();
}
