// SetCameraFollowBone.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
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

extern HMODULE g_module;

#define ComboBox_Clear(hwndCtl)						while (ComboBox_GetCount(hwndCtl) != 0) { ComboBox_DeleteString((hwndCtl), 0); }
#define ComboBox_AddStringA(hwndCtl, lpsz)			((int)(DWORD)::SendMessageA((hwndCtl), CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz)))
#define ComboBox_SetDroppedWidth(hwndCtl, len)		((int)(DWORD)::SendMessage((hwndCtl), CB_SETDROPPEDWIDTH, (WPARAM)(len), 0L))
//////////////////////////////////////////////////////////////////////
class SetCameraFollowBonePlugin : public MMDPluginDLL3, public CMmdCtrls, public Singleton<SetCameraFollowBonePlugin>, public CPostMsgHook
{
private:
	MMDMainData*									m_mmdDataP;
	bool											m_bShowWindow;		// ����� WM_SHOWWINDOW ����p
	CKeyState										m_testKey;			// �e�X�g�̃L�[
	CMenu											m_menu;				// ���j���[
	std::vector<int32_t>							m_modelIdx;			// ���f���R���{�{�b�N�X�ɕ\�����郂�f���̃C���f�b�N�X���X�g
	std::vector<int32_t>							m_boneIdx;			// �{�[���R���{�{�b�N�X�ɕ\������{�[���̃C���f�b�N�X���X�g

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
		// �t�b�N�J�n
		HookStart(g_module);

		m_menu.AddSeparator(CMenu::MmdMenu::Edit);
		m_menu.Create(CMenu::MmdMenu::Edit, _T("��ׂ��ްݒǏ]�ݒ�(&F)"), _T("set camera follow bone(&F)")
					, [this](CMenu*) { OnMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(m_mmdDataP->is_camera_edit_mode); } ); // �J�����ҏW���[�h���̂ݑI�ׂ�l�ɂ���
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

