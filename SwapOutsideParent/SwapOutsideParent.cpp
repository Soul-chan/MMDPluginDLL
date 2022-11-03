// SwapOutsideParent.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include <deque>
#include <map>
#include <Tchar.h>
#include <windows.h>
#include <WindowsX.h>
#include "../MMDPlugin/mmd_plugin.h"
using namespace mmp;

#include "../Common/Common.h"

#ifdef _DEBUG
#define	USE_DEBUG
#else
#endif // _DEBUG

extern HMODULE g_module;

#define ComboBox_Clear(hwndCtl)						while (ComboBox_GetCount(hwndCtl) != 0) { ComboBox_DeleteString((hwndCtl), 0); }
#define ComboBox_AddStringA(hwndCtl, lpsz)			((int)(DWORD)::SendMessageA((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBox_SetDroppedWidth(hwndCtl, len)		((int)(DWORD)::SendMessage((hwndCtl), CB_SETDROPPEDWIDTH, (WPARAM)(len), 0L))
//////////////////////////////////////////////////////////////////////
class SwapOutsideParentPlugin : public MMDPluginDLL3, public CMmdCtrls, public Singleton<SwapOutsideParentPlugin>, public CPostMsgHook
{
private:
	MMDMainData*									m_mmdDataP;
	CInifile										m_ini;
	bool											m_bShowWindow;		// 初回の WM_SHOWWINDOW 判定用
	CKeyState										m_testKey;			// テストのキー
	CMenu											m_menu;				// メニュー
	std::vector<int32_t>							m_modelIdx;			// モデルコンボボックスに表示するモデルのインデックスリスト
	std::vector<int32_t>							m_fromBoneIdx;		// ボーンコンボボックスに表示するボーンのインデックスリスト
	std::vector<int32_t>							m_toBoneIdx;		// ボーンコンボボックスに表示するボーンのインデックスリスト
	HWND											m_parentBtnH;		// 「モデル操作」パネルの「外」ボタン

public:
	const char* getPluginTitle() const override { return "SwapOutsideParent"; }

	SwapOutsideParentPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
		, m_parentBtnH(NULL)
	{
		// DLLと同じパス/名前でiniファイル名を作る
		extern HMODULE g_module;
		wchar_t iniPath[MAX_PATH + 1];
		GetModuleFileNameW(g_module, iniPath, MAX_PATH);
		PathRenameExtension(iniPath, L".ini");
		m_ini.SetIniFile(iniPath);

		// iniファイルの読み込み
		m_ini.Read([](CInifile &ini)
		{
			// デフォルト値
			ini.Int(L"MdlCheck") = 1;
			ini.Int(L"CamCheck") = 1;
			ini.Int(L"AccCheck") = 1;
		});

		m_testKey.SetKeyCode(L"C");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
		// フック開始
		HookStart(g_module);

		m_menu.AddSeparator(CMenu::MmdMenu::Edit);
		m_menu.Create(CMenu::MmdMenu::Edit, _T("外部親差し替え(&O)"), _T("swap outside parent(&O)")
					, [this](CMenu*) { OnMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // 常時選べる様にする
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
		//	_dbgPrint("-----------------------------");
		//	CUnknownChecker::Print();
		//	_dbgPrint("-----------------------------");

			auto modelP = d->model_data[d->select_model];
			if (modelP != nullptr)
			{
				for (int idx = 0; idx < 1000; idx++)
				{
					auto &data = modelP->configuration_keyframe[idx];
				}
			}
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

		{
			HWND hWnd = param->hwnd;
			UINT iMsg = param->message;
			WPARAM wParam = param->wParam;
			LPARAM lParam = param->lParam;

			switch (iMsg)
			{
				case WM_CONTEXTMENU:
					// 外部親ボタンが右クリックされた
					if (param->hwnd  == m_parentBtnH &&
						(HWND)wParam == m_parentBtnH)
					{
						// 外部親差し替え ダイアログを開く
						OnMenuExec();
					}
					break;
			}
		}
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
					auto startFrameEdit	= GetDlgItem(hWnd, START_FRAME_EDIT);
					auto endFrameEdit	= GetDlgItem(hWnd, END_FRAME_EDIT);
					auto fromModelCombo = GetDlgItem(hWnd, FROM_MODEL_COMBO);
					auto toModelCombo	= GetDlgItem(hWnd, TO_MODEL_COMBO);
					auto mdlCheck		= GetDlgItem(hWnd, MODEL_CHECK);
					auto camCheck		= GetDlgItem(hWnd, CAMERA_CHECK);
					auto accCheck		= GetDlgItem(hWnd, ACC_CHECK);

					int startFrame = 0;
					int endFrame = -1;

					// 設定読み込み
					m_ini.Read();

					if (fromModelCombo && toModelCombo)
					{
						// モデルのインデックスリストを作成
						m_modelIdx.clear();	// モデルはどちらのコンボボックスも同じなのでリストは一つでOK
						m_fromBoneIdx.clear();
						m_toBoneIdx.clear();
						for (int mIdx = 0; mIdx<ARRAYSIZE(m_mmdDataP->model_data); mIdx++)
						{
							auto modelP = m_mmdDataP->model_data[mIdx];
							if (modelP)
							{
								m_modelIdx.push_back(mIdx);
							}
						}

						// 全項目削除
						ComboBox_Clear(fromModelCombo);
						ComboBox_Clear(  toModelCombo);

						// モデル名をコンボボックスに追加
						ComboBox_AddStringA(fromModelCombo, m_mmdDataP->is_english_mode ? "non" : "なし");
						ComboBox_AddStringA(  toModelCombo, m_mmdDataP->is_english_mode ? "non" : "なし");
						ComboBox_AddStringA(fromModelCombo, m_mmdDataP->is_english_mode ? "ground" : "地面");
						ComboBox_AddStringA(  toModelCombo, m_mmdDataP->is_english_mode ? "ground" : "地面");

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
								ComboBox_AddStringA(fromModelCombo, nameP);
								ComboBox_AddStringA(  toModelCombo, nameP);

								// 最大の文字数を調べておく
								int32_t len = static_cast<int32_t>(strlen(nameP));
								if (len > maxLen) { maxLen = len; }
							}
							// ドロップダウンした時の幅を調節 文字数で適当に…
							ComboBox_SetDroppedWidth(fromModelCombo, 7.5 * maxLen);
							ComboBox_SetDroppedWidth(  toModelCombo, 7.5 * maxLen);
						}

						// 現在「モデル操作」で選択されているモデルの要素番号を探す
						int selectIndex = 0;
						auto itr = std::find(m_modelIdx.begin(), m_modelIdx.end(), m_mmdDataP->select_model );
						if ( itr != m_modelIdx.end() )
						{
							selectIndex = (int)std::distance(m_modelIdx.begin(), itr) + 2;
						}

						ComboBox_SetCurSel(fromModelCombo, selectIndex);
						ComboBox_SetCurSel(  toModelCombo, selectIndex);
						// ボーンのコンボボックスを設定する為に自前で呼び出さないといけない…
						DlgWndProc(hWnd, WM_COMMAND, MAKELONG(FROM_MODEL_COMBO, CBN_SELCHANGE), 0);
						DlgWndProc(hWnd, WM_COMMAND, MAKELONG(  TO_MODEL_COMBO, CBN_SELCHANGE), 0);
					}

					if (startFrameEdit && endFrameEdit)
					{
						Edit_SetText(startFrameEdit, std::to_wstring(startFrame).c_str());
						Edit_SetText(endFrameEdit, std::to_wstring(endFrame).c_str());
					}
					if (mdlCheck && camCheck && accCheck)
					{
						// 設定状態を復帰
						Button_SetCheck(mdlCheck, m_ini.Int(L"MdlCheck"));
						Button_SetCheck(camCheck, m_ini.Int(L"CamCheck"));
						Button_SetCheck(accCheck, m_ini.Int(L"AccCheck"));
					}
				}
				return TRUE;

			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
					case FROM_MODEL_COMBO:
					case   TO_MODEL_COMBO:
						// モデルの選択項目が変更されたとき
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							int		mIdx = -1;

