// SetCameraFollowBone.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include <atltime.h>
#include <deque>
#include <map>
#include <windows.h>
#include <WindowsX.h>
#include "../MMDPlugin/mmd_plugin.h"
using namespace mmp;

#include "../Common/Common.h"

extern HMODULE g_module;

#define ComboBox_Clear(hwndCtl)						while (ComboBox_GetCount(hwndCtl) != 0) { ComboBox_DeleteString((hwndCtl), 0); }
#define ComboBox_AddStringA(hwndCtl, lpsz)			((int)(DWORD)::SendMessageA((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBox_SetDroppedWidth(hwndCtl, len)		((int)(DWORD)::SendMessage((hwndCtl), CB_SETDROPPEDWIDTH, (WPARAM)(len), 0L))
//////////////////////////////////////////////////////////////////////
class SetCameraFollowBonePlugin : public MMDPluginDLL3, public Singleton<SetCameraFollowBonePlugin>, public CPostMsgHook
{
private:
	MMDMainData*									m_mmdDataP;
	bool											m_bShowWindow;		// 初回の WM_SHOWWINDOW 判定用
	CKeyState										m_testKey;			// テストのキー
	CMenu											m_menu;				// メニュー
	std::vector<int32_t>							m_modelIdx;			// モデルコンボボックスに表示するモデルのインデックスリスト
	std::vector<int32_t>							m_boneIdx;			// ボーンコンボボックスに表示するボーンのインデックスリスト

public:
	const char* getPluginTitle() const override { return "SetCameraFollowBone"; }

	SetCameraFollowBonePlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
	{
		m_testKey.SetKeyCode(L"C");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
		// フック開始
		HookStart(g_module);

		m_menu.AddSeparator(CMenu::MmdMenu::Edit);
		m_menu.Create(CMenu::MmdMenu::Edit, _T("ｶﾒﾗのﾎﾞｰﾝ追従設定(&F)"), _T("set camera follow bone(&F)")
					, [this](CMenu*) { OnMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(m_mmdDataP->is_camera_edit_mode); } ); // カメラ編集モード時のみ選べる様にする
	}

	void stop() override
	{
		// フック終了
		HookStop();
	}

	void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
	{
#ifdef USE_DEBUG
		if (m_testKey.IsTrg(wParam, lParam))
		{
			auto d = getMMDMainData();
			_dbgPrint("-----------------------------");
			CUnknownChecker::Print();
			_dbgPrint("-----------------------------");
		}
#endif // USE_DEBUG

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

		m_menu.OnWndProc(param);
	}

	void PostMsgProc(int code, MSG* param) override
	{
		m_menu.OnPostMsg(param);
	}

	// メニューが実行されたとき
	void OnMenuExec()
	{
		DialogBox( (HINSTANCE)g_module,
			MAKEINTRESOURCE(m_mmdDataP->is_english_mode ? IDD_DIALOG_EN : IDD_DIALOG_JP),
			getHWND(),
			(DLGPROC)SDlgWndProc);
	}