		m_menu.OnWndProc(param);
	}

	void PostMsgProc(int code, MSG* param) override
	{
		m_menu.OnPostMsg(param);
	}

	// ���j���[�����s���ꂽ�Ƃ�
	void OnMenuExec()
	{
		DialogBox( (HINSTANCE)g_module,
			MAKEINTRESOURCE(m_mmdDataP->is_english_mode ? IDD_DIALOG_EN : IDD_DIALOG_JP),
			getHWND(),
			(DLGPROC)SDlgWndProc);
	}

	// �_�C�A���O�p�E�B���h�E�v���V�[�W��
	static BOOL CALLBACK SDlgWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		return GetInstance()->DlgWndProc(hWnd, iMsg, wParam, lParam);
	}
	BOOL DlgWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (iMsg)
		{
			case WM_INITDIALOG:
				//�_�C�A���O�{�b�N�X���������ꂽ�Ƃ�
				{
					auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
					if (modelCombo)
					{
						// ���f���̃C���f�b�N�X���X�g���쐬
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

						// �S���ڍ폜
						ComboBox_Clear(modelCombo);

						// ���f�������R���{�{�b�N�X�ɒǉ�
						ComboBox_AddStringA(modelCombo, m_mmdDataP->is_english_mode ? "non" : "�Ȃ�");

						if (!m_modelIdx.empty())
						{
							int32_t maxLen = 20;

							// ���f���`�揇�Ń\�[�g
							std::sort(m_modelIdx.begin(), m_modelIdx.end(), [this](const int32_t l, const int32_t r)
							{
								return m_mmdDataP->model_data[l]->render_order < m_mmdDataP->model_data[r]->render_order;
							});

							// ���f�������R���{�{�b�N�X�ɒǉ�
							for (auto mIdx : m_modelIdx)
							{
								auto modelP = m_mmdDataP->model_data[mIdx];
								char *nameP = m_mmdDataP->is_english_mode ? modelP->name_en : modelP->name_jp;
								ComboBox_AddStringA(modelCombo, nameP);

								// �ő�̕������𒲂ׂĂ���
								int32_t len = static_cast<int32_t>(strlen(nameP));
								if (len > maxLen) { maxLen = len; }
							}
							// �h���b�v�_�E���������̕��𒲐� �������œK���Ɂc
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
						// ���f���̑I�����ڂ��ύX���ꂽ�Ƃ�
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							int		mIdx = -1;

							// ���݂̃��f���R���{�̃C���f�b�N�X���擾
							auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
							if (modelCombo)
							{
								// 0�Ԗڂ́u�Ȃ��v�Ȃ̂ŁA���f��������̂�1�`
								mIdx = ComboBox_GetCurSel(modelCombo) - 1;
							}

							auto boneCombo = GetDlgItem(hWnd, BONE_COMBO);
							if (boneCombo)
							{
								m_boneIdx.clear();
								// �S���ڍ폜
								ComboBox_Clear(boneCombo);

								if (0 <= mIdx && mIdx < m_modelIdx.size())
								{
									auto modelP = m_mmdDataP->model_data[ m_modelIdx[mIdx] ];
									if (modelP)
									{
										// �\������{�[���̃��X�g�����
										for (int bIdx = 0; bIdx < modelP->bone_count; bIdx++)
										{
											// �u�\���v���uIK�v��ON�̃{�[�����Ώ�
											// IK��Link�̃{�[�����ΏۂɂȂ�
											// �t���O���Ȃ��̃{�[�����ΏۂɂȂ�
											auto &boneR = modelP->bone_current_data[bIdx];
											if (boneR.flg & BoneCurrentData::BoneFlag::IK)
											{
												_addIfNotExist(m_boneIdx, bIdx);
												// Link���T�[�`
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
											else if (boneR.flg == 0)
											{
												_addIfNotExist(m_boneIdx, bIdx);
											}
										}

										// �����Ń\�[�g
										std::sort(m_boneIdx.begin(), m_boneIdx.end());

										int32_t maxLen = 20;

										// �{�[�������R���{�{�b�N�X�ɃZ�b�g
										for (auto bIdx : m_boneIdx)
										{
											auto &boneR = modelP->bone_current_data[bIdx];
											// ��������wchar_t
											wchar_t *nameP = m_mmdDataP->is_english_mode ? boneR.wname_en : boneR.wname_jp;

											ComboBox_AddString(boneCombo, nameP);

											// �ő�̕������𒲂ׂĂ���
											int32_t len = lstrlen(nameP);
											if (len > maxLen) { maxLen = len; }
										}

										// �h���b�v�_�E���������̕��𒲐� �������œK���Ɂc
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

						// ���݂̃��f���R���{�̃C���f�b�N�X���擾
						auto modelCombo = GetDlgItem(hWnd, MODEL_COMBO);
						if (modelCombo)
						{
							// 0�Ԗڂ́u�Ȃ��v�Ȃ̂ŁA���f��������̂�1�`
							auto mIdx = ComboBox_GetCurSel(modelCombo) - 1;
							if (mIdx == -1)
							{
								targetModel = -1; // -1�̏ꍇ�͉��������
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

						// �{�[���Ǐ]���Z�b�g����
						if (targetModel >= -1 && targetBone >= 0)
						{
							_setFollowBone(targetModel, targetBone);
							// �ĕ`��
							Repaint2();
						}
					}
						/// break; �X���[���ĉ���
					case IDCANCEL:
						m_modelIdx.clear();
						m_boneIdx.clear();
						//���[�_���_�C�A���O�{�b�N�X��j��
						EndDialog(hWnd, 0);
						break;
				}
				return TRUE;

			default:
				return FALSE;
		}
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

	// �I�𒆂̃J�����t���[���Ƀ��f���Ǐ]���Z�b�g����
	// lookingModel	�{�[���Ǐ]�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1
	// lookingBone	�{�[���̃C���f�b�N�X 0�`
	void _setFollowBone(int lookingModel, int lookingBone)
	{
		for (int cIdx = 0; cIdx < ARRAYSIZE(m_mmdDataP->camera_key_frame);)
		{
			auto &c = m_mmdDataP->camera_key_frame[cIdx];

			// �I�𒆂Ȃ�Ǐ]���Z�b�g����
			if (c.is_selected)
			{
				c.looking_model_index = lookingModel;
				c.looking_bone_index = lookingBone;
			}

			// ����������ΏI��
			if (c.next_index == 0) { break; }
			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}
	}

	void _addIfNotExist(std::vector<int32_t> &vec, int32_t no)
	{
		auto it = std::find(vec.cbegin(), vec.cend(), no);
		if (it == vec.cend()) { vec.push_back(no); } // ������Βǉ�
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMD�̃o�[�W������ 9.31 �ł͂���܂���B\nSetCameraFollowBone�� Ver.9.31 �ȊO�ł͐���ɍ쓮���܂���B"), _T("SetCameraFollowBone"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return SetCameraFollowBonePlugin::GetInstance();
}