							// 現在のモデルコンボのインデックスを取得
							auto modelCombo = GetDlgItem(hWnd, LOWORD(wParam));
							if (modelCombo)
							{
								// 0番目は「なし」1番目は「地面」なので、モデルがあるのは2～
								mIdx = ComboBox_GetCurSel(modelCombo) - 2;
							}

							// 対応するボーンのコンボボックスのID
							bool bFrom = (LOWORD(wParam) == FROM_MODEL_COMBO);
							auto boneComboId = bFrom ? FROM_BONE_COMBO : TO_BONE_COMBO;
							auto &boneIdx = bFrom ? m_fromBoneIdx : m_toBoneIdx;
							auto boneCombo = GetDlgItem(hWnd, boneComboId);
							if (boneCombo)
							{
								boneIdx.clear();
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
											// フラグがなしのボーンも対象になる
											auto &boneR = modelP->bone_current_data[bIdx];
											if (boneR.flg & BoneCurrentData::BoneFlag::IK)
											{
												_addIfNotExist(boneIdx, bIdx);
												// Linkをサーチ
												for (int lIdx = 0; lIdx < boneR.ik_link_count; lIdx++)
												{
													auto link = boneR.ik_link[lIdx].link_bone;
													if (link >= 0)
													{
														_addIfNotExist(boneIdx, link);
													}
												}
											}
											else if (boneR.flg & BoneCurrentData::BoneFlag::Visible)
											{
												_addIfNotExist(boneIdx, bIdx);
											}
											else if (boneR.flg == 0)
											{
												_addIfNotExist(boneIdx, bIdx);
											}
										}

										// 昇順でソート
										std::sort(boneIdx.begin(), boneIdx.end());

										int32_t maxLen = 20;

