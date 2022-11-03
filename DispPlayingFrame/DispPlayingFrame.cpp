// DispPlayingFrame.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
//

#include "stdafx.h"
#include <deque>
#include <map>
#include <Tchar.h>
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
	bool											m_bShowWindow;		// ����� WM_SHOWWINDOW ����p
	HWND											m_myTextH;			// �t���[���\���p�Ɏ��삷��e�L�X�g
	HFONT											m_myTextFontH;		// �t���[���\���p�e�L�X�g�̃t�H���g
	int												m_is_playing;		// �Đ������ǂ��� 0:��~�� 1:�Đ����ω����m�p�R�s�[
	int												m_now_frame;		// ���݂̃t���[���ω����m�p�R�s�[
	float											m_play_sec;			// �Đ����̐擪����̕b���ω����m�p�R�s�[

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
		// �^�C�}�[OFF
		KillTimer(m_myTextH, (UINT_PTR)this);

		// �t�H���g���f�t�H���g�ɖ߂�
		if (m_myTextH)
		{
			SendMessage(m_myTextH, WM_SETFONT, (WPARAM)nullptr, TRUE);
		}
		// �t�H���g�̉��
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

		if (param->message == WM_WINDOWPOSCHANGED)
		{
			auto posP = (WINDOWPOS*)param->lParam;
			// �ړ��Ȃ�
			if ((posP->flags & SWP_NOMOVE) == 0)
			{
				// MMD�́u�����v�̃G�f�B�b�g�{�b�N�X���������玩��̃{�^�����Ǐ]����
				if (param->hwnd == m_lenEditH)
				{
					// �u�ʑ��v�Őe���ς���Ă����ꍇ�A�����E�B���h�E�Ɉړ�����
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
					SetBkMode(hDC, TRANSPARENT);			// �w�i��h��Ԃ�
					SetTextColor(hDC, RGB(255, 0, 0));		// �e�L�X�g�̐F
					SetBkColor(hDC, RGB(192, 192, 192));	// �e�L�X�g��������Ă��镔���̃e�L�X�g�̔w�i�̐F
				//	return{ true, (LRESULT)COLOR_WINDOW };	// �e�L�X�g��������Ă��Ȃ������̔w�i�̐F
				}
			}
		}
	}

private:
	// ����� WM_SHOWWINDOW �O�ɌĂ΂��
	// ���̎��_�ł̓{�^�����̃R���g���[�����o���Ă���͂�
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hMmd = getHWND();
		TCHAR *defaultTxtP = _T("000000 (00:00.00)");

		// �R���g���[���n���h���̏�����
		InitCtrl();

		// �u�����v�̃G�f�B�b�g�{�b�N�X�Ɠ����t�H���g�ɂ��Ă���
		LOGFONT lfont = { 0 };
		HFONT fontH = (HFONT)SendMessage(m_lenEditH, WM_GETFONT, 0, 0);
		GetObject(fontH, sizeof(LOGFONT), &lfont);
		lfont.lfHeight = -24;
		lfont.lfWidth = 0;
		m_myTextFontH = CreateFontIndirect(&lfont);

		// �f�t�H���g�������ł̕��������擾
		auto screenHdc = GetDC(nullptr);
		HDC hdc = CreateCompatibleDC(screenHdc);
		SelectObject(hdc, m_myTextFontH);
		SIZE txtSize = { 0 };
		GetTextExtentPoint32(hdc, defaultTxtP, lstrlen(defaultTxtP), &txtSize);
		DeleteObject(hdc);
		ReleaseDC(nullptr, screenHdc);

		// �X�^�e�B�b�N�e�L�X�g�����
		m_myTextH = CreateWindow(_T("EDIT"), defaultTxtP, WS_VISIBLE | WS_CHILD | ES_READONLY | ES_RIGHT,
								0, 0, txtSize.cx + 16, 24, hMmd, (HMENU)(UINT_PTR)MY_PLAY_FRAME_STATIC_CTRL_ID, hInstance, NULL);
		EnableWindow(m_myTextH, false);

		SendMessage(m_myTextH, WM_SETFONT, (WPARAM)m_myTextFontH, TRUE);

		// �^�C�}�[�J�n
		SetTimer(m_myTextH, (UINT_PTR)this, 10, [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			DispPlayingFramePlugin * thisP = (DispPlayingFramePlugin *)idEvent;
			thisP->_frameCheck(false);
		});
	}

	// ���݂̃t���[�����ς���Ă�����ĕ`�悷��
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

			// �\��
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
		MessageBox(getHWND(), _T("MMD�̃o�[�W������ 9.31 �ł͂���܂���B\nDispPlayingFrame�� Ver.9.31 �ȊO�ł͐���ɍ쓮���܂���B"), _T("DispPlayingFrame"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return DispPlayingFramePlugin::GetInstance();
}