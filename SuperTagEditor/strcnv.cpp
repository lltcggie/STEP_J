#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strcnv.h"
//#include "imm.h"

#ifndef _UNICODE

//#ifndef iskanji
//#define iskanji(c)		((c) >= 0x81 && (c) <= 0x9f || (c) >= 0xe0 && (c) <= 0xfc)
//#endif

#endif


#ifndef _UNICODE
unsigned short han2zen(int c, DWORD flag)
{
	if (c <= 0x19 || (c >= 0x80 && c <= 0xa0) || c >= 0xe0) {
		return c;					// 制御未定義
	}
	if (flag & CONV_ALPHA) {
		if (c >= 'A' && c <= 'Z') return(0x8260U + (c - 'A'));	// 英大文字
		if (c >= 'a' && c <= 'z') return(0x8281U + (c - 'a'));	// 英小文字
	}
	if (flag & CONV_SUJI) {
		if (c >= '0' && c <= '9') return(0x824FU + (c - '0'));	// 数字
	}

	int		i;
	static const unsigned char *s1 = (const unsigned char *)
		/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
		  "   ,. :;?!゛゜   "		/* 212X:　　、。，．・：；？！゛゜´｀¨ */
		  /* STEP 016 *///" 、。,.・:;?!゛゜   "		/* 212X:　　、。，．・：；？！゛゜´｀¨ */
		  /* BeachMonster 102 *///"^~_         ー- /"	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  /* Baja 169 *///"^~_          - /"	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  "^~_            /"	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  "   |  `\' \"()  []"	/* 214X:＼～‖｜…‥‘’“”（）〔〕［］ */
		  "{}          +-   "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  /* STEP 016 *///"{}    「」    +-   "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  " = <>          \\"	/* 216X:÷＝≠＜＞≦≧∞∴♂♀°′″℃¥ */
		  "$  %#&*@        ";	/* 217X:＄¢£％＃＆＊＠§☆★○●◎◇   */
	static const unsigned char *s2 = (const unsigned char *)
		/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
		  "ァアィイゥウェエォオガギク"		/* 252X:　ァアィイゥウェエォオカガキギク */
		  "゛ゲゴザジズゼゾタ"	/* 253X:グケゲコゴサザシジスズセゼソゾタ */
		  "゛ヂッヅデドナニヌネノハ"	/* 254X:ダチヂッツヅテデトドナニヌネノハ */
		  "゛゜ビ゜ブ゜ベ゜ボ゜マミ "	/* 255X:バパヒビピフブプヘベペホボポマミ7F */
		  "ムメモャヤュユョヨラリルレロ ワ"	/* 256X:ムメモャヤュユョヨラリルレロヮワ */
		  "  ヲン";				/* 257X:ヰヱヲンヴヵヶ */
	static const unsigned char *s3 = (const unsigned char *) /* BeachMonster 102 */
		/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
		  "                "	/* 212X:　　、。，．・：；？！゛゜´｀¨ */
		  "            ー   "	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  "                "	/* 214X:＼～‖｜…‥‘’“”（）〔〕［］ */
		  "                "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  "                "	/* 216X:÷＝≠＜＞≦≧∞∴♂♀°′″℃¥ */
		  "                ";	/* 217X:＄¢£％＃＆＊＠§☆★○●◎◇   */
		 ;
	static const unsigned char *s4 = (const unsigned char *) /* STEP 016 */
		/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
		  "  、。  ・         "	/* 212X:　　、。，．・：；？！゛゜´｀¨ */
		  "                "	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  "                "	/* 214X:＼～‖｜…‥‘’“”（）〔〕［］ */
		  "      「」        "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  "                "	/* 216X:÷＝≠＜＞≦≧∞∴♂♀°′″℃¥ */
		  "                ";	/* 217X:＄¢£％＃＆＊＠§☆★○●◎◇   */
		 ;
	if (flag & CONV_KIGOU) {
		for (i = 0; s1[i]; i++) if (c == s1[i]) return(0x8140U+i);	// 記号
	}
	if (flag & CONV_KATA) {
		for (i = 0; s2[i]; i++) if (c == s2[i]) return(0x8340U+i);	// カタカナ
	}
	if (flag & CONV_KATA) { /* BeachMonster 102 */
		for (i = 0; s3[i]; i++) if (c == s3[i]) return(0x8140U+i-1/* STEP 016*/);	// 記号だけどカタカナ扱い
	}
	extern bool g_bZenHanKigouKana; /* STEP 016 */
	if ((!g_bZenHanKigouKana && flag & CONV_KIGOU) 
		|| (g_bZenHanKigouKana && flag & CONV_KATA)) { /* STEP 016 */
		for (i = 0; s4[i]; i++) if (c == s4[i]) return(0x8140U+i-1);
	}
	return(c);
}

