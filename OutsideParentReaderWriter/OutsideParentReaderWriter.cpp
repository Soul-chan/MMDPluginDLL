// OutsideParentReaderWriter.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
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
	bool											m_bShowWindow;		// ����� WM_SHOWWINDOW ����p
	CKeyState										m_testKey;			// �e�X�g�̃L�[
	CMenu											m_showMenu;			// �\�����j���[
	CMenu											m_loadMenu;			// �Ǎ����j���[
	CMenu											m_saveMenu;			// �ۑ����j���[
	std::vector<int32_t>							m_modelIdx;			// ���f���R���{�{�b�N�X�ɕ\�����郂�f���̃C���f�b�N�X���X�g
	std::vector<int32_t>							m_fromBoneIdx;		// �{�[���R���{�{�b�N�X�ɕ\������{�[���̃C���f�b�N�X���X�g
	std::vector<int32_t>							m_toBoneIdx;		// �{�[���R���{�{�b�N�X�ɕ\������{�[���̃C���f�b�N�X���X�g
	

	static constexpr int							NonIndex		= -1;		// �u�Ȃ��v�̃C���f�b�N�X
	static constexpr int							GroundIndex	= -2;		// �u�n�ʁv�̃C���f�b�N�X
	static constexpr int							AllBoneIndex	= -3;		// �u�S�Ώۃ{�[���v�̃C���f�b�N�X

	static constexpr char*							ModelEn = "Model";
	static constexpr char*							ModelJp = "���f��";
	static constexpr char*							CameraEn = "Camera";
	static constexpr char*							CameraJp = "�J����";
	static constexpr char*							AccEn = "Accessory";
	static constexpr char*							AccJp = "�A�N�Z�T��";
	static constexpr char*							RootEn = "root";
	static constexpr char*							RootJp = "���[�g";
	static constexpr char*							NonEn = "non";
	static constexpr char*							NonJp = "�Ȃ�";
	static constexpr char*							GroundEn = "ground";
	static constexpr char*							GroundJp = "�n��";
	static constexpr char*							AllBoneEn = "all subject bones";
	static constexpr char*							AllBoneJp = "�S�Ώۃ{�[��";

inline char* EnOrJp(char* en, char* jp)				{ return m_mmdDataP->is_english_mode ? en : jp; }
inline wchar_t* EnOrJp(wchar_t* en, wchar_t* jp)	{ return m_mmdDataP->is_english_mode ? en : jp; }

