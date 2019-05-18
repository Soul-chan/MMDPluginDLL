#include "stdafx.h"
#include <functional>
#include <windows.h>
#include <commctrl.h>
#pragma comment (lib, "comctl32.lib")

#pragma once

//////////////////////////////////////////////////////////////////////
// コントロールをサブクラス化する為のクラス
class CCtrlSubClass
{
	HWND																	m_winH;
	std::function<bool(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, LRESULT&)>	m_proc;

public:
	CCtrlSubClass() : m_winH(nullptr), m_proc(nullptr) {}
	~CCtrlSubClass() { Stop(); }

	// サブクラス化開始
	// proc の戻り値は DefSubclassProc を呼び出さずに関数を戻るかどうか
	// true  : 呼び出さずに LRESULT& を戻り値として返す
	// false : DefSubclassProc() を呼び出してその戻り値を返す
	void Start(HWND targetHwnd, std::function <bool(HWND, UINT, WPARAM, LPARAM, DWORD_PTR, LRESULT&)> proc, DWORD_PTR dwRefData = 0)
	{
		m_winH = targetHwnd;
		m_proc = proc;
		SetWindowSubclass(m_winH, (SUBCLASSPROC)SubClassProc, (UINT_PTR)this, dwRefData);
	}

	// サブクラス化終了
	void Stop()
	{
		if (m_winH)
		{
			RemoveWindowSubclass(m_winH, (SUBCLASSPROC)SubClassProc, (UINT_PTR)this);
			m_winH = nullptr;
			m_proc = nullptr;
		}
	}

	static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		CCtrlSubClass * pThis = reinterpret_cast<CCtrlSubClass *>(uIdSubclass);
		LRESULT lresult = 0;
		if (pThis->m_proc(hWnd, uMsg, wParam, lParam, dwRefData, lresult))
		{
			return lresult;
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
};

