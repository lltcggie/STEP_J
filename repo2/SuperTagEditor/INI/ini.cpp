#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Shlwapi.h"
#include "ini.h"
#include "..\strcnv.h"

static void __fastcall strtrim(TCHAR *psz)
{
    StrTrim(psz, _T("\r\n\t"));//半角スペースはトリムしない(値に使えなくなるため)
}
static BOOL __fastcall IsSection(TCHAR *pszLine,
                                 TCHAR **ppszSection);
static BOOL __fastcall IsKeyValue(TCHAR *pszLine,
                                  TCHAR **ppszKey,
                                  TCHAR **ppszValue);
///////////////////////////////////////////////////////////////////////////////
static BOOL __fastcall IsSection(TCHAR *pszLine,
                                 TCHAR **ppszSection)
{ // pszLine が
  // [SectionName]
  // 形式の行なら ppszSection にセクション名を入れて TRUE を返す
    if (*pszLine != _T('[')) {
        *ppszSection = NULL;
        return FALSE;
    }
    *ppszSection = ++pszLine;
    while (*pszLine) {
        if (*pszLine == _T(']') && !*(pszLine + 1)) {
            *pszLine = 0;
            strtrim(*ppszSection);
            return TRUE;
        }
        pszLine++;
    }
    *ppszSection = NULL;
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
static BOOL __fastcall IsKeyValue(TCHAR *pszLine,
                                  TCHAR **ppszKey,
                                  TCHAR **ppszValue)
{ // pszLine が
  // KeyName=Value
  // 形式の行なら *ppszKey にキー名、*ppszValue に値を入れて TRUE を返す
  // コメントを除去(; 以降を捨てる)
  //TCHAR *pszComment = _tcschr(pszLine, _T(';'));
  //if (pszComment) {
  //    *pszComment = 0;
  //}
  //↑STEP では値に ';' を使う可能性があるのでコメントは使用不可とする
  //きちんとやるなら "" で括られた範囲内の ; はコメントと見なさないような
  //処理が必要で少し面倒なので省略
    *ppszKey = _tcstok_s(pszLine, _T("="), ppszValue);
    if (!*ppszKey || !*ppszValue) {
        *ppszKey = *ppszValue = NULL;
        return FALSE;
    }
    if(_tcschr(*ppszKey, _T(';'))){//キー名に ; を含む場合はコメント行と見なす
        *ppszKey = *ppszValue = NULL;
        return FALSE;
    }
    strtrim(*ppszKey);
    if (**ppszKey) {
        //値が "" で括られていれば "" を取り除く
        TCHAR *pszValue = *ppszValue;
        strtrim(pszValue);
        if(*pszValue == _T('\"')){
            TCHAR *last_dbl_quote = _tcsrchr(pszValue+1, _T('\"'));
            if(last_dbl_quote && !last_dbl_quote[1]){//終端文字が "
                *last_dbl_quote = 0;
            }
            strtrim(++pszValue);
            *ppszValue = pszValue;
        }
        return TRUE;
    }
    *ppszKey = *ppszValue = NULL;
    return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
CIniKeyValue::CIniKeyValue(const TCHAR *cszKeyName,
                           const TCHAR *cszValue)
{ // Key と Value はメモリを共有する
    const size_t nKeyLen = _tcslen(cszKeyName) + 1;
    const size_t nValLen = _tcslen(cszValue) + 1;
    m_pszKey = (TCHAR*)malloc((nKeyLen + nValLen)*sizeof(TCHAR));
    m_pszValue = m_pszKey + nKeyLen;
    memcpy(m_pszKey, cszKeyName, nKeyLen*sizeof(TCHAR));
    memcpy(m_pszValue, cszValue, nValLen*sizeof(TCHAR));
}
///////////////////////////////////////////////////////////////////////////////
CIniKeyValue::~CIniKeyValue(void)
{
    free(m_pszKey);
    // free(m_pszValue);//m_pszKey と共有しているので不要
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniKeyValue::SetValue(const TCHAR *cszValue) {
    const size_t nOldValLen = _tcslen(m_pszValue) + 1;
    const size_t nNewValLen = _tcslen(cszValue) + 1;
    if (nNewValLen > nOldValLen) {
        const size_t nKeyLen = m_pszValue - m_pszKey;
        m_pszKey = (TCHAR*)realloc(m_pszKey,
            (nKeyLen + nNewValLen)*sizeof(TCHAR));
        m_pszValue = m_pszKey + nKeyLen;
    }
    memcpy(m_pszValue, cszValue, nNewValLen*sizeof(TCHAR));
}
///////////////////////////////////////////////////////////////////////////////
//CIniSection
///////////////////////////////////////////////////////////////////////////////
CIniSection::CIniSection(const TCHAR *cszSectionName)
{
    m_pszSection = _tcsdup(cszSectionName);
    m_ppKeyValues = NULL;
    m_nCapacity = m_nCount = 0;
}
///////////////////////////////////////////////////////////////////////////////
CIniSection::~CIniSection(void)
{
    free(m_pszSection);
    const int nCount = m_nCount;
    int i;
    for (i = 0; i < nCount; i++) {
        delete m_ppKeyValues[i];
    }
    if (m_ppKeyValues) {
        free(m_ppKeyValues);
    }
}
///////////////////////////////////////////////////////////////////////////////
CIniKeyValue* __fastcall CIniSection::FindKeyValue(const TCHAR *cszKey)
{
    const int nCount = m_nCount;
    int i;
    for (i = 0; i < nCount; i++) {
        if (_tcsicmp(m_ppKeyValues[i]->GetKeyName(), cszKey) == 0) {
            return m_ppKeyValues[i];
        }
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CIniKeyValue* __fastcall CIniSection::GetKeyValue(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_nCount) {
        return NULL;
    }
    return m_ppKeyValues[nIndex];
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniSection::AddKeyValue(const TCHAR *cszKey,
                                         const TCHAR *cszValue)
{
    CIniKeyValue *pKeyValue = FindKeyValue(cszKey);
    if (pKeyValue) {
        pKeyValue->SetValue(cszValue);
        return;
    }
    if (m_nCount >= m_nCapacity) {
        if (!m_nCapacity) {
            m_nCapacity = 16;
        }
        else {
            m_nCapacity *= 2;
        }
        m_ppKeyValues = (CIniKeyValue * *)realloc(m_ppKeyValues,
            sizeof(CIniKeyValue*)*m_nCapacity);
    }
    pKeyValue = new CIniKeyValue(cszKey, cszValue);
    m_ppKeyValues[m_nCount++] = pKeyValue;
}
///////////////////////////////////////////////////////////////////////////////
//CIniFile
///////////////////////////////////////////////////////////////////////////////
CIniFile::CIniFile(void)
{
    m_szFileName[0] = 0;
    m_ppSections = NULL;
    m_nCount = m_nCapacity = 0;
}
///////////////////////////////////////////////////////////////////////////////
CIniFile::CIniFile(const TCHAR *cszFileName)
{
    m_szFileName[0] = 0;
    m_ppSections = NULL;
    m_nCount = m_nCapacity = 0;
    Open(cszFileName);
}
///////////////////////////////////////////////////////////////////////////////
CIniFile::~CIniFile(void)
{
    Close();
}
///////////////////////////////////////////////////////////////////////////////
CIniSection* __fastcall CIniFile::GetIniSection(int nIndex)
{
    if (nIndex < 0 || nIndex >= m_nCount) {
        return NULL;
    }
    return m_ppSections[nIndex];
}
///////////////////////////////////////////////////////////////////////////////
int __fastcall CIniFile::ReadInt(const TCHAR *cszSection,
                                 const TCHAR *cszKey,
                                 int nDefault)
{
    CIniSection *pSection = FindSection(cszSection, FALSE);
    int nRet = nDefault;
    if (pSection) {
        CIniKeyValue *pKeyValue = pSection->FindKeyValue(cszKey);
        const TCHAR *pszValue = pKeyValue ? pKeyValue->GetValue() : NULL;
        if(pszValue && *pszValue){
            TCHAR *end;
            nRet = _tcstol(pszValue, &end, 10);
            if(end && end[0]){
                nRet = nDefault;
            }
        }
    }
    return nRet;
}
///////////////////////////////////////////////////////////////////////////////
TCHAR* __fastcall CIniFile::ReadStr(const TCHAR *cszSection,
                                    const TCHAR *cszKey,
                                    const TCHAR *cszDefault,
                                    TCHAR *pszStr,
                                    int    nSize)
{
    static const TCHAR cszEmpty[] = _T("");
    CIniSection *pSection = FindSection(cszSection, FALSE);
    if (pSection) {
        CIniKeyValue *pKeyValue = pSection->FindKeyValue(cszKey);
        if (pKeyValue) {
            _tcsncpy_s(pszStr, nSize, pKeyValue->GetValue(), _TRUNCATE);
            return pszStr;
        }
    }
    if(!cszDefault){
        cszDefault = cszEmpty;
    }
    _tcsncpy_s(pszStr, nSize, cszDefault, _TRUNCATE);
    return pszStr;
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniFile::WriteInt(const TCHAR *cszSection,
                                   const TCHAR *cszKey,
                                   int nValue)
{
    CIniSection *pSection = FindSection(cszSection, TRUE);
    TCHAR szValue[128];
    _sntprintf_s(szValue, _TRUNCATE, _T("%d"), nValue);
    pSection->AddKeyValue(cszKey, szValue);
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniFile::WriteStr(const TCHAR *cszSectionName,
                                   const TCHAR *cszKey,
                                   const TCHAR *cszValue)
{
    CIniSection *pSection = FindSection(cszSectionName, TRUE);
    pSection->AddKeyValue(cszKey, cszValue);
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniFile::Flush(void)
{
    if(!m_szFileName[0]){
        return;
    }
    HANDLE hFile = CreateFile(m_szFileName,
                              GENERIC_WRITE|GENERIC_READ,
                              FILE_SHARE_WRITE,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        return ;
    }
    const int nCount = m_nCount;
    int i, j;
    DWORD dwWritten;
#ifdef _UNICODE
    // BOM 書き込み
    WriteFile(hFile, "\xFF\xFE", 2, &dwWritten, NULL);
#endif
    for(i = 0; i < nCount; i++){
        CIniSection *pSection = m_ppSections[i];
        const TCHAR *cszSectionName = pSection->GetSectionName();
        WriteFile(hFile, _T("["), sizeof(TCHAR), &dwWritten, NULL);
        WriteFile(hFile, cszSectionName, _tcslen(cszSectionName)*sizeof(TCHAR), &dwWritten, NULL);
        WriteFile(hFile, _T("]\r\n"), 3*sizeof(TCHAR), &dwWritten, NULL);
        j = 0;
        while(1){
            CIniKeyValue *pKeyValue = pSection->GetKeyValue(j++);
            if(!pKeyValue){
                break;
            }
            const TCHAR *cszKeyName = pKeyValue->GetKeyName();
            const TCHAR *cszValue = pKeyValue->GetValue();
            WriteFile(hFile, cszKeyName, _tcslen(cszKeyName)*sizeof(TCHAR), &dwWritten, NULL);
            WriteFile(hFile, _T("="), sizeof(TCHAR), &dwWritten, NULL);
            WriteFile(hFile, cszValue, _tcslen(cszValue)*sizeof(TCHAR), &dwWritten, NULL);
            WriteFile(hFile, _T("\r\n"), 2*sizeof(TCHAR), &dwWritten, NULL);
        }
    }
    CloseHandle(hFile);
}
///////////////////////////////////////////////////////////////////////////////
CIniSection* __fastcall CIniFile::FindSection(const TCHAR *cszSection,
                                              BOOL bCreate)
{ // セクションを検索
  // 見つからない場合は、bCreate が TRUE なら作る
    const int nCount = m_nCount;
    int i;
    for (i = 0; i < nCount; i++) {
        if (_tcsicmp(m_ppSections[i]->GetSectionName(), cszSection) == 0) {
            return m_ppSections[i];
        }
    }
    if (!bCreate) {
        return NULL;
    }
    if (m_nCount >= m_nCapacity) {
        if (!m_nCapacity) {
            m_nCapacity = 16;
        }
        else {
            m_nCapacity *= 2;
        }
        m_ppSections = (CIniSection * *)realloc(m_ppSections,
            sizeof(CIniSection*)*m_nCapacity);
    }
    CIniSection *pNewSection = new CIniSection(cszSection);
    m_ppSections[m_nCount++] = pNewSection;
    return pNewSection;
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CIniFile::Open(const TCHAR *cszFileName)
{
    Close();
    HANDLE hFile = CreateFile(cszFileName, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        const DWORD MAX_SIZE = 16 * 1024 * 1024; // 16MB まで読み取る
        DWORD dwSizeHigh = 0;
        DWORD dwSize = GetFileSize(hFile, &dwSizeHigh);
        if (dwSizeHigh) {
            CloseHandle(hFile);
            return FALSE;
        }
        if (dwSize > MAX_SIZE) {
            dwSize = MAX_SIZE;
        }
        BYTE* buf = (BYTE*)malloc(static_cast<size_t>(dwSize) + 3);
        ReadFile(hFile, buf, dwSize, &dwSize, NULL);
        buf[dwSize] = buf[dwSize+1] = buf[dwSize+2] = 0;
        OpenFromBuffer(buf, dwSize+3);
        free(buf);
        CloseHandle(hFile);
    }
    _tcsncpy_s(m_szFileName, cszFileName, _TRUNCATE);
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
BOOL __fastcall CIniFile::OpenFromBuffer(BYTE *pBuffer,
                                         DWORD dwSize)
{
    CIniSection *pCurrentSection = NULL;
    void  *pFree = NULL;//後で free する必要がある場合は 非NULL
    TCHAR *pszLine = conv_data_to_tstr(pBuffer, dwSize, &pFree);
    TCHAR *pszNextLine;
    TCHAR *pszSection = NULL;
    TCHAR *pszNextSection;
    TCHAR *pszKey;
    TCHAR *pszValue;
    while (_tcstok_s(pszLine, _T("\r\n"), &pszNextLine)) {
        strtrim(pszLine);
        if (!*pszLine) {
            pszLine = pszNextLine;
            continue;
        }
        if (IsSection(pszLine, &pszNextSection)) {
            pCurrentSection = NULL;
            pszSection = pszNextSection;
        }
        else if (pszSection && *pszSection && IsKeyValue(pszLine, &pszKey, &pszValue)) {
            if (!pCurrentSection) {
                pCurrentSection = FindSection(pszSection, TRUE);
            }
            pCurrentSection->AddKeyValue(pszKey, pszValue);
        }
        pszLine = pszNextLine;
    }
    if (pFree) {
        free(pFree);
    }
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
void __fastcall CIniFile::Close(void)
{
    m_szFileName[0] = 0;
    const int nCount = m_nCount;
    if (nCount) {
        int i;
        for (i = 0; i < nCount; i++) {
            delete m_ppSections[i];
        }
        free(m_ppSections);
        m_ppSections = NULL;
        m_nCount = m_nCapacity = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////

