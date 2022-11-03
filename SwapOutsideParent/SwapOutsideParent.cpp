// SwapOutsideParent.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
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
	bool											m_bShowWindow;		// ����� WM_SHOWWINDOW ����p
	CKeyState										m_testKey;			// �e�X�g�̃L�[
	CMenu											m_menu;				// ���j���[
	std::vector<int32_t>							m_modelIdx;			// ���f���R���{�{�b�N�X�ɕ\�����郂�f���̃C���f�b�N�X���X�g
	std::vector<int32_t>							m_fromBoneIdx;		// �{�[���R���{�{�b�N�X�ɕ\������{�[���̃C���f�b�N�X���X�g
	std::vector<int32_t>							m_toBoneIdx;		// �{�[���R���{�{�b�N�X�ɕ\������{�[���̃C���f�b�N�X���X�g
	HWND											m_parentBtnH;		// �u���f������v�p�l���́u�O�v�{�^��

public:
	const char* getPluginTitle() const override { return "SwapOutsideParent"; }

	SwapOutsideParentPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
		, m_parentBtnH(NULL)
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
			ini.Int(L"MdlCheck") = 1;
			ini.Int(L"CamCheck") = 1;
			ini.Int(L"AccCheck") = 1;
		});

		m_testKey.SetKeyCode(L"C");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();
		// �t�b�N�J�n
		HookStart(g_module);

		m_menu.AddSeparator(CMenu::MmdMenu::Edit);
		m_menu.Create(CMenu::MmdMenu::Edit, _T("�O���e�����ւ�(&O)"), _T("swap outside parent(&O)")
					, [this](CMenu*) { OnMenuExec(); }
					, [this](CMenu*m) {m->SetEnable(true); } ); // �펞�I�ׂ�l�ɂ���
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

		m_menu.OnWndProc(param);

		{
			HWND hWnd = param->hwnd;
			UINT iMsg = param->message;
			WPARAM wParam = param->wParam;
			LPARAM lParam = param->lParam;

			switch (iMsg)
			{
				case WM_CONTEXTMENU:
					// �O���e�{�^�����E�N���b�N���ꂽ
					if (param->hwnd  == m_parentBtnH &&
						(HWND)wParam == m_parentBtnH)
					{
						// �O���e�����ւ� �_�C�A���O���J��
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
					auto startFrameEdit	= GetDlgItem(hWnd, START_FRAME_EDIT);
					auto endFrameEdit	= GetDlgItem(hWnd, END_FRAME_EDIT);
					auto fromModelCombo = GetDlgItem(hWnd, FROM_MODEL_COMBO);
					auto toModelCombo	= GetDlgItem(hWnd, TO_MODEL_COMBO);
					auto mdlCheck		= GetDlgItem(hWnd, MODEL_CHECK);
					auto camCheck		= GetDlgItem(hWnd, CAMERA_CHECK);
					auto accCheck		= GetDlgItem(hWnd, ACC_CHECK);

					int startFrame = 0;
					int endFrame = -1;

					// �ݒ�ǂݍ���
					m_ini.Read();

					if (fromModelCombo && toModelCombo)
					{
						// ���f���̃C���f�b�N�X���X�g���쐬
						m_modelIdx.clear();	// ���f���͂ǂ���̃R���{�{�b�N�X�������Ȃ̂Ń��X�g�͈��OK
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

						// �S���ڍ폜
						ComboBox_Clear(fromModelCombo);
						ComboBox_Clear(  toModelCombo);

						// ���f�������R���{�{�b�N�X�ɒǉ�
						ComboBox_AddStringA(fromModelCombo, m_mmdDataP->is_english_mode ? "non" : "�Ȃ�");
						ComboBox_AddStringA(  toModelCombo, m_mmdDataP->is_english_mode ? "non" : "�Ȃ�");
						ComboBox_AddStringA(fromModelCombo, m_mmdDataP->is_english_mode ? "ground" : "�n��");
						ComboBox_AddStringA(  toModelCombo, m_mmdDataP->is_english_mode ? "ground" : "�n��");

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
								ComboBox_AddStringA(fromModelCombo, nameP);
								ComboBox_AddStringA(  toModelCombo, nameP);

								// �ő�̕������𒲂ׂĂ���
								int32_t len = static_cast<int32_t>(strlen(nameP));
								if (len > maxLen) { maxLen = len; }
							}
							// �h���b�v�_�E���������̕��𒲐� �������œK���Ɂc
							ComboBox_SetDroppedWidth(fromModelCombo, 7.5 * maxLen);
							ComboBox_SetDroppedWidth(  toModelCombo, 7.5 * maxLen);
						}

						// ���݁u���f������v�őI������Ă��郂�f���̗v�f�ԍ���T��
						int selectIndex = 0;
						auto itr = std::find(m_modelIdx.begin(), m_modelIdx.end(), m_mmdDataP->select_model );
						if ( itr != m_modelIdx.end() )
						{
							selectIndex = (int)std::distance(m_modelIdx.begin(), itr) + 2;
						}

						ComboBox_SetCurSel(fromModelCombo, selectIndex);
						ComboBox_SetCurSel(  toModelCombo, selectIndex);
						// �{�[���̃R���{�{�b�N�X��ݒ肷��ׂɎ��O�ŌĂяo���Ȃ��Ƃ����Ȃ��c
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
						// �ݒ��Ԃ𕜋A
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
						// ���f���̑I�����ڂ��ύX���ꂽ�Ƃ�
						if (HIWORD(wParam) == CBN_SELCHANGE)
						{
							int		mIdx = -1;

							// ���݂̃��f���R���{�̃C���f�b�N�X���擾
							auto modelCombo = GetDlgItem(hWnd, LOWORD(wParam));
							if (modelCombo)
							{
								// 0�Ԗڂ́u�Ȃ��v1�Ԗڂ́u�n�ʁv�Ȃ̂ŁA���f��������̂�2�`
								mIdx = ComboBox_GetCurSel(modelCombo) - 2;
							}

							// �Ή�����{�[���̃R���{�{�b�N�X��ID
							bool bFrom = (LOWORD(wParam) == FROM_MODEL_COMBO);
							auto boneComboId = bFrom ? FROM_BONE_COMBO : TO_BONE_COMBO;
							auto &boneIdx = bFrom ? m_fromBoneIdx : m_toBoneIdx;
							auto boneCombo = GetDlgItem(hWnd, boneComboId);
							if (boneCombo)
							{
								boneIdx.clear();
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
												_addIfNotExist(boneIdx, bIdx);
												// Link���T�[�`
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

										// �����Ń\�[�g
										std::sort(boneIdx.begin(), boneIdx.end());

										int32_t maxLen = 20;

										// �{�[�������R���{�{�b�N�X�ɃZ�b�g
										if (bFrom)	{ ComboBox_AddStringA(boneCombo, m_mmdDataP->is_english_mode ? "<all bones>" : "<�S�{�[��>"); }
										else		{ ComboBox_AddStringA(boneCombo, m_mmdDataP->is_english_mode ? "<same name>" : "<�����{�[��>"); }
										for (auto bIdx : boneIdx)
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
								else
								{
								}
								ComboBox_SetCurSel(boneCombo, 0);
							}
						}
						break;

					case SWAP_BTN:
					{
						// ���݂̃��f���R���{�̃C���f�b�N�X���擾
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

								// 0�Ԗڂ́u�Ȃ��v1�Ԗڂ́u�n�ʁv�Ȃ̂ŁA���f��������̂�2�`
								auto fmIdx = ComboBox_GetCurSel(fromModelCombo) - 2;
								auto tmIdx = ComboBox_GetCurSel(  toModelCombo) - 2;
								// 0�Ԗڂ́u<�S�{�[��>�v�Ȃ̂ŁA�{�[��������̂�1�`
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
										// �R���{�{�b�N�X�ł͕��ѓI�� -1:�n�� -2:�Ȃ� �����A
										// �����I�ɂ� -1:�Ȃ� -2:�n�ʂȂ̂ŋt�ɂ���
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

								// �R���{�{�b�N�X�̃C���f�b�N�X�����f����{�[���̔ԍ��ɕϊ�����
								idx2No(m_modelIdx, m_fromBoneIdx, fmIdx, fbIdx, fromModel, fromBone);
								idx2No(m_modelIdx,   m_toBoneIdx, tmIdx, tbIdx,   toModel,   toBone);

								// �{�[���Ǐ]���Z�b�g����
								if (fromModel >= -2 && fromBone >= -1 &&
									  toModel >= -2 &&   toBone >= -1)
								{
									// ���f��
									if (bMdl)
									{
										for (auto &model : m_mmdDataP->model_data)
										{
											_swapOutsideParent(model, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
										}
									}
									// �J����
									if (bCam)
									{
										_swapFollowBone(m_mmdDataP->camera_key_frame, true, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
									}
									// �A�N�Z�T��
									if (bAcc)
									{
										for (auto &acc : m_mmdDataP->accessory_key_frames)
										{
											_swapFollowBone(*acc, false, startFrame, endFrame, fromModel, fromBone, toModel, toBone);
										}
									}
									
									// �ĕ`��
									Repaint2();

									// �ݒ��ۑ�
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
						/// break; �X���[���ĉ���
					case IDCANCEL:
						m_modelIdx.clear();
						m_fromBoneIdx.clear();
						m_toBoneIdx.clear();
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

		// �u���f������v�p�l���́u�O�v�{�^�����擾����
		m_parentBtnH = GetDlgItem(hMmd, MODEL_PARENT_BTN_CTRL_ID);
	}

	// ���f���̊O���e�������ւ���
	// nowAry		�t���[���z�� configuration_keyframe
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// fromModel	�O���e�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1 �n�ʂ̏ꍇ��-2
	// fromBone		�{�[���̃C���f�b�N�X -1�̏ꍇ�́u�S�{�[���v
	// toModel		�O���e�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1 �n�ʂ̏ꍇ��-2
	// toBone		�{�[���̃C���f�b�N�X -1�̏ꍇ�́u�����{�[���v
	void _swapOutsideParent(MMDModelData *modelP, int startFrame, int endFrame, int fromModel, int fromBone, int toModel, int toBone)
	{
		if (modelP == nullptr)	{ return; }

		auto &nowAry = modelP->configuration_keyframe;

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				// �O���e�ݒ�́u�Ώۃ{�[���v�������[�v
				for (int rIdx = 0; rIdx < modelP->parentable_bone_count; rIdx++ )
				{
					auto &r = c.relation_setting[rIdx];

					// �����ւ�
					_swap(fromModel, fromBone, toModel, toBone, r.parent_model_index, r.parent_bone_index);
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

	// �J����/�A�N�Z�T���t���[���̃��f���Ǐ]�������ւ���
	// nowAry		�t���[���z�� camera_key_frame �� accessory_key_frames[] ��
	// bCamera		�����J�����̔z��
	// startFrame	�J�n�t���[��
	// endFrame		�I���t���[��
	// fromModel	�{�[���Ǐ]�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1 �n�ʂ̏ꍇ��-2(�J�����ł͉������Ȃ�)
	// fromBone		�{�[���̃C���f�b�N�X -1�̏ꍇ�́u�S�{�[���v
	// toModel		�{�[���Ǐ]�̃��f���̃C���f�b�N�X �Ȃ��̏ꍇ��-1 �n�ʂ̏ꍇ��-2(�J�����ł͉������Ȃ�)
	// toBone		�{�[���̃C���f�b�N�X -1�̏ꍇ�́u�����{�[���v
	template<class FrameDataType>
	void _swapFollowBone(FrameDataType(&nowAry)[10000], bool bCamera, int startFrame, int endFrame, int fromModel, int fromBone, int toModel, int toBone)
	{
	/// �J����/�A�N�Z�T���́u�n�ʁv�́u�Ȃ��v�����̕����ǂ��̂�?
	///	// �J�����̏ꍇ�́u�n�ʁv�͉������Ȃ��A�A�N�Z�T���̏ꍇ�́u�Ȃ��v�͉������Ȃ�
	///	if (fromModel == (bCamera ? -2 : -1)) { return; }
	///	if (  toModel == (bCamera ? -2 : -1)) { return; }

		// �u�n�ʁv�̏ꍇ�́u�Ȃ��v�ɂ��Ă���
		if (fromModel < -1) { fromModel = -1; }
		if (  toModel < -1) {   toModel = -1; }

		for (int cIdx = 0; cIdx < ARRAYSIZE(nowAry);)
		{
			auto &c = nowAry[cIdx];

			if (c.frame_no >= startFrame)
			{
				// �����ւ�
				_swap(fromModel, fromBone, toModel, toBone, c.looking_model_index, c.looking_bone_index);

				// ����������ΏI��
				if (c.next_index == 0) { break; }
				// �I���t���[�����߂���ΏI��
				if (endFrame >= 0 && c.frame_no >= endFrame) { break; }
			}

			// ���̃L�[�t���[����
			cIdx = c.next_index;
		}
	}

	// �����ւ����s����
	// rModelIndex	�����ւ��郂�f���̃C���f�b�N�X
	// rBoneIndex	�����ւ���{�[���̃C���f�b�N�X
	void _swap(int fromModel, int fromBone, int toModel, int toBone, int &rModelIndex, int &rBoneIndex)
	{
		bool bSwap = false;	// �ύX�O�̃{�[������v���邩?

		// ���f���̃C���f�b�N�X��������?
		if (fromModel == rModelIndex)
		{
			// �u�Ȃ��v���w�肳��Ă���΁u�Ȃ��v�̃t���[�����Ώ�
			if (fromModel == -1)
			{
				bSwap = true;
			}
			// �S�{�[����Ώۂɂ��邩�A�{�[���̃C���f�b�N�X����v����ꍇ
			else if (fromBone == -1 ||
						fromBone == rBoneIndex)
			{
				bSwap = true;
			}
		}

		// �����ւ���
		if (bSwap)
		{
			// ���Ɠ����{�[���ɕς���(���f�������ς���)�ꍇ�A�����̃{�[�������邩�T��
			if (toBone == -1)
			{
				// �����ւ��O�ƌ�ǂ�������f�����ݒ肳��Ă���
				if (rModelIndex >= 0 &&
					toModel >= 0)
				{
					auto fromModelP = m_mmdDataP->model_data[rModelIndex];
					auto   toModelP = m_mmdDataP->model_data[toModel];
					// ���f�����ǂݍ��܂�Ă���
					if (fromModelP != nullptr && toModelP != nullptr)
					{
						auto &fromBoneR = fromModelP->bone_current_data[rBoneIndex];
						auto fromName = m_mmdDataP->is_english_mode ? fromBoneR.wname_en : fromBoneR.wname_jp;

						for (int idx = 0; idx < toModelP->bone_count; idx++)
						{
							auto &toBoneR = toModelP->bone_current_data[idx];
							auto   toName = m_mmdDataP->is_english_mode ?   toBoneR.wname_en :   toBoneR.wname_jp;

							// ���O���ꏏ�Ȃ�m��
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
			// ����ȊO�͂��̂܂�
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
		if (it == vec.cend()) { vec.push_back(no); } // ������Βǉ�
	}
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMD�̃o�[�W������ 9.31 �ł͂���܂���B\nSwapOutsideParent�� Ver.9.31 �ȊO�ł͐���ɍ쓮���܂���B"), _T("SwapOutsideParent"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return SwapOutsideParentPlugin::GetInstance();
}