	// ダイアログ用ウィンドウプロシージャ
	static BOOL CALLBACK SDlgWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		return GetInstance()->DlgWndProc(hWnd, iMsg, wParam, lParam);
	}
	BOOL DlgWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (iMsg)
		{
			case WM_INITDIALOG:
				//ダイアログボックスが生成されたとき
				{
					auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
					if (modelCombo)
					{
						// モデルのインデックスリストを作成
						m_modelIdx.clear();
						m_boneIdx.clear();
						for (int mIdx = 0; mIdx<ARRAYSIZE(m_mmdDataP->model_data); mIdx++)
						{
							auto modelP = m_mmdDataP->model_data[mIdx];
							if (modelP)
							{
								m_modelIdx.push_back(mIdx);
							}
						}

						// 全項目削除
						ComboBox_Clear(modelCombo);

						// モデル名をコンボボックスに追加
						ComboBox_AddStringA(modelCombo, m_mmdDataP->is_english_mode ? "non" : "なし");

						if (!m_modelIdx.empty())
						{
							int32_t maxLen = 20;

							// モデル描画順でソート
							std::sort(m_modelIdx.begin(), m_modelIdx.end(), [this](const int32_t l, const int32_t r)
							{
								return m_mmdDataP->model_data[l]->render_order < m_mmdDataP->model_data[r]->render_order;
							});

							// モデル名をコンボボックスに追加
							for (auto mIdx : m_modelIdx)
							{
								auto modelP = m_mmdDataP->model_data[mIdx];
								char *nameP = m_mmdDataP->is_english_mode ? modelP->name_en : modelP->name_jp;
								ComboBox_AddStringA(modelCombo, nameP);

								// 最大の文字数を調べておく
								int32_t len = static_cast<int32_t>(strlen(nameP));
								if (len > maxLen) { maxLen = len; }
							}
							// ドロップダウンした時の幅を調節 文字数で適当に…
							ComboBox_SetDroppedWidth(modelCombo, 7.5 * maxLen);
						}

						ComboBox_SetCurSel(modelCombo, 0);
					}
				}
				return TRUE;

			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
					case MODEL_COMBO:
						// モデルの選択項目が変更されたとき
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							int		mIdx = -1;

							// 現在のモデルコンボのインデックスを取得
							auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
							if (modelCombo)
							{
								// 0番目は「なし」なので、モデルがあるのは1～
								mIdx = ComboBox_GetCurSel(modelCombo) - 1;
							}

							auto boneCombo = GetDlgItem(hWnd, BONE_COMBO);
							if (boneCombo)
							{
								m_boneIdx.clear();
								// 全項目削除
								ComboBox_Clear(boneCombo);

								if (0 <= mIdx && mIdx < m_modelIdx.size())
								{
									auto modelP = m_mmdDataP->model_data[ m_modelIdx[mIdx] ];
									if (modelP)
									{
										// 表示するボーンのリストを作る
										for (int bIdx = 0; bIdx < modelP->bone_count; bIdx++)
										{
											// 「表示」か「IK」がONのボーンが対象
											// IKはLinkのボーンも対象になる
											auto &boneR = modelP->bone_current_data[bIdx];
											if (boneR.flg & BoneCurrentData::BoneFlag::IK)
											{
												_addIfNotExist(m_boneIdx, bIdx);
												// Linkをサーチ
												for (int lIdx = 0; lIdx < boneR.ik_link_count; lIdx++)
												{
													auto link = boneR.ik_link[lIdx].link_bone;
													if (link >= 0)
													{
														_addIfNotExist(m_boneIdx, link);
													}
												}
											}
											else if (boneR.flg & BoneCurrentData::BoneFlag::Visible)
											{
												_addIfNotExist(m_boneIdx, bIdx);
											}
										}

										// 昇順でソート
										std::sort(m_boneIdx.begin(), m_boneIdx.end());

										int32_t maxLen = 20;

										// ボーン名をコンボボックスにセット
										for (auto bIdx : m_boneIdx)
										{
											auto &boneR = modelP->bone_current_data[bIdx];
											// こっちはwchar_t
											wchar_t *nameP = m_mmdDataP->is_english_mode ? boneR.wname_en : boneR.wname_jp;

											ComboBox_AddString(boneCombo, nameP);

											// 最大の文字数を調べておく
											int32_t len = lstrlen(nameP);
											if (len > maxLen) { maxLen = len; }
										}

										// ドロップダウンした時の幅を調節 文字数で適当に…
										ComboBox_SetDroppedWidth(boneCombo, 7.5 * maxLen);
									}
								}
								ComboBox_SetCurSel(boneCombo, 0);
							}
						}
						break;

					case REGIST_BTN:
					{
						int targetModel = -1000;
						int targetBone = -1000;

						// 現在のモデルコンボのインデックスを取得
						auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
						if (modelCombo)
						{
							// 0番目は「なし」なので、モデルがあるのは1～
							auto mIdx = ComboBox_GetCurSel(modelCombo) - 1;
							if (mIdx == -1)
							{
								targetModel = -1; // -1の場合は解除される
								targetBone = 0;
							}
							else if (0 <= mIdx && mIdx < m_modelIdx.size())
							{
								targetModel = m_modelIdx[mIdx];
							}
						}

						auto boneCombo = GetDlgItem(hWnd, BONE_COMBO);
						if (boneCombo)
						{
							auto bIdx = ComboBox_GetCurSel(boneCombo);
							if (0 <= bIdx && bIdx < m_boneIdx.size())
							{
								targetBone = m_boneIdx[bIdx];
							}
						}

						// ボーン追従をセットする
						if (targetModel >= -1 && targetBone >= 0)
						{
							_setFollowBone(targetModel, targetBone);
						}
					}
						/// break; スルーして下へ
					case IDCANCEL:
						m_modelIdx.clear();
						m_boneIdx.clear();
						//モーダルダイアログボックスを破棄
						EndDialog(hWnd, 0);
						break;
				}
				return TRUE;

			default:
				return FALSE;
		}
	}

private:
	// 初回の WM_SHOWWINDOW 前に呼ばれる
	// この時点ではボタン等のコントロールが出来ているはず
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hMmd = getHWND();
	}

	// 選択中のカメラフレームにモデル追従をセットする
	// lookingModel	ボーン追従のモデルのインデックス なしの場合は-1
	// lookingBone	ボーンのインデックス 0～
	void _setFollowBone(int lookingModel, int lookingBone)
	{
		for (int cIdx = 0; cIdx < ARRAYSIZE(m_mmdDataP->camera_key_frame);)
		{
			auto &c = m_mmdDataP->camera_key_frame[cIdx];

			// 選択中なら追従をセットする
			if (c.is_selected)
			{
				c.looking_model_index = lookingModel;
				c.looking_bone_index = lookingBone;
			}

			// 次が無ければ終了
			if (c.next_index == 0) { break; }
			// 次のキーフレームへ
			cIdx = c.next_index;
		}
	}

	void _addIfNotExist(std::vector<int32_t> &vec, int32_t no)
	{
		auto it = std::find(vec.cbegin(), vec.cend(), no);
		if (it == vec.cend()) { vec.push_back(no); } // 無ければ追加
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMDのバージョンが 9.31 ではありません。\nSetCameraFollowBoneは Ver.9.31 以外では正常に作動しません。"), _T("SetCameraFollowBone"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return SetCameraFollowBonePlugin::GetInstance();
}