int conv_han2zens(TCHAR *_zen, int han_size, const TCHAR *_han, DWORD flag)
{
    unsigned char *zen = (unsigned char*)_zen;
    const unsigned char *han = (const unsigned char*)_han;
	int		i;
	int		pzen = 0;		// 全角位置
	int		l = strlen((const char *)han);
	zen[0] = '\0';

	for (i = 0; i < l; i++, pzen++, zen[pzen] = '\0') {
		if (han[i] == ' ') {		// 空白
			if (flag & CONV_KIGOU) {
				zen[pzen] = 0x81U;
				pzen++;
				zen[pzen] = 0x40U;
			} else {
				zen[pzen] = ' ';
			}
			continue;
		}

		if (_istlead(han[i])) {		// 漢字
			zen[pzen] = han[i];
			pzen++;
			i++;
			zen[pzen] = han[i];
			continue;
		}

		if (flag & CONV_KATA) {
			unsigned int	x;
			static const unsigned char *s1 = (const unsigned char *)
			/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
			             "カ キ ク"	/* 252X:　ァアィイゥウェエォオカガキギク */
			  " ケ コ サ シ ス セ ソ タ"	/* 253X:グケゲコゴサザシジスズセゼソゾタ */
			  " チ ッツ テ ト ナニヌネノハ"	/* 254X:ダチヂッツヅテデトドナニヌネノハ */
			  "  ヒ  フ  ヘ  ホ";		/* 255X:バパヒビピフブプヘベペホボポ     */
			static const unsigned char *s2 = (const unsigned char *)
			/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
			                 "ハ"	/* 254X:                              ハ */
			  "  ヒ  フ  ヘ  ホ";		/* 255X:バパヒビピフブプヘベペホボポ     */

			// [ヴ] => [ヴ] の変換
			if (han[i] == (unsigned char)'ウ' && han[i+1] == (unsigned char)'゛') {
				zen[pzen] = 0x83U;
				pzen++;
				i++;
				zen[pzen] = 0x94U;
				goto cont;
			}

			// 濁音の変換
			for (x = 0; s1[x]; x++) {
				if (han[i] == s1[x] && han[i+1] == (unsigned char)'゛') {
					zen[pzen] = 0x83U;
					pzen++;
					i++;
					zen[pzen] = 0x4BU + x;
					goto cont;
				}
			}

			// 半濁音の変換
			for (x = 0; s2[x]; x++) {
				if (han[i] == s2[x] && han[i+1] == (unsigned char)'゜') {
					// 半濁音
					zen[pzen] = 0x83U;
					pzen++;
					i++;
					zen[pzen] = 0x70U + x;
					goto cont;
				}
			}
		}

		// その他の変換
		{
			unsigned short z = han2zen(han[i], flag);
			if (z >= 0x100) {
				zen[pzen] = (unsigned char)(z >> 8);
				pzen++;
				zen[pzen] = (unsigned char)z;
			} else {
				zen[pzen] = (unsigned char)z;
			}
		}
cont:	;
	}

	return(pzen);
}

unsigned short zen2han(unsigned short c, DWORD flag)
{
	if (c <= 0x100) return(c);				// 非漢字
	if (flag & CONV_ALPHA) {
		if (c == 0x8140U) return(' ');			// 空白
		if (c >= 0x8260U && c <= 0x8279U) {		// Ａ−Ｚ
			return('A' + (c - 0x8260U));
		}
		if (c >= 0x8281U && c <= 0x829AU) {		// ａ−ｚ
			return('a' + (c - 0x8281U));
		}
	}
	if (flag & CONV_SUJI) {
		if (c >= 0x824FU && c <= 0x8258U) {		// ０−９
			return('0' + (c - 0x824FU));
		}
	}

	int		i;
	if (flag & CONV_KIGOU) {
		// なんで"～"を"-"にしてるのか覚えてないので削除 041018
		static const unsigned char *han = (const unsigned char *)
			",.:;?!゛゜^~_-/|`\'\"()[]{}+-=<>\\$%#&*@"; /* BeachMonster 102 *//* STEP 016 */
		static const unsigned char *zen = (const unsigned char *)
			"，．：；？！゛゜＾‾＿−／｜｀’”（）［］｛｝＋−＝＜＞¥＄％＃＆＊＠\0"; /* BeachMonster 102 *//* STEP 016 */
		for (i = 0; zen[i+1]; i+=2) {
			if (c == ((zen[i] << 8) | zen[i+1])) return(han[i/2]);
		}
		extern bool g_bZenHanKigouKana; /* STEP 016 */
		if (!g_bZenHanKigouKana) { /* STEP 016 */
			static const unsigned char *han2 = (const unsigned char *)
				"・、。「」";
			static const unsigned char *zen2 = (const unsigned char *)
				"・、。「」\0";
			for (i = 0; zen2[i+1]; i+=2) {
				if (c == ((zen2[i] << 8) | zen2[i+1])) return(han2[i/2]);
			}
		}
	}
	if (flag & CONV_KATA) {
		static const unsigned char *han = (const unsigned char *)
			"ァアィイゥウェエォオカキクケコサシスセソタチッツテトナニヌネノハヒフヘホマミムメモャヤュユョヨラリルレロワヲンー";
		static const unsigned char *kata = (const unsigned char *)
			"ァアィイゥウェエォオカキクケコサシスセソタチッツテトナニヌネノ"
			"ハヒフヘホマミムメモャヤュユョヨラリルレロワヲンー\0";
		// カタカナ
		for (i = 0; kata[i+1]; i+=2) {
			if (c == ((kata[i] << 8) | kata[i+1])) return(han[i/2]);
		}
		extern bool g_bZenHanKigouKana; /* STEP 016 */
		if (g_bZenHanKigouKana) { /* STEP 016 */
			static const unsigned char *han2 = (const unsigned char *)
				"・、。「」";
			static const unsigned char *kata2 = (const unsigned char *)
				"・、。「」\0";
			for (i = 0; kata2[i+1]; i+=2) {
				if (c == ((kata2[i] << 8) | kata2[i+1])) return(han2[i/2]);
			}
		}
	}

	// 変換できなかった
	return(c);
}