										// ボーン名をコンボボックスにセット
										if (bFrom)	{ ComboBox_AddStringA(boneCombo, m_mmdDataP->is_english_mode ? "<all bones>" : "<全ボーン>"); }
										else		{ ComboBox_AddStringA(boneCombo, m_mmdDataP->is_english_mode ? "<same name>" : "<同名ボーン>"); }
										for (auto bIdx : boneIdx)
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
								else
								{
								}
								ComboBox_SetCurSel(boneCombo, 0);
							}
						}
						break;

					case SWAP_BTN:
					{
						// 現在のモデルコンボのインデックスを取得
						auto startFrameEdit	= GetDlgItem(hWnd, START_FRAME_EDIT);
						auto endFrameEdit	= GetDlgItem(hWnd, END_FRAME_EDIT);
						auto fromModelCombo = GetDlgItem(hWnd, FROM_MODEL_COMBO);
						auto toModelCombo	= GetDlgItem(hWnd, TO_MODEL_COMBO);
						auto fromBoneCombo	= GetDlgItem(hWnd, FROM_BONE_COMBO);
						auto toBoneCombo	= GetDlgItem(hWnd, TO_BONE_COMBO);
						auto mdlCheck		= GetDlgItem(hWnd, MODEL_CHECK);
						auto camCheck		= GetDlgItem(hWnd, CAMERA_CHECK);
						auto accCheck		= GetDlgItem(hWnd, ACC_CHECK);

						if (startFrameEdit && endFrameEdit && fromModelCombo && toModelCombo && fromBoneCombo && toBoneCombo && mdlCheck && camCheck && accCheck)
						{
							try
							{
								int fromModel = -1000;
								int fromBone = -1000;
								int toModel = -1000;
								int toBone = -1000;
								int startFrame = 0;
								int endFrame = -1;
								bool bMdl = false;
								bool bCam = false;
								bool bAcc = false;
								TCHAR txt[256];

								// 0番目は「なし」1番目は「地面」なので、モデルがあるのは2～
								auto fmIdx = ComboBox_GetCurSel(fromModelCombo) - 2;
								auto tmIdx = ComboBox_GetCurSel(  toModelCombo) - 2;
								// 0番目は「<全ボーン>」なので、ボーンがあるのは1～
								auto fbIdx = ComboBox_GetCurSel(fromBoneCombo) - 1;
								auto tbIdx = ComboBox_GetCurSel(  toBoneCombo) - 1;

								Edit_GetText(startFrameEdit, txt, ARRAYSIZE(txt));
								startFrame = std::stoi(txt);
								Edit_GetText(endFrameEdit, txt, ARRAYSIZE(txt));
								endFrame = std::stoi(txt);

								bMdl = Button_GetCheck(mdlCheck) == BST_CHECKED;
								bCam = Button_GetCheck(camCheck) == BST_CHECKED;
								bAcc = Button_GetCheck(accCheck) == BST_CHECKED;

								auto idx2No = []( const std::vector<int32_t> &modelIdxVec, const std::vector<int32_t> &boneIdx, int mIdx, int bIdx, int &modelOut, int &boneOut)
								{
									if (mIdx < 0)
									{
										// コンボボックスでは並び的に -1:地面 -2:なし だが、
										// 内部的には -1:なし -2:地面なので逆にする
										if		(mIdx == -1) { mIdx = -2; }
										else if	(mIdx == -2) { mIdx = -1; }
										modelOut = mIdx;
										boneOut = 0;
									}
									else if (0 <= mIdx && mIdx < modelIdxVec.size())
									{
										modelOut = modelIdxVec[mIdx];

										if (bIdx < 0)
										{
											boneOut = bIdx;
										}
										else if (0 <= bIdx && bIdx < boneIdx.size())
										{
											boneOut = boneIdx[bIdx];
										}
									}
								};

								// コンボボックスのインデックスをモデルやボーンの番号に変換する
								idx2No(m_modelIdx, m_fromBoneIdx, fmIdx, fbIdx, fromModel, fromBone);
								idx2No(m_modelIdx,   m_toBoneIdx, tmIdx, tbIdx,   toModel,   toBone);

								// ボーン追従をセットする
								if (fromModel >= -2 && fromBone >= -1 &&
									  toModel >= -2 &&   toBone >= -1)
								{
									// モデル
									if (bMdl)
									{
										for (auto &model : m_mmdDataP->model_data)
										{
											_swapOutsideParent(model, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
										}
									}
									// カメラ
									if (bCam)
									{
										_swapFollowBone(m_mmdDataP->camera_key_frame, true, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
									}
									// アクセサリ
									if (bAcc)
									{
										for (auto &acc : m_mmdDataP->accessory_key_frames)
										{
											_swapFollowBone(*acc, false, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
										}
									}
									
									// 再描画
									Repaint2();

									// 設定を保存
									m_ini.Int(L"MdlCheck") = bMdl;
									m_ini.Int(L"CamCheck") = bCam;
									m_ini.Int(L"AccCheck") = bAcc;
									m_ini.Write();
								}
							}
							catch (...)
							{
							}
						}
					}
						/// break; スルーして下へ
					case IDCANCEL:
						m_modelIdx.clear();
						m_fromBoneIdx.clear();
						m_toBoneIdx.clear();
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

		// コントロールハンドルの初期化
		InitCtrl();

		// 「モデル操作」パネルの「外」ボタンを取得する
		m_parentBtnH = GetDlgItem(hMmd, MODEL_PARENT_BTN_CTRL_ID);
	}

	// モデルの外部親を差し替える
	// nowAry		フレーム配列 configuration_keyframe
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// fromModel	外部親のモデルのインデックス なしの場合は-1 地面の場合は-2
	// fromBone		ボーンのインデックス -1の場合は「全ボーン」
	// toModel		外部親のモデルのインデックス なしの場合は-1 地面の場合は-2
	// toBone		ボーンのインデックス -1の場合は「同名ボーン」
	void _swapOutsideParent(MMDModelData *modelP, int startFrame, int endFrame, int fromModel, int fromBone, int toModel, int toBone)
	{
		if (modelP == nullptr)	{ return; }

		auto &nowAry = modelP->configuration_keyframe;

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				// 外部親設定の「対象ボーン」数分ループ
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++ )
				{
					auto &r = c.relation_setting[rIdx];

					// 差し替え
					_swap(fromModel, fromBone, toModel, toBone, r.parent_model_index, r.parent_bone_index);
				}

				// 次が無ければ終了
				if (c.next_index == 0) { break; }
				// 終了フレームを過ぎれば終了
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// 次のキーフレームへ
			cIdx = c.next_index;
		}
	}

	// カメラ/アクセサリフレームのモデル追従を差し替える
	// nowAry		フレーム配列 camera_key_frame か accessory_key_frames[] か
	// bCamera		↑がカメラの配列か
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// fromModel	ボーン追従のモデルのインデックス なしの場合は-1 地面の場合は-2(カメラでは何もしない)
	// fromBone		ボーンのインデックス -1の場合は「全ボーン」
	// toModel		ボーン追従のモデルのインデックス なしの場合は-1 地面の場合は-2(カメラでは何もしない)
	// toBone		ボーンのインデックス -1の場合は「同名ボーン」
	template<class FrameDataType>
	void _swapFollowBone(FrameDataType(&nowAry)[10000], bool bCamera, int startFrame, int endFrame, int fromModel, int fromBone, int toModel, int toBone)
	{
	/// カメラ/アクセサリは「地面」は「なし」扱いの方が良いのか?
	///	// カメラの場合は「地面」は何もしない、アクセサリの場合は「なし」は何もしない
	///	if (fromModel == (bCamera ? -2 : -1)) { return; }
	///	if (  toModel == (bCamera ? -2 : -1)) { return; }

		// 「地面」の場合は「なし」にしておく
		if (fromModel < -1) { fromModel = -1; }
		if (  toModel < -1) {   toModel = -1; }

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				// 差し替え
				_swap(fromModel, fromBone, toModel, toBone, c.looking_model_index, c.looking_bone_index);

				// 次が無ければ終了
				if (c.next_index == 0) { break; }
				// 終了フレームを過ぎれば終了
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// 次のキーフレームへ
			cIdx = c.next_index;
		}
	}

	// 差し替え実行処理
	// rModelIndex	差し替えるモデルのインデックス
	// rBoneIndex	差し替えるボーンのインデックス
	void _swap(int fromModel, int fromBone, int toModel, int toBone, int &rModelIndex, int &rBoneIndex)
	{
		bool bSwap = false;	// 変更前のボーンが一致するか?

		// モデルのインデックスが同じか?
		if (fromModel == rModelIndex)
		{
			// 「なし」が指定されていれば「なし」のフレームが対象
			if (fromModel == -1)
			{
				bSwap = true;
			}
			// 全ボーンを対象にするか、ボーンのインデックスが一致する場合
			else if (fromBone == -1 ||
						fromBone == rBoneIndex)
			{
				bSwap = true;
			}
		}

		// 差し替える
		if (bSwap)
		{
			// 元と同名ボーンに変える(モデルだけ変える)場合、同名のボーンがあるか探す
			if (toBone == -1)
			{
				// 差し替え前と後どちらもモデルが設定されていて
				if (rModelIndex >= 0 &&
					toModel >= 0)
				{
					auto fromModelP = m_mmdDataP->model_data[rModelIndex];
					auto   toModelP = m_mmdDataP->model_data[toModel];
					// モデルが読み込まれていて
					if (fromModelP != nullptr && toModelP != nullptr)
					{
						auto &fromBoneR = fromModelP->bone_current_data[rBoneIndex];
						auto fromName = m_mmdDataP->is_english_mode ? fromBoneR.wname_en : fromBoneR.wname_jp;

						for (int idx = 0; idx < toModelP->bone_count; idx++)
						{
							auto &toBoneR = toModelP->bone_current_data[idx];
							auto   toName = m_mmdDataP->is_english_mode ?   toBoneR.wname_en :   toBoneR.wname_jp;

							// 名前が一緒なら確定
							if (StrCmpW(fromName, toName) == 0)
							{
								rModelIndex = toModel;
								rBoneIndex = idx;
								break;
							}
						}
					}
				}
			}
			// それ以外はそのまま
			else
			{
				rModelIndex = toModel;
				rBoneIndex = toBone;
			}
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
		MessageBox(getHWND(), _T("MMDのバージョンが 9.31 ではありません。\nSwapOutsideParentは Ver.9.31 以外では正常に作動しません。"), _T("SwapOutsideParent"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return SwapOutsideParentPlugin::GetInstance();
}

