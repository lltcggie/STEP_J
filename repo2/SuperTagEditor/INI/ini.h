#pragma once

class CIniKeyValue
{//Key=Value
private:
    TCHAR *m_pszKey;  //キー名
    TCHAR *m_pszValue;//値
    CIniKeyValue(void);
public:
    const TCHAR* __fastcall GetKeyName(void)const{return m_pszKey;}
    const TCHAR* __fastcall GetValue(void)const{return m_pszValue;}
    void __fastcall SetValue(const TCHAR *cszValue);
    CIniKeyValue(const TCHAR *cszKeyName, const TCHAR *cszValue);
    ~CIniKeyValue(void);
};

class CIniSection
{//[Section]
private:
    TCHAR         *m_pszSection; //セクション名
    CIniKeyValue **m_ppKeyValues;//
    int            m_nCapacity;
    int            m_nCount;
    CIniSection(void);
public:
    const TCHAR*  __fastcall GetSectionName(void)const{return m_pszSection;}
    CIniKeyValue* __fastcall FindKeyValue(const TCHAR *cszKey);
    CIniKeyValue* __fastcall GetKeyValue(int nIndex);
    void __fastcall AddKeyValue(const TCHAR *cszKey, const TCHAR *cszValue);
    CIniSection(const TCHAR *cszSectionName);
    ~CIniSection(void);
};

class CIniFile
{
private:
    TCHAR         m_szFileName[MAX_PATH];
    CIniSection **m_ppSections;
    int           m_nCapacity;
    int           m_nCount;
    CIniSection* __fastcall FindSection(const TCHAR *cszSection,
                                         BOOL bCreate);
    BOOL __fastcall OpenFromBuffer(BYTE *pBuffer, DWORD dwSize);
public:
    CIniSection* __fastcall GetIniSection(int nIndex);
    int   __fastcall ReadInt(const TCHAR *cszSection,
                             const TCHAR *cszKey,
                             int nDefault);
    TCHAR* __fastcall ReadStr(const TCHAR *cszSection,
                              const TCHAR *cszKey,
                              const TCHAR *cszDefault,
                              TCHAR *pszStr,
                              int    nSize);//文字数単位
    void  __fastcall WriteInt(const TCHAR *cszSection,
                              const TCHAR *cszKey,
                              int nValue);
    void  __fastcall WriteStr(const TCHAR *cszSection,
                              const TCHAR *cszKey,
                              const TCHAR *cszValue);
    void __fastcall Flush(void);
    BOOL __fastcall Open(const TCHAR *cszFileName);
    void __fastcall Close(void);
    CIniFile(void);
    CIniFile(const TCHAR *cszFileName);
    ~CIniFile(void);
};

