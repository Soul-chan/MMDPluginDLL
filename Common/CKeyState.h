#include "stdafx.h"
#include <string>
#include <algorithm>

#pragma once

//////////////////////////////////////////////////////////////////////
// キー入力を管理するクラス
class CKeyState
{
	bool		m_bCtrl;
	bool		m_bShift;
	bool		m_bAlt;
	char		m_keyCode;
	int32_t		m_repCnt;		// リピート用カウンタ
	int32_t		m_repInterval;	// リピート用間隔
public:
	CKeyState()
		: m_bCtrl(false)
		, m_bShift(false)
		, m_bAlt(false)
		, m_keyCode(0)
		, m_repCnt(0)
		, m_repInterval(0)
	{}

	// "W" "Ctrl+X" "Shift+Y" "Ctrl + Alt + Z" 等の形で指定する
	CKeyState(std::wstring str)
		: CKeyState()
	{
		SetKeyCode(str);
	}

	void SetKeyCode(std::wstring str)
	{
		// 大文字に変換
		std::transform(str.cbegin(), str.cend(), str.begin(), toupper);

		// CTRL
		size_t pos = str.find(L"CTRL");
		if (pos != std::wstring::npos)
		{
			m_bCtrl = true;
			str = str.erase(pos, 4);
		}
		// SHIFT
		pos = str.find(L"SHIFT");
		if (pos != std::wstring::npos)
		{
			m_bShift = true;
			str = str.erase(pos, 5);
		}
		// ALT
		pos = str.find(L"ALT");
		if (pos != std::wstring::npos)
		{
			m_bAlt = true;
			str = str.erase(pos, 3);
		}

		// +とスペースを取り除く
		str.erase(std::remove(str.begin(), str.end(), '+'), str.end());
		str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

		// 最初に出てきた文字のキーコードで判定する
		if (!str.empty())
		{
			m_keyCode = (char)str[0];
		}
	}

	// ボタン判定
	// vKey		仮想キーコード
	// lParam	キーストロークメッセージの情報
	bool IsBtn(WPARAM vKey, LPARAM lParam)
	{
		if (vKey == m_keyCode)
		{
			if (m_bCtrl  != (GetAsyncKeyState(VK_CONTROL) < 0))	{ return false; }
			if (m_bShift != (GetAsyncKeyState(VK_SHIFT)   < 0))	{ return false; }
			if (m_bAlt   != (GetAsyncKeyState(VK_MENU)    < 0))	{ return false; }
			return true;
		}
		return false;
	}

	// トリガ判定
	// vKey		仮想キーコード
	// lParam	キーストロークメッセージの情報
	bool IsTrg(WPARAM vKey, LPARAM lParam)
	{
		if ((lParam & 0x40000000) == 0)	// 直前にキーが押されていなければ
		{
			return IsBtn(vKey, lParam);
		}
		return false;
	}

	// アップ判定
	// vKey		仮想キーコード
	// lParam	キーストロークメッセージの情報
	bool IsUp(WPARAM vKey, LPARAM lParam)
	{
		if (lParam & 0x80000000)		// キーが離されているなら
		{
			return IsBtn(vKey, lParam);
		}
		return false;
	}

	// リピート判定
	// vKey		仮想キーコード
	// lParam	キーストロークメッセージの情報
	// start	開始時リピート間隔
	// end		最終時リピート間隔
	// step		リピート間隔のステップ
	bool IsRepeat(WPARAM vKey, LPARAM lParam, unsigned int start, unsigned int end, unsigned int step)
	{
		// 初回にカウンタクリア
		if ((lParam & 0x40000000) == 0) { m_repCnt = 0; m_repInterval = start; }
		
		bool bRepeat = false;
		if (IsBtn(vKey, lParam))
		{
			// カウンタが0になったら判定ON
			if (m_repCnt <= 0)
			{
				bRepeat = true;
				m_repCnt = m_repInterval;
				if (start > end)	{ m_repInterval = std::max(m_repInterval - (int)step, (int)end); }
				else				{ m_repInterval = std::min(m_repInterval + (int)step, (int)end); }
				
			}
			m_repCnt--;
		}
		return bRepeat;
	}
};
