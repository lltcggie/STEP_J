#pragma once

enum {
    CONV_SUJI = 0x01, 
    CONV_ALPHA = 0x01 << 1, 
    CONV_KATA = 0x01 << 2, 
    CONV_KIGOU = 0x01 << 3, 
    //CONV_KIGOU_KATA = 0x01 << 4, //、。「」・をカタカナとして扱う
    CONV_ALL = 0xFFFFFFFF};

int conv_han2zens(TCHAR *zen, int zen_size, const TCHAR *han, DWORD flag, bool bZenHanKigouKana);//zen_size は文字数単位
int conv_zen2hans(TCHAR *han, int han_size, const TCHAR *zen, DWORD flag, bool bZenHanKigouKana);//han_size は文字数単位
void conv_kata2hira(TCHAR *str);
void conv_kata_erase_dakuon(TCHAR *str);
void conv_hira2kata(TCHAR *str);
void conv_upper(TCHAR *str);
void conv_lower(TCHAR *str);
void conv_first_upper(TCHAR *str, const TCHAR *suffixs, const TCHAR* separators , bool bUseSuffix);
void conv_fixed_upper_lower(TCHAR *_str, CStringArray& fixedWords); /* STEP 040 */

//以下の関数は戻り値を呼び出し側で free する
//変換元が 0 で終わらない可能性がある場合、文字数単位のサイズを size に渡す(0 で終わる場合は-1)
WCHAR* conv_ansi_to_utf16(const char *str_ansi, int size = -1);  //ansi => utf16(le)
char*  conv_utf16_to_ansi(const WCHAR *str_utf16, int size = -1);//utf16(le) => ansi
WCHAR* conv_utf8_to_utf16(const char *str_utf8, int size = -1);  //utf8 => utf16
char*  conv_utf16_to_utf8(const WCHAR *str_utf16, int size = -1);//utf16 => utf8
char*  conv_ansi_to_utf8(const char *str_ansi, int size = -1);   //ansi => utf8
char*  conv_utf8_to_ansi(const char *str_utf8, int size = -1);   //utf8 => ansi

#ifdef _UNICODE
#define conv_tstr_to_utf8 conv_utf16_to_utf8
#define conv_utf8_to_tstr conv_utf8_to_utf16
#else
#define conv_tstr_to_utf8 conv_ansi_to_utf8
#define conv_utf8_to_tstr conv_utf8_to_ansi
#endif

//BOM を自動判別して TCHAR* 変換
//BOM なし UTF-8 の自動判別には未対応(BOM なしは ANSI と見なす)
//他の conv_xxx_to_xxx とは異なり、戻り値を free してはいけない
//*pFree != NULL のときは *pFree を呼び出し側で free する
//*pFree == NULL のときは 戻り値が data の一部であるため、free しない
//dwBufferSize はバッファのサイズ(文字数ではない)
//終端が 0 で終わることが分かっていても dwBufferSize に -1 を渡してはいけない
TCHAR* conv_data_to_tstr(const BYTE *data, DWORD dwBufferSize, void **ppFree);