public:
	const char* getPluginTitle() const override { return "OutsideParentReaderWriter"; }

	OutsideParentReaderWriterPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
	{
		// DLL�Ɠ����p�X/���O��ini�t�@�C���������
		extern HMODULE g_module;
		wchar_t iniPath[MAX_PATH + 1];
		GetModuleFileNameW(g_module, iniPath, MAX_PATH);
		PathRenameExtension(iniPath, L".ini");
		m_ini.SetIniFile(iniPath);

		// ini�t�@�C���̓ǂݍ���
		m_ini.Read([](CInifile &ini)
		{
			// �f�t�H���g�l
		});

		m_testKey.SetKeyCode(L"C");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
		// �t�b�N�J�n
		HookStart(g_module);

		// MMD�{�͓̂��p�ؑ֎��ɏォ�珇�Ԃɕ����񂪊��蓖�Ă��Ă������ۂ�
		// �Ȃ̂ő}������ƁA���̃��j���[�̕����񂪑}���������ڂɃZ�b�g����Ă��܂�
		// ��U�����čđ}���Ƃ�����K�v������̂��c
		// Show relationship of A
		// Show Parent Relationship
		m_showMenu.Create(CMenu::MmdMenu::File, _T("�O���e�֌W�\��(&R)"), _T("show outside parent relationship(&R)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnShowMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // �펞�I�ׂ�l�ɂ���
		m_loadMenu.Create(CMenu::MmdMenu::File, _T("�O���e�f�[�^�Ǎ�(&O)"), _T("load outside parent data(&O)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnLoadMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // �펞�I�ׂ�l�ɂ���
		m_saveMenu.Create(CMenu::MmdMenu::File, _T("�O���e�f�[�^�ۑ�(&T)"), _T("save outside parent data(&T)")
					, CMenu::LOAD_WAV_FILE_MENU_ID
					, [this](CMenu*) { OnSaveMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // �펞�I�ׂ�l�ɂ���
		m_saveMenu.AddSeparator(CMenu::MmdMenu::File, CMenu::LOAD_WAV_FILE_MENU_ID);
	}

	void stop() override
	{
		// �t�b�N�I��
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

	// MMD���̌Ăяo��������ł���v���V�[�W��
	// false : MMD�̃v���V�[�W�����Ă΂�܂��BLRESULT�͖�������܂�
	// true  : MMD�₻�̑��̃v���O�C���̃v���V�[�W���͌Ă΂�܂���BLRESULT���S�̂̃v���V�[�W���̖߂�l�Ƃ��ĕԂ���܂��B
	std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return{ false, 0 };
	}

	void WndProc(CWPSTRUCT* param) override
	{
		// ���񂩂ǂ�������
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

			// �������O�͉������Ȃ�
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
				case WM_DROPFILES:	// �h���b�O&�h���b�v�̏���
					{
						auto hDrop = (HDROP)wParam;
						auto uFileNo = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
						if (uFileNo > 0)
						{
							TCHAR szFile[MAX_PATH];

							DragQueryFile(hDrop, 0, szFile, sizeof(szFile));

							// vod�t�@�C���Ȃ�
							if (_tcsicmp(PathFindExtension(szFile), _T(".vod")) == 0)
							{
								// vod�t�@�C�������݂̃��f���ɓK�p����
								_applyVod(szFile);
							}
						}

						// DragFinish�͌ĂԂƑ��̃h���b�O&�h���b�v����������Ȃ��̂ŁA�Ă΂Ȃ�
						// DragFinish(hDrop);
					}
					break;
			}
		}
	}

	// �֌W�\�����j���[�����s���ꂽ�Ƃ�
	void OnShowMenuExec()
	{
		std::string		modelTxt;
		std::string		cameraTxt;
		std::string		accTxt;

		// �����W
		// ���f��
		for (auto &model : CMMDDataUtil::RenderOrderModels())
		{
			_showDataOutsideParent(model, 0, -1, modelTxt);
		}

		// �J����
		char *camNameP = EnOrJp(CameraEn, CameraJp);
		_showDataFollowBone(camNameP, m_mmdDataP->camera_key_frame, 0, -1, cameraTxt);

		// �A�N�Z�T��
		for (int aIdx : CMMDDataUtil::RenderOrderAccessorysIndex())
		{
			if (m_mmdDataP->accessory_data[aIdx] != nullptr)
			{
				_showDataFollowBone(m_mmdDataP->accessory_data[aIdx]->name, *m_mmdDataP->accessory_key_frames[aIdx], 0, -1, accTxt);
			}
		}

		// �^�C�g��
		auto titleP = EnOrJp("outside parent relationship", "�O���e�֌W");
		std::string msg;
		
		if (! modelTxt.empty())
		{
			msg += EnOrJp("---- Model -------------------------\n", "---- ���f�� -------------------------\n");
			msg += modelTxt;
			msg += "\n";
		}
		if (!cameraTxt.empty())
		{
			msg += EnOrJp("---- Camera -------------------------\n", "---- �J���� -------------------------\n");
			msg += cameraTxt;
			msg += "\n";
		}
		if (!accTxt.empty())
		{
			msg += EnOrJp("---- Accessory -------------------------\n", "---- �A�N�Z�T�� -------------------------\n");
			msg += accTxt;
			msg += "\n";
		}

		if (msg.empty())
		{
			msg = EnOrJp("No model/camera/accessory has an outside parent.",
						 "�O���e�������f��/�J����/�A�N�Z�T���͂���܂���B");
		}

		MessageBoxA(getHWND(), msg.c_str(), titleP, MB_OK);
	}

	// �Ǎ����j���[�����s���ꂽ�Ƃ�
	void OnLoadMenuExec()
	{
		// �t�@�C�����J���_�C�A���O
		OPENFILENAME		ofn ={};
		TCHAR				szPath[ MAX_PATH ] = _T("");
		TCHAR				szFile[ MAX_PATH ] = _T("");
		auto				hWnd = getHWND();
		
		// �ݒ�ǂݍ���
		m_ini.Read();

		if (StringCchCopy(szPath, MAX_PATH, m_ini.Str(L"CurrentPath").c_str()) != S_OK)
		{
			// �R�s�[�ł��Ȃ���΃N���A���Ă���
			szPath[0] = _T('\0');
		}

		if (szPath[0] == _T('\0'))
		{
			GetCurrentDirectory( MAX_PATH, szPath );
		}

		ofn.lStructSize         = sizeof(OPENFILENAME);
		ofn.hwndOwner           = hWnd;
		ofn.lpstrInitialDir     = szPath;       // �����t�H���_�ʒu
		ofn.lpstrFile           = szFile;       // �I���t�@�C���i�[
		ofn.nMaxFile            = MAX_PATH;
		ofn.lpstrFilter         = _T("Vocaloid OutsideParent Data files(*.vod)\0*.vod\0")
								  _T("All files(*.*)\0*.*\0");
		ofn.lpstrTitle          = EnOrJp(_T("load outside parent data"), _T("�O���e�f�[�^�Ǎ�"));
		ofn.Flags               = OFN_FILEMUSTEXIST;

		if ( ! GetOpenFileName(&ofn) )
		{
			// �L�����Z�����ꂽ
			return;
		}

		// �p�X��ۑ����Ă���
		m_ini.Str(L"CurrentPath").assign(szFile, ofn.nFileOffset);
		m_ini.Write();


		// vod�t�@�C�������݂̃��f���ɓK�p����
		_applyVod(szFile);
	}

	// vod�t�@�C�������݂̃��f���ɓK�p����
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
					auto title = EnOrJp("load outside parent data", "�O���e�f�[�^�Ǎ�");
					std::string msg = EnOrJp("Failed to parse data.", "�O���e�f�[�^�̉�͂Ɏ��s���܂����B");
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

		// �I������Ă��郂�f��/�J����/�A�N�Z�T���̔���
		// -1:���I�� 0:���f�� 1:�J���� 2:�A�N�Z�T��
		int				selectType = -1;
		const char		*selectName = "";
		MMDModelData	*modelP = nullptr;

		// �J�����ҏW���[�h��?
		if (m_mmdDataP->is_camera_edit_mode)
		{
			// �J�������I������Ă��邩?
			if (m_mmdDataP->is_camera_select)
			{
				selectType = 1;
				selectName = EnOrJp(CameraEn, CameraJp);
			}
			else
			{
				// �u�A�N�Z�T������v�p�l���I�𒆂̃A�N�Z�T����
				auto accP = m_mmdDataP->accessory_data[ m_mmdDataP->select_accessory ];
				if (accP != nullptr)
				{
					// �^�C�����C���Ŏ��ۂɑI������Ă���΁A���̃A�N�Z�T�����g��
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
			// �u���f������v�p�l���I�𒆂̃��f��������΂�����g��
			modelP = m_mmdDataP->model_data[ m_mmdDataP->select_model ];
			if (modelP != nullptr)
			{
				selectType = 0;
				selectName = EnOrJp(modelP->name_en, modelP->name_jp);
			}
		}

		// �f�[�^���`�F�b�N���đ��s���邩�̊m�F�_�C�A���O���o��
		if (! _checkAndContinueDig(inDataVec, selectType, selectName))
		{
			return;
		}

		bool result = false;
		switch (selectType)
		{
			case 0:	// ���f��
				result = _loadOutsideParent(modelP, m_mmdDataP->now_frame, inDataVec);
				break;

			case 1:	// �J����
				result = _loadFollowBone(m_mmdDataP->camera_key_frame, true, m_mmdDataP->now_frame, inDataVec);
				break;

			case 2:	// �A�N�Z�T��
				result = _loadFollowBone(*m_mmdDataP->accessory_key_frames[ m_mmdDataP->select_accessory ], false, m_mmdDataP->now_frame, inDataVec);
				break;
		}

		// �ĕ`��
		Repaint2();

		if (! result)
		{
			_loadErrorDlg(_T("Failed to complete setting data.\nPlease check the data."), _T("�O���e�̐ݒ�������ł��܂���ł����B\n�f�[�^���m�F���Ă��������B"));
		}
	}

	// �ۑ����j���[�����s���ꂽ�Ƃ�
	void OnSaveMenuExec()
	{
		auto									hWnd = getHWND();
		std::vector<OutsideParentOutputData>	outDataVec;
		int										startFrame = 0;
		int										endFrame = -1;

		// �J�����ҏW���[�h��?
		if (m_mmdDataP->is_camera_edit_mode)
		{
			// �J�������I������Ă��邩?
			if (m_mmdDataP->is_camera_select)
			{
				_saveCameraFollowBone(startFrame, endFrame, outDataVec);
			}
			else
			{
				// �u�A�N�Z�T������v�p�l���I�𒆂̃A�N�Z�T����
				auto accP = m_mmdDataP->accessory_data[ m_mmdDataP->select_accessory ];
				if (accP != nullptr)
				{
					// �^�C�����C���Ŏ��ۂɑI������Ă���΁A���̃A�N�Z�T�����g��
					if (accP->is_timeline_select)
					{
						_saveAccFollowBone(accP, *m_mmdDataP->accessory_key_frames[ m_mmdDataP->select_accessory ], startFrame, endFrame, outDataVec);
					}
				}
			}
		}
		else
		{
			// �u���f������v�p�l���I�𒆂̃��f��������΂�����g��
			auto modelP = m_mmdDataP->model_data[ m_mmdDataP->select_model ];
			if (modelP != nullptr)
			{
				_saveOutsideParent(modelP, startFrame, endFrame, outDataVec);
			}
		}

		// �o�̓f�[�^�������ꍇ
		if (outDataVec.empty() ||
			(outDataVec.size() == 1 && outDataVec[0].IsNonData(0)) ||							// �Ȃ�
			(outDataVec.size() == 1 && outDataVec[0].IsGroundData(0) && outDataVec[0].IsAcc()))	// �A�N�Z�T���̒n��
		{

			_saveErrorDlg(_T("Outside parent/Follow bone not set."),
						  _T("�O���e/�{�[���Ǐ]���ݒ肳��Ă��܂���B"));
			return;
		}

		// ���O��t���ĕۑ��_�C�A���O
		OPENFILENAME		ofn = {};
		TCHAR				szPath[ MAX_PATH ] = _T("");
		TCHAR				szFile[ MAX_PATH ] = _T("");
		
		// �ݒ�ǂݍ���
		m_ini.Read();

		if (StringCchCopy(szPath, MAX_PATH, m_ini.Str(L"CurrentPath").c_str()) != S_OK)
		{
			// �R�s�[�ł��Ȃ���΃N���A���Ă���
			szPath[0] = _T('\0');
		}

		if (szPath[0] == _T('\0'))
		{
			GetCurrentDirectory( MAX_PATH, szPath );
		}

		ofn.lStructSize         = sizeof(OPENFILENAME);
		ofn.hwndOwner           = hWnd;
		ofn.lpstrInitialDir     = szPath;		// �����t�H���_�ʒu
		ofn.lpstrFile           = szFile;		// �I���t�@�C���i�[
		ofn.nMaxFile            = MAX_PATH;
		ofn.lpstrDefExt         = _T("*.vod");
		ofn.lpstrFilter         = _T("Vocaloid OutsideParent Data files(*.vod)\0*.vod\0")
								  _T("All files(*.*)\0*.*\0");
		ofn.lpstrTitle          = EnOrJp(_T("save outside parent data"), _T("�O���e�f�[�^�ۑ�"));
		ofn.Flags               = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

		if ( ! GetSaveFileName(&ofn) )
		{
			// �L�����Z�����ꂽ
			return;
		}
		
		// �t�@�C���o��
		std::ofstream	ofs(szFile, std::ios::out);
		// �w�b�_
		ofs << "#���(���f��/�J����/�A�N�Z�T��),�t���[��,���f����,���f���`�揇Index,�Ώۃ{�[����,�Ώۃ{�[��Index,�O���e���f����,�O���e���f���`�揇Index,�O���e�{�[����,�O���e�{�[��Index" << std::endl;
		for (auto& data : outDataVec)
		{
			ofs << data.ToCsv() << std::endl;
		}
		ofs << std::flush;
		if (ofs.fail())
		{
			_saveErrorDlg(_T("Failed to save data."),
						  _T("�O���e�f�[�^�̕ۑ��Ɏ��s���܂����B"));
		}
		ofs.close();

		// �p�X��ۑ����Ă���
		m_ini.Str(L"CurrentPath").assign(szFile, ofn.nFileOffset);
		m_ini.Write();
	}

private:
	// ����� WM_SHOWWINDOW �O�ɌĂ΂��
	// ���̎��_�ł̓{�^�����̃R���g���[�����o���Ă���͂�
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hMmd = getHWND();

		// �R���g���[���n���h���̏�����
		InitCtrl();
	}

	// �O���e���\���p�f�[�^�܂Ƃ߃N���X
	class ShowDataSummary
	{
		std::vector<std::tuple<int, std::string>>		m_vec;

	public:
		// �ǉ�
		void Push(int idx)
		{
			auto mmdDataP = getMMDMainData();

			// ���ǉ��Ȃ�O���e���f����ǉ�
			auto it = std::find_if(m_vec.begin(), m_vec.end(), [&](const auto& e) {return std::get<0>(e) == idx; });
			if (it == m_vec.end())
			{
				char *parentModelNameP = "????";

				// ���f���� GroundIndex ���n�ʁA�A�N�Z�T���� NonIndex ���n�ʁA�J�����͒n�ʂȂ�
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

		// ������ɕϊ�
		void AddToString(std::string modelName, std::string &txt)
		{

			// �O���e���f���������Ă����
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

	// �O���e���\���p�f�[�^�����
	// modelP		���f���f�[�^
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// showTxtR		�\����񂪒ǋL�����
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
				// �O���e�ݒ�́u�Ώۃ{�[���v�������[�v
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++)
				{
					auto &r = c.relation_setting[rIdx];

					if (r.parent_model_index == GroundIndex || r.parent_model_index >= 0)
					{
						summ.Push(r.parent_model_index);
					}
				}

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}

		summ.AddToString(modelNameP, showTxtR);
	}

	// �J����/�A�N�Z�T���t���[���̊O���e���\���p�f�[�^�����
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// showTxtR		�\����񂪒ǋL�����
	template<class FrameDataType>
	void _showDataFollowBone(const char *nameP, FrameDataType(&nowAry)[10000], int startFrame, int endFrame, std::string &showTxtR)
	{
		ShowDataSummary		summ;

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// �{�[���Ǐ]�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1

				// �{�[���Ǐ]���u�Ȃ��v�u�n�ʁv�łȂ�
				if (m != NonIndex)
				{
					summ.Push(m);
				}

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}

		summ.AddToString(nameP, showTxtR);
	}


	// �O���e���o�̓f�[�^
	struct OutsideParentOutputData : public CCsv
	{
	public:
		std::string		typeName;			// ���(���f��/�J����/�A�N�Z�T��)
		int				frameNo;			// �t���[��
		std::string		modelName;			// ���f����
		int				modelIdx;			// ���f���`�揇Index 1�`
		std::string		boneName;			// �Ώۃ{�[����
		int				boneIdx;			// �Ώۃ{�[��Index -1:���[�g -3:�S�Ώۃ{�[��
		std::string		parentModelName;	// �O���e���f����
		int				parentModelIdx;		// �O���e���f���`�揇Index 1�` -2:�n�� -1:�Ȃ�
		std::string		parentBoneName;		// �O���e�{�[����
		int				parentBoneIdx;		// �O���e�{�[��Index

		bool			bExists;			// ���f����{�[�������݂��邩�ǂ���(�K�p���̃��[�N �o�͂͂��Ȃ�)

		// �R���X�g���N�^
		OutsideParentOutputData()
			: frameNo			(0)
			, modelIdx			(0)
			, boneIdx			(0)
			, parentModelIdx	(0)
			, parentBoneIdx		(0)
			, bExists			(false)
		{
		}

		// �R���X�g���N�^
		template<class S1, class S2, class S3, class S4, class S5>
		OutsideParentOutputData(	S1	_typeName			// ���(���f��/�J����/�A�N�Z�T��)
								,	int	_frameNo			// �t���[��
								,	S2	_modelName			// ���f����
								,	int	_modelIdx			// ���f���`�揇Index
								,	S3	_boneName			// �Ώۃ{�[����
								,	int	_boneIdx			// �Ώۃ{�[��Index
								,	S4	_parentModelName	// �O���e���f����
								,	int	_parentModelIdx		// �O���e���f���`�揇Index
								,	S5	_parentBoneName		// �O���e�{�[����
								,	int	_parentBoneIdx		// �O���e�{�[��Index)
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
		
		// ���݂̃����o��CSV�o�͂��镶�����Ԃ�
		std::string ToCsv()
		{
			return Format( "%s,%d,%s,%d,%s,%d,%s,%d,%s,%d"
				, typeName.c_str()			// ���(���f��/�J����/�A�N�Z�T��)
				, frameNo					// �t���[��
				, modelName.c_str()			// ���f����
				, modelIdx					// ���f���`�揇Index
				, boneName.c_str()			// �Ώۃ{�[����
				, boneIdx					// �Ώۃ{�[��Index
				, parentModelName.c_str()	// �O���e���f����
				, parentModelIdx			// �O���e���f���`�揇Index
				, parentBoneName.c_str()	// �O���e�{�[����
				, parentBoneIdx				// �O���e�{�[��Index
			);
		}

		// CSV�𕪉����ă����o��ݒ肷��
		// �߂�l -1:�R�����g���s�ŕ�������Ȃ����� 0:���� 1:���s
		int FromCsv(std::string csv)
		{
			std::vector<std::string>	splitVec;

			// �R�����g���ŕ����ł��Ȃ���΂��̎|��Ԃ�
			if (Split(csv, splitVec) == 0)	{ return -1; }

			// CSV�̗v�f���������o�̐��Ɠ����łȂ���Ύ��s
			if (splitVec.size() != 10)	{ return 1; }
			
			try
			{
				typeName			= RemoveDQuot(splitVec[0]);	// ���(���f��/�J����/�A�N�Z�T��)
				frameNo				= std::stoi(splitVec[1]);		// �t���[��
				modelName			= RemoveDQuot(splitVec[2]);	// ���f����
				modelIdx			= std::stoi(splitVec[3]);		// ���f���`�揇Index
				boneName			= RemoveDQuot(splitVec[4]);	// �Ώۃ{�[����
				boneIdx				= std::stoi(splitVec[5]);		// �Ώۃ{�[��Index
				parentModelName		= RemoveDQuot(splitVec[6]);	// �O���e���f����
				parentModelIdx		= std::stoi(splitVec[7]);		// �O���e���f���`�揇Index
				parentBoneName		= RemoveDQuot(splitVec[8]);	// �O���e�{�[����
				parentBoneIdx		= std::stoi(splitVec[9]);		// �O���e�{�[��Index
			}
			catch (...)
			{
				// int�ւ̕ϊ��Ɏ��s�����ꍇ�����s
				return 1;
			}

			// �ȈՃ`�F�b�N
			if (! IsModel() &&
				! IsCamera() &&
				! IsAcc() )
			{
				return 1;
			}

			// ����
			return 0;
		}

		// �w��t���[���Őe���f�����u�Ȃ��v�̃f�[�^��
		bool IsNonData(int _frameNo)
		{
			return (frameNo == _frameNo) &&
					(boneIdx == AllBoneIndex || boneIdx == NonIndex) &&
					(parentModelIdx == NonIndex);
		}
		
		// �w��t���[���Őe���f�����u�n�ʁv�̃f�[�^��
		bool IsGroundData(int _frameNo)
		{
			return (frameNo == _frameNo) &&
					(boneIdx == AllBoneIndex || boneIdx == NonIndex) &&
					(parentModelIdx == GroundIndex);
		}

		// ��ނ̔���֐�
		bool IsModel()	{ return typeName == ModelEn || typeName == ModelJp; }
		bool IsCamera()	{ return typeName == CameraEn || typeName == CameraJp; }
		bool IsAcc()	{ return typeName == AccEn || typeName == AccJp; }

		// ��ނ�ԍ��ɂ��ĕԂ� 0:���f�� 1:�J���� 2:�A�N�Z�T�� 
		int TypeNameToNo()
		{
			if      (IsModel())		{ return 0; }
			else if (IsCamera())	{ return 1; }
			else if (IsAcc())		{ return 2; }
			else					{ return 0; }
		}
	};

	
	// �O���e�o�̓f�[�^�̃`�F�b�N�ƓK�p�𑱍s���邩�̃_�C�A���O���o��
	// dataVecR		�K�p�\��̃f�[�^
	// selectType	�I������Ă���^�C�v -1:���I�� 0:���f�� 1:�J���� 2:�A�N�Z�T��
	// selectNameP	�I������Ă��郂�f�����̖��O
	// �߂�l true:���s false:�L�����Z��
	bool _checkAndContinueDig(std::vector<OutsideParentOutputData> &dataVecR, int selectType, const char *selectNameP)
	{
		if (dataVecR.empty())
		{
			_loadErrorDlg(_T("Data is empty."), _T("�O���e�f�[�^����ł��B"));
			return false;
		}

		if (selectType < 0 || selectType > 2)
		{
			_loadErrorDlg(_T("No model/camera/accessory to read from has been selected."), _T("�Ǎ���̃��f��/�J����/�A�N�Z�T�����I������Ă��܂���B"));
			return false;
		}

		std::vector<std::string>		vec;
		std::string						models = "";

		// �O���e���f���������W
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

		auto			title = EnOrJp("load outside parent data", "�O���e�f�[�^�Ǎ�");
		bool			bWarning = false;
		std::string		msg;
		UINT			dlgFlg;
		
		// �^�C�v���Ⴄ�f�[�^��K�p���悤�Ƃ��Ă���ꍇ�͌x�����b�Z�[�W��ǉ�����
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
				msg =	"��" + typeNameTbl[ dataVecR[0].TypeNameToNo() ] + "�p�̊O���e�f�[�^��" + typeNameTbl[selectType] + "�ɓǂݍ������Ƃ��Ă��܂��B\n";
			}

			// ���f���ɃJ����/�A�N�Z�T���̃f�[�^���K�p�����ꍇ�́A�u���[�g�v�ɓK�p�����|��\��
			if (selectType == 0)
			{
				msg += EnOrJp("* Set to the outside parent of 'root'.\n", "���u���[�g�v�̊O���e�ɐݒ肳��܂��B\n");
			}

			msg += "\n";
			bWarning = true;
		}

		// �ǂݍ��݊m�F�̃_�C�A���O���b�Z�[�W
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
			msg  +=	"���̃t�@�C���́u" + dataVecR[0].modelName + "�v�p�̊O���e�f�[�^�ł��B\n"
				  + "�ȉ��̃��f�����u" + selectNameP + "�v�̊O���e�Ƃ��Đݒ肵�܂��B\n"
				  + "\n"
				  + models + "\n"
				  + "\n"
				  + "���f�������݂��Ȃ��ꍇ�A���̃��f���͖������܂��B\n"
				  + "���s���܂����H\n";
		}

		dlgFlg = MB_OKCANCEL;
		if (bWarning)	{ dlgFlg |= MB_ICONWARNING; }
		if (MessageBoxA(getHWND(), msg.c_str(), title, dlgFlg) == IDCANCEL)
		{
			// �L�����Z�����ꂽ�B
			return false;
		}

		return true;
	}

	// ���f���ɊO���e�f�[�^��K�p����
	// modelP		���f���f�[�^
	// offsetFrame	�K�p���J�n����t���[���̃I�t�Z�b�g
	// inDataVecR	�K�p����f�[�^�̃x�N�^
	bool _loadOutsideParent(MMDModelData *modelP, int offsetFrame, std::vector<OutsideParentOutputData> &inDataVecR)
	{
		int beforeFrame = -1;
		for (auto &data : inDataVecR)
		{
			// ���f����{�[�������݂��邩���ׂĂ���
			auto [bExists, boneIdx, parentModelIdx, parentBoneIdx] = _dataExistCheck(data, modelP);

			// NG�Ȃ炻�̃f�[�^�̓X�L�b�v
			if(! bExists)	{ continue; }

			// ���f���̏ꍇ�́u�Ȃ��v�u�n�ʁv�������Ă�����{�[����0�ɂ���
			if (parentModelIdx < 0)				{ parentBoneIdx = 0; }
			// �O�̂��߁A�ςȃC���f�b�N�X�̏ꍇ�́u�Ȃ��v�ɂȂ�l�ɂ��Ă���
			if (parentModelIdx < GroundIndex)	{ parentModelIdx = NonIndex; }

			auto frame = data.frameNo + offsetFrame;

			// �L�[�t���[�����擾(������Βǉ�)
			auto cp = CKeyFrameUtil::FindConfigurationKeyFrame(modelP, frame, true);
			// �L�[�t���[�������Ȃ��Ȃ�����I��(�ő�L�[�t���[���܂Ŗ��܂��Ă���ꍇ)
			if (cp == nullptr)	{ return false; }

			auto &c = *cp;

			// �w��t���[�������߂Ď擾���ꂽ���́A��U�S�O���e���N���A����
			// (�f�[�^���k�ׂ̈Ɂu�Ȃ��v�͑S���o�͂��Ă����ł͂Ȃ��̂�)
			if (beforeFrame < frame)
			{
				std::for_each(&c.relation_setting[0], &c.relation_setting[modelP->parentable_bone_count], [](auto &r){ r.Clear(); });
			}
			beforeFrame = frame;
			
			// �u�S�Ώۃ{�[���v�Ȃ�
			if (data.boneIdx == AllBoneIndex)
			{
				// �S�ăZ�b�g����
				std::for_each(&c.relation_setting[0], &c.relation_setting[modelP->parentable_bone_count], [&](auto &r){ r.Set(parentModelIdx, parentBoneIdx); });
			}
			// �u���[�g�v�Ȃ�
			else if (data.boneIdx == NonIndex)
			{
				// �擪�ɃZ�b�g����
				c.relation_setting[0].Set(parentModelIdx, parentBoneIdx);
			}
			else
			{
				// ���ʂɃ{�[���ɊO���e��ݒ肷��
				auto &boneR = modelP->bone_current_data[boneIdx];
				auto rIdx = boneR.parentable_bone_index;

				c.relation_setting[rIdx].Set(parentModelIdx, parentBoneIdx);
			}
		}

		return true;
	}

	// �J����/�A�N�Z�T���ɊO���e�f�[�^��K�p����
	// nowAry		�t���[���z�� camera_key_frame �� accessory_key_frames[] ��
	// bCamera		�����J�����̔z��
	// offsetFrame	�K�p���J�n����t���[���̃I�t�Z�b�g
	// inDataVecR	�K�p����f�[�^�̃x�N�^
	template<class FrameDataType>
	bool _loadFollowBone(FrameDataType(&nowAry)[10000], bool bCamera, int offsetFrame, std::vector<OutsideParentOutputData> &inDataVecR)
	{
		int cIdx = 0;
		int curParentModelIdx = NonIndex;
		int curParentBoneIdx = 0;
		int frame = 0;
		int beforeFrame = 0;
		// �J����/�A�N�Z�T���͈ʒu�ƒǏ]�������t���[���f�[�^�ɓ����Ă���ׁA�Ǐ]���Ă��Ȃ��Ă��L�[�t���[������ʂɑ��݂���
		// �Ȃ̂ŁA�ω��̂������t���[���̂ݏo�͂��āA���A���͊����t���[�����O���珇�ԂɃ��[�v���Đݒ肵�Ă���
		// (�f�[�^����ŏ���������΃t���[���������ɂȂ��Ă��Ȃ��f�[�^�����邯�ǁA�����܂ōl�����Ȃ��Ă������c)
		
		// �����̃t���[���ɂ����̐e��ݒ肵�������ǂ���? �̃_�C�A���O�o���ĕ����������ǂ��̂�?
		for (auto &data : inDataVecR)
		{
			// ���f����{�[�������݂��邩���ׂĂ���
			auto [bExists, boneIdx, parentModelIdx, parentBoneIdx] = _dataExistCheck(data, nullptr);

			// NG�Ȃ炻�̃f�[�^�̓X�L�b�v
			if(! bExists)	{ continue; }

			// �J����/�A�N�Z�T���̏ꍇ�́u�Ȃ��v�u�n�ʁv�������Ă�����-1,0�ɂ���
			if (parentModelIdx < 0) { parentModelIdx = NonIndex; parentBoneIdx = 0; }

			beforeFrame = frame;
			frame = data.frameNo + offsetFrame;

			// �f�[�^�������łȂ���΁A���̎��_�Œ��~
			if (beforeFrame > frame)	{ return false; }

			// �L�[�t���[�����擾(������Βǉ�)
			auto cp = CKeyFrameUtil::_findKeyFrame<FrameDataType>(nowAry, frame, 0, true, 1, FrameDataType::InitFrom);
			// �L�[�t���[�������Ȃ��Ȃ�����I��(�ő�L�[�t���[���܂Ŗ��܂��Ă���ꍇ)
			if (cp == nullptr)	{ return false; }

			// �f�[�^�̃L�[�t���[���܂ł͌���̊O���e��ݒ肵�Ă���
			for (; cIdx < ARRAYSIZE(nowAry);)
			{
				auto &c = nowAry[cIdx];

				// �I�t�Z�b�g���O�̃t���[���̓X�L�b�v
				if (c.frame_no >= offsetFrame)
				{
					// �f�[�^�̃L�[�t���[�����o�Ă����猻��̊O���e���X�V
					if (c.frame_no == frame)
					{
						curParentModelIdx = parentModelIdx;
						curParentBoneIdx = parentBoneIdx;
					}

					// �O���e��ݒ肷��
					c.SetLook(curParentModelIdx, curParentBoneIdx);

					// �f�[�^�̃L�[�t���[���܂Őݒ肵���烋�[�v�𔲂���
					// ���̎��_�ł͎��̃L�[�t���[�����܂��ݒ肳��Ă��Ȃ�(���̃f�[�^�ŘA�������)�\��������̂�
					// ����̃��[�v�����̃t���[������n�܂�l�� cIdx �͍X�V���Ȃ�
					// ���f���̃f�[�^���g�����ꍇ�ɂ́A�����t���[�������������ꍇ�����邵��
					if (c.frame_no == frame) { break; }
				}

				// ���̃L�[�t���[����
				cIdx = c.next_index;
				// ����������ΏI��(�����`�F�b�N����Ŕ�����l�ɂ������A��{�I�ɂ͂���Ŕ����鎖�͖����Ǝv������)
				if (c.next_index == 0) { return false; }
			}
		}

		return true;
	}

	// �O���e�f�[�^�ɂ��郂�f����{�[�������݂��Ă��邩���`�F�b�N����
	// �`�F�b�N���ʂƃ{�[��/�e���f��/�e���f���̃{�[���̃C���f�b�N�X��Ԃ�
	std::tuple<bool, int, int, int> _dataExistCheck(OutsideParentOutputData data, MMDModelData *modelP)
	{
		bool	bExists = false;
		int		boneIdx = 0;
		int		parentModelIdx = data.parentModelIdx;	// -2:�n�� -1:�Ȃ� �̎��ׂ̈Ƀf�[�^���̃C���f�b�N�X�ŏ�����
		int		parentBoneIdx = 0;

		// ���f�����w�肳��Ă���΁A���f�����̃{�[�����`�F�b�N����
		if (modelP != nullptr)
		{
			// �u���[�g�v��u�S�Ώۃ{�[���v�łȂ����
			if (data.boneIdx != NonIndex &&
				data.boneIdx != AllBoneIndex)
			{
				// �w�薼�ŊO���e�́u�Ώۃ{�[���v�ɂȂ�{�[����T��
				boneIdx = _findBone(modelP, data.boneName, data.boneIdx, [](auto &b){return b.parentable_bone_index >= 1; });
				// �O���e�u���v�ݒ肷��{�[����������Ȃ���΁A���̃f�[�^�̓X�L�b�v
				if (boneIdx < 0)	{ goto _exit; }
			}
		}

		// �e���Ȃ�/�n�ʂłȂ��Ȃ�
		if (data.parentModelIdx >= 0)
		{
			// �O���e�u�Ɂv�ݒ肷�郂�f���ƃ{�[����T��
			parentModelIdx = _findModel(data.parentModelName, data.parentModelIdx);
			// ������Ȃ���΁A���̃f�[�^�̓X�L�b�v
			if (parentModelIdx < 0)	{ goto _exit; }
			auto parentModelP = m_mmdDataP->model_data[ parentModelIdx ];

			// �O���e�u�Ɂv�ݒ肷��{�[����T��
			parentBoneIdx = _findBone(parentModelP, data.parentBoneName, data.parentBoneIdx);
			// ������Ȃ���΁A���̃f�[�^�̓X�L�b�v
			if (parentBoneIdx < 0)	{ goto _exit; }
		}

		// �����܂ł���ΑS��OK
		bExists = true;

_exit:
		return { bExists, boneIdx, parentModelIdx , parentBoneIdx };
	}

	// ���f����T��
	// modelIdx �͕`�揇�ł̔ԍ�(1�`)
	// �߂�l�̓��f���z��ł̔ԍ�(0�`)
	// 
	// modelIdx �̔ԍ��Ɏw�薼�̃��f��������΂����D�悵�đI������(�����̃��f���������������ꍇ)
	// modelIdx ���Ⴄ���f�����Ȃ�擪���猟�����ĒT��
	// �w�薼���f����������Ȃ����-1
	// �ǉ��ŏ������肪�K�v�Ȃ� pred �Ŏw�肷��
	int _findModel(std::string modelName, int modelIdx, std::function<bool(MMDModelData *)> pred = nullptr)
	{
		// �������͕`�揇�ōs���āA�Ԃ��A�Ԃ̓��f���z�񏇂ƌ������Ƃ��ʓ|�Ȏ��Ɂc
		auto idxs = CMMDDataUtil::RenderOrderModelsIndex();

		// �o�͂��� render_order ��1����n�܂��Ă��邪�A�`�揇�Ƀ\�[�g�����x�N�^��0����Ȃ̂ł���ɍ��킹��
		modelIdx--;

		// �w��̔ԍ����`�F�b�N
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

		// ���f��������
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

	// ���f���̃{�[����T��
	// boneIdx �̔ԍ��Ɏw�薼�̃{�[��������΂����D�悵�đI������(�����̃{�[���������������ꍇ)
	// boneIdx ���Ⴄ�{�[�����Ȃ�擪���猟�����ĒT��
	// �w�薼�{�[����������Ȃ����-1
	// �ǉ��ŏ������肪�K�v�Ȃ� pred �Ŏw�肷��
	int _findBone(MMDModelData *modelP, std::string boneName, int boneIdx, std::function<bool(BoneCurrentData&)> pred = nullptr)
	{
		// �w��̔ԍ����`�F�b�N
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

		// �{�[��������
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

	// ���f���̊O���e�o�̓f�[�^�����
	// modelP		���f���f�[�^
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// outDataVecR	�f�[�^�����W����x�N�^
	void _saveOutsideParent(MMDModelData *modelP, int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		if (modelP == nullptr)	{ return; }

		auto &nowAry = modelP->configuration_keyframe;
		char *modelNameP = EnOrJp(modelP->name_en, modelP->name_jp);
		char *typeNameP = EnOrJp(ModelEn, ModelJp);

		// �Ώۃ{�[�������X�g�A�b�v
		struct BoneNameIdx
		{
			char			*name;
			int32_t			idx;
		};
		std::vector<BoneNameIdx>	boneIdx( modelP->parentable_bone_count );		// X�Ԗڂ́u�Ώۃ{�[���v�͑S�{�[�����̉��Ԗڂ̃{�[�����̔z��

		// 0�Ԗڂ́u�Ώۃ{�[���v�́u���[�g�v�Ȃ̂ŁA�{�[���̒��ɂ͖���
		boneIdx[0].name = EnOrJp(RootEn, RootJp);
		boneIdx[0].idx = NonIndex;

		// �u�Ώۃ{�[���z��̗v�f�ԍ��v�����{�[�����
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
			bool bEmptyFrame = true;	// �S�Ă̑Ώۃ{�[�����u�Ȃ��v�̃t���[�����ǂ���

			if (c.frame_no >= startFrame)
			{
				char *parentModelNameP;
				int parentModelIdx;
				char *parentBoneNameP;
				int parentBoneIdx;

				// �O���e�ݒ�́u�Ώۃ{�[���v�������[�v
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++)
				{
					auto &r = c.relation_setting[rIdx];

					// parent_model_index �͓����̃��f���z��̃C���f�b�N�X(�ǂݍ��܂ꂽ����)��MMD��ł̃R���{�{�b�N�X�ł̕��т� render_order �̕�
					// ���[�U�[����͌����ڏ�̕��тł̔ԍ��̕���������₷���� render_order ���o�͂���
					if (r.parent_model_index == GroundIndex || r.parent_model_index >= 0)
					{
						// �u�O���e���f���v�Łu�n�ʁv���I������Ă��邩
						if (r.parent_model_index == GroundIndex)
						{
							parentModelNameP = EnOrJp(GroundEn, GroundJp);
							parentModelIdx = GroundIndex;		// �n�ʂ�-2�ƌ������ɂ��Ă���
							parentBoneNameP = "------";
							parentBoneIdx = GroundIndex;		// �n�ʂ�-2�ƌ������ɂ��Ă���
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

						// ���(���f��/�J����/�A�N�Z�T��), �t���[��, ���f����, ���f���`�揇Index, �Ώۃ{�[����, �Ώۃ{�[��Index, �O���e���f����, �O���e���f���`�揇Index, �O���e�{�[����, �O���e�{�[��Index
						outDataVecR.emplace_back (
									  typeNameP							// ���
									, c.frame_no						// �t���[��
									, modelNameP						// ���f����
									, modelP->render_order				// ���f���`�揇Index
									, boneIdx[rIdx].name				// �Ώۃ{�[����
									, boneIdx[rIdx].idx					// �Ώۃ{�[��Index
									, parentModelNameP					// �O���e���f����
									, parentModelIdx					// �O���e���f���`�揇Index
									, parentBoneNameP					// �O���e�{�[����
									, parentBoneIdx						// �O���e�{�[��Index
						);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG

						// 1�ł��o�͂��Ă������t���[���ł͂Ȃ�
						bEmptyFrame = false;
					}
				}

				// ��t���[���Ȃ�A�u�S�Ώۃ{�[���v���u�Ȃ��v�ŏo�͂��Ă���
				if (bEmptyFrame)
				{
					char *allBoneNameP = EnOrJp(AllBoneEn, AllBoneJp);
					int allBoneIdx = AllBoneIndex;	// �u�S�Ώۃ{�[���v��-3�ƌ������ɂ��Ă���
					parentModelNameP = EnOrJp(NonEn, NonJp);
					parentModelIdx = NonIndex;			// �Ȃ���-1�ƌ������ɂ��Ă���
					parentBoneNameP = "------";
					parentBoneIdx = NonIndex;			// �Ȃ���-1�ƌ������ɂ��Ă���

					// ���(���f��/�J����/�A�N�Z�T��), �t���[��, ���f����, ���f���`�揇Index, �Ώۃ{�[����, �Ώۃ{�[��Index, �O���e���f����, �O���e���f���`�揇Index, �O���e�{�[����, �O���e�{�[��Index
					outDataVecR.emplace_back (
								  typeNameP							// ���
								, c.frame_no						// �t���[��
								, modelNameP						// ���f����
								, modelP->render_order				// ���f���`�揇Index
								, allBoneNameP						// �Ώۃ{�[����
								, allBoneIdx						// �Ώۃ{�[��Index
								, parentModelNameP					// �O���e���f����
								, parentModelIdx					// �O���e���f���`�揇Index
								, parentBoneNameP					// �O���e�{�[����
								, parentBoneIdx						// �O���e�{�[��Index
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
				}

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}
	}

	// �J�����t���[���̊O���e�o�̓f�[�^�����
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// outDataVecR	�f�[�^�����W����x�N�^
	void _saveCameraFollowBone(int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		auto &nowAry = m_mmdDataP->camera_key_frame;
		int beforeModelIdx = 0xFFFF;	// 0�t���[���ڂ��o�͂����l�ɂ��蓾�Ȃ��l�ŏ�����
		int beforeBoneIdx = 0xFFFF;
		char *typeNameP = EnOrJp(CameraEn, CameraJp);

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// �{�[���Ǐ]�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1
				auto &b = c.looking_bone_index;		// �{�[���̃C���f�b�N�X 0�`

				// �J����/�A�N�Z�T���͈ʒu�ƒǏ]�������t���[���f�[�^�ɓ����Ă���ׁA�Ǐ]���Ă��Ȃ��Ă��L�[�t���[������ʂɑ��݂���
				// �Ȃ̂ŁA�ω��̂������t���[���̂ݏo�͂��āA���A���͊����t���[�����������Đݒ肵�Ă���
				// �ω��̂���t���[���̂ݏo�͂���
				if (beforeModelIdx != m || beforeBoneIdx != b)
				{
					beforeModelIdx = m;
					beforeBoneIdx = b;

					char *parentModelNameP;
					int parentModelIdx;
					char *parentBoneNameP;
					int parentBoneIdx;

					// �{�[���Ǐ]���u�Ȃ��v��
					if (m == NonIndex)
					{
						parentModelNameP = EnOrJp(NonEn, NonJp);
						parentModelIdx = NonIndex;			// �Ȃ���-1�ƌ������ɂ��Ă���
						parentBoneNameP = "------";
						parentBoneIdx = NonIndex;			// �Ȃ���-1�ƌ������ɂ��Ă���
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

					// ���(���f��/�J����/�A�N�Z�T��), �t���[��, ���f����, ���f���`�揇Index, �Ώۃ{�[����, �Ώۃ{�[��Index, �O���e���f����, �O���e���f���`�揇Index, �O���e�{�[����, �O���e�{�[��Index
					outDataVecR.emplace_back (
								  typeNameP				// ���
								, c.frame_no			// �t���[��
								, typeNameP				// ���f���� �J�����Ƀ��f�����͖������Ǖ\����u�J�����v�ɂ��Ă���
								, NonIndex				// ���f���`�揇Index
								, "------"				// �Ώۃ{�[����
								, NonIndex				// �Ώۃ{�[��Index
								, parentModelNameP		// �O���e���f����
								, parentModelIdx		// �O���e���f���`�揇Index
								, parentBoneNameP		// �O���e�{�[����
								, parentBoneIdx			// �O���e�{�[��Index
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
				}

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}
	}

	// �A�N�Z�T���t���[���̊O���e�o�̓f�[�^�����
	// accP			�A�N�Z�T���f�[�^
	// nowAry		�t���[���z�� accessory_key_frames[] ��
	// bCamera		�����J�����̔z��
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// outDataVecR	�f�[�^�����W����x�N�^
	void _saveAccFollowBone(MMDAccessoryData *accP, AccessoryKeyFrameData (&nowAry)[10000], int startFrame, int endFrame, std::vector<OutsideParentOutputData> &outDataVecR)
	{
		if (accP == nullptr)	{ return; }

		int beforeModelIdx = 0xFFFF;	// 0�t���[���ڂ��o�͂����l�ɂ��蓾�Ȃ��l�ŏ�����
		int beforeBoneIdx = 0xFFFF;
		char *typeNameP = EnOrJp(AccEn, AccJp);

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				auto &m = c.looking_model_index;	// �{�[���Ǐ]�̃��f���̃C���f�b�N�X �n�ʂ̏ꍇ��-1
				auto &b = c.looking_bone_index;		// �{�[���̃C���f�b�N�X 0�`
				
				// �J����/�A�N�Z�T���͈ʒu�ƒǏ]�������t���[���f�[�^�ɓ����Ă���ׁA�Ǐ]���Ă��Ȃ��Ă��L�[�t���[������ʂɑ��݂���
				// �Ȃ̂ŁA�ω��̂������t���[���̂ݏo�͂��āA���A���͊����t���[�����������Đݒ肵�Ă���
				// �ω��̂���t���[���̂ݏo�͂���
				if (beforeModelIdx != m || beforeBoneIdx != b)
				{
					beforeModelIdx = m;
					beforeBoneIdx = b;

					char *parentModelNameP;
					int parentModelIdx;
					char *parentBoneNameP;
					int parentBoneIdx;

					// �{�[���Ǐ]���u�n�ʁv��
					if (m == NonIndex)
					{
						parentModelNameP = EnOrJp(GroundEn, GroundJp);
						parentModelIdx = GroundIndex;		// �n�ʂ�-2�ƌ������ɂ��Ă���
						parentBoneNameP = "------";
						parentBoneIdx = GroundIndex;		// �n�ʂ�-2�ƌ������ɂ��Ă���
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

					// ���(���f��/�J����/�A�N�Z�T��), �t���[��, ���f����, ���f���`�揇Index, �Ώۃ{�[����, �Ώۃ{�[��Index, �O���e���f����, �O���e���f���`�揇Index, �O���e�{�[����, �O���e�{�[��Index
					outDataVecR.emplace_back (
								  typeNameP				// ���
								, c.frame_no			// �t���[��
								, accP->name			// ���f����
								, accP->render_order	// ���f���`�揇Index
								, "------"				// �Ώۃ{�[����
								, NonIndex				// �Ώۃ{�[��Index
								, parentModelNameP		// �O���e���f����
								, parentModelIdx		// �O���e���f���`�揇Index
								, parentBoneNameP		// �O���e�{�[����
								, parentBoneIdx			// �O���e�{�[��Index
					);
#ifdef USE_DEBUG
					_dbgPrint(outDataVecR.back().ToCsv().c_str());
#endif // USE_DEBUG
				}

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}
	}

	// �G���[�_�C�A���O
	void _saveErrorDlg(wchar_t* en, wchar_t* jp) { _errorDlg(en, jp, true); }
	void _loadErrorDlg(wchar_t* en, wchar_t* jp) { _errorDlg(en, jp, false); }
	void _errorDlg(wchar_t* en, wchar_t* jp, bool bSave)
	{
		auto title = bSave 
						? EnOrJp(_T("save outside parent data"), _T("�O���e�f�[�^�ۑ�"))
						: EnOrJp(_T("load outside parent data"), _T("�O���e�f�[�^�Ǎ�"));
		MessageBox(getHWND(), EnOrJp(en, jp), title, MB_OK | MB_ICONERROR);
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMD�̃o�[�W������ 9.31 �ł͂���܂���B\nOutsideParentReaderWriter�� Ver.9.31 �ȊO�ł͐���ɍ쓮���܂���B"), _T("OutsideParentReaderWriter"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return OutsideParentReaderWriterPlugin::GetInstance();
}

