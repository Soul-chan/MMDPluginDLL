// DispPlayingFrame.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include <atltime.h>
#include <deque>
#include <map>
#include <windows.h>
#include <WindowsX.h>
#include "../MMDPlugin/mmd_plugin.h"
using namespace mmp;

#include "../Common/Common.h"


//////////////////////////////////////////////////////////////////////
class DispPlayingFramePlugin : public MMDPluginDLL3, public CMmdCtrls, public Singleton<DispPlayingFramePlugin>
{
private:
	MMDMainData*									m_mmdDataP;
	bool											m_bShowWindow;		// 初回の WM_SHOWWINDOW 判定用
	HWND											m_myTextH;			// フレーム表示用に自作するテキスト
	HFONT											m_myTextFontH;		// フレーム表示用テキストのフォント
	HBRUSH											m_myTextBrushH;		// フレーム表示用に自作するテキストの背景用ブラシ
	int												m_is_playing;		// 再生中かどうか 0:停止中 1:再生中変化検知用コピー
	int												m_now_frame;		// 現在のフレーム変化検知用コピー
	float											m_play_sec;			// 再生中の先頭からの秒数変化検知用コピー

public:
	const char* getPluginTitle() const override { return "DispPlayingFrame"; }

	DispPlayingFramePlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
		, m_myTextH(nullptr)
		, m_myTextFontH(nullptr)
		, m_is_playing(true)
		, m_now_frame(INT_MAX)
		, m_play_sec(FLT_MAX)
	{
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
	}

	void stop() override
	{
		// タイマーOFF
		KillTimer(getHWND(), (UINT_PTR)this);

		// フォントをデフォルトに戻す
		if (m_myTextH)
		{
			SendMessage(m_myTextH, WM_SETFONT, (WPARAM)nullptr, TRUE);
		}
		// フォントの解放
		if (m_myTextFontH)
		{
			DeleteObject(m_myTextFontH);
			m_myTextFontH = nullptr;
		}
	}

	void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
	{
	}

	void MouseProc(WPARAM wParam, MOUSEHOOKSTRUCT* lParam) override
	{
	}

	void MsgProc(int code, MSG* param) override
	{
	}

	void GetMsgProc(int code, MSG* param) override
	{
	}

