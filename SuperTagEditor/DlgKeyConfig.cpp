// DlgKeyConfig.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "supertageditor.h"
#include "DlgKeyConfig.h"

#include "Plugin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgKeyConfig ダイアログ
static	int		g_nLastSelectGroup = 0;
static	int		g_nLastSelectCommand = 0;

#define KEY_CODE_CTRL	0x0002
#define KEY_CODE_ALT	0x0004

struct	KEY_DATA	{
	TCHAR	*sKeyName;
	WORD	wKeyCode;
};

static	KEY_DATA	g_keyData[] = {
	{_T("(なし)"), 0x0000},
	{_T("F1"), VK_F1},
	{_T("F2"), VK_F2},
	{_T("F3"), VK_F3},
	{_T("F4"), VK_F4},
	{_T("F5"), VK_F5},
	{_T("F6"), VK_F6},
	{_T("F7"), VK_F7},
	{_T("F8"), VK_F8},
	{_T("F9"), VK_F9},
	{_T("F10"), VK_F10},
	{_T("F11"), VK_F11},
	{_T("F12"), VK_F12},
	{_T("Bksp"), VK_BACK},
	{_T("Ins"), VK_INSERT},
	{_T("Del"), VK_DELETE},
	{_T("Return"), VK_RETURN},
	{_T("Home"), VK_HOME},
	{_T("End"), VK_END},
	{_T("Space"), VK_SPACE},
//	{_T("Up"), VK_UP},
//	{_T("Down"), VK_DOWN},
//	{_T("Right"), VK_RIGHT},
//	{_T("Left"), VK_LEFT},
	{_T("1"), '1'},
	{_T("2"), '2'},
	{_T("3"), '3'},
	{_T("4"), '4'},
	{_T("5"), '5'},
	{_T("6"), '6'},
	{_T("7"), '7'},
	{_T("8"), '8'},
	{_T("9"), '9'},
	{_T("0"), '0'},
	{_T("A"), 'A'},
	{_T("B"), 'B'},
	{_T("C"), 'C'},
	{_T("D"), 'D'},
	{_T("E"), 'E'},
	{_T("F"), 'F'},
	{_T("G"), 'G'},
	{_T("H"), 'H'},
	{_T("I"), 'I'},
	{_T("J"), 'J'},
	{_T("K"), 'K'},
	{_T("L"), 'L'},
	{_T("M"), 'M'},
	{_T("N"), 'N'},
	{_T("O"), 'O'},
	{_T("P"), 'P'},
	{_T("Q"), 'Q'},
	{_T("R"), 'R'},
	{_T("S"), 'S'},
	{_T("T"), 'T'},
	{_T("U"), 'U'},
	{_T("V"), 'V'},
	{_T("W"), 'W'},
	{_T("X"), 'X'},
	{_T("Y"), 'Y'},
	{_T("Z"), 'Z'},
//	{_T("["), '['},
//	{_T("\\"), '\\'},
//	{_T("]"), ']'},
//	{_T("^"), '^'},
	{NULL, 0x0000},
};

IMPLEMENT_DYNCREATE(CDlgKeyConfig, COptionPage)


CDlgKeyConfig::CDlgKeyConfig() : COptionPage(CDlgKeyConfig::IDD)
{
	//{{AFX_DATA_INIT(CDlgKeyConfig)
	//}}AFX_DATA_INIT
}

CDlgKeyConfig::~CDlgKeyConfig()
{
}