int conv_zen2hans(TCHAR *_han, int han_size, const TCHAR *_zen, DWORD flag)
{
    unsigned char *han = (unsigned char *)_han;
    const unsigned char *zen = (const unsigned char *)_zen;

	int		i;
	int		phan = 0;		// 半角位置
	int		l = strlen((const char *)zen);
	han[0] = '\0';

	for (i = 0; i < l; i++, phan++, han[phan] = '\0') {
		if (!_istlead(zen[i])) {		// 非漢字
			han[phan] = zen[i];
			continue;				// 無変換
		}

		if (flag & CONV_KATA) {
			int		j;
			static const unsigned char *k1 = (const unsigned char *)"ガギグゲゴザジズゼゾダヂヅデドバビブベボヴ\0";
			static const unsigned char *h1 = (const unsigned char *)"がぎぐげござじずぜぞだぢづでどばびぶべぼ\0\0\0";
			static const unsigned char *k2 = (const unsigned char *)"パピプペポ\0";
			static const unsigned char *h2 = (const unsigned char *)"ぱぴぷぺぽ\0";
			// 濁音の変換
			for (j = 0; k1[j+1]; j+=2) {
#if 1
				if (zen[i] == k1[j] && zen[i+1] == k1[j+1]) {	// カタカナ
#else
				if ((zen[i] == k1[j] && zen[i+1] == k1[j+1])	// カタカナ
				||  (zen[i] == h1[j] && zen[i+1] == h1[j+1])) {	// ひらがな
#endif
					static const unsigned char *h = (const unsigned char *)"カキクケコサシスセソタチツテトハヒフヘホウ";
					han[phan] = h[j/2];
					phan++;
					han[phan] = (unsigned char)'゛';
					i++;
					goto cont;
				}
			}

			// 半濁音の変換
			for (j = 0; k2[j+1]; j+=2) {
#if 1
				if (zen[i] == k2[j] && zen[i+1] == k2[j+1]) {	// カタカナ
#else
				if ((zen[i] == k2[j] && zen[i+1] == k2[j+1])	// カタカナ
				||  (zen[i] == h2[j] && zen[i+1] == h2[j+1])) {	// ひらがな
#endif
					static const unsigned char *h = (const unsigned char *)"ハヒフヘホ";
					han[phan] = h[j/2];
					phan++;
					han[phan] = (unsigned char)'゜';
					i++;
					goto cont;
				}
			}
		}

		// その他の変換
		{
			unsigned short c;
			c = zen2han((zen[i] << 8) | zen[i+1], flag);
			i++;
			if (c >= 0x100) {
				han[phan] = (unsigned char)(c >> 8);
				phan++;
				han[phan] = (unsigned char)c;
			} else {
				han[phan] = (unsigned char)c;
			}
		}

cont:	;
	}

	return(phan);
}
#else

static const WCHAR kigou_han[] = 
    //012345678901234 5 678901234567 8901234567890
    L",.:;?!゛゜^~_-/|`\'\"()[]{}+-=<>\\$%#&*@ ";//・、。「」 ";
static const WCHAR kigou_zen[] = 
    //0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0(最後は全角スペース)
    L"，．：；？！゛゜＾‾＿−／｜｀’”（）［］｛｝＋−＝＜＞¥＄％＃＆＊＠　";//・、。「」　";
/*
static const WCHAR kigou_kana_han[] = //、。「」・をカタカナとして扱う
    //012
    L"、。・";
static const WCHAR kigou_kana_zen[] = //、。「」・をカタカナとして扱う
    //0 1 2
    L"、。・";
*/
static const WCHAR kata_han[] = 
    L"ァアィイゥウェエォオカキクケコサシスセソタチッツテトナニヌネノハヒフヘホマミムメモャヤュユョヨラリルレロワヲンー";
static const WCHAR kata_zen[] = 
    L"ァアィイゥウェエォオカキクケコサシスセソタチッツテトナニヌネノ"
	L"ハヒフヘホマミムメモャヤュユョヨラリルレロワヲンー";
static const WCHAR kata_dakuon_han[] = 
    L"ウカキクケコサシスセソタチツテトハヒフヘホ";
static const WCHAR kata_dakuon_zen[] = 
    L"ヴガギグゲゴザジズゼゾダジヅデドバビブベボ";
static const WCHAR kata_handakuon_han[] = 
    L"ハヒフヘホ";
static const WCHAR kata_handakuon_zen[] = 
    L"パピプペポ";

//全角・半角変換で、。「」をカタカナとして扱う(g_bZenHanKigouKana == true のとき使う)
static const WCHAR kata_han2[] = L"・、。「」";
static const WCHAR kata_zen2[] = L"・、。「」";

int conv_han2zens(TCHAR *zen, int zen_size, const TCHAR *han, DWORD flag, bool bZenHanKigouKana)
{
    //extern bool g_bZenHanKigouKana;
    WCHAR *dst = (WCHAR*)zen;
    const WCHAR *src = (const WCHAR*)han;
    wcsncpy_s(zen, zen_size, han, _TRUNCATE);
    int len = wcslen(zen);
    //濁音・半濁音が消えるだけなので、以降サイズを気にする必要はない
    int i, j;
    if(flag & CONV_SUJI){
        for(i = 0 ; dst[i]; i++){
            if(L'0' <= dst[i] && dst[i] <= L'9'){
                dst[i] += L'０' -L'0';
            }
        }
    }
    if(flag & CONV_ALPHA){
        for(i = 0 ; dst[i]; i++){
            if(L'a' <= dst[i] && dst[i] <= L'z'){
                dst[i] += L'ａ' -L'a';
            }
            else if(L'A' <= dst[i] && dst[i] <= L'Z'){
                dst[i] += L'Ａ' - L'A';
            }
            //else if(dst[i] == L' '){//半角スペース->全角スペース(オリジナルに合わせて変換しない)
            //全角スペース=>半角スペースは変換する
            //    dst[i] = L'　';
            //}
        }
    }
    
	if(flag & CONV_KIGOU) {
        for(i = 0; dst[i]; i++){
            for(j = 0; kigou_han[j]; j++){
                if(dst[i] == kigou_han[j]){
                    dst[i] = kigou_zen[j];
                    break;
                }
            }
            if(!bZenHanKigouKana){
                for(j = 0; kata_han2[j]; j++){
                    if(dst[i] == kata_han2[j]){
                        dst[i] = kata_zen2[j];
                        break;
                    }
                }
            }
        }
	}
	if(flag & CONV_KATA) {
        for(i = 0; dst[i]; i++){
            bool bDakuon = false;//濁音/半濁音の場合 true
            if(dst[i+1] == L'゜'){//半濁音
                for(j = 0; kata_handakuon_han[j]; j++){
                    if(dst[i] == kata_handakuon_han[j]){
                        dst[i] = kata_handakuon_zen[j];
                        //゜を除去
                        memmove(&dst[i+1], &dst[i+2], (len-i)*sizeof(WCHAR));
                        len--;
                        break;
                    }
                }
                if(kata_handakuon_han[j]){
                    continue;
                }
                bDakuon = true;
            }
            else if(dst[i+1] == L'゛'){//濁音
                for(j = 0; kata_dakuon_han[j]; j++){
                    if(dst[i] == kata_dakuon_han[j]){
                        dst[i] = kata_dakuon_zen[j];
                        //゛を除去
                        memmove(&dst[i+1], &dst[i+2], (len-i)*sizeof(WCHAR));
                        len--;
                        break;
                    }
                }
                if(kata_dakuon_han[j]){
                    continue;
                }
                bDakuon = true;
            }
            for(j = 0; kata_han[j]; j++){
                if(dst[i] == kata_han[j]){
                    dst[i] = kata_zen[j];
                    if(bDakuon){
                    //濁音/半濁音をつけるべきでない半角カナの場合は濁音/半濁音を除去
                    //ア゛ => ア (オリジナルの方だとアガになる)
                        memmove(&dst[i+1], &dst[i+2], (len-i)*sizeof(WCHAR));
                        len--;
                    }
                    break;
                }
            }
            if(bZenHanKigouKana){
                for(j = 0; kata_han2[j]; j++){
                    if(dst[i] == kata_han2[j]){
                        dst[i] = kata_zen2[j];
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
int conv_zen2hans(TCHAR *han, int han_size, const TCHAR *zen, DWORD flag, bool bZenHanKigouKana)
{
    //extern bool g_bZenHanKigouKana;
    WCHAR *dst = (WCHAR*)han;
    const WCHAR *src = (const WCHAR*)zen;
    wcsncpy_s(dst, han_size, src, _TRUNCATE);
    int len = wcslen(dst);
    int i, j;
    if(flag & CONV_SUJI){
        for(i = 0 ; dst[i]; i++){
            if(L'０' <= dst[i] && dst[i] <= L'９'){
                dst[i] -= (L'０' -L'0');
            }
        }
    }
    if(flag & CONV_ALPHA){
        for(i = 0 ; dst[i]; i++){
            if(L'ａ' <= dst[i] && dst[i] <= L'ｚ'){
                dst[i] -= (L'ａ' -L'a');
            }
            else if(L'Ａ' <= dst[i] && dst[i] <= L'Ｚ'){
                dst[i] -= (L'Ａ' - L'A');
            }
            else if(dst[i] == L'　'){//全角スペース=>半角スペース
                //(オリジナルに合わせて半角スペース->全角スペースは変換しない)
                dst[i] = L' ';
            }        }
    }
	if(flag & CONV_KIGOU) {
        for(i = 0; dst[i]; i++){
            for(j = 0; kigou_zen[j]; j++){
                if(dst[i] == kigou_zen[j]){
                    dst[i] = kigou_han[j];
                    break;
                }
            }
            if(!bZenHanKigouKana){
                for(j = 0; kata_zen2[j]; j++){
                    if(dst[i] == kata_zen2[j]){
                        dst[i] = kata_han2[j];
                        break;
                    }
                }
            }        
        }

	}
	if(flag & CONV_KATA) {
    //濁音・半濁音を付加するときにサイズが大きくなる
        for(i = 0; dst[i] && i < han_size; i++){
            //半濁音
            for(j = 0; kata_handakuon_zen[j]; j++){
                if(dst[i] == kata_handakuon_zen[j]){
                    dst[i] = kata_handakuon_han[j];
                    if(i + 1 >= han_size){
                        break;
                    }
                    //゜を付加
                    memmove(&dst[i+2], &dst[i+1], (len-i)*sizeof(WCHAR));
                    dst[i+1] = L'゜';
                    len++;
                    break;
                }
            }
            if(kata_handakuon_zen[j]){
                continue;
            }
            //濁音
            for(j = 0; kata_dakuon_zen[j]; j++){
                if(dst[i] == kata_dakuon_zen[j]){
                    dst[i] = kata_dakuon_han[j];
                    if(i + 1 >= han_size){
                        break;
                    }
                    //゛を付加
                    memmove(&dst[i+2], &dst[i+1], (len-i)*sizeof(WCHAR));
                    dst[i+1] = L'゛';
                    len++;
                    break;
                }
            }
            if(kata_dakuon_zen[j]){
                continue;
            }
            for(j = 0; kata_zen[j]; j++){
                if(dst[i] == kata_zen[j]){
                    dst[i] = kata_han[j];
                    break;
                }
            }
            if(bZenHanKigouKana){
                for(j = 0; kata_zen2[j]; j++){
                    if(dst[i] == kata_zen2[j]){
                        dst[i] = kata_han2[j];
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
#endif
static const TCHAR kata[] = //(const unsigned char *)
	_T("ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾ")
	_T("タダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポ")
	_T("マミムメモャヤュユョヨラリルレロヮワヲン")
#ifdef _UNICODE
    _T("ヴ");//UNICODE の場合だけ「ヴ」と「ゔ」に対応
#else
    ;
#endif
static const TCHAR hira[] = //(const unsigned char *)
	_T("ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞ")
	_T("ただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽ")
	_T("まみむめもゃやゅゆょよらりるれろゎわをん")
#ifdef _UNICODE
    _T("ゔ");//UNICODE の場合だけ「ヴ」と「ゔ」に対応
#else
    ;
#endif

static const TCHAR alphaS[] = //(const unsigned char *)
	_T("ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ’０１２３４５６７８９") /* STEP 026 */;
static const TCHAR alphaL[] = //(const unsigned char *)
	_T("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ’０１２３４５６７８９") /* STEP 026 */;

static const TCHAR kata_dakuon[] = //(const unsigned char *) /* FunnyCorn 179 */
	_T("ァアィイゥウェエォオカカキキククケケココササシシススセセソソ")
	_T("タタチチッツツテテトトナニヌネノハハハヒヒヒフフフヘヘヘホホホ")
	_T("マミムメモャヤュユョヨラリルレロヮワヲン");

//void conv_table(const unsigned char *before, const unsigned char *after, unsigned char *str)
void conv_table(const TCHAR *_before, const TCHAR *_after, TCHAR *_str)
{
#ifndef _UNICODE
    const unsigned char *before = (const unsigned char*)_before;
    const unsigned char *after = (const unsigned char*)_after;
    unsigned char *str = (unsigned char*)_str;
	while(*str != '\0') {
		if (_istlead(*str)) {		// 漢字
			int i; for (i = 0; before[i]; i+=2) {
				if (before[i] == str[0] && before[i+1] == str[1]) {
					str[0] = after[i];
					str[1] = after[i+1];
					break;
				}
			}
			str++;
		}
		str++;
	}
#else
    const WCHAR *before = _before;
    const WCHAR *after = _after;
    WCHAR *str = _str;
	while(*str) {
		int i; 
        for (i = 0; before[i]; i++) {
			if (before[i] == str[0]) {
				str[0] = after[i];
				break;
			}
		}
		str++;
	}
#endif
}
// [カタカナ]の濁音、半濁音をなくす
void conv_kata_erase_dakuon(TCHAR *str)
{
	conv_table(kata, kata_dakuon, str);
}
// [カタカナ]=>[ひらがな]に変換
void conv_kata2hira(TCHAR *str)
{
	conv_table(kata, hira, str);
}
// [ひらがな]=>[カタカナ]に変換
void conv_hira2kata(TCHAR *str)
{
	conv_table(hira, kata, str);
}
// [小文字]=>[大文字]に変換
void conv_upper(TCHAR *str)
{
	_tcsupr_s(str, _tcslen(str)+1);
	conv_table(alphaS, alphaL, str);
}
// [大文字]=>[小文字]に変換
void conv_lower(TCHAR *str)
{
    _tcslwr_s(str, _tcslen(str)+1);
	conv_table(alphaL, alphaS, str);
}

#ifndef _UNICODE
void lower_suffix_word(unsigned char *str, int len, CString suffixs) /* STEP 026*/
{
	CString strZWord((LPCTSTR)str, len);
	CString strWord;
	conv_zen2hans(strWord.GetBuffer((len+1)*2), (len+1)*2, strZWord, CONV_ALL);
	strWord.ReleaseBuffer();
	strWord.MakeLower();
	while (1) {
		CString suffix = suffixs.SpanExcluding(_T(","));
		if (suffix == strWord) {
			if (_istlead(*str)) {		// 漢字
				int i; for (i = 0; alphaS[i]; i+=2) {
					if ((alphaS[i] == str[0] && alphaS[i+1] == str[1])
					||  (alphaL[i] == str[0] && alphaL[i+1] == str[1])) {
						str[0] = alphaS[i];
						str[1] = alphaS[i+1];
						break;
					}
				}
			} else {
				*str = tolower(*str);
			}
			return;
		}
		suffixs.TrimLeft(suffix);
		suffixs.TrimLeft(',');
		if (suffixs.GetLength() == 0) {
			return;
		}
	}
}
bool isSentenceSeparate(unsigned char* str, int len, CString separator) /* STEP 026 */
{
	CString strZWord((LPCTSTR)str, len);
	CString strChar;
	conv_zen2hans(strChar.GetBuffer(2+1), 2+1, strZWord, CONV_ALL);
	strChar.ReleaseBuffer();
	for (int i=0;i<separator.GetLength();i++) {
		unsigned char c = separator.GetAt(i);
		if (_istlead(c)) {		// 漢字
			char	kanji[3];
			kanji[0] = separator.GetAt(i);
			kanji[1] = separator.GetAt(i+1);
			kanji[2] = '\0';
			if (strChar == kanji) {
				return true;
			}
		} else {
			if (strChar == separator.GetAt(i)) {
				return true;
			}
		}
	}
	return false;
}

// 単語の１文字目のみ、[小文字]=>[大文字]に変換
void conv_first_upper(TCHAR *_str, const TCHAR *suffixs, const TCHAR* separator , bool bUseSuffix)
{
	bool	bFirst = true;
	unsigned char*	str = (unsigned char*)_str;
	unsigned char*	pFirstPos = NULL; /* STEP 026 */
	unsigned char*	pEndPos = NULL; /* STEP 026 */
	bool	bFirstWord = true; /* STEP 026 */
	while(TRUE) {
		if (_istlead(*str)) {		// 漢字
			int i; for (i = 0; alphaS[i]; i+=2) {
				if ((alphaS[i] == str[0] && alphaS[i+1] == str[1])
				||  (alphaL[i] == str[0] && alphaL[i+1] == str[1])) {
					if (bFirst) {
						str[0] = alphaL[i];
						str[1] = alphaL[i+1];
						pFirstPos = str; /* STEP 026 */
					}
					bFirst = false;
					break;
				}
			}
			if (alphaS[i] == '\0') {	// 単語の区切り
				pEndPos = str-1; /* STEP 026 */
				if (pFirstPos != NULL && pEndPos != NULL && !bFirstWord && bUseSuffix) { /* STEP 026 */
					lower_suffix_word(pFirstPos, pEndPos - pFirstPos + 1, suffixs);
				}
				pFirstPos = NULL; /* STEP 026 */
				pEndPos = NULL; /* STEP 026 */
				if (!bFirst) bFirstWord = FALSE; /* STEP 026 */
				if (bUseSuffix) { /* STEP 026 */
					if (isSentenceSeparate(str, 2, separator)) {
						bFirstWord = TRUE;
					}
				}
				bFirst = true;
			}
			if (*str == '\0') break; /* STEP 026 */
			str++;
		} else {					// 非漢字
			if ((*str >= 'a' && *str <= 'z')
			||  (*str >= 'A' && *str <= 'Z')
			||  (*str >= '0' && *str <= '9') /* STEP 026 2005.12.02 */
			||  (*str == '\'')) {
				if (bFirst) {
					*str = toupper(*str);
					bFirst = false;
					pFirstPos = str; /* STEP 026 */
				}
			} else {
				// 単語の区切り
				pEndPos = str-1;
				if (pFirstPos != NULL && pEndPos != NULL && !bFirstWord && bUseSuffix) { /* STEP 026 */
					lower_suffix_word(pFirstPos, pEndPos - pFirstPos + 1, suffixs);
				}
				pFirstPos = NULL; /* STEP 026 */
				pEndPos = NULL; /* STEP 026 */
				if (!bFirst) bFirstWord = FALSE; /* STEP 026 */
				if (bUseSuffix) { /* STEP 026 */
					if (isSentenceSeparate(str, 1, separator)) {
						bFirstWord = TRUE;
					}
				}
				if (*str == '\n') {
					bFirstWord = TRUE;
				}
				bFirst = true;
			}
		}
		if (*str == '\0') break; /* STEP 026 */
		str++;
	}
}

bool isKigou(char ch) {
	static const unsigned char *s1 = (const unsigned char *)
		/* 0123456789ABCDEF		        ０１２３４５６７８９ＡＢＣＤＥＦ */
		  "   ,. :;?!゛゜   "		/* 212X:　　、。，．・：；？！゛゜´｀¨ */
		  "^~_            /"	/* 213X:＾‾＿ヽヾゝゞ〃仝々〆〇ー—‐／ */
		  "   |  `\' \"()  []"	/* 214X:＼～‖｜…‥‘’“”（）〔〕［］ */
		  "{}          +-   "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  /* STEP 016 *///"{}    「」    +-   "	/* 215X:｛｝〈〉《》「」『』【】＋−±×7F */
		  " = <>          \\"	/* 216X:÷＝≠＜＞≦≧∞∴♂♀°′″℃¥ */
		  "$  %#&*@        ";	/* 217X:＄¢£％＃＆＊＠§☆★○●◎◇   */
	for (int i=0;s1[i]!='\0';i++) {
		if (ch == s1[i]) {
			return true;
		}
	}
	return false;
}

unsigned char* fixed_upper_lower(unsigned char *str, CStringArray& fixedWords) /* STEP 040*/
{
	CString strZWord((LPCTSTR)str);
	CString strWord;
	conv_zen2hans(strWord.GetBuffer(strlen((char*)str)+1), strlen((char*)str)+1, strZWord, CONV_ALL);
	strWord.ReleaseBuffer();
	strWord.MakeLower();
	for (int i=0;i<fixedWords.GetSize();i++) {
		CString fixed = fixedWords.GetAt(i);
		CString strFixZWord(fixed);
		CString strFixWord;
		conv_zen2hans(strFixWord.GetBuffer(strFixZWord.GetLength()+1), strFixZWord.GetLength()+1, strFixZWord, CONV_ALL);
		strFixWord.ReleaseBuffer();
		fixed = strFixWord;
		fixed.MakeLower();
//		if (fixed == strWord) {
		if (strWord.Find(fixed, 0) == 0 && (strWord == fixed || (strWord.GetLength() >= fixed.GetLength() && isKigou(strWord.GetAt(_tcslen(fixed)))) )) {
			unsigned char* pos = str;
			for (int j=0;j<fixed.GetLength();j++) {
				if (_istlead(*pos)) {		// 漢字
					for (int k = 0; alphaS[k]; k+=2) {
						if ((alphaS[k] == pos[0] && alphaS[k+1] == pos[1])
						||  (alphaL[k] == pos[0] && alphaL[k+1] == pos[1])) {
							if (isupper(strFixWord.GetAt(j))) {
								pos[0] = alphaL[k];
								pos[1] = alphaL[k+1];
							} else {
								pos[0] = alphaS[k];
								pos[1] = alphaS[k+1];
							}
							break;
						}
					}
					pos += 2;
				} else {
					*pos = strFixWord.GetAt(j);
					pos++;
				}
			}
			return pos;
		}
	}
	return NULL;
}

void conv_fixed_upper_lower(TCHAR *_str, CStringArray& fixedWords) /* STEP 040 */
{
	unsigned char* str = (unsigned char*)_str;
	unsigned char* current = str;
	bool bConv = false;
	while (TRUE) {
		if ((current = fixed_upper_lower(str, fixedWords))) {
			bConv = true;
			str = current;
		} else {
			bConv = false;
			current = str;
		}
		if (*str == '\0') break;
		if (bConv) continue;
		//if (_istlead(*str)) {		// 漢字
		//	str++;
		//}
		//str++;
		bool bBreak;
		if (current == str) {
			bBreak = TRUE;
		}
		while(TRUE) {
			if (*str == '\0') break;
			if (_istlead(*str)) {		// 漢字
				int i; for (i = 0; alphaS[i]; i+=2) {
					if ((alphaS[i] == str[0] && alphaS[i+1] == str[1])
					||  (alphaL[i] == str[0] && alphaL[i+1] == str[1])) {
						break;
					}
				}
				if (alphaS[i] == '\0') {	// 単語の区切り
					str += 2;
					continue;
				} else {
					if (!bBreak) {
						break;
					}
					str += 2;
				}
			} else {					// 非漢字
				if ((*str >= 'a' && *str <= 'z')
				||  (*str >= 'A' && *str <= 'Z')
				||  (*str >= '0' && *str <= '9')
				||  (*str == '\'')) {
					if (!bBreak) {
						break;
					}
					str++;
				} else {
					str++;
					if (bBreak) {
						break;
					}
					continue;
				}
			}
		}
	}
}

#else
//by Kobarin
//UNICODE 版 conv_first_upper,conv_fixed_upper_lower
//単語の区切り文字がオリジナル(ANSI版)と異なる
//オリジナルは英数字と'を除くほとんどの文字?
static const WCHAR word_delimiters[] = //単語の区切り文字(' を除くほとんどの記号)
    L" !\"#$%&()=-~^\\|@`[{+;*:}]<,>.?/_\r\n\t"
    L"　！¥”＃＄＆（）＝−～＾｜＠‘「｛＋；＊：｝」＜，＞．？／";

static WCHAR* wcstok_internal(WCHAR *pszStr, const WCHAR *cszDelimiters, WCHAR **ppszNext, WCHAR *pDelimiter)
{//C ランタイム関数の _wcstok と大体同じ(conv_first_upper/conv_fixed_upper_lower 用)
 //状態を内部で持たずに *ppszNext に代入する(スレッドローカル変数を使わない)点と
 //cszDelimiters のどの文字で区切ったかを *pDelimiter に入れる点が異なる
 //二度目以降の呼び出し時は pszStr に NULL を渡すのではなく、前回呼び出し時に受け取った *ppszNext を渡す
 //NULL が返ったら終了
    *pDelimiter = NULL;
    *ppszNext = NULL;
    wchar_t *p = pszStr;
    if (p) {
        while (*p) {
            const wchar_t *delim = cszDelimiters;
            while (*delim) {
                if (*p == *delim++) {
                    *pDelimiter = *p;
                    *p++ = 0;
                    *ppszNext = p;
                    return pszStr;
                }
            }
            p++;
        }
        if (*pszStr) {
            return pszStr;
        }
    }
    return NULL;
}
// 単語の１文字目のみ、[小文字]=>[大文字]に変換
void conv_first_upper(TCHAR *str, const TCHAR *suffixs, const TCHAR* separators , bool bUseSuffix)
{//by Kobarin
 //ANSI 版 conv_first_upper と若干仕様が異なる
 //原作者が意図しているのとほぼ同じ動作になってると思うけど…。
    WCHAR *pOrgStr = str;
    //wcstok_internal によって書き換えられてしまうためコピーを作成
    WCHAR *pCpyStr = _wcsdup(pOrgStr);//コピーを作成
    WCHAR *pszNext;
    WCHAR delimiter;
    WCHAR *pszzSuffix = NULL;//１文字目を大文字に変換しない単語のリスト
    int i;
    if(bUseSuffix){//1文字目を大文字に変換しない語のリストを作成
    //, で区切られた文字列を \0 で区切られた文字列(\0\0で終了)に置換
        int len = wcslen(suffixs);
        pszzSuffix = (WCHAR*)malloc((len+2)*sizeof(WCHAR));//2=\0\0
        i = 0;
        while(suffixs[i]){
            if(suffixs[i] != L','){
                pszzSuffix[i] = suffixs[i];
            }
            else{
                pszzSuffix[i] = 0;
            }
            i++;
        }
        pszzSuffix[i] = pszzSuffix[i+1] = 0;//末尾を\0\0に
    }
    BOOL  bFirst = TRUE;
    WCHAR *p = wcstok_internal(pCpyStr, word_delimiters, &pszNext, &delimiter);
    while(p){
        if(!pszzSuffix || bFirst){
            if(*p){
                pOrgStr[p - pCpyStr] = towupper(*p);
                bFirst = FALSE;
            }
        }
        else if(*p){//1文字目を大文字に変換するかどうか調べる
            WCHAR *suffix = pszzSuffix;
            while(*suffix){
                if(wcscmp(p, suffix) == 0){
                    break;//変換しない単語のリストに該当
                }
                suffix += wcslen(suffix) + 1;
            }
            if(!*suffix){
                pOrgStr[p - pCpyStr] = towupper(*p);
            }
        }
        if(pszzSuffix && !bFirst){
        //(以下の文字に続く単語は文頭とする)の文字で区切られているか調べる
            //区切り文字が suffixs に含まれない場合
            WCHAR *last = p;//最後の文字
            if(*last){
                while(last[1]){
                    last++;
                }
                i = 0;
                while(separators[i]){
                    if(separators[i++] == *last){
                        bFirst = TRUE;
                        break;
                    }
                }
            }
            //区切り文字が suffixs に含まれる場合
            if(!bFirst){
                i = 0;
                while(separators[i]){//
                    if(separators[i++] == delimiter){
                        bFirst = TRUE;
                        break;
                    }
                }
            }
        }
        p = wcstok_internal(pszNext, word_delimiters, &pszNext, &delimiter);
    }
    if(pszzSuffix){
        free(pszzSuffix);
    }
    free(pCpyStr);
}

void conv_fixed_upper_lower(TCHAR *str, CStringArray& fixedWords) /* STEP 040 */
{//大文字小文字固定 UNICODE 版 by Kobarin
 //単語の区切り文字がオリジナルと異なる(オリジナルは英数字以外?)
    int nCount = fixedWords.GetCount();
    if(nCount == 0){
        return;
    }
    int i;
    WCHAR *pszCpy = _wcsdup(str);//wcstok_internal によって書き換えられるのでコピー作成
    WCHAR *pszNext;
    WCHAR *p;
    WCHAR delimiter;
    p = wcstok_internal(pszCpy, word_delimiters, &pszNext, &delimiter);
    while(p){
        for(i = 0; i < nCount; i++){
            if(_wcsicmp(p, fixedWords[i]) == 0){
                memcpy(&str[p-pszCpy], fixedWords[i], fixedWords[i].GetLength()*sizeof(WCHAR));
                break;
            }
        }
        p = wcstok_internal(pszNext, word_delimiters, &pszNext, &delimiter);
    }
    free(pszCpy);
}
#endif

WCHAR* conv_ansi_to_utf16(const char *str_ansi, int size)
{//ansi => utf16
    int len_utf16 = MultiByteToWideChar(CP_ACP, 0, str_ansi, size, 0, 0);
    WCHAR *str_utf16 = (WCHAR*)malloc((len_utf16+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, str_ansi, size, str_utf16, len_utf16);
    str_utf16[len_utf16] = 0;
    return str_utf16;
}

char* conv_utf16_to_ansi(const WCHAR *str_utf16, int size)
{//utf16 => ansi
    int len_ansi = WideCharToMultiByte(CP_ACP, 0, str_utf16, size, 0, 0, NULL, NULL);
    char *str_ansi = (char*)malloc(len_ansi+1);
    WideCharToMultiByte(CP_ACP, 0, str_utf16, size, str_ansi, len_ansi, NULL, NULL);
    str_ansi[len_ansi] = 0;
    return str_ansi;
}

WCHAR* conv_utf8_to_utf16(const char *str_utf8, int size)
{//utf8 => utf16
    int len_utf16 = MultiByteToWideChar(CP_UTF8, 0, str_utf8, size, 0, 0);
    WCHAR *str_utf16 = (WCHAR*)malloc((len_utf16+1)*sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str_utf8, size, str_utf16, len_utf16);
    return str_utf16;
}

char* conv_utf16_to_utf8(const WCHAR *str_utf16, int size)
{//utf16 => utf8 
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, str_utf16, size, 0, 0, NULL, NULL);
    char *str_utf8 = (char*)malloc(utf8_len+1);
    WideCharToMultiByte(CP_UTF8, 0, str_utf16, size, str_utf8, utf8_len, NULL, NULL);
    str_utf8[utf8_len] = 0;
    return str_utf8;
}

char* conv_ansi_to_utf8(const char *str_ansi, int size)
{//ansi => utf8
    WCHAR *str_utf16 = conv_ansi_to_utf16(str_ansi, size);
    char *str_utf8 = conv_utf16_to_ansi(str_utf16);
    free(str_utf16);
    return str_utf8;
}

char* conv_utf8_to_ansi(const char *str_utf8, int size)
{//utf8 => ansi
    WCHAR *str_utf16 = conv_utf8_to_utf16(str_utf8, size);
    char *str_ansi = conv_utf16_to_ansi(str_utf16);
    free(str_utf16);
    return str_ansi;
}

static void le2be(WCHAR *dst)
{//リトルエンディアン/ビッグエンディアン入れ替え
    while(*dst){
        BYTE *p = (BYTE*)dst;
        BYTE tmp = p[0];
        p[0] = p[1];
        p[1] = tmp;
        dst++;
    }
}
TCHAR* conv_data_to_tstr(const BYTE *data, DWORD dwBufferSize, void **ppFree)
{// BOM を自動判別して TCHAR* 変換
 // *pFree != NULL のときは *pFree を呼び出し側で free する
    enum{
        CODE_ANSI = 0, CODE_UTF16LE, CODE_UTF16BE, CODE_UTF8
    };
    int nCode = CODE_ANSI;
    *ppFree = NULL;
    //BOM 判別
    if(dwBufferSize >= 2){
        if(memcmp(data, "\xFF\xFE", 2) == 0){
            nCode = CODE_UTF16LE;
            data += 2;
            dwBufferSize -= 2;
        }
        else if(memcmp(data, "\xFE\xFF", 2) == 0){
            nCode = CODE_UTF16BE;
            data += 2;
            dwBufferSize -= 2;
        }
        else if(dwBufferSize >= 3 && memcmp(data, "\xEF\xBB\xBF", 3) == 0){
            nCode = CODE_UTF8;
            data += 3;
            dwBufferSize -= 3;
        }
    }
#ifdef _UNICODE
    WCHAR *str_utf16;
    if(nCode == CODE_ANSI){
        str_utf16 = conv_ansi_to_utf16((const char*)data, dwBufferSize);
    }
    else if(nCode == CODE_UTF8){
        str_utf16 = conv_utf8_to_utf16((const char*)data, dwBufferSize);
    }
    else{
        str_utf16 = (WCHAR*)data;
        if(dwBufferSize >= sizeof(WCHAR) && 
           str_utf16[dwBufferSize / sizeof(WCHAR) - 1] == 0){//終端が 0
            if(nCode != CODE_UTF16BE){
            //UTF16BE の場合は変換が必要(第1引数を const にしなければここで変換しても良いが…)
                return str_utf16;
            }
        }
        //0 で終わっていない or 変換が必要
        str_utf16 = (WCHAR*)malloc(dwBufferSize+sizeof(WCHAR));
        if(dwBufferSize >= sizeof(WCHAR)){
            memcpy(str_utf16, data, dwBufferSize);
            str_utf16[dwBufferSize / sizeof(WCHAR)-1] = 0;
            if(nCode == CODE_UTF16BE){
                le2be(str_utf16);
            }
        }
        else{
            str_utf16[0] = 0;
        }
    }
    *ppFree= str_utf16;
    return str_utf16;
#else
    char *str_ansi;
    if(nCode == CODE_ANSI){
        str_ansi = (char*)malloc(dwBufferSize + 1);
        memcpy(str_ansi, data, dwBufferSize);
        str_ansi[dwBufferSize] = 0;
        //マルチバイト2バイト目で切れているかどうかの確認
        DWORD i = 0;
        while(str_ansi[i]){
            if(IsDBCSLeadByte((BYTE)str_ansi[i])){
                i++;
                if(!str_ansi[i]){
                    str_ansi[i-1] = 0;
                    break;
                }
            }
            i++;
        }
    }
    else if(nCode == CODE_UTF8){
        str_ansi = conv_utf8_to_ansi((const char*)data);
    }
    else{
        WCHAR *str_utf16 = (WCHAR*)malloc(dwBufferSize+sizeof(WCHAR));
        if(dwBufferSize >= 2){
            memcpy(str_utf16, data, dwBufferSize);
            str_utf16[dwBufferSize / sizeof(WCHAR)-1] = 0;
            if(nCode == CODE_UTF16BE){
                le2be(str_utf16);
            }
        }
        else{
            str_utf16[0] = 0;
        }
        str_ansi = conv_utf16_to_ansi(str_utf16);
        free(str_utf16);
    }
    *ppFree = str_ansi;
    return str_ansi;
#endif
}