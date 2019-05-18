#include "stdafx.h"
#include <windows.h>
#include <windowsX.h>

#pragma once

//////////////////////////////////////////////////////////////////////
// MMDのコントロールを管理するクラス
class CMmdCtrls
{
protected:
	HWND											m_frameNoEditH;		// MMDの「フレーム番号」のエディットボックスのハンドル
	HWND											m_hscrollH;			// MMDのタイムラインの水平スクロールバーのハンドル
	HWND											m_curveComboH;		// MMDの「補間曲線操作」パネルのコンボボックスのハンドル
	HWND											m_modelComboH;		// MMDの「モデル操作」パネルのコンボボックスのハンドル
	HWND											m_accComboH;		// MMDの「アクセサリ操作」パネルのコンボボックスのハンドル
	HWND											m_undoBtnH;			// MMDの「元に戻す」ボタン
	HWND											m_redoBtnH;			// MMDの「やり直し」ボタン
	HWND											m_lenEditH;			// MMDの「距離」のエディットボックス

public:
	CMmdCtrls()
		: m_frameNoEditH(nullptr)
		, m_hscrollH(nullptr)
		, m_curveComboH(nullptr)
		, m_modelComboH(nullptr)
		, m_accComboH(nullptr)
		, m_undoBtnH(nullptr)
		, m_redoBtnH(nullptr)
		, m_lenEditH(nullptr)
	{}
	~CMmdCtrls() {}

	// コントロールハンドルの初期化
	void InitCtrl()
	{
		HWND hMmd = getHWND();

		// 「フレーム番号」のエディットボックスのハンドルを取得する
		m_frameNoEditH = GetDlgItem(hMmd, FRAME_NO_EDIT_CTRL_ID);
		// タイムラインの水平スクロールバーのハンドルを取得する
		m_hscrollH = GetDlgItem(hMmd, HSCROLL_CTRL_ID);
		// 「補間曲線操作」パネルのコンボボックスのハンドルを取得する
		m_curveComboH = GetDlgItem(hMmd, CURVE_COMBO_CTRL_ID);
		// 「モデル操作」パネルのコンボボックスのハンドルを取得する
		m_modelComboH = GetDlgItem(hMmd, MODEL_COMBO_CTRL_ID);
		// 「アクセサリ操作」パネルのコンボボックスのハンドルを取得する
		m_accComboH = GetDlgItem(hMmd, ACCESSORY_COMBO_CTRL_ID);
		// MMDの「元に戻す」ボタンと「やり直し」ボタンを取得する
		m_undoBtnH = GetDlgItem(hMmd, UNDO_BTN_CTRL_ID);
		m_redoBtnH = GetDlgItem(hMmd, REDO_BTN_CTRL_ID);
		// 「距離」のエディットボックスのハンドルを取得する
		m_lenEditH = GetDlgItem(hMmd, LENGTH_EDIT_CTRL_ID);

	}

	// タイムラインと補間曲線の再描画
	void Repaint()
	{
		HWND hMmd = getHWND();

		// カメラ等の再描画
		// フレーム番号を入力して再描画させる
		SendMessage(m_frameNoEditH, WM_KEYDOWN, VK_RETURN, 0);

		// タイムラインの再描画
		// スクロールバーに現在位置を設定することで再描画させる
		SCROLLINFO info = { sizeof(SCROLLINFO), SIF_POS };
		GetScrollInfo(m_hscrollH, SB_CTL, &info);
		SendMessage(hMmd, WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, info.nPos), (LPARAM)m_hscrollH);

		// 補間曲線の再描画
		// 補間曲線のコンボボックスを同じ値で設定
		ComboBox_SetCurSel(m_curveComboH, ComboBox_GetCurSel(m_curveComboH));
		SendMessage(hMmd, WM_COMMAND, MAKEWPARAM(CURVE_COMBO_CTRL_ID, CBN_SELCHANGE), (LPARAM)m_curveComboH);
	}

	// 「フレーム番号」のエディットボックスのコントロールID
	static constexpr int FRAME_NO_EDIT_CTRL_ID = 0x1A1;
	// タイムラインの水平スクロールバーのコントロールID
	static constexpr int HSCROLL_CTRL_ID = 0x1AC;
	// 「補間曲線操作」パネルのコンボボックスのコントロールID
	static constexpr int CURVE_COMBO_CTRL_ID = 0x1B1;
	// 「モデル操作」パネルのコンボボックスのコントロールID
	static constexpr int MODEL_COMBO_CTRL_ID = 0x1B4;
	// 「カメラ操作」パネルの「登録」ボタンのコントロールID
	static constexpr int CAMERA_BTN_CTRL_ID = 0x1C4;
	// 「照明操作」パネルの「登録」ボタンのコントロールID
	static constexpr int LIGHT_BTN_CTRL_ID = 0x1D4;
	// 「セルフ影操作」パネルの「登録」ボタンのコントロールID
	static constexpr int SELF_SHADOW_BTN_CTRL_ID = 0x235;
	// 「アクセサリ操作」パネルの「登録」ボタンのコントロールID
	static constexpr int ACCESSORY_BTN_CTRL_ID = 0x1E7;
	// タイムラインの「ペースト」ボタンのコントロールID
	static constexpr int PASTE_BTN_CTRL_ID = 0x1A5;
	// タイムラインの「削除」ボタンのコントロールID
	static constexpr int DELETE_BTN_CTRL_ID = 0x1A7;
	// 「アクセサリ操作」パネルのコンボボックスのコントロールID
	static constexpr int ACCESSORY_COMBO_CTRL_ID = 0x1D7;
	// 「元に戻す」ボタンと「やり直し」ボタンのコントロールID
	static constexpr int UNDO_BTN_CTRL_ID = 0x190;
	static constexpr int REDO_BTN_CTRL_ID = 0x191;
	// 「距離」のボタンのコントロールID
	static constexpr int LENGTH_BTN_CTRL_ID = 0x21F;
	// 「距離」のエディットボックスのコントロールID
	static constexpr int LENGTH_EDIT_CTRL_ID = 0x226;
};