void CDlgKeyConfig::DoDataExchange(CDataExchange* pDX)
{
	COptionPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgKeyConfig)
	DDX_Control(pDX, IDC_CH_ALT, m_btAlt);
	DDX_Control(pDX, IDC_CH_CTRL, m_btCtrl);
	DDX_Control(pDX, IDC_LIST_KEY, m_listKey);
	DDX_Control(pDX, IDC_LIST_GROUP, m_listGroup);
	DDX_Control(pDX, IDC_LIST_COMMAND, m_listCommand);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgKeyConfig, COptionPage)
	//{{AFX_MSG_MAP(CDlgKeyConfig)
	ON_BN_CLICKED(IDC_CH_CTRL, OnChCtrl)
	ON_BN_CLICKED(IDC_CH_ALT, OnChAlt)
	ON_LBN_SELCHANGE(IDC_LIST_COMMAND, OnSelChangeListCommand)
	ON_CBN_SELCHANGE(IDC_LIST_GROUP, OnSelChangeListGroup)
	ON_LBN_SELCHANGE(IDC_LIST_KEY, OnSelChangeListKey)
	ON_BN_DOUBLECLICKED(IDC_CH_CTRL, OnChCtrl)
	ON_BN_DOUBLECLICKED(IDC_CH_ALT, OnChAlt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgKeyConfig メッセージ ハンドラ
BOOL CDlgKeyConfig::OnInitDialog() 
{
	COptionPage::OnInitDialog();

	m_bExecCommandChange = false;

	// コマンドグループの作成
	static	TCHAR	*sGroupList[COMMAND_GROUP_MAX+1] = {
		_T("ファイル"), _T("編集"), _T("表示"), _T("変換")/* 2006.03.02 */, _T("プレイリスト"), _T("プレイヤー制御"), _T("プラグイン"), NULL
	};
	m_listGroup.ResetContent();
	for (int nGroup = 0; sGroupList[nGroup] != NULL; nGroup++) {
		m_listGroup.AddString(sGroupList[nGroup]);
	}
	m_listGroup.SetCurSel(g_nLastSelectGroup);
	UpdateCommandList();
	UpdateKeyList();
	m_listCommand.SetCurSel(g_nLastSelectCommand);
	OnSelChangeListCommand();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CDlgKeyConfig::UpdateHotKey(void)
{
	KEY_CONFIG *pKeyData;
	WORD	wCmdID = (WORD)m_listCommand.GetItemData(m_listCommand.GetCurSel());
	if (wCmdID != 0x0000 && (pKeyData = SearchKeyConfigID(wCmdID)) != NULL) {
		// ホットキーコードを取得＆更新
		WORD	wKeyCode, wModifiers;
		wKeyCode = (WORD)m_listKey.GetItemData(m_listKey.GetCurSel());
		wModifiers = (((wKeyCode < 0x30)   ? 0x0008 : 0x0000)
		           |  (m_btCtrl.GetCheck() ? 0x0002 : 0x0000)
		           |  (m_btAlt.GetCheck()  ? 0x0004 : 0x0000));
		pKeyData->dwKeyCode = wKeyCode | ((DWORD)wModifiers << 16);

		// 既に登録されていないかどうかをチェック
		if (wKeyCode != 0x0000) {
			int i; for (i = 0; g_listKeyConfig[i].sName != NULL; i++) {
				if (&g_listKeyConfig[i] != pKeyData
				&&  g_listKeyConfig[i].dwKeyCode == pKeyData->dwKeyCode) {
					CString	strBuffer;
					strBuffer.Format(_T("指定されたショートカットキーは、\n\n[%s]\n\nでも使用されています\n\nどちらかのショートカットを変更してください"), g_listKeyConfig[i].sName);
					MessageBox(strBuffer, _T("ショートカットキーの重複"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
					break;
				}
			}
			extern CPlugin plugins;
			for (i=0;i<plugins.arPluginKey.GetSize();i++) {
				KEY_CONFIG* pKey = (KEY_CONFIG*)plugins.arPluginKey.GetAt(i);
				if (pKey != pKeyData
				&&  pKey->dwKeyCode == pKeyData->dwKeyCode) {
					CString	strBuffer;
					strBuffer.Format(_T("指定されたショートカットキーは、\n\n[%s]\n\nでも使用されています\n\nどちらかのショートカットを変更してください"), pKey->sName);
					MessageBox(strBuffer, _T("ショートカットキーの重複"), MB_ICONSTOP|MB_OK|MB_TOPMOST);
					break;
				}
			}
		}
	}
}

void CDlgKeyConfig::UpdateKeyList(void)
{
	bool	bCtrl = m_btCtrl.GetCheck() ? true : false;
	bool	bAlt = m_btAlt.GetCheck() ? true : false;

	CString	strExt;
	if (bCtrl) strExt += _T("Ctrl+");
	if (bAlt) strExt += _T("Alt+");

	// 現在の選択を保存
	int		nOldSel = m_listKey.GetCurSel();

	// キー一覧の作成
	m_listKey.ResetContent();
	int i; for (i = 0; g_keyData[i].sKeyName != NULL; i++) {
		int		nIndex;
		CString	strName;
		WORD	wKeyCode = g_keyData[i].wKeyCode;

		if (bAlt && bCtrl && wKeyCode == VK_DELETE) {
			// Alt + Ctrl + Del は割り当て禁止
		} else {
			strName.Format(_T("%s%s"), strExt, g_keyData[i].sKeyName);
			nIndex = m_listKey.AddString(strName);
			// アイテムデータにキーコードを設定
			m_listKey.SetItemData(nIndex, wKeyCode);

			// Ctrl も Alt もチェックされていない場合は、DEL キーまでしか使えない
			if (bAlt == false && bCtrl == false) {
				if (wKeyCode == VK_DELETE) {
					break;
				}
			}
		}
	}

	// 選択しなおす
	m_listKey.SetCurSel((nOldSel >= 0 && nOldSel < i) ? nOldSel : 0);
}

void CDlgKeyConfig::UpdateCommandList(void)
{
	// コマンド一覧の作成
	m_listCommand.ResetContent();
	int i; for (i = 0; g_listKeyConfig[i].sName != NULL; i++) {
		KEY_CONFIG	*pKey = &g_listKeyConfig[i];
		if (pKey->nGroupID == m_listGroup.GetCurSel()) {
			int		nIndex;
			CString	strName = pKey->sName;
			switch(pKey->wCmdID) {
			case ID_CONV_FORMAT_EX_01:
				strName.Format(_T("01：%s"), g_userConvFormatEx[0].strName);
				break;
			case ID_CONV_FORMAT_EX_02:
				strName.Format(_T("02：%s"), g_userConvFormatEx[1].strName);
				break;
			case ID_CONV_FORMAT_EX_03:
				strName.Format(_T("03：%s"), g_userConvFormatEx[2].strName);
				break;
			case ID_CONV_FORMAT_EX_04:
				strName.Format(_T("04：%s"), g_userConvFormatEx[3].strName);
				break;
			case ID_CONV_FORMAT_EX_05:
				strName.Format(_T("05：%s"), g_userConvFormatEx[4].strName);
				break;
			case ID_CONV_FORMAT_EX_06:
				strName.Format(_T("06：%s"), g_userConvFormatEx[5].strName);
				break;
			case ID_CONV_FORMAT_EX_07:
				strName.Format(_T("07：%s"), g_userConvFormatEx[6].strName);
				break;
			case ID_CONV_FORMAT_EX_08:
				strName.Format(_T("08：%s"), g_userConvFormatEx[7].strName);
				break;
			case ID_CONV_FORMAT_EX_09:
				strName.Format(_T("09：%s"), g_userConvFormatEx[8].strName);
				break;
			case ID_CONV_FORMAT_EX_10:
				strName.Format(_T("10：%s"), g_userConvFormatEx[9].strName);
				break;
			case ID_CONV_FORMAT_USER_01: /* TyphoonSwell 027 */
				strName.Format(_T("01：%s"), g_userConvFormat[0].strName);
			case ID_CONV_FORMAT_USER_T2F_01: /* STEP 030 */
			case ID_CONV_FORMAT_USER_F2T_01: /* STEP 030 */
				strName.Format(_T("01：%s"), g_userConvFormat[0].strName);
				break;
			case ID_CONV_FORMAT_USER_02: /* TyphoonSwell 027 */
			case ID_CONV_FORMAT_USER_T2F_02: /* STEP 030 */
			case ID_CONV_FORMAT_USER_F2T_02: /* STEP 030 */
				strName.Format(_T("02：%s"), g_userConvFormat[1].strName);
				break;
			case ID_CONV_FORMAT_USER_03: /* TyphoonSwell 027 */
			case ID_CONV_FORMAT_USER_T2F_03: /* STEP 030 */
			case ID_CONV_FORMAT_USER_F2T_03: /* STEP 030 */
				strName.Format(_T("03：%s"), g_userConvFormat[2].strName);
				break;
			case ID_CONV_FORMAT_USER_04: /* LastTrain 057 */
			case ID_CONV_FORMAT_USER_T2F_04: /* STEP 030 */
			case ID_CONV_FORMAT_USER_F2T_04: /* STEP 030 */
				strName.Format(_T("04：%s"), g_userConvFormat[3].strName);
				break;
			case ID_CONV_FORMAT_USER_05: /* LastTrain 057 */
			case ID_CONV_FORMAT_USER_T2F_05: /* STEP 030 */
			case ID_CONV_FORMAT_USER_F2T_05: /* STEP 030 */
				strName.Format(_T("05：%s"), g_userConvFormat[4].strName);
				break;
			case ID_CONV_TAG_TO_TAG_01: /* STEP 034 */
				strName.Format(_T("01：%s"), g_userConvFormatTag2Tag[0].strName);
				break;
			case ID_CONV_TAG_TO_TAG_02: /* STEP 034 */
				strName.Format(_T("02：%s"), g_userConvFormatTag2Tag[1].strName);
				break;
			case ID_CONV_TAG_TO_TAG_03: /* STEP 034 */
				strName.Format(_T("03：%s"), g_userConvFormatTag2Tag[2].strName);
				break;
			case ID_CONV_TAG_TO_TAG_04: /* STEP 034 */
				strName.Format(_T("04：%s"), g_userConvFormatTag2Tag[3].strName);
				break;
			case ID_CONV_TAG_TO_TAG_05: /* STEP 034 */
				strName.Format(_T("05：%s"), g_userConvFormatTag2Tag[4].strName);
				break;
			case ID_TEIKEI_01: /* SeaKnows 030 */
				strName.Format(_T("01: %s"), shortString(g_teikeiInfo[0].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_02: /* SeaKnows 030 */
				strName.Format(_T("02: %s"), shortString(g_teikeiInfo[1].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_03: /* SeaKnows 030 */
				strName.Format(_T("03: %s"), shortString(g_teikeiInfo[2].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_04: /* SeaKnows 030 */
				strName.Format(_T("04: %s"), shortString(g_teikeiInfo[3].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_05: /* SeaKnows 030 */
				strName.Format(_T("05: %s"), shortString(g_teikeiInfo[4].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_06: /* SeaKnows 030 */
				strName.Format(_T("06: %s"), shortString(g_teikeiInfo[5].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_07: /* SeaKnows 030 */
				strName.Format(_T("07: %s"), shortString(g_teikeiInfo[6].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_08: /* SeaKnows 030 */
				strName.Format(_T("08: %s"), shortString(g_teikeiInfo[7].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_09: /* SeaKnows 030 */
				strName.Format(_T("09: %s"), shortString(g_teikeiInfo[8].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_10: /* SeaKnows 030 */
				strName.Format(_T("10: %s"), shortString(g_teikeiInfo[9].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_01: /* FreeFall 046 */
				strName.Format(_T("01: %s"), shortString(g_teikeiInfo[10].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_02: /* FreeFall 046 */
				strName.Format(_T("02: %s"), shortString(g_teikeiInfo[11].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_03: /* FreeFall 046 */
				strName.Format(_T("03: %s"), shortString(g_teikeiInfo[12].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_04: /* FreeFall 046 */
				strName.Format(_T("04: %s"), shortString(g_teikeiInfo[13].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_05: /* FreeFall 046 */
				strName.Format(_T("05: %s"), shortString(g_teikeiInfo[14].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_06: /* FreeFall 046 */
				strName.Format(_T("06: %s"), shortString(g_teikeiInfo[15].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_07: /* FreeFall 046 */
				strName.Format(_T("07: %s"), shortString(g_teikeiInfo[16].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_08: /* FreeFall 046 */
				strName.Format(_T("08: %s"), shortString(g_teikeiInfo[17].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_09: /* FreeFall 046 */
				strName.Format(_T("09: %s"), shortString(g_teikeiInfo[18].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_2_10: /* FreeFall 046 */
				strName.Format(_T("10: %s"), shortString(g_teikeiInfo[19].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_01: /* FreeFall 046 */
				strName.Format(_T("01: %s"), shortString(g_teikeiInfo[20].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_02: /* FreeFall 046 */
				strName.Format(_T("02: %s"), shortString(g_teikeiInfo[21].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_03: /* FreeFall 046 */
				strName.Format(_T("03: %s"), shortString(g_teikeiInfo[22].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_04: /* FreeFall 046 */
				strName.Format(_T("04: %s"), shortString(g_teikeiInfo[23].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_05: /* FreeFall 046 */
				strName.Format(_T("05: %s"), shortString(g_teikeiInfo[24].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_06: /* FreeFall 046 */
				strName.Format(_T("06: %s"), shortString(g_teikeiInfo[25].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_07: /* FreeFall 046 */
				strName.Format(_T("07: %s"), shortString(g_teikeiInfo[26].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_08: /* FreeFall 046 */
				strName.Format(_T("08: %s"), shortString(g_teikeiInfo[27].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_09: /* FreeFall 046 */
				strName.Format(_T("09: %s"), shortString(g_teikeiInfo[28].strTeikei, TEIKEI_MENU_STRING_MAX)); /* STEP 035, FreeFall 051 */
				break;
			case ID_TEIKEI_3_10: /* Nightmare */
				strName.Format(_T("10: %s"), shortString(g_teikeiInfo[29].strTeikei/* STEP 035 */, TEIKEI_MENU_STRING_MAX));
				break;
			}
			nIndex = m_listCommand.AddString(strName);

			// アイテムデータにコマンドＩＤを設定
			m_listCommand.SetItemData(nIndex, pKey->wCmdID);
		}
	}
	extern CPlugin plugins;
	for (i=0;i<plugins.arPluginKey.GetSize();i++) {
		KEY_CONFIG* pKey = (KEY_CONFIG*)plugins.arPluginKey.GetAt(i);
		if (pKey->nGroupID == m_listGroup.GetCurSel()) {
			int		nIndex;
			CString	strName = pKey->sName;
			nIndex = m_listCommand.AddString(strName);

			// アイテムデータにコマンドＩＤを設定
			m_listCommand.SetItemData(nIndex, pKey->wCmdID);
		}
	}
	m_listCommand.SetCurSel(0);
}

void CDlgKeyConfig::OnChCtrl() 
{
	if (m_bExecCommandChange == false) {
		UpdateHotKey();
		UpdateKeyList();
	}
}

void CDlgKeyConfig::OnChAlt() 
{
	if (m_bExecCommandChange == false) {
		UpdateHotKey();
		UpdateKeyList();
	}
}

void CDlgKeyConfig::OnSelChangeListKey() 
{
	if (m_bExecCommandChange == false) {
		UpdateHotKey();
	}
}

void CDlgKeyConfig::OnSelChangeListGroup() 
{
	g_nLastSelectGroup = m_listGroup.GetCurSel();
	UpdateCommandList();
	OnSelChangeListCommand();
}

void CDlgKeyConfig::OnSelChangeListCommand() 
{
	g_nLastSelectCommand = m_listGroup.GetCurSel();

	m_bExecCommandChange = true;

	KEY_CONFIG *pKeyData;
	WORD	wCmdID = (WORD)m_listCommand.GetItemData(m_listCommand.GetCurSel());
	if (wCmdID != 0x0000 && (pKeyData = SearchKeyConfigID(wCmdID)) != NULL) {
		// ホットキーコードを取得
		WORD	wKeyCode, wModifiers;
		wKeyCode = (WORD)pKeyData->dwKeyCode;
		wModifiers = (WORD)(pKeyData->dwKeyCode >> 16);

		if (wKeyCode != 0x0000) {
			// 特殊キーのチェック状態を設定
			BOOL	bCtrlOld = m_btCtrl.GetCheck();
			BOOL	bAltOld = m_btAlt.GetCheck();
			m_btCtrl.SetCheck((wModifiers & 0x0002) ? TRUE : FALSE);
			m_btAlt.SetCheck((wModifiers & 0x0004) ? TRUE : FALSE);
			if (bCtrlOld != m_btCtrl.GetCheck() || bAltOld != m_btAlt.GetCheck()) {
				// キー一覧を更新
				UpdateKeyList();
			}
		}

		// キーを選択状態にする
		int i; for (i = 0; g_keyData[i].sKeyName != NULL; i++) {
			if (wKeyCode == g_keyData[i].wKeyCode) {
				m_listKey.SetCurSel(i);
				break;
			}
		}
	}

	m_bExecCommandChange = false;
}