	// MMD側の呼び出しが制御できるプロシージャ
	// false : MMDのプロシージャも呼ばれます。LRESULTは無視されます
	// true  : MMDやその他のプラグインのプロシージャは呼ばれません。LRESULTが全体のプロシージャの戻り値として返されます。
	std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return{ false, 0 };
	}

	void WndProc(CWPSTRUCT* param) override
	{
		// 初回かどうか判定
		if (!m_bShowWindow)
		{
			if (param->message == WM_SHOWWINDOW)
			{
				if (param->hwnd == getHWND())
				{
					_beforeShowWindow();
					m_bShowWindow = true;
				}
			}

			// 初期化前は何もしない
			return;
		}

		if (param->message == WM_WINDOWPOSCHANGED)
		{
			auto posP = (WINDOWPOS*)param->lParam;
			// 移動なら
			if ((posP->flags & SWP_NOMOVE) == 0)
			{
				// MMDの「距離」のエディットボックスが動いたら自作のボタンも追従する
				if (param->hwnd == m_lenEditH)
				{
					// 「別窓」で親が変わっていた場合、同じウィンドウに移動する
					HWND parentH = GetParent(m_lenEditH);
					if (parentH != GetParent(m_myTextH))
					{
						SetParent(m_myTextH, parentH);
					}

					SetWindowPos(m_myTextH, HWND_NOTOPMOST, posP->x + posP->cx + 10, posP->y - 6, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

					_frameCheck(true);
				}
			}
		}

		if (param->message == WM_CTLCOLORSTATIC ||
			param->message == WM_CTLCOLOREDIT)
		{
			if (param->hwnd == getHWND())
			{
				HDC hDC = (HDC)param->wParam;
				HWND hCtrl = (HWND)param->lParam;
				if (hCtrl == m_myTextH)
				{
					_dbgPrint("HIT!!!!");
					SetBkMode(hDC, TRANSPARENT);		// 背景を塗りつぶし
					SetTextColor(hDC, RGB(255, 0, 0));	// テキストの色
					SetBkColor(hDC, RGB(192, 192, 192));	// テキストが書かれている部分のテキストの背景の色
				//	return{ true, (LRESULT)COLOR_WINDOW };	// テキストが書かれていない部分の背景の色
				}
			}
		}
	}

private:
	// 初回の WM_SHOWWINDOW 前に呼ばれる
	// この時点ではボタン等のコントロールが出来ているはず
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hMmd = getHWND();
		TCHAR *defaultTxtP = _T("000000 (00:00.00)");

		// コントロールハンドルの初期化
		InitCtrl();

		// 「距離」のエディットボックスと同じフォントにしておく
		LOGFONT lfont = { 0 };
		HFONT fontH = (HFONT)SendMessage(m_lenEditH, WM_GETFONT, 0, 0);
		GetObject(fontH, sizeof(LOGFONT), &lfont);
		lfont.lfHeight = -24;
		lfont.lfWidth = 0;
		m_myTextFontH = CreateFontIndirect(&lfont);

		// デフォルト文字数での文字幅を取得
		auto screenHdc = GetDC(nullptr);
		HDC hdc = CreateCompatibleDC(screenHdc);
		SelectObject(hdc, m_myTextFontH);
		SIZE txtSize = { 0 };
		GetTextExtentPoint32(hdc, defaultTxtP, lstrlen(defaultTxtP), &txtSize);
		DeleteObject(hdc);
		ReleaseDC(nullptr, screenHdc);

		// スタティックテキストを作る
		m_myTextH = CreateWindow(_T("EDIT"), defaultTxtP, WS_VISIBLE | WS_CHILD | ES_READONLY | ES_RIGHT,
								0, 0, txtSize.cx + 16, 24, hMmd, (HMENU)(UINT_PTR)MY_PLAY_FRAME_STATIC_CTRL_ID, hInstance, NULL);
		EnableWindow(m_myTextH, false);

		SendMessage(m_myTextH, WM_SETFONT, (WPARAM)m_myTextFontH, TRUE);

		// タイマー開始
		// メッセージをポストするやり方だと、何故かは知らないがMMD操作中にはWM_TIMERが送られてこない
		// (タイトルバーとかを操作すると遅れて届き始める)
		// それじゃ役に立たないので、コールバックで処理する
		SetTimer(getHWND(), (UINT_PTR)this, 10, [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			DispPlayingFramePlugin * thisP = (DispPlayingFramePlugin *)idEvent;
			thisP->_frameCheck(false);
		});
	}

	// 現在のフレームが変わっていたら再描画する
	void _frameCheck(bool bForce)
	{
		if (bForce ||
			m_is_playing != m_mmdDataP->is_playing ||
			m_now_frame  != m_mmdDataP->now_frame ||
			m_play_sec   != m_mmdDataP->play_sec)
		{
			int frame;
			frame = m_mmdDataP->is_playing
				? (int)(m_mmdDataP->play_sec * 30)
				: m_mmdDataP->now_frame;

			int f = frame;
		//	int h = f / (60 * 60 * 30);	f -= h * (60 * 60 * 30);
			int m = f / (60 * 30);		f -= m * (60 * 30);
			int s = f / (30);			f -= s * (30);

			// 表示
			TCHAR txt[64] = _T("");
			_sntprintf(txt, ARRAYSIZE(txt), _T("%5d (%02d:%02d.%02d)"), frame, m, s, f);
			SetWindowText(m_myTextH, txt);

			m_is_playing = m_mmdDataP->is_playing;
			m_now_frame  = m_mmdDataP->now_frame;
			m_play_sec   = m_mmdDataP->play_sec;
		}
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMDのバージョンが 9.31 ではありません。\nDispPlayingFrameは Ver.9.31 以外では正常に作動しません。"), _T("DispPlayingFrame"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return DispPlayingFramePlugin::GetInstance();
}