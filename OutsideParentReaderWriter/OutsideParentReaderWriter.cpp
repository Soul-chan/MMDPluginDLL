// OutsideParentReaderWriter.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include <deque>
#include <map>
#include <regex>
#include <fstream>
#include <Tchar.h>
#include <windows.h>
#include <WindowsX.h>
#include <commdlg.h>
#include <strsafe.h>	// StringCchCopy
#include <shellapi.h>	// DragQueryFile
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
class OutsideParentReaderWriterPlugin : public MMDPluginDLL3, public CMmdCtrls, public Singleton<OutsideParentReaderWriterPlugin>, public CPostMsgHook
{
private:
	MMDMainData*									m_mmdDataP;
	CInifile										m_ini;
	bool											m_bShowWindow;		// 初回の WM_SHOWWINDOW 判定用
	CKeyState										m_testKey;			// テストのキー
	CMenu											m_showMenu;			// 表示メニュー
	CMenu											m_loadMenu;			// 読込メニュー
	CMenu											m_saveMenu;			// 保存メニュー
	std::vector<int32_t>							m_modelIdx;			// モデルコンボボックスに表示するモデルのインデックスリスト
	std::vector<int32_t>							m_fromBoneIdx;		// ボーンコンボボックスに表示するボーンのインデックスリスト
	std::vector<int32_t>							m_toBoneIdx;		// ボーンコンボボックスに表示するボーンのインデックスリスト
	

	static constexpr int							NonIndex		= -1;		// 「なし」のインデックス
	static constexpr int							GroundIndex	= -2;		// 「地面」のインデックス
	static constexpr int							AllBoneIndex	= -3;		// 「全対象ボーン」のインデックス

	static constexpr char*							ModelEn = "Model";
	static constexpr char*							ModelJp = "モデル";
	static constexpr char*							CameraEn = "Camera";
	static constexpr char*							CameraJp = "カメラ";
	static constexpr char*							AccEn = "Accessory";
	static constexpr char*							AccJp = "アクセサリ";
	static constexpr char*							RootEn = "root";
	static constexpr char*							RootJp = "ルート";
	static constexpr char*							NonEn = "non";
	static constexpr char*							NonJp = "なし";
	static constexpr char*							GroundEn = "ground";
	static constexpr char*							GroundJp = "地面";
	static constexpr char*							AllBoneEn = "all subject bones";
	static constexpr char*							AllBoneJp = "全対象ボーン";

inline char* EnOrJp(char* en, char* jp)				{ return m_mmdDataP->is_english_mode ? en : jp; }
inline wchar_t* EnOrJp(wchar_t* en, wchar_t* jp)	{ return m_mmdDataP->is_english_mode ? en : jp; }

public:
	const char* getPluginTitle() const override { return "OutsideParentReaderWriter"; }

	OutsideParentReaderWriterPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
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
		});

		m_testKey.SetKeyCode(L"C");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
		// フック開始
		HookStart(g_module);

		// MMD本体は日英切替時に上から順番に文字列が割り当てられていくっぽい
		// なので挿入すると、下のメニューの文字列が挿入した項目にセットされてしまう
		// 一旦消して再挿入とかする必要があるのか…
		// Show relationship of A
		// Show Parent Relationship
		m_showMenu.Create(CMenu::MmdMenu::File, _T("外部親関係表示(&R)"), _T("show outside parent relationship(&R)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnShowMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // 常時選べる様にする
		m_loadMenu.Create(CMenu::MmdMenu::File, _T("外部親データ読込(&O)"), _T("load outside parent data(&O)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnLoadMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // 常時選べる様にする
		m_saveMenu.Create(CMenu::MmdMenu::File, _T("外部親データ保存(&T)"), _T("save outside parent data(&T)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnSaveMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // 常時選べる様にする
		m_saveMenu.AddSeparator(CMenu::MmdMenu::File, CMenu::LOAD_WAV_FILE_MENU_ID);
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

		m_showMenu.OnWndProc(param);
		m_loadMenu.OnWndProc(param);
		m_saveMenu.OnWndProc(param);
	}

	void PostMsgProc(int code, MSG* param) override
	{
		m_showMenu.OnPostMsg(param);
		m_loadMenu.OnPostMsg(param);
		m_saveMenu.OnPostMsg(param);
		{
			HWND hWnd = param->hwnd;
			UINT iMsg = param->message;
			WPARAM wParam = param->wParam;
			LPARAM lParam = param->lParam;

			switch (iMsg)
			{
				case WM_DROPFILES:	// ドラッグ&ドロップの処理
					{
						auto hDrop = (HDROP)wParam;
						auto uFileNo = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
						if (uFileNo > 0)
						{
							TCHAR szFile[MAX_PATH];

							DragQueryFile(hDrop, 0, szFile, sizeof(szFile));

							// vodファイルなら
							if (_tcsicmp(PathFindExtension(szFile), _T(".vod")) == 0)
							{
								// vodファイルを現在のモデルに適用する
								_applyVod(szFile);
							}
						}

						// DragFinishは呼ぶと他のドラッグ&ドロップが処理されないので、呼ばない
						// DragFinish(hDrop);
					}
					break;
			}
		}
	}

	// 関係表示メニューが実行されたとき
	void OnShowMenuExec()
	{
		std::string		modelTxt;
		std::string		cameraTxt;
		std::string		accTxt;

		// 情報収集
		// モデル
		for (auto &model : CMMDDataUtil::RenderOrderModels())
		{
			_showDataOutsideParent(model, 0, -1, modelTxt);
		}

		// カメラ
		char *camNameP = EnOrJp(CameraEn, CameraJp);
		_showDataFollowBone(camNameP, m_mmdDataP->camera_key_frame, 0, -1, cameraTxt);

		// アクセサリ
		for (int aIdx : CMMDDataUtil::RenderOrderAccessorysIndex())
		{
			if (m_mmdDataP->accessory_data[aIdx] != nullptr)
			{
				_showDataFollowBone(m_mmdDataP->accessory_data[aIdx]->name, *m_mmdDataP->accessory_key_frames[aIdx], 0, -1, accTxt);
			}
		}

		// タイトル
		auto titleP = EnOrJp("outside parent relationship", "外部親関係");
		std::string msg;
		
		if (! modelTxt.empty())
		{
			msg += EnOrJp("---- Model -------------------------\n", "---- モデル -------------------------\n");
			msg += modelTxt;
			msg += "\n";
		}
		if (!cameraTxt.empty())
		{
			msg += EnOrJp("---- Camera -------------------------\n", "---- カメラ -------------------------\n");
			msg += cameraTxt;
			msg += "\n";
		}
		if (!accTxt.empty())
		{
			msg += EnOrJp("---- Accessory -------------------------\n", "---- アクセサリ -------------------------\n");
			msg += accTxt;
			msg += "\n";
		}

		if (msg.empty())
		{
			msg = EnOrJp("No model/camera/accessory has an outside parent.",
						 "外部親を持つモデル/カメラ/アクセサリはありません。");
		}

		MessageBoxA(getHWND(), msg.c_str(), titleP, MB_OK);
	}

	// 読込メニューが実行されたとき
	void OnLoadMenuExec()
	{
		// ファイルを開くダイアログ
		OPENFILENAME		ofn ={};
		TCHAR				szPath[ MAX_PATH ] = _T("");
		TCHAR				szFile[ MAX_PATH ] = _T("");
		auto				hWnd = getHWND();
		
		// 設定読み込み
		m_ini.Read();

		if (StringCchCopy(szPath, MAX_PATH, m_ini.Str(L"CurrentPath").c_str()) != S_OK)
		{
			// コピーできなければクリアしておく
			szPath[0] = _T('\0');
		}

		if (szPath[0] == _T('\0'))
		{
			GetCurrentDirectory( MAX_PATH, szPath );
		}

		ofn.lStructSize         = sizeof(OPENFILENAME);
		ofn.hwndOwner           = hWnd;
		ofn.lpstrInitialDir     = szPath;       // 初期フォルダ位置
		ofn.lpstrFile           = szFile;       // 選択ファイル格納
		ofn.nMaxFile            = MAX_PATH;
		ofn.lpstrFilter         = _T("Vocaloid OutsideParent Data files(*.vod)\0*.vod\0")
								  _T("All files(*.*)\0*.*\0");
		ofn.lpstrTitle          = EnOrJp(_T("load outside parent data"), _T("外部親データ読込"));
		ofn.Flags               = OFN_FILEMUSTEXIST;

		if ( ! GetOpenFileName(&ofn) )
		{
			// キャンセルされた
			return;
		}

		// パスを保存しておく
		m_ini.Str(L"CurrentPath").assign(szFile, ofn.nFileOffset);
		m_ini.Write();


		// vodファイルを現在のモデルに適用する
		_applyVod(szFile);
	}

	// vodファイルを現在のモデルに適用する
	void _applyVod(PTCHAR vodFile)
	{
		std::string								readStr;
		std::ifstream							ifs(vodFile, std::ios::in);
		std::vector<OutsideParentOutputData>	inDataVec;
		OutsideParentOutputData					data;
		auto									hWnd = getHWND();
		if (! ifs.fail())
		{
			while (getline(ifs, readStr))
			{
				auto result = data.FromCsv(readStr);
				if (result > 0)
				{
					auto title = EnOrJp("load outside parent data", "外部親データ読込");
					std::string msg = EnOrJp("Failed to parse data.", "外部親データの解析に失敗しました。");
					msg += "\n\n" + readStr;
					MessageBoxA(hWnd, msg.c_str(), title, MB_OK | MB_ICONERROR);
					ifs.close();
					return;
				}
				else if (result == 0)
				{
					inDataVec.emplace_back(data);
				}
			}
		}
		ifs.close();

		// 選択されているモデル/カメラ/アクセサリの判定
		// -1:未選択 0:モデル 1:カメラ 2:アクセサリ
		int				selectType = -1;
		const char		*selectName = "";
		MMDModelData	*modelP = nullptr;

		// カメラ編集モードか?
		if (m_mmdDataP->is_camera_edit_mode)
		{
			// カメラが選択されているか?
			if (m_mmdDataP->is_camera_select)
			{
				selectType = 1;
				selectName = EnOrJp(CameraEn, CameraJp);
			}
			else
			{
				// 「アクセサリ操作」パネル選択中のアクセサリが
				auto accP = m_mmdDataP->accessory_data[ m_mmdDataP->select_accessory ];
				if (accP != nullptr)
				{
					// タイムラインで実際に選択されていれば、そのアクセサリを使う
					if (accP->is_timeline_select)
					{
						selectType = 2;
						selectName = accP->name;
					}
				}
			}
		}
		else
		{
			// 「モデル操作」パネル選択中のモデルがあればそれを使う
			modelP = m_mmdDataP->model_data[ m_mmdDataP->select_model ];
			if (modelP != nullptr)
			{
				selectType = 0;
				selectName = EnOrJp(modelP->name_en, modelP->name_jp);
			}
		}

		// データをチェックして続行するかの確認ダイアログを出す
		if (! _checkAndContinueDig(inDataVec, selectType, selectName))
		{
			return;
		}

		bool result = false;
		switch (selectType)
		{
			case 0:	// モデル
				result = _loadOutsideParent(modelP, m_mmdDataP->now_frame, inDataVec);
				break;

			case 1:	// カメラ
				result = _loadFollowBone(m_mmdDataP->camera_key_frame, true, m_mmdDataP->now_frame, inDataVec);
				break;

			case 2:	// アクセサリ
				result = _loadFollowBone(*m_mmdDataP->accessory_key_frames[ m_mmdDataP->select_accessory ], false, m_mmdDataP->now_frame, inDataVec);
				break;
		}

		// 再描画
		Repaint2();

		if (! result)
		{
			_loadErrorDlg(_T("Failed to complete setting data.\nPlease check the data."), _T("外部親の設定を完了できませんでした。\nデータを確認してください。"));
		}
	}

	// 保存メニューが実行されたとき
	void OnSaveMenuExec()
	{
		auto									hWnd = getHWND();
		std::vector<OutsideParentOutputData>	outDataVec;
		int										startFrame = 0;
		int										endFrame = -1;

		// カメラ編集モードか?
		if (m_mmdDataP->is_camera_edit_mode)
		{
			// カメラが選択されているか?
			if (m_mmdDataP->is_camera_select)
			{
				_saveCameraFollowBone(startFrame, endFrame, outDataVec);
			}
			else
			{
				// 「アクセサリ操作」パネル選択中のアクセサリが
				auto accP = m_mmdDataP->accessory_data[ m_mmdDataP->select_accessory ];
				if (accP != nullptr)
				{
					// タイムラインで実際に選択されていれば、そのアクセサリを使う
					if (accP->is_timeline_select)
					{
						_saveAccFollowBone(accP, *m_mmdDataP->accessory_key_frames[ m_mmdDataP->select_accessory ], startFrame, endFrame, outDataVec);
					}
				}
			}
		}
		else
		{
			// 「モデル操作」パネル選択中のモデルがあればそれを使う
			auto modelP = m_mmdDataP->model_data[ m_mmdDataP->select_model ];
			if (modelP != nullptr)
			{
				_saveOutsideParent(modelP, startFrame, endFrame, outDataVec);
			}
		}

		// 出力データが無い場合
		if (outDataVec.empty() ||
			(outDataVec.size() == 1 && outDataVec[0].IsNonData(0)) ||							// なし
			(outDataVec.size() == 1 && outDataVec[0].IsGroundData(0) && outDataVec[0].IsAcc()))	// アクセサリの地面
		{

			_saveErrorDlg(_T("Outside parent/Follow bone not set."),
						  _T("外部親/ボーン追従が設定されていません。"));
			return;
		}

		// 名前を付けて保存ダイアログ
		OPENFILENAME		ofn = {};
		TCHAR				szPath[ MAX_PATH ] = _T("");
		TCHAR				szFile[ MAX_PATH ] = _T("");
		
		// 設定読み込み
		m_ini.Read();

		if (StringCchCopy(szPath, MAX_PATH, m_ini.Str(L"CurrentPath").c_str()) != S_OK)
		{
			// コピーできなければクリアしておく
			szPath[0] = _T('\0');
		}

		if (szPath[0] == _T('\0'))
		{
			GetCurrentDirectory( MAX_PATH, szPath );
		}

		ofn.lStructSize         = sizeof(OPENFILENAME);
		ofn.hwndOwner           = hWnd;
		ofn.lpstrInitialDir     = szPath;		// 初期フォルダ位置
		ofn.lpstrFile           = szFile;		// 選択ファイル格納
		ofn.nMaxFile            = MAX_PATH;
		ofn.lpstrDefExt         = _T("*.vod");
		ofn.lpstrFilter         = _T("Vocaloid OutsideParent Data files(*.vod)\0*.vod\0")
								  _T("All files(*.*)\0*.*\0");
		ofn.lpstrTitle          = EnOrJp(_T("save outside parent data"), _T("外部親データ保存"));
		ofn.Flags               = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

		if ( ! GetSaveFileName(&ofn) )
		{
			// キャンセルされた
			return;
		}
		
		// ファイル出力
		std::ofstream	ofs(szFile, std::ios::out);
		// ヘッダ
		ofs << "#種類(モデル/カメラ/アクセサリ),フレーム,モデル名,モデル描画順Index,対象ボーン名,対象ボーンIndex,外部親モデル名,外部親モデル描画順Index,外部親ボーン名,外部親ボーンIndex" << std::endl;
		for (auto& data : outDataVec)
		{
			ofs << data.ToCsv() << std::endl;
		}
		ofs << std::flush;
		if (ofs.fail())
		{
			_saveErrorDlg(_T("Failed to save data."),
						  _T("外部親データの保存に失敗しました。"));
		}
		ofs.close();

		// パスを保存しておく
		m_ini.Str(L"CurrentPath").assign(szFile, ofn.nFileOffset);
		m_ini.Write();
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
	}

	// 外部親情報表示用データまとめクラス
	class ShowDataSummary
	{
		std::vector<std::tuple<int, std::string>>		m_vec;

	public:
		// 追加
		void Push(int idx)
		{
			auto mmdDataP = getMMDMainData();

			// 未追加なら外部親モデルを追加
			auto it = std::find_if(m_vec.begin(), m_vec.end(), [&](const auto& e) {return std::get<0>(e) == idx; });
			if (it == m_vec.end())
			{
				char *parentModelNameP = "????";

				// モデルは GroundIndex が地面、アクセサリは NonIndex が地面、カメラは地面なし
				if (idx == NonIndex ||
					idx == GroundIndex)
				{
					parentModelNameP = (mmdDataP->is_english_mode ? GroundEn : GroundJp);
				}
				else
				{
					auto parentModelP = mmdDataP->model_data[ idx ];
					parentModelNameP = mmdDataP->is_english_mode ? parentModelP->name_en : parentModelP->name_jp;
				}
				m_vec.emplace_back(idx, parentModelNameP);
			}
		}

		// 文字列に変換
		void AddToString(std::string modelName, std::string &txt)
		{

			// 外部親モデルを持っていれば
			if (!m_vec.empty())
			{
				txt += modelName;
				txt += " -> ";

				for (int cnt = 0; cnt < m_vec.size(); cnt++)
				{
					auto name = std::get<1>(m_vec[cnt]);
					if (cnt > 0)	{ txt += ", "; }
					txt += name;
				}

				txt += "\n";
			}
		}
	};

	// 外部親情報表示用データを作る
	// modelP		モデルデータ
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// showTxtR		表示情報が追記される
	void _showDataOutsideParent(MMDModelData *modelP, int startFrame, int endFrame, std::string &showTxtR)
	{
		if (modelP == nullptr)	{ return; }

		auto &nowAry = modelP->configuration_keyframe;
		char *modelNameP = EnOrJp(modelP->name_en, modelP->name_jp);

		ShowDataSummary		summ;

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				// 外部親設定の「対象ボーン」数分ループ
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++)
				{
					auto &r = c.relation_setting[rIdx];

					if (r.parent_model_index == GroundIndex || r.parent_model_index >= 0)
					{
						summ.Push(r.parent_model_index);
					}
				}

				// 次が無ければ終了
				if (c.next_index == 0) { break; }
				// 終了フレームを過ぎれば終了
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// 次のキーフレームへ
			cIdx = c.next_index;
		}

		summ.AddToString(modelNameP, showTxtR);
	}

	// カメラ/アクセサリフレームの外部親情報表示用データを作る
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// showTxtR		表示情報が追記される
	template<class FrameDataType>
	void _showDataFollowBone(const char *nameP, FrameDataType(&nowAry)[10000], int startFrame, int endFrame, std::string &showTxtR)
	{
		ShowDataSummary		summ;

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// ボーン追従のモデルのインデックス なしの場合は-1

				// ボーン追従が「なし」「地面」でない
				if (m != NonIndex)
				{
					summ.Push(m);
				}

				// 次が無ければ終了
				if (c.next_index == 0) { break; }
				// 終了フレームを過ぎれば終了
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// 次のキーフレームへ
			cIdx = c.next_index;
		}

		summ.AddToString(nameP, showTxtR);
	}


	// 外部親情報出力データ
	struct OutsideParentOutputData : public CCsv
	{
	public:
		std::string		typeName;			// 種類(モデル/カメラ/アクセサリ)
		int				frameNo;			// フレーム
		std::string		modelName;			// モデル名
		int				modelIdx;			// モデル描画順Index 1～
		std::string		boneName;			// 対象ボーン名
		int				boneIdx;			// 対象ボーンIndex -1:ルート -3:全対象ボーン
		std::string		parentModelName;	// 外部親モデル名
		int				parentModelIdx;		// 外部親モデル描画順Index 1～ -2:地面 -1:なし
		std::string		parentBoneName;		// 外部親ボーン名
		int				parentBoneIdx;		// 外部親ボーンIndex

		bool			bExists;			// モデルやボーンが存在するかどうか(適用時のワーク 出力はしない)

		// コンストラクタ
		OutsideParentOutputData()
			: frameNo			(0)
			, modelIdx			(0)
			, boneIdx			(0)
			, parentModelIdx	(0)
			, parentBoneIdx		(0)
			, bExists			(false)
		{
		}

		// コンストラクタ
		template<class S1, class S2, class S3, class S4, class S5>
		OutsideParentOutputData(	S1	_typeName			// 種類(モデル/カメラ/アクセサリ)
								,	int	_frameNo			// フレーム
								,	S2	_modelName			// モデル名
								,	int	_modelIdx			// モデル描画順Index
								,	S3	_boneName			// 対象ボーン名
								,	int	_boneIdx			// 対象ボーンIndex
								,	S4	_parentModelName	// 外部親モデル名
								,	int	_parentModelIdx		// 外部親モデル描画順Index
								,	S5	_parentBoneName		// 外部親ボーン名
								,	int	_parentBoneIdx		// 外部親ボーンIndex)
		)
			: typeName			(_typeName)
			, frameNo			(_frameNo)
			, modelName			(_modelName)
			, modelIdx			(_modelIdx)
			, boneName			(_boneName)
			, boneIdx			(_boneIdx)
			, parentModelName	(_parentModelName)
			, parentModelIdx	(_parentModelIdx)
			, parentBoneName	(_parentBoneName)
			, parentBoneIdx		(_parentBoneIdx)
		{
			AddDQuot(typeName);
			AddDQuot(modelName);
			AddDQuot(boneName);
			AddDQuot(parentModelName);
			AddDQuot(parentBoneName);
		}
		
		// 現在のメンバのCSV出力する文字列を返す
		std::string ToCsv()
		{
			return Format( "%s,%d,%s,%d,%s,%d,%s,%d,%s,%d"
				, typeName.c_str()			// 種類(モデル/カメラ/アクセサリ)
				, frameNo					// フレーム
				, modelName.c_str()			// モデル名
				, modelIdx					// モデル描画順Index
				, boneName.c_str()			// 対象ボーン名
				, boneIdx					// 対象ボーンIndex
				, parentModelName.c_str()	// 外部親モデル名
				, parentModelIdx			// 外部親モデル描画順Index
				, parentBoneName.c_str()	// 外部親ボーン名
				, parentBoneIdx				// 外部親ボーンIndex
			);
		}

		// CSVを分解してメンバを設定する
		// 戻り値 -1:コメントや空行で分解されなかった 0:成功 1:失敗
		int FromCsv(std::string csv)
		{
			std::vector<std::string>	splitVec;

			// コメント等で分解できなければその旨を返す
			if (Split(csv, splitVec) == 0)	{ return -1; }

			// CSVの要素数がメンバの数と同じでなければ失敗
			if (splitVec.size() != 10)	{ return 1; }
			
			try
			{
				typeName			= RemoveDQuot(splitVec[0]);	// 種類(モデル/カメラ/アクセサリ)
				frameNo				= std::stoi(splitVec[1]);		// フレーム
				modelName			= RemoveDQuot(splitVec[2]);	// モデル名
				modelIdx			= std::stoi(splitVec[3]);		// モデル描画順Index
				boneName			= RemoveDQuot(splitVec[4]);	// 対象ボーン名
				boneIdx				= std::stoi(splitVec[5]);		// 対象ボーンIndex
				parentModelName		= RemoveDQuot(splitVec[6]);	// 外部親モデル名
				parentModelIdx		= std::stoi(splitVec[7]);		// 外部親モデル描画順Index
				parentBoneName		= RemoveDQuot(splitVec[8]);	// 外部親ボーン名
				parentBoneIdx		= std::stoi(splitVec[9]);		// 外部親ボーンIndex
			}
			catch (...)
			{
				// intへの変換に失敗した場合も失敗
				return 1;
			}

			// 簡易チェック
			if (! IsModel() &&
				! IsCamera() &&
				! IsAcc() )
			{
				return 1;
			}

			// 成功
			return 0;
		}

		// 指定フレームで親モデルが「なし」のデータか
		bool IsNonData(int _frameNo)
		{
			return (frameNo == _frameNo) &&
					(boneIdx == AllBoneIndex || boneIdx == NonIndex) &&
					(parentModelIdx == NonIndex);
		}
		
		// 指定フレームで親モデルが「地面」のデータか
		bool IsGroundData(int _frameNo)
		{
			return (frameNo == _frameNo) &&
					(boneIdx == AllBoneIndex || boneIdx == NonIndex) &&
					(parentModelIdx == GroundIndex);
		}

		// 種類の判定関数
		bool IsModel()	{ return typeName == ModelEn || typeName == ModelJp; }
		bool IsCamera()	{ return typeName == CameraEn || typeName == CameraJp; }
		bool IsAcc()	{ return typeName == AccEn || typeName == AccJp; }

		// 種類を番号にして返す 0:モデル 1:カメラ 2:アクセサリ 
		int TypeNameToNo()
		{
			if      (IsModel())		{ return 0; }
			else if (IsCamera())	{ return 1; }
			else if (IsAcc())		{ return 2; }
			else					{ return 0; }
		}
	};

	
	// 外部親出力データのチェックと適用を続行するかのダイアログを出す
	// dataVecR		適用予定のデータ
	// selectType	選択されているタイプ -1:未選択 0:モデル 1:カメラ 2:アクセサリ
	// selectNameP	選択されているモデル等の名前
	// 戻り値 true:続行 false:キャンセル
	bool _checkAndContinueDig(std::vector<OutsideParentOutputData> &dataVecR, int selectType, const char *selectNameP)
	{
		if (dataVecR.empty())
		{
			_loadErrorDlg(_T("Data is empty."), _T("外部親データが空です。"));
			return false;
		}

		if (selectType < 0 || selectType > 2)
		{
			_loadErrorDlg(_T("No model/camera/accessory to read from has been selected."), _T("読込先のモデル/カメラ/アクセサリが選択されていません。"));
			return false;
		}

		std::vector<std::string>		vec;
		std::string						models = "";

		// 外部親モデル名を収集
		for (auto &data : dataVecR)
		{
			auto it = std::find(vec.begin(), vec.end(), data.parentModelName);
			if (it == vec.end())
			{
				vec.emplace_back(data.parentModelName);
			}
		}
		
		// Join
		for (int cnt = 0; cnt < vec.size(); cnt++)
		{
			if (cnt > 0)	{ models += ", "; }
			models += vec[cnt];
		}

		auto			title = EnOrJp("load outside parent data", "外部親データ読込");
		bool			bWarning = false;
		std::string		msg;
		UINT			dlgFlg;
		
		// タイプが違うデータを適用しようとしている場合は警告メッセージを追加する
		if ((dataVecR[0].IsModel()  && selectType != 0) ||
			(dataVecR[0].IsCamera() && selectType != 1) ||
			(dataVecR[0].IsAcc()    && selectType != 2))
		{
			if (m_mmdDataP->is_english_mode)
			{
				static const std::string typeNameTbl[] ={ ModelEn, CameraEn, AccEn };
				msg =	"* I'm trying to load the data for the " + typeNameTbl[ dataVecR[0].TypeNameToNo() ] + " into the " + typeNameTbl[selectType] + ".\n";
			}
			else
			{
				static const std::string typeNameTbl[] = { ModelJp, CameraJp, AccJp };
				msg =	"※" + typeNameTbl[ dataVecR[0].TypeNameToNo() ] + "用の外部親データを" + typeNameTbl[selectType] + "に読み込もうとしています。\n";
			}

			// モデルにカメラ/アクセサリのデータが適用される場合は、「ルート」に適用される旨を表示
			if (selectType == 0)
			{
				msg += EnOrJp("* Set to the outside parent of 'root'.\n", "※「ルート」の外部親に設定されます。\n");
			}

			msg += "\n";
			bWarning = true;
		}

		// 読み込み確認のダイアログメッセージ
		if (m_mmdDataP->is_english_mode)
		{
			msg  +=	"This outside parent file is the data for '" + dataVecR[0].modelName + "'.\n"
				  + "Set the model below as outside parent of '" + selectNameP + "'.\n"
				  + "\n"
				  + models + "\n"
				  + "\n"
				  + "If the model does not exist, ignore it.\n"
				  + "Are you OK?\n";
		}
		else
		{
			msg  +=	"このファイルは「" + dataVecR[0].modelName + "」用の外部親データです。\n"
				  + "以下のモデルを「" + selectNameP + "」の外部親として設定します。\n"
				  + "\n"
				  + models + "\n"
				  + "\n"
				  + "モデルが存在しない場合、そのモデルは無視します。\n"
				  + "続行しますか？\n";
		}

		dlgFlg = MB_OKCANCEL;
		if (bWarning)	{ dlgFlg |= MB_ICONWARNING; }
		if (MessageBoxA(getHWND(), msg.c_str(), title, dlgFlg) == IDCANCEL)
		{
			// キャンセルされた。
			return false;
		}

		return true;
	}

	// モデルに外部親データを適用する
	// modelP		モデルデータ
	// offsetFrame	適用を開始するフレームのオフセット
	// inDataVecR	適用するデータのベクタ
	bool _loadOutsideParent(MMDModelData *modelP, int offsetFrame, std::vector<OutsideParentOutputData> &inDataVecR)
	{
		int beforeFrame = -1;
		for (auto &data : inDataVecR)
		{
			// モデルやボーンが存在するか調べておく
			auto [bExists, boneIdx, parentModelIdx, parentBoneIdx] = _dataExistCheck(data, modelP);

			// NGならそのデータはスキップ
			if(! bExists)	{ continue; }

			// モデルの場合は「なし」「地面」が入っていたらボーンは0にする
			if (parentModelIdx < 0)				{ parentBoneIdx = 0; }
			// 念のため、変なインデックスの場合は「なし」になる様にしておく
			if (parentModelIdx < GroundIndex)	{ parentModelIdx = NonIndex; }

			auto frame = data.frameNo + offsetFrame;

			// キーフレームを取得(無ければ追加)
			auto cp = CKeyFrameUtil::FindConfigurationKeyFrame(modelP, frame, true);
			// キーフレームが取れなくなったら終了(最大キーフレームまで埋まっている場合)
			if (cp == nullptr)	{ return false; }

			auto &c = *cp;

			// 指定フレームが初めて取得された時は、一旦全外部親をクリアする
			// (データ圧縮の為に「なし」は全部出力している訳ではないので)
			if (beforeFrame < frame)
			{
				std::for_each(&c.relation_setting[0], &c.relation_setting[modelP->parentable_bone_count], [](auto &r){ r.Clear(); });
			}
			beforeFrame = frame;
			
			// 「全対象ボーン」なら
			if (data.boneIdx == AllBoneIndex)
			{
				// 全てセットする
				std::for_each(&c.relation_setting[0], &c.relation_setting[modelP->parentable_bone_count], [&](auto &r){ r.Set(parentModelIdx, parentBoneIdx); });
			}
			// 「ルート」なら
			else if (data.boneIdx == NonIndex)
			{
				// 先頭にセットする
				c.relation_setting[0].Set(parentModelIdx, parentBoneIdx);
			}
			else
			{
				// 普通にボーンに外部親を設定する
				auto &boneR = modelP->bone_current_data[boneIdx];
				auto rIdx = boneR.parentable_bone_index;

				c.relation_setting[rIdx].Set(parentModelIdx, parentBoneIdx);
			}
		}

		return true;
	}

	// カメラ/アクセサリに外部親データを適用する
	// nowAry		フレーム配列 camera_key_frame か accessory_key_frames[] か
	// bCamera		↑がカメラの配列か
	// offsetFrame	適用を開始するフレームのオフセット
	// inDataVecR	適用するデータのベクタ
	template<class FrameDataType>
	bool _loadFollowBone(FrameDataType(&nowAry)[10000], bool bCamera, int offsetFrame, std::vector<OutsideParentOutputData> &inDataVecR)
	{
		int cIdx = 0;
		int curParentModelIdx = NonIndex;
		int curParentBoneIdx = 0;
		int frame = 0;
		int beforeFrame = 0;
		// カメラ/アクセサリは位置と追従が同じフレームデータに入っている為、追従していなくてもキーフレームが大量に存在する
		// なので、変化のあったフレームのみ出力して、復帰時は既存フレームも前から順番にループして設定していく
		// (データを手で書き換えればフレームが昇順になっていないデータも作れるけど、そこまで考慮しなくていいか…)
		
		// 既存のフレームにも今の親を設定した方が良いか? のダイアログ出して聞いた方が良いのか?
		for (auto &data : inDataVecR)
		{
			// モデルやボーンが存在するか調べておく
			auto [bExists, boneIdx, parentModelIdx, parentBoneIdx] = _dataExistCheck(data, nullptr);

			// NGならそのデータはスキップ
			if(! bExists)	{ continue; }

			// カメラ/アクセサリの場合は「なし」「地面」が入っていたら-1,0にする
			if (parentModelIdx < 0) { parentModelIdx = NonIndex; parentBoneIdx = 0; }

			beforeFrame = frame;
			frame = data.frameNo + offsetFrame;

			// データが昇順でなければ、その時点で中止
			if (beforeFrame > frame)	{ return false; }

			// キーフレームを取得(無ければ追加)
			auto cp = CKeyFrameUtil::_findKeyFrame<FrameDataType>(nowAry, frame, 0, true, 1, FrameDataType::InitFrom);
			// キーフレームが取れなくなったら終了(最大キーフレームまで埋まっている場合)
			if (cp == nullptr)	{ return false; }

			// データのキーフレームまでは現状の外部親を設定していく
			for (; cIdx < ARRAYSIZE(nowAry);)
			{
				auto &c = nowAry[cIdx];

				// オフセットより前のフレームはスキップ
				if (c.frame_no >= offsetFrame)
				{
					// データのキーフレームが出てきたら現状の外部親を更新
					if (c.frame_no == frame)
					{
						curParentModelIdx = parentModelIdx;
						curParentBoneIdx = parentBoneIdx;
					}

					// 外部親を設定する
					c.SetLook(curParentModelIdx, curParentBoneIdx);

					// データのキーフレームまで設定したらループを抜ける
					// この時点では次のキーフレームがまだ設定されていない(次のデータで連結される)可能性があるので
					// 次回のループがこのフレームから始まる様に cIdx は更新しない
					// モデルのデータを使った場合には、同じフレームが複数続く場合もあるしね
					if (c.frame_no == frame) { break; }
				}

				// 次のキーフレームへ
				cIdx = c.next_index;
				// 次が無ければ終了(昇順チェックも上で抜ける様にしたし、基本的にはこれで抜ける事は無いと思うけど)
				if (c.next_index == 0) { return false; }
			}
		}

		return true;
	}

	// 外部親データにあるモデルやボーンが存在しているかをチェックする
	// チェック結果とボーン/親モデル/親モデルのボーンのインデックスを返す
	std::tuple<bool, int, int, int> _dataExistCheck(OutsideParentOutputData data, MMDModelData *modelP)
	{
		bool	bExists = false;
		int		boneIdx = 0;
		int		parentModelIdx = data.parentModelIdx;	// -2:地面 -1:なし の時の為にデータ内のインデックスで初期化
		int		parentBoneIdx = 0;

		// モデルが指定されていれば、モデル内のボーンをチェックする
		if (modelP != nullptr)
		{
			// 「ルート」や「全対象ボーン」でなければ
			if (data.boneIdx != NonIndex &&
				data.boneIdx != AllBoneIndex)
			{
				// 指定名で外部親の「対象ボーン」になるボーンを探す
				boneIdx = _findBone(modelP, data.boneName, data.boneIdx, [](auto &b){return b.parentable_bone_index >= 1; });
				// 外部親「を」設定するボーンが見つからなければ、そのデータはスキップ
				if (boneIdx < 0)	{ goto _exit; }
			}
		}

		// 親がなし/地面でないなら
		if (data.parentModelIdx >= 0)
		{
			// 外部親「に」設定するモデルとボーンを探す
			parentModelIdx = _findModel(data.parentModelName, data.parentModelIdx);
			// 見つからなければ、そのデータはスキップ
			if (parentModelIdx < 0)	{ goto _exit; }
			auto parentModelP = m_mmdDataP->model_data[ parentModelIdx ];

			// 外部親「に」設定するボーンを探す
			parentBoneIdx = _findBone(parentModelP, data.parentBoneName, data.parentBoneIdx);
			// 見つからなければ、そのデータはスキップ
			if (parentBoneIdx < 0)	{ goto _exit; }
		}

		// ここまでくれば全てOK
		bExists = true;

_exit:
		return { bExists, boneIdx, parentModelIdx , parentBoneIdx };
	}

	// モデルを探す
	// modelIdx は描画順での番号(1～)
	// 戻り値はモデル配列での番号(0～)
	// 
	// modelIdx の番号に指定名のモデルがあればそれを優先して選択する(同名のモデルが複数あった場合)
	// modelIdx が違うモデル名なら先頭から検索して探す
	// 指定名モデルが見つからなければ-1
	// 追加で条件判定が必要なら pred で指定する
	int _findModel(std::string modelName, int modelIdx, std::function<bool(MMDModelData *)> pred = nullptr)
	{
		// 検索順は描画順で行って、返す連番はモデル配列順と言う何とも面倒な事に…
		auto idxs = CMMDDataUtil::RenderOrderModelsIndex();

		// 出力した render_order は1から始まっているが、描画順にソートしたベクタは0からなのでそれに合わせる
		modelIdx--;

		// 指定の番号をチェック
		if (modelIdx < idxs.size())
		{
			auto modelP = m_mmdDataP->model_data[ idxs[modelIdx] ];
			if (modelP != nullptr)
			{
				if (modelName == modelP->name_jp ||
					modelName == modelP->name_en)
				{
					if (pred == nullptr ||
						pred(modelP))
					{
						return idxs[modelIdx];
					}
				}
			}
		}

		// モデルを検索
		for (auto &idx : idxs)
		{
			auto modelP = m_mmdDataP->model_data[ idxs[idx] ];
			if (modelP != nullptr)
			{
				if (modelName == modelP->name_jp ||
					modelName == modelP->name_en)
				{
					if (pred == nullptr ||
						pred(modelP))
					{
						return idxs[idx];
					}
				}
			}
		}

		return -1;
	}

	// モデルのボーンを探す
	// boneIdx の番号に指定名のボーンがあればそれを優先して選択する(同名のボーンが複数あった場合)
	// boneIdx が違うボーン名なら先頭から検索して探す
	// 指定名ボーンが見つからなければ-1
	// 追加で条件判定が必要なら pred で指定する
	int _findBone(MMDModelData *modelP, std::string boneName, int boneIdx, std::function<bool(BoneCurrentData&)> pred = nullptr)
	{
		// 指定の番号をチェック
		if (boneIdx < modelP->bone_count)
		{
			auto &boneR = modelP->bone_current_data[boneIdx];
			if (boneName == boneR.name_jp ||
				boneName == boneR.name_en)
			{
				if (pred == nullptr ||
					pred(boneR))
				{
					return boneIdx;
				}
			}
		}

		// ボーンを検索
		for (int bIdx = 0; bIdx < modelP->bone_count; bIdx++)
		{
			auto &boneR = modelP->bone_current_data[bIdx];
			if (boneName == boneR.name_jp ||
				boneName == boneR.name_en)
			{
				if (pred == nullptr ||
					pred(boneR))
				{
					return bIdx;
				}
			}
		}

		return -1;
	}

	// モデルの外部親出力データを作る
	// modelP		モデルデータ
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// outDataVecR	データを収集するベクタ
	void _saveOutsideParent(MMDModelData *modelP, int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		if (modelP == nullptr)	{ return; }

		auto &nowAry = modelP->configuration_keyframe;
		char *modelNameP = EnOrJp(modelP->name_en, modelP->name_jp);
		char *typeNameP = EnOrJp(ModelEn, ModelJp);

		// 対象ボーンをリストアップ
		struct BoneNameIdx
		{
			char			*name;
			int32_t			idx;
		};
		std::vector<BoneNameIdx>	boneIdx( modelP->parentable_bone_count );		// X番目の「対象ボーン」は全ボーン中の何番目のボーンかの配列

		// 0番目の「対象ボーン」は「ルート」なので、ボーンの中には無い
		boneIdx[0].name = EnOrJp(RootEn, RootJp);
		boneIdx[0].idx = NonIndex;

		// 「対象ボーン配列の要素番号」を持つボーンを列挙
		for (int bIdx = 0; bIdx < modelP->bone_count; bIdx++)
		{
			auto &boneR = modelP->bone_current_data[bIdx];
			auto rIdx = boneR.parentable_bone_index;

			if ( 1 <= rIdx && rIdx < boneIdx.size() )
			{
				boneIdx[rIdx].name = EnOrJp(boneR.name_en, boneR.name_jp);
				boneIdx[rIdx].idx = bIdx;
			}
		}

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];
			bool bEmptyFrame = true;	// 全ての対象ボーンが「なし」のフレームかどうか

			if (c.frame_no >= startFrame)
			{
				char *parentModelNameP;
				int parentModelIdx;
				char *parentBoneNameP;
				int parentBoneIdx;

				// 外部親設定の「対象ボーン」数分ループ
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++)
				{
					auto &r = c.relation_setting[rIdx];

					// parent_model_index は内部のモデル配列のインデックス(読み込まれた順番)でMMD上でのコンボボックスでの並びは render_order の方
					// ユーザーからは見た目上の並びでの番号の方が分かりやすいし render_order を出力する
					if (r.parent_model_index == GroundIndex || r.parent_model_index >= 0)
					{
						// 「外部親モデル」で「地面」が選択されているか
						if (r.parent_model_index == GroundIndex)
						{
							parentModelNameP = EnOrJp(GroundEn, GroundJp);
							parentModelIdx = GroundIndex;		// 地面は-2と言う事にしておく
							parentBoneNameP = "------";
							parentBoneIdx = GroundIndex;		// 地面は-2と言う事にしておく
						}
						else
						{
							auto parentModelP = m_mmdDataP->model_data[ r.parent_model_index ];
							auto &parentBoneR = parentModelP->bone_current_data[ r.parent_bone_index ];	

							parentModelNameP = EnOrJp(parentModelP->name_en, parentModelP->name_jp);
							parentModelIdx = parentModelP->render_order;
							parentBoneNameP = EnOrJp(parentBoneR.name_en, parentBoneR.name_jp);
							parentBoneIdx = r.parent_bone_index;
						}

						// 種類(モデル/カメラ/アクセサリ), フレーム, モデル名, モデル描画順Index, 対象ボーン名, 対象ボーンIndex, 外部親モデル名, 外部親モデル描画順Index, 外部親ボーン名, 外部親ボーンIndex
						outDataVecR.emplace_back (
									  typeNameP							// 種類
									, c.frame_no						// フレーム
									, modelNameP						// モデル名
									, modelP->render_order				// モデル描画順Index
									, boneIdx[rIdx].name				// 対象ボーン名
									, boneIdx[rIdx].idx					// 対象ボーンIndex
									, parentModelNameP					// 外部親モデル名
									, parentModelIdx					// 外部親モデル描画順Index
									, parentBoneNameP					// 外部親ボーン名
									, parentBoneIdx						// 外部親ボーンIndex
						);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG

						// 1つでも出力していたら空フレームではない
						bEmptyFrame = false;
					}
				}

				// 空フレームなら、「全対象ボーン」が「なし」で出力しておく
				if (bEmptyFrame)
				{
					char *allBoneNameP = EnOrJp(AllBoneEn, AllBoneJp);
					int allBoneIdx = AllBoneIndex;	// 「全対象ボーン」は-3と言う事にしておく
					parentModelNameP = EnOrJp(NonEn, NonJp);
					parentModelIdx = NonIndex;			// なしは-1と言う事にしておく
					parentBoneNameP = "------";
					parentBoneIdx = NonIndex;			// なしは-1と言う事にしておく

					// 種類(モデル/カメラ/アクセサリ), フレーム, モデル名, モデル描画順Index, 対象ボーン名, 対象ボーンIndex, 外部親モデル名, 外部親モデル描画順Index, 外部親ボーン名, 外部親ボーンIndex
					outDataVecR.emplace_back (
								  typeNameP							// 種類
								, c.frame_no						// フレーム
								, modelNameP						// モデル名
								, modelP->render_order				// モデル描画順Index
								, allBoneNameP						// 対象ボーン名
								, allBoneIdx						// 対象ボーンIndex
								, parentModelNameP					// 外部親モデル名
								, parentModelIdx					// 外部親モデル描画順Index
								, parentBoneNameP					// 外部親ボーン名
								, parentBoneIdx						// 外部親ボーンIndex
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
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

	// カメラフレームの外部親出力データを作る
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// outDataVecR	データを収集するベクタ
	void _saveCameraFollowBone(int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		auto &nowAry = m_mmdDataP->camera_key_frame;
		int beforeModelIdx = 0xFFFF;	// 0フレーム目も出力される様にあり得ない値で初期化
		int beforeBoneIdx = 0xFFFF;
		char *typeNameP = EnOrJp(CameraEn, CameraJp);

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// ボーン追従のモデルのインデックス なしの場合は-1
				auto &b = c.looking_bone_index;		// ボーンのインデックス 0～

				// カメラ/アクセサリは位置と追従が同じフレームデータに入っている為、追従していなくてもキーフレームが大量に存在する
				// なので、変化のあったフレームのみ出力して、復帰時は既存フレームも検索して設定していく
				// 変化のあるフレームのみ出力する
				if (beforeModelIdx != m || beforeBoneIdx != b)
				{
					beforeModelIdx = m;
					beforeBoneIdx = b;

					char *parentModelNameP;
					int parentModelIdx;
					char *parentBoneNameP;
					int parentBoneIdx;

					// ボーン追従が「なし」か
					if (m == NonIndex)
					{
						parentModelNameP = EnOrJp(NonEn, NonJp);
						parentModelIdx = NonIndex;			// なしは-1と言う事にしておく
						parentBoneNameP = "------";
						parentBoneIdx = NonIndex;			// なしは-1と言う事にしておく
					}
					else
					{
						auto parentModelP = m_mmdDataP->model_data[ m ];
						auto &parentBoneR = parentModelP->bone_current_data[ b ];	

						parentModelNameP = EnOrJp(parentModelP->name_en, parentModelP->name_jp);
						parentModelIdx = parentModelP->render_order;
						parentBoneNameP = EnOrJp(parentBoneR.name_en, parentBoneR.name_jp);
						parentBoneIdx = b;
					}

					// 種類(モデル/カメラ/アクセサリ), フレーム, モデル名, モデル描画順Index, 対象ボーン名, 対象ボーンIndex, 外部親モデル名, 外部親モデル描画順Index, 外部親ボーン名, 外部親ボーンIndex
					outDataVecR.emplace_back (
								  typeNameP				// 種類
								, c.frame_no			// フレーム
								, typeNameP				// モデル名 カメラにモデル名は無いけど表示上「カメラ」にしておく
								, NonIndex				// モデル描画順Index
								, "------"				// 対象ボーン名
								, NonIndex				// 対象ボーンIndex
								, parentModelNameP		// 外部親モデル名
								, parentModelIdx		// 外部親モデル描画順Index
								, parentBoneNameP		// 外部親ボーン名
								, parentBoneIdx			// 外部親ボーンIndex
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
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

	// アクセサリフレームの外部親出力データを作る
	// accP			アクセサリデータ
	// nowAry		フレーム配列 accessory_key_frames[] か
	// bCamera		↑がカメラの配列か
	// startFrame	開始フレーム
	// endFrame		終了フレーム
	// outDataVecR	データを収集するベクタ
	void _saveAccFollowBone(MMDAccessoryData *accP, AccessoryKeyFrameData (&nowAry)[10000], int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		if (accP == nullptr)	{ return; }

		int beforeModelIdx = 0xFFFF;	// 0フレーム目も出力される様にあり得ない値で初期化
		int beforeBoneIdx = 0xFFFF;
		char *typeNameP = EnOrJp(AccEn, AccJp);

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// ボーン追従のモデルのインデックス 地面の場合は-1
				auto &b = c.looking_bone_index;		// ボーンのインデックス 0～
				
				// カメラ/アクセサリは位置と追従が同じフレームデータに入っている為、追従していなくてもキーフレームが大量に存在する
				// なので、変化のあったフレームのみ出力して、復帰時は既存フレームも検索して設定していく
				// 変化のあるフレームのみ出力する
				if (beforeModelIdx != m || beforeBoneIdx != b)
				{
					beforeModelIdx = m;
					beforeBoneIdx = b;

					char *parentModelNameP;
					int parentModelIdx;
					char *parentBoneNameP;
					int parentBoneIdx;

					// ボーン追従が「地面」か
					if (m == NonIndex)
					{
						parentModelNameP = EnOrJp(GroundEn, GroundJp);
						parentModelIdx = GroundIndex;		// 地面は-2と言う事にしておく
						parentBoneNameP = "------";
						parentBoneIdx = GroundIndex;		// 地面は-2と言う事にしておく
					}
					else
					{
						auto parentModelP = m_mmdDataP->model_data[ m ];
						auto &parentBoneR = parentModelP->bone_current_data[ b ];	

						parentModelNameP = EnOrJp(parentModelP->name_en, parentModelP->name_jp);
						parentModelIdx = parentModelP->render_order;
						parentBoneNameP = EnOrJp(parentBoneR.name_en, parentBoneR.name_jp);
						parentBoneIdx = b;
					}

					// 種類(モデル/カメラ/アクセサリ), フレーム, モデル名, モデル描画順Index, 対象ボーン名, 対象ボーンIndex, 外部親モデル名, 外部親モデル描画順Index, 外部親ボーン名, 外部親ボーンIndex
					outDataVecR.emplace_back (
								  typeNameP				// 種類
								, c.frame_no			// フレーム
								, accP->name			// モデル名
								, accP->render_order	// モデル描画順Index
								, "------"				// 対象ボーン名
								, NonIndex				// 対象ボーンIndex
								, parentModelNameP		// 外部親モデル名
								, parentModelIdx		// 外部親モデル描画順Index
								, parentBoneNameP		// 外部親ボーン名
								, parentBoneIdx			// 外部親ボーンIndex
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
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

	// エラーダイアログ
	void _saveErrorDlg(wchar_t* en, wchar_t* jp) { _errorDlg(en, jp, true); }
	void _loadErrorDlg(wchar_t* en, wchar_t* jp) { _errorDlg(en, jp, false); }
	void _errorDlg(wchar_t* en, wchar_t* jp, bool bSave)
	{
		auto title = bSave 
						? EnOrJp(_T("save outside parent data"), _T("外部親データ保存"))
						: EnOrJp(_T("load outside parent data"), _T("外部親データ読込"));
		MessageBox(getHWND(), EnOrJp(en, jp), title, MB_OK | MB_ICONERROR);
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMDのバージョンが 9.31 ではありません。\nOutsideParentReaderWriterは Ver.9.31 以外では正常に作動しません。"), _T("OutsideParentReaderWriter"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return OutsideParentReaderWriterPlugin::GetInstance();
}

