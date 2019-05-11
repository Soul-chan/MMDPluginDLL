#include "stdafx.h"
#include <windows.h>

#pragma once

//////////////////////////////////////////////////////////////////////
// PostMessageのフッククラス
class CPostMsgHook
{
	static CPostMsgHook								*m_pThis;
	HHOOK											m_postHookH;

public:
	CPostMsgHook() : m_postHookH(nullptr) {};
	~CPostMsgHook() {};

	// start() から呼び出す
	void HookStart(HMODULE module)
	{
		m_pThis = this;
		// PostMessageのフック
		m_postHookH = SetWindowsHookEx(WH_GETMESSAGE, PostMsgHookProc, module, 0);
	}

	// stop() から呼び出す
	void HookStop()
	{
		if (m_postHookH)
		{
			UnhookWindowsHookEx(m_postHookH);
			m_postHookH = nullptr;
		}
	}

private:
	// PostMessgeのフック処理
	static LRESULT CALLBACK PostMsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode < 0) { return CallNextHookEx(NULL, nCode, wParam, lParam); }
		if (m_pThis) { m_pThis->PostMsgProc(nCode, (MSG *)lParam); }
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}

	// メッセージ処理を実装する
	virtual void PostMsgProc(int code, MSG* param) {}
};
CPostMsgHook* CPostMsgHook::m_pThis;
