#include "stdafx.h"
#include <windows.h>
#include <functional>

#pragma once

//////////////////////////////////////////////////////////////////////
// メニューを追加する為のクラス
class CMenu
{
	std::wstring				m_jpTitle;		// 日本語のメニュー
	std::wstring				m_enTitle;		// 英語のメニュー
	std::function<void(CMenu*)>	m_onExec;		// 選択時の実行関数
	std::function<void(CMenu*)>	m_onDisp;		// 表示されようとした時の実行関数
	HMENU						m_subMenuH;		// MMDのサブメニュー
	UINT						m_id;			// メニューのID

												// 日本語/英語の切り替えメニューID
	static constexpr int JP_EN_MODE_CHANGE_MENU_ID = 0x104;

public:
	CMenu() : m_subMenuH(nullptr), m_id(-1) {}
	virtual ~CMenu() {}

	// 取得関数
	UINT GetId() { return m_id; }
	HMENU GetSubMenuH() { return m_subMenuH; }

	// MMDのメニュー
	enum class MmdMenu : int
	{
		File = 0,		// ファイル(F)
		Edit,			// 編集(D)
		View,			// 表示(V)
		Background,		// 背景(B)
		FacialExp,		// 表情(M)
		Physics,		// 物理演算(P)
		MotionCap,		// ﾓｰｼｮﾝｷｬﾌﾟﾁｬ(K)
		Help,			// ヘルプ(H)
	};

	// MMDのメニューに項目を追加する
	void Create(MmdMenu addPos, const std::wstring &jpTitle, const std::wstring &enTitle, const std::function<void(CMenu*)> &onExec, const std::function<void(CMenu*)> &onDisp = nullptr)
	{
		m_jpTitle = jpTitle;
		m_enTitle = enTitle;
		m_onExec = onExec;
		m_onDisp = onDisp;

		// 既に作成済みの場合は、何もする必要も無いが、一応文字列と実行関数だけ差し替えてみる
		if (m_id != -1) { return; }

		m_subMenuH = GetSubMenu(GetMenu(getHWND()), static_cast<int>(addPos));
		if (m_subMenuH)
		{
			auto mmdDataP = getMMDMainData();
			MENUITEMINFO mii;

			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING;
			mii.fType = MFT_STRING;
			mii.dwTypeData = const_cast<LPWSTR>(mmdDataP->is_english_mode ? m_enTitle.c_str() : m_jpTitle.c_str());
			mii.wID = m_id = createWM_APP_ID();

			InsertMenuItem(m_subMenuH, mii.wID, FALSE, &mii);
		}
	}

	// セパレーター追加
	static void AddSeparator(MmdMenu addPos)
	{
		HMENU subMenuH = GetSubMenu(GetMenu(getHWND()), static_cast<int>(addPos));
		if (subMenuH)
		{
			InsertMenu(subMenuH, 0xffffffff, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		}
	}

	// メニューの有効/無効を切り替え
	void SetEnable(bool bEnable)
	{
		if (m_subMenuH)
		{
			MENUITEMINFO mii;

			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_STATE;

			GetMenuItemInfo(m_subMenuH, m_id, FALSE, &mii);
			mii.fState &= ~(MFS_ENABLED | MFS_DISABLED);
			mii.fState |= (bEnable) ? MFS_ENABLED : MFS_DISABLED;
			SetMenuItemInfo(m_subMenuH, m_id, FALSE, &mii);
		}
	}

	// ポストメッセージ処理
	void OnPostMsg(MSG* param)
	{
		if (m_id == -1) { return; }

		switch (param->message)
		{
			case WM_COMMAND:
				auto id = LOWORD(param->wParam);
				if (id == m_id)
				{
					if (m_onExec) { m_onExec(this); }
				}
				else if (id == JP_EN_MODE_CHANGE_MENU_ID)
				{
					if (m_subMenuH)
					{
						// ここに来る時点での is_english_mode は変更前の値
						auto mmdDataP = getMMDMainData();
						MENUITEMINFO mii;

						mii.cbSize = sizeof(MENUITEMINFO);
						mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING;
						mii.fType = MFT_STRING;
						mii.dwTypeData = const_cast<LPWSTR>(mmdDataP->is_english_mode ? m_jpTitle.c_str() : m_enTitle.c_str());
						mii.wID = m_id;

						SetMenuItemInfo(m_subMenuH, mii.wID, FALSE, &mii);
					}
				}
				break;
		}
	}

	// メッセージ処理
	void OnWndProc(CWPSTRUCT* param)
	{
		if (m_id == -1) { return; }

		switch (param->message)
		{
			case WM_INITMENUPOPUP:
				HMENU menuH = (HMENU)param->wParam;
				if (m_subMenuH &&
					m_subMenuH == menuH)
				{
					if (m_onDisp) { m_onDisp(this); }
				}
				break;
		}
	}
};
