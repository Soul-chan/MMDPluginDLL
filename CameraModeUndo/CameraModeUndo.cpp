// CameraUndo.cpp : DLL �A�v���P�[�V�����p�ɃG�N�X�|�[�g�����֐����`���܂��B
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
struct KeyFrameChange
{
	union DataUnion
	{
		uint32_t	use_bit;
		int32_t		iVal;
		float		fVal;
		int16_t		sVal[2];
		int8_t		cVal[4];

		DataUnion() {}
		DataUnion(int32_t v) : iVal(v) {}
		DataUnion(float v) : fVal(v) {}
		DataUnion(int16_t v1, int16_t v2) { sVal[0]=v1; sVal[1]=v2; }
		DataUnion(int8_t v1, int8_t v2, int8_t v3, int8_t v4) { cVal[0]=v1; cVal[1]=v2; cVal[2]=v3; cVal[3]=v4; }
	};

	enum FromOrTo
	{
		From = 0,
		To = 1,
		Max,
	};

	union DataFromTo
	{
		// �f�o�b�K�ł̊m�F�p
		struct
		{
			DataUnion	from;	// �ύX�O�̏��
			DataUnion	to;		// �ύX��̏��
		};
		DataUnion ary[Max];		// �z��A�N�Z�X�p[From / To]
	};

	// �擪���p�r���L������r�b�g ���̌� DataFromTo �����̃r�b�g��������
	struct DataFormat
	{
		uint32_t	use_bit;
		DataFromTo	data[32];		// ���ۂɋL������f�[�^ [x].ary[From / To]
	};
	
protected:
	// �x�N�^��������
	static void Init(std::vector<DataUnion> & out)
	{
		out.resize(1);
		out[0].use_bit = 0;
	}

	// int�^���f�[�^�x�N�^�ɒǉ����� out�͐擪�� use_bit ���m�ۍς̑O��
	static void PushInt(std::vector<DataUnion> & out, int32_t from, int32_t to, int32_t flg)
	{
		if (from != to)
		{
			out.emplace_back(from);
			out.emplace_back(to);
			out[0].use_bit |= flg;
		}
	}

	// float�^���f�[�^�x�N�^�ɒǉ����� out�͐擪�� use_bit ���m�ۍς̑O��
	static void PushFloat(std::vector<DataUnion> & out, float from, float to, int32_t flg)
	{
		if (from != to)
		{
			out.emplace_back(from);
			out.emplace_back(to);
			out[0].use_bit |= flg;
		}
	}

	// short�^2���f�[�^�x�N�^�ɒǉ����� out�͐擪�� use_bit ���m�ۍς̑O��
	static void PushShort(std::vector<DataUnion> & out, int16_t f1, int16_t f2, int16_t t1, int16_t t2, int32_t flg1, int32_t flg2)
	{
		if ((f1 != t1) || (f2 != t2))
		{
			out.emplace_back(f1, f2);
			out.emplace_back(t1, t2);
			if (f1 != t1) { out[0].use_bit |= flg1; }
			if (f2 != t2) { out[0].use_bit |= flg2; }
		}
	}

	// char�^4���f�[�^�x�N�^�ɒǉ����� out�͐擪�� use_bit ���m�ۍς̑O��
	static void PushChar(std::vector<DataUnion> & out, int8_t f1, int8_t f2, int8_t f3, int8_t f4, int8_t t1, int8_t t2, int8_t t3, int8_t t4, int32_t flg)
	{
		if ((f1 != t1) || (f2 != t2) || (f3 != t3) || (f4 != t4))
		{
			out.emplace_back(f1, f2, f3, f4);
			out.emplace_back(t1, t2, t3, t4);
			out[0].use_bit |= flg;
		}
	}

	// int�^���f�[�^�x�N�^������o�� ���o�����ꍇ�� idx ���i��
	template<class T>
	static void PopInt(DataUnion const * inP, int32_t & idx, FromOrTo fot, T & value, int32_t flg)
	{
		auto myP = (DataFormat const *)inP;
		if (myP->use_bit & flg)
		{
			value = (T)myP->data[idx].ary[fot].iVal;
			idx++;
		}
	}

	// float�^���f�[�^�x�N�^������o�� ���o�����ꍇ�� idx ���i��
	template<class T>
	static void PopFloat(DataUnion const * inP, int32_t & idx, FromOrTo fot, T & value, int32_t flg)
	{
		auto myP = (DataFormat const *)inP;
		if (myP->use_bit & flg)
		{
			value = myP->data[idx].ary[fot].fVal;
			idx++;
		}
	}

	// short�^���f�[�^�x�N�^������o�� ���o�����ꍇ�� idx ���i��
	template<class T>
	static void PopShort(DataUnion const * inP, int32_t & idx, FromOrTo fot, T & v1, T & v2, int32_t flg1, int32_t flg2)
	{
		auto myP = (DataFormat const *)inP;
		if ((myP->use_bit & flg1) ||
			(myP->use_bit & flg2))
		{
			if (myP->use_bit & flg1) { v1 = (T)myP->data[idx].ary[fot].sVal[0]; }
			if (myP->use_bit & flg2) { v2 = (T)myP->data[idx].ary[fot].sVal[1]; }
			idx++;
		}
	}

	// char�^���f�[�^�x�N�^������o�� ���o�����ꍇ�� idx ���i��
	template<class T>
	static void PopChar(DataUnion const * inP, int32_t & idx, FromOrTo fot, T & v1, T & v2, T & v3, T & v4, int32_t flg)
	{
		auto myP = (DataFormat const *)inP;
		if (myP->use_bit & flg)
		{
			v1 = myP->data[idx].ary[fot].cVal[0];
			v2 = myP->data[idx].ary[fot].cVal[1];
			v3 = myP->data[idx].ary[fot].cVal[2];
			v4 = myP->data[idx].ary[fot].cVal[3];
			idx++;
		}
	}
};

//////////////////////////////////////////////////////////////////////
// �J�����̃L�[�t���[���������
struct CameraKeyFrameChange : public KeyFrameChange
{
	enum UseBitType
	{
		UseFrameNo			= 1<<  0,	// iVal
		UsePreIndex			= 1<<  1,	// sVal[0]
		UseNextIndex		= 1<<  2,	// sVal[1]
		UseLength			= 1<<  3,	// fVal
		UseX				= 1<<  4,	// fVal
		UseY				= 1<<  5,	// fVal
		UseZ				= 1<<  6,	// fVal
		UseRX				= 1<<  7,	// fVal
		UseRY				= 1<<  8,	// fVal
		UseRZ				= 1<<  9,	// fVal
		UseHokanX			= 1<< 10,	// cVal[0]�`[4]
		UseHokanY			= 1<< 11,	// cVal[0]�`[4]
		UseHokanZ			= 1<< 12,	// cVal[0]�`[4]
		UseHokanR			= 1<< 13,	// cVal[0]�`[4]
		UseHokanL			= 1<< 14,	// cVal[0]�`[4]
		UseHokanA			= 1<< 15,	// cVal[0]�`[4]
		UsePerspective		= 1<< 16,	// iVal
		UseViewAngle		= 1<< 17,	// iVal
		UseLookingModel		= 1<< 18,	// sVal[0][1]
	};

	// ����T���č������̃x�N�^�����
	static void Push(std::vector<DataUnion> & out, const CameraKeyFrameData & from, const CameraKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushFloat(out, from.length, to.length, UseLength);
		PushFloat(out, from.xyz.x, to.xyz.x, UseX);
		PushFloat(out, from.xyz.y, to.xyz.y, UseY);
		PushFloat(out, from.xyz.z, to.xyz.z, UseZ);
		PushFloat(out, from.rxyz.x, to.rxyz.x, UseRX);
		PushFloat(out, from.rxyz.y, to.rxyz.y, UseRY);
		PushFloat(out, from.rxyz.z, to.rxyz.z, UseRZ);
		PushChar(out,
			from.hokan1_x[0], from.hokan1_y[0], from.hokan2_x[0], from.hokan2_y[0],
			  to.hokan1_x[0],   to.hokan1_y[0],   to.hokan2_x[0],   to.hokan2_y[0], UseHokanX);
		PushChar(out,
			from.hokan1_x[1], from.hokan1_y[1], from.hokan2_x[1], from.hokan2_y[1],
			  to.hokan1_x[1],   to.hokan1_y[1],   to.hokan2_x[1],   to.hokan2_y[1], UseHokanY);
		PushChar(out,
			from.hokan1_x[2], from.hokan1_y[2], from.hokan2_x[2], from.hokan2_y[2],
			  to.hokan1_x[2],   to.hokan1_y[2],   to.hokan2_x[2],   to.hokan2_y[2], UseHokanZ);
		PushChar(out,
			from.hokan1_x[3], from.hokan1_y[3], from.hokan2_x[3], from.hokan2_y[3],
			  to.hokan1_x[3],   to.hokan1_y[3],   to.hokan2_x[3],   to.hokan2_y[3], UseHokanR);
		PushChar(out,
			from.hokan1_x[4], from.hokan1_y[4], from.hokan2_x[4], from.hokan2_y[4],
			  to.hokan1_x[4],   to.hokan1_y[4],   to.hokan2_x[4],   to.hokan2_y[4], UseHokanL);
		PushChar(out,
			from.hokan1_x[5], from.hokan1_y[5], from.hokan2_x[5], from.hokan2_y[5],
			  to.hokan1_x[5],   to.hokan1_y[5],   to.hokan2_x[5],   to.hokan2_y[5], UseHokanA);
		PushInt(out, from.is_perspective, to.is_perspective, UsePerspective);
		PushInt(out, from.view_angle, to.view_angle, UseViewAngle);
		PushShort(out, from.looking_model_index, from.looking_bone_index, to.looking_model_index, to.looking_bone_index, UseLookingModel, UseLookingModel);
	}
	
	// �������̐擪�|�C���^����L�[�t���[���𕜋A����
	static void Pop(CameraKeyFrameData & out, DataUnion const * inP, FromOrTo fot)
	{
		int32_t idx = 0;
		PopInt(inP, idx, fot, out.frame_no, UseFrameNo);
		PopShort(inP, idx, fot, out.pre_index, out.next_index, UsePreIndex, UseNextIndex);
		PopFloat(inP, idx, fot, out.length, UseLength);
		PopFloat(inP, idx, fot, out.xyz.x, UseX);
		PopFloat(inP, idx, fot, out.xyz.y, UseY);
		PopFloat(inP, idx, fot, out.xyz.z, UseZ);
		PopFloat(inP, idx, fot, out.rxyz.x, UseRX);
		PopFloat(inP, idx, fot, out.rxyz.y, UseRY);
		PopFloat(inP, idx, fot, out.rxyz.z, UseRZ);
		PopChar(inP, idx, fot,
			out.hokan1_x[0], out.hokan1_y[0], out.hokan2_x[0], out.hokan2_y[0], UseHokanX);
		PopChar(inP, idx, fot,
			out.hokan1_x[1], out.hokan1_y[1], out.hokan2_x[1], out.hokan2_y[1], UseHokanY);
		PopChar(inP, idx, fot,
			out.hokan1_x[2], out.hokan1_y[2], out.hokan2_x[2], out.hokan2_y[2], UseHokanZ);
		PopChar(inP, idx, fot,
			out.hokan1_x[3], out.hokan1_y[3], out.hokan2_x[3], out.hokan2_y[3], UseHokanR);
		PopChar(inP, idx, fot,
			out.hokan1_x[4], out.hokan1_y[4], out.hokan2_x[4], out.hokan2_y[4], UseHokanL);
		PopChar(inP, idx, fot,
			out.hokan1_x[5], out.hokan1_y[5], out.hokan2_x[5], out.hokan2_y[5], UseHokanA);
		PopInt(inP, idx, fot, out.is_perspective, UsePerspective);
		PopInt(inP, idx, fot, out.view_angle, UseViewAngle);
		PopShort(inP, idx, fot, out.looking_model_index, out.looking_bone_index, UseLookingModel, UseLookingModel);

		// ���f���Ǐ]������ꍇ�A���f���������Ă��Ȃ������ׂ�
		// �ꉞ�A���������� ResetModel() �ŏC�����Ă���͂������O�̂���
		if (out.looking_model_index >= 0)
		{
			auto mmdDataP = getMMDMainData();
			if (mmdDataP->model_data[out.looking_model_index] == nullptr)
			{
				// �����Ă����̂ŁA���f���Ǐ]����������
				out.looking_model_index = -1;
				out.looking_bone_index = 0;
			}
		}
	}

	// �������̃��f���Ǐ]�Ń��f���������Ă��鍷�����C������
	static void ResetModel(DataUnion * inP)
	{
		auto mmdDataP = getMMDMainData();

		auto myP = (DataFormat *)inP;
		if (myP->use_bit & UseLookingModel)
		{
			int32_t idx = 0;
			for (int shift = 0; shift < 32; shift++)
			{
				uint32_t bit = (1 << shift);

				if (bit == UseLookingModel)
				{
					// ���f���Ǐ]��
					for (int fot = 0; fot < Max; fot++)
					{
						auto &modelIdx = myP->data[idx].ary[fot].sVal[0];
						if (modelIdx >= 0)
						{
							// ���f�����������
							if (mmdDataP->model_data[modelIdx] == nullptr)
							{
								myP->data[idx].ary[fot].sVal[0] = -1;
								myP->data[idx].ary[fot].sVal[1] = 0;
							}
						}
					}
					break;
				}
				else
				// UseLookingModel �ȉ��̃r�b�g�𐔂���
				if(bit & myP->use_bit)
				{
					idx++;
				}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////
// ���C�g�̃L�[�t���[���������
struct LightKeyFrameChange : public KeyFrameChange
{
	enum UseBitType
	{
		UseFrameNo			= 1<<  0,	// iVal
		UsePreIndex			= 1<<  1,	// sVal[0]
		UseNextIndex		= 1<<  2,	// sVal[1]
		UseX				= 1<<  3,	// fVal
		UseY				= 1<<  4,	// fVal
		UseZ				= 1<<  5,	// fVal
		UseR				= 1<<  6,	// fVal
		UseG				= 1<<  7,	// fVal
		UseB				= 1<<  8,	// fVal
	};

	// ����T���č������̃x�N�^�����
	static void Push(std::vector<DataUnion> & out, const LightKeyFrameData & from, const LightKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushFloat(out, from.xyz.x, to.xyz.x, UseX);
		PushFloat(out, from.xyz.y, to.xyz.y, UseY);
		PushFloat(out, from.xyz.z, to.xyz.z, UseZ);
		PushFloat(out, from.rgb.r, to.rgb.r, UseR);
		PushFloat(out, from.rgb.g, to.rgb.g, UseG);
		PushFloat(out, from.rgb.b, to.rgb.b, UseB);
	}
	
	// �������̐擪�|�C���^����L�[�t���[���𕜋A����
	static void Pop(LightKeyFrameData & out, DataUnion const * inP, FromOrTo fot)
	{
		int32_t idx = 0;
		PopInt(inP, idx, fot, out.frame_no, UseFrameNo);
		PopShort(inP, idx, fot, out.pre_index, out.next_index, UsePreIndex, UseNextIndex);
		PopFloat(inP, idx, fot, out.xyz.x, UseX);
		PopFloat(inP, idx, fot, out.xyz.y, UseY);
		PopFloat(inP, idx, fot, out.xyz.z, UseZ);
		PopFloat(inP, idx, fot, out.rgb.r, UseR);
		PopFloat(inP, idx, fot, out.rgb.g, UseG);
		PopFloat(inP, idx, fot, out.rgb.b, UseB);
	}
};

//////////////////////////////////////////////////////////////////////
// �Z���t�e�̃L�[�t���[���������
struct SelfShadowKeyFrameChange : public KeyFrameChange
{
	enum UseBitType
	{
		UseFrameNo			= 1<<  0,	// iVal
		UsePreIndex			= 1<<  1,	// sVal[0]
		UseNextIndex		= 1<<  2,	// sVal[1]
		UseMode				= 1<<  3,	// iVal
		UseRange			= 1<<  4,	// fVal
	};

	// ����T���č������̃x�N�^�����
	static void Push(std::vector<DataUnion> & out, const SelfShadowKeyFrameData & from, const SelfShadowKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushInt(out, (int32_t)from.mode, (int32_t)to.mode, UseMode);
		PushFloat(out, from.range, to.range, UseRange);
	}
	
	// �������̐擪�|�C���^����L�[�t���[���𕜋A����
	static void Pop(SelfShadowKeyFrameData & out, DataUnion const * inP, FromOrTo fot)
	{
		int32_t idx = 0;
		PopInt(inP, idx, fot, out.frame_no, UseFrameNo);
		PopShort(inP, idx, fot, out.pre_index, out.next_index, UsePreIndex, UseNextIndex);
		PopInt(inP, idx, fot, out.mode, UseMode);
		PopFloat(inP, idx, fot, out.range, UseRange);
	}
};

//////////////////////////////////////////////////////////////////////
// �d�͂̃L�[�t���[���������
struct GravityKeyFrameChange : public KeyFrameChange
{
	enum UseBitType
	{
		UseFrameNo			= 1<<  0,	// iVal
		UsePreIndex			= 1<<  1,	// sVal[0]
		UseNextIndex		= 1<<  2,	// sVal[1]
		UseAccel			= 1<<  3,	// fVal
		UseX				= 1<<  4,	// fVal
		UseY				= 1<<  5,	// fVal
		UseZ				= 1<<  6,	// fVal
		UseNoise			= 1<<  7,	// iVal
		UseIsNoise			= 1<<  8,	// iVal
	};

	// ����T���č������̃x�N�^�����
	static void Push(std::vector<DataUnion> & out, const GravityKeyFrameData & from, const GravityKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushFloat(out, from.accel, to.accel, UseAccel);
		PushFloat(out, from.xyz.x, to.xyz.x, UseX);
		PushFloat(out, from.xyz.y, to.xyz.y, UseY);
		PushFloat(out, from.xyz.z, to.xyz.z, UseZ);
		PushInt(out, from.noise, to.noise, UseNoise);
		PushInt(out, from.is_noise, to.is_noise, UseIsNoise);
	}
	
	// �������̐擪�|�C���^����L�[�t���[���𕜋A����
	static void Pop(GravityKeyFrameData & out, DataUnion const * inP, FromOrTo fot)
	{
		int32_t idx = 0;
		PopInt(inP, idx, fot, out.frame_no, UseFrameNo);
		PopShort(inP, idx, fot, out.pre_index, out.next_index, UsePreIndex, UseNextIndex);
		PopFloat(inP, idx, fot, out.accel, UseAccel);
		PopFloat(inP, idx, fot, out.xyz.x, UseX);
		PopFloat(inP, idx, fot, out.xyz.y, UseY);
		PopFloat(inP, idx, fot, out.xyz.z, UseZ);
		PopInt(inP, idx, fot, out.noise, UseNoise);
		PopInt(inP, idx, fot, out.is_noise, UseIsNoise);
	}
};

//////////////////////////////////////////////////////////////////////
// �A�N�Z�T���̃L�[�t���[���������
struct AccessoryKeyFrameChange : public KeyFrameChange
{
	enum UseBitType
	{
		UseFrameNo			= 1<<  0,	// iVal
		UsePreIndex			= 1<<  1,	// sVal[0]
		UseNextIndex		= 1<<  2,	// sVal[1]
		UseVisible			= 1<<  3,	// sVal[0]
		UseShadow			= 1<<  4,	// sVal[1]
		UseLookingModel		= 1<<  5,	// sVal[0][1]
		UseX				= 1<<  6,	// fVal
		UseY				= 1<<  7,	// fVal
		UseZ				= 1<<  8,	// fVal
		UseRX				= 1<<  9,	// fVal
		UseRY				= 1<< 10,	// fVal
		UseRZ				= 1<< 11,	// fVal
		UseSi				= 1<< 12,	// fVal
		UseTr				= 1<< 13,	// fVal
	};

	// ����T���č������̃x�N�^�����
	static void Push(std::vector<DataUnion> & out, const AccessoryKeyFrameData & from, const AccessoryKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushShort(out, from.is_visible, from.is_shadow, to.is_visible, to.is_shadow, UseVisible, UseShadow);
		PushShort(out, from.looking_model_index, from.looking_bone_index, to.looking_model_index, to.looking_bone_index, UseLookingModel, UseLookingModel);
		PushFloat(out, from.xyz.x, to.xyz.x, UseX);
		PushFloat(out, from.xyz.y, to.xyz.y, UseY);
		PushFloat(out, from.xyz.z, to.xyz.z, UseZ);
		PushFloat(out, from.rxyz.x, to.rxyz.x, UseRX);
		PushFloat(out, from.rxyz.y, to.rxyz.y, UseRY);
		PushFloat(out, from.rxyz.z, to.rxyz.z, UseRZ);
		PushFloat(out, from.si, to.si, UseSi);
		PushFloat(out, from.tr, to.tr, UseTr);
	}

	// �������̐擪�|�C���^����L�[�t���[���𕜋A����
	static void Pop(AccessoryKeyFrameData & out, DataUnion const * inP, FromOrTo fot)
	{
		int32_t idx = 0;
		PopInt(inP, idx, fot, out.frame_no, UseFrameNo);
		PopShort(inP, idx, fot, out.pre_index, out.next_index, UsePreIndex, UseNextIndex);
		PopShort(inP, idx, fot, out.is_visible, out.is_shadow, UseVisible, UseShadow);
		PopShort(inP, idx, fot, out.looking_model_index, out.looking_bone_index, UseLookingModel, UseLookingModel);
		PopFloat(inP, idx, fot, out.xyz.x, UseX);
		PopFloat(inP, idx, fot, out.xyz.y, UseY);
		PopFloat(inP, idx, fot, out.xyz.z, UseZ);
		PopFloat(inP, idx, fot, out.rxyz.x, UseRX);
		PopFloat(inP, idx, fot, out.rxyz.y, UseRY);
		PopFloat(inP, idx, fot, out.rxyz.z, UseRZ);
		PopFloat(inP, idx, fot, out.si, UseSi);
		PopFloat(inP, idx, fot, out.tr, UseTr);

		// ���f���Ǐ]������ꍇ�A���f���������Ă��Ȃ������ׂ�
		// �ꉞ�A���������� ResetModel() �ŏC�����Ă���͂������O�̂���
		if (out.looking_model_index >= 0)
		{
			auto mmdDataP = getMMDMainData();
			if (mmdDataP->model_data[out.looking_model_index] == nullptr)
			{
				// �����Ă����̂ŁA���f���Ǐ]����������
				out.looking_model_index = -1;
				out.looking_bone_index = 0;
			}
		}
	}

	// �������̃��f���Ǐ]�Ń��f���������Ă��鍷�����C������
	static void ResetModel(DataUnion * inP)
	{
		auto mmdDataP = getMMDMainData();

		auto myP = (DataFormat *)inP;
		if (myP->use_bit & UseLookingModel)
		{
			int32_t idx = 0;
			for (int shift = 0; shift < 32; shift++)
			{
				uint32_t bit = (1 << shift);

				if (bit == UseLookingModel)
				{
					// ���f���Ǐ]��
					for (int fot = 0; fot < Max; fot++)
					{
						auto &modelIdx = myP->data[idx].ary[fot].sVal[0];
						if (modelIdx >= 0)
						{
							// ���f�����������
							if (mmdDataP->model_data[modelIdx] == nullptr)
							{
								myP->data[idx].ary[fot].sVal[0] = -1;
								myP->data[idx].ary[fot].sVal[1] = 0;
							}
						}
					}
					break;
				}
				else
					// UseLookingModel �ȉ��̃r�b�g�𐔂���
					if (bit & myP->use_bit)
					{
						idx++;
					}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////
// �L�[�t���[���̔�r�p�R�s�[�Ǘ��N���X
template<class FrameDataType>
class KeyFrameCopy
{
	FrameDataType					m_keyFrameCopy[10000];		// �t���[���f�[�^�̃R�s�[ �ω����o�p
	int								m_useMax;					// m_keyFrameCopy �̍ő�g�p�v�f
	
public:
	// ��r�p�R�s�[���X�V����
	void UpdateKeyFrameCopy(FrameDataType(&nowAry)[10000])
	{
		if (nowAry != nullptr)
		{
			CopyMemory(m_keyFrameCopy, nowAry, sizeof(m_keyFrameCopy));
			m_useMax = GetMaxFrameIndex(m_keyFrameCopy);
		}
	}

	const FrameDataType & GetFrame(int idx) const { return m_keyFrameCopy[idx]; }
	int UseMax() const { return m_useMax; }

	// �L�[�t���[���z��̎g�p���Ă���ő�v�f�𒲂ׂ�
	static int GetMaxFrameIndex(FrameDataType(&frameAry)[10000])
	{
		int maxIdx = -1;

		for (int idx = 0; idx < 10000;)
		{
			auto &c = frameAry[idx];

			maxIdx = std::max(maxIdx, c.next_index);
			// ����������ΏI��
			if (c.next_index == 0) { break; }
			// ���̃L�[�t���[����
			idx = c.next_index;
		}

		return maxIdx;
	}
};

//////////////////////////////////////////////////////////////////////
// �J����/�Ɩ�/�Z���t�e/�d�� �̃L�[�t���[���̃R�s�[
struct CameraModeCopy
{
public:
	KeyFrameCopy<CameraKeyFrameData>		cameraCopy;
	KeyFrameCopy<LightKeyFrameData>			lightCopy;
	KeyFrameCopy<SelfShadowKeyFrameData>	shadowCopy;
	KeyFrameCopy<GravityKeyFrameData>		gravityCopy;

	// ��r�p�R�s�[���X�V����
	void UpdateKeyFrameCopy(int idx)
	{
		auto mmdDataP = getMMDMainData();
		cameraCopy.UpdateKeyFrameCopy(mmdDataP->camera_key_frame);
		lightCopy.UpdateKeyFrameCopy(mmdDataP->light_key_frame);
		shadowCopy.UpdateKeyFrameCopy(mmdDataP->self_shadow_key_frame);
		gravityCopy.UpdateKeyFrameCopy(mmdDataP->gravity_key_frame);
	}
};

//////////////////////////////////////////////////////////////////////
// �A�N�Z�T�� �̃L�[�t���[���̃R�s�[
struct AccessoryModeCopy
{
public:
	KeyFrameCopy<AccessoryKeyFrameData>		accessoryCopy;

	// ��r�p�R�s�[���X�V����
	void UpdateKeyFrameCopy(int idx)
	{
		auto mmdDataP = getMMDMainData();
		accessoryCopy.UpdateKeyFrameCopy(*mmdDataP->accessory_key_frames[idx]);
	}
};


//////////////////////////////////////////////////////////////////////
template<class CChange, class FrameDataType>
class UndoRedo
{
protected:
	std::vector< std::pair<int, int>>		m_indexTable;	// �L�[�t���[���̗v�f�ԍ��̃f�[�^�����̉�������J�n���邩�̃e�[�u��
	std::vector<KeyFrameChange::DataUnion>	m_changeData;	// �ω������L�[�t���[����ۑ����邽�߂̃x�N�^
	int										m_size;			// ��2�̃f�[�^�̃T�C�Y

public:
	typedef std::shared_ptr<UndoRedo>	SP;

	static SP Create(KeyFrameCopy<FrameDataType> const & copy, FrameDataType(&nowAry)[10000] )
	{
		std::vector<std::pair<int, int> >		indexTable;
		std::vector<KeyFrameChange::DataUnion>	changeData;

		// �R�s�[�ƌ��݂̍ő�v�f�ő傫�����܂Œ��ׂ�
		int nowUseMax = KeyFrameCopy<FrameDataType>::GetMaxFrameIndex(nowAry);
		int useMax = std::max(copy.UseMax(), nowUseMax);
		// �O�̂���
		useMax = std::min(useMax, 10000-1);

		// �ő�g�p�ӏ��܂ł̕ω����`�F�b�N����
		std::vector<KeyFrameChange::DataUnion> out;
		for (int idx = 0; idx <= useMax; idx++)
		{
			CChange::Push(out, copy.GetFrame(idx), nowAry[idx]);
			if (out[0].use_bit)
			{
				// �ω������t���[�����L��
				indexTable.emplace_back(idx, (int)changeData.size()); // ���݂̃T�C�Y(==�Ō��+1)�����Œǉ�����擪�ɂȂ�
				changeData.insert(changeData.cend(), out.cbegin(), out.cend());
			}
		}

		// �ω����Ă��Ȃ����nullptr
		if (changeData.empty()) { return nullptr; }

		auto sp = std::make_shared<UndoRedo>();
		sp->m_indexTable.swap(indexTable);
		sp->m_changeData.swap(changeData);
		// ���ʗ̈���폜
		std::vector<std::pair<int, int>>(sp->m_indexTable).swap(sp->m_indexTable);
		std::vector<KeyFrameChange::DataUnion>(sp->m_changeData).swap(sp->m_changeData);
		sp->m_size = (int)sp->m_indexTable.size() * sizeof(std::pair<int, int>) +
					 (int)sp->m_changeData.size() * sizeof(KeyFrameChange::DataUnion);
		return sp;
	}


	//////////////////////////////////////////////////////
	// �A���h�D
	void Undo(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			auto& idx = s.first;
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			CChange::Pop(nowAry[idx], topP, KeyFrameChange::From);
		}
	}

	// ���h�D
	void Redo(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			auto& idx = s.first;
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			CChange::Pop(nowAry[idx], topP, KeyFrameChange::To);
		}
	}

	// ���f�����폜���ꂽ�Ƃ��̃R�[���o�b�N
	void OnModelDelete(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			// ���f���Ǐ]�̏C��
			CChange::ResetModel(topP);
		}
	}

	// �g�p�T�C�Y��Ԃ�
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
typedef UndoRedo<CameraKeyFrameChange, CameraKeyFrameData>				CameraUndoRedo;
typedef UndoRedo<LightKeyFrameChange, LightKeyFrameData>				LightUndoRedo;
typedef UndoRedo<SelfShadowKeyFrameChange, SelfShadowKeyFrameData>		SelfShadowUndoRedo;
typedef UndoRedo<GravityKeyFrameChange, GravityKeyFrameData>			GravityUndoRedo;
typedef UndoRedo<AccessoryKeyFrameChange, AccessoryKeyFrameData>		AccessoryUndoRedo;


//////////////////////////////////////////////////////////////////////
// �J����/�Ɩ�/�Z���t�e/�d�� �̃A���h�D/���h�D�����킹�čs��
class CameraModeUndoRedo
{
	MMDMainData*							m_mmdDataP;

	CameraUndoRedo::SP						m_cameraP;
	LightUndoRedo::SP						m_lightP;
	SelfShadowUndoRedo::SP					m_shadowP;
	GravityUndoRedo::SP						m_gravityP;
	int										m_size;

public:
	typedef std::shared_ptr<CameraModeUndoRedo>	SP;

	static SP Create(CameraModeCopy & copy, int idx)
	{
		auto mmdDataP = getMMDMainData();

		auto cameraUR  = CameraUndoRedo::		Create(copy.cameraCopy,  mmdDataP->camera_key_frame);
		auto lightUR   = LightUndoRedo::		Create(copy.lightCopy,   mmdDataP->light_key_frame);
		auto shadowUR  = SelfShadowUndoRedo::	Create(copy.shadowCopy,  mmdDataP->self_shadow_key_frame);
		auto gravityUR = GravityUndoRedo::		Create(copy.gravityCopy, mmdDataP->gravity_key_frame);
		if (cameraUR  != nullptr ||
			lightUR   != nullptr ||
			shadowUR  != nullptr ||
			gravityUR != nullptr)
		{
			auto sp = std::make_shared<CameraModeUndoRedo>();
			sp->m_mmdDataP	= mmdDataP;
			sp->m_cameraP	= cameraUR;
			sp->m_lightP	= lightUR;
			sp->m_shadowP	= shadowUR;
			sp->m_gravityP	= gravityUR;

			sp->m_size		= ((sp->m_cameraP)	? (sp->m_cameraP->	GetSize()) : (0)) +
							  ((sp->m_lightP)	? (sp->m_lightP->	GetSize()) : (0)) +
							  ((sp->m_shadowP)	? (sp->m_shadowP->	GetSize()) : (0)) +
							  ((sp->m_gravityP)	? (sp->m_gravityP->	GetSize()) : (0));
			return sp;
		}
		return nullptr;
	}


	//////////////////////////////////////////////////////
	// �A���h�D
	void Undo(int idx)
	{
		if (m_cameraP)	{ m_cameraP->	Undo(m_mmdDataP->camera_key_frame); }
		if (m_lightP)	{ m_lightP->	Undo(m_mmdDataP->light_key_frame); }
		if (m_shadowP)	{ m_shadowP->	Undo(m_mmdDataP->self_shadow_key_frame); }
		if (m_gravityP)	{ m_gravityP->	Undo(m_mmdDataP->gravity_key_frame); }
	}

	// ���h�D
	void Redo(int idx)
	{
		if (m_cameraP)	{ m_cameraP->	Redo(m_mmdDataP->camera_key_frame); }
		if (m_lightP)	{ m_lightP->	Redo(m_mmdDataP->light_key_frame); }
		if (m_shadowP)	{ m_shadowP->	Redo(m_mmdDataP->self_shadow_key_frame); }
		if (m_gravityP)	{ m_gravityP->	Redo(m_mmdDataP->gravity_key_frame); }
	}

	// ���f�����폜���ꂽ�Ƃ��̃R�[���o�b�N
	void OnModelDelete(int idx)
	{
		// ���f�����֌W����̂̓J�����̂�
		if (m_cameraP)	{ m_cameraP->	OnModelDelete(m_mmdDataP->camera_key_frame); }
	}

	// �g�p�T�C�Y��Ԃ�
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
// �A�N�Z�T�� �̃A���h�D/���h�D���s��
class AccessoryModeUndoRedo
{
	MMDMainData*							m_mmdDataP;

	AccessoryUndoRedo::SP					m_accessoryP;
	int										m_size;

public:
	typedef std::shared_ptr<AccessoryModeUndoRedo>	SP;

	static SP Create(AccessoryModeCopy & copy, int idx)
	{
		auto mmdDataP = getMMDMainData();

		auto cameraUR  = AccessoryUndoRedo::Create(copy.accessoryCopy, *mmdDataP->accessory_key_frames[idx]);
		if (cameraUR  != nullptr)
		{
			auto sp = std::make_shared<AccessoryModeUndoRedo>();
			sp->m_mmdDataP	= mmdDataP;
			sp->m_accessoryP= cameraUR;

			sp->m_size		= ((sp->m_accessoryP)	? (sp->m_accessoryP->	GetSize()) : (0));
			return sp;
		}
		return nullptr;
	}


	//////////////////////////////////////////////////////
	// �A���h�D
	void Undo(int idx)
	{
		if (m_accessoryP)	{ m_accessoryP->Undo(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// ���h�D
	void Redo(int idx)
	{
		if (m_accessoryP)	{ m_accessoryP->Redo(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// ���f�����폜���ꂽ�Ƃ��̃R�[���o�b�N
	void OnModelDelete(int idx)
	{
		// ���f�����֌W����̂̓J�����̂�
		if (m_accessoryP)	{ m_accessoryP->OnModelDelete(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// �g�p�T�C�Y��Ԃ�
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
template<class CCopy, class CUndo>
class KeyFrameUndoRedo
{
private:
	MMDMainData*						m_mmdDataP;

	int									m_idx;						// �R�s�[����v�f�ԍ�(��ɃA�N�Z�T���p)
	CCopy								m_copy;
	std::deque<typename CUndo::SP>		m_undoDec;					// �A���h�D�p�̃L���[
	std::deque<typename CUndo::SP>		m_redoDec;					// ���h�D�p�̃L���[
	int									m_buffSize;					// ���݂̃o�b�t�@�̃T�C�Y���v
	int									m_dataMaxSize;				// �ő�o�b�t�@�T�C�Y
	
	// �o�b�t�@�I�[�o�[���̍폜�v�f��
	static constexpr int DATA_REMOVE_NUM = 10;

public:
	KeyFrameUndoRedo(int idx, int dataMaxSizeMB)
		: m_mmdDataP(nullptr)
		, m_idx(idx)
		, m_buffSize(0)
		, m_dataMaxSize(std::max(1, dataMaxSizeMB) * (1024 * 1024))
	{
		m_mmdDataP = getMMDMainData();

		m_copy.UpdateKeyFrameCopy(m_idx);

		_dbgPrint("�A���h�D/���h�D ������");
	}

	// �A���h�D
	void Undo()
	{
		if (m_undoDec.empty()) { return; }
		
		DbgTimer dbgTimer([this](float count) { _dbgPrint("��������:%f ms Buff:%9.2f KB �L���[:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); } );

		// �A���h�D�L���[������΁A���o���Ė߂�
		m_undoDec.back()->Undo(m_idx);
		// ���h�D�L���[�ɐς�
		m_redoDec.push_back(m_undoDec.back());
		m_undoDec.pop_back();
		// ��r�p�R�s�[���X�V
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

	// ���h�D
	void Redo()
	{
		if (m_redoDec.empty()) { return; }
		
		DbgTimer dbgTimer([this](float count) { _dbgPrint("��������:%f ms Buff:%9.2f KB �L���[:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); });

		// ���h�D�L���[������΁A���o���Ė߂�
		m_redoDec.back()->Redo(m_idx);
		// �A���h�D�L���[�ɐς�
		m_undoDec.push_back(m_redoDec.back());
		m_redoDec.pop_back();
		// ��r�p�R�s�[���X�V
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

	// �L�[�t���[���ω������o����
	bool CheckFrame()
	{
		int beforeSize = m_buffSize;
		DbgTimer dbgTimer([this, beforeSize](float count) {if (beforeSize != m_buffSize) { _dbgPrint("���o����������:%f ms Buff:%9.2f KB �L���[:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); }});

		bool bDo = false;

		// �O�̏�Ԃƌ��݂̏�Ԃŕω����Ă���΃N���X������ĕԂ�
		auto camModeUR = CUndo::Create(m_copy, m_idx);
		if (camModeUR != nullptr)
		{
			// �ω����Ă�����A�ω������A���h�D�L���[�ɒǉ�
			bDo = true;
			m_undoDec.emplace_back(camModeUR);
			m_buffSize += camModeUR->GetSize();

			// ���h�D�L���[�̓N���A
			m_redoDec.clear();

			// ��r�p�R�s�[���X�V
			m_copy.UpdateKeyFrameCopy(m_idx);

			// �o�b�t�@�T�C�Y�𒴂�����A�Â��f�[�^�������Ă���
			if (m_buffSize > m_dataMaxSize)
			{
				// �T�C�Y�I��10�����Ȃ����͂��܂�Ȃ��Ǝv�����A�O�̂��߃L���[�ɍŒᔼ���͎c��l�ɂ��Ă���
				int removeNum = std::min(DATA_REMOVE_NUM, (int)m_undoDec.size()/2);
				for (int cnt = 0; cnt < removeNum; cnt++)
				{
					m_buffSize -= m_undoDec.front()->GetSize();
					m_undoDec.pop_front();
				}
			}
		}
		
		return bDo;
	}

	// ���݂̃A���h�D/���h�D�\��
	size_t GetUndoNum() { return m_undoDec.size(); }
	size_t GetRedoNum() { return m_redoDec.size(); }

	// ���f�����폜���ꂽ�Ƃ��̃R�[���o�b�N
	void OnModelDelete()
	{
		for (auto& d : m_undoDec) { d->OnModelDelete(m_idx); }
		for (auto& d : m_redoDec) { d->OnModelDelete(m_idx); }

		// ���f���폜���L�[�t���[���ύX�Ō��o�����Ȃ��ׂɔ�r�p�R�s�[���X�V
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

#ifdef USE_DEBUG
	// �_�~�[�f�[�^����
	void CreateDummyData()
	{
		auto mmdDataP = getMMDMainData();
		// auto &keyFrameAry = mmdDataP->camera_key_frame;
		// auto &keyFrameAry = mmdDataP->light_key_frame;
		//auto &keyFrameAry = mmdDataP->self_shadow_key_frame;
		auto &keyFrameAry = mmdDataP->gravity_key_frame;

		if (keyFrameAry[9000].frame_no == 0)
		{
			{
				auto &c = keyFrameAry[0];
				c.frame_no = 0;
				c.pre_index = 0;
				c.next_index = 9999;
			}
			for (int cnt = 9999; cnt > 0; cnt--)
			{
				auto &c = keyFrameAry[cnt];
				c = keyFrameAry[0];
				c.frame_no = 10000 - cnt;
				c.pre_index = (cnt == 9999) ? 0 : (cnt + 1);
				c.next_index = (cnt == 1) ? 0 : (cnt - 1);
				// c.range = 0.1f - (c.frame_no * 0.00001f);
				// c.xyz.x = c.frame_no / 10.0f;
				c.accel = c.frame_no / 100.0f;
			}

			mmdDataP->max_use_frame = 9999;
		}
	}
#endif // USE_DEBUG

private:
#ifdef USE_DEBUG
	void _testPrint()
	{
		auto mmdDataP = getMMDMainData();

		_dbgPrint("�A���h�D�L���[:%d ���h�D�L���[:%d", m_undoDec.size(), m_redoDec.size());
		for (int cnt = 0; cnt < 10; cnt++)
		{
			auto &c = mmdDataP->camera_key_frame[cnt];
			_dbgPrint(" %3d �O:%3d ��:%3d �I:%d %f %f %f", c.frame_no, c.pre_index, c.next_index, c.is_selected, c.xyz.x, c.xyz.y, c.xyz.z);
		}
		_dbgPrint("------------------------");
	}
#endif // USE_DEBUG
};

typedef KeyFrameUndoRedo<CameraModeCopy, CameraModeUndoRedo>			CameraModeKeyFrameUndoRedo;
typedef KeyFrameUndoRedo<AccessoryModeCopy, AccessoryModeUndoRedo>		AccessoryModeKeyFrameUndoRedo;

//////////////////////////////////////////////////////////////////////
class CameraModeUndoPlugin : public MMDPluginDLL3, public CMmdCtrls, public Singleton<CameraModeUndoPlugin>
{
private:
	MMDMainData*									m_mmdDataP;
	CInifile										m_ini;
	bool											m_bShowWindow;		// ����� WM_SHOWWINDOW ����p
	CameraKeyFrameData *							m_cameraKeyFrameP;	// �J�����̃L�[�t���[���z��̐擪�|�C���^
	std::shared_ptr<CameraModeKeyFrameUndoRedo>		m_camUndoP;
	std::shared_ptr<AccessoryModeKeyFrameUndoRedo>	m_accUndoP[255];

	CKeyState										m_camUndoKey;		// �A���h�D�̃L�[
	CKeyState										m_camUedoKey;		// ���h�D�̃L�[
	CKeyState										m_accUndoKey;		// �A�N�Z�T���A���h�D�̃L�[
	CKeyState										m_accUedoKey;		// �A�N�Z�T�����h�D�̃L�[
	CKeyState										m_testKey;			// �e�X�g�̃L�[
	int												m_undoBuffMB;		// �A���h�D�o�b�t�@�T�C�Y(MB)

	HWND											m_myCamUndoBtnH;	// �J�����p�Ɏ��삷��u���ɖ߂��v�{�^��
	HWND											m_myCamRedoBtnH;	// �J�����p�Ɏ��삷��u��蒼���v�{�^��
	HWND											m_myAccUndoBtnH;	// �A�N�Z�T���p�Ɏ��삷��u���ɖ߂��v�{�^��
	HWND											m_myAccRedoBtnH;	// �A�N�Z�T���p�Ɏ��삷��u��蒼���v�{�^��
	bool											m_bCheckFrame;		// �L�[�t���[���̕ύX���o
	bool											m_bRepaint;			// �ĕ`�悷��
	bool											m_bUpdateBtnState;	// �u���ɖ߂��v�u��蒼���v�{�^�����X�V����
	bool											m_bUpdateBtnVisible;// �u���ɖ߂��v�u��蒼���v�{�^���̕\����Ԃ��X�V����
	bool											m_bModelDelete;		// ���f�����폜���ꂽ
	bool											m_bAccAddDelete;	// �A�N�Z�T�����ǉ�/�폜���ꂽ
	
public:
	const char* getPluginTitle() const override { return "CameraModeUndo"; }

	CameraModeUndoPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
		, m_cameraKeyFrameP(nullptr)
		, m_myCamUndoBtnH(nullptr)
		, m_myCamRedoBtnH(nullptr)
		, m_myAccUndoBtnH(nullptr)
		, m_myAccRedoBtnH(nullptr)
		, m_bCheckFrame(false)
		, m_bRepaint(false)
		, m_bUpdateBtnState(false)
		, m_bUpdateBtnVisible(false)
		, m_bModelDelete(false)
		, m_bAccAddDelete(false)
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
			ini.Str(L"CameraUndo") = L"Ctrl + Z";
			ini.Str(L"CameraRedo") = L"Ctrl + X";
			ini.Str(L"AccessoryUndo") = L"Ctrl + Shift + Z";
			ini.Str(L"AccessoryRedo") = L"Ctrl + Shift + X";
			ini.Int(L"UndoBuffMaxMB") = 100;
		});

		// ���͔����������
		m_camUndoKey.SetKeyCode(m_ini.Str(L"CameraUndo"));
		m_camUedoKey.SetKeyCode(m_ini.Str(L"CameraRedo"));
		m_accUndoKey.SetKeyCode(m_ini.Str(L"AccessoryUndo"));
		m_accUedoKey.SetKeyCode(m_ini.Str(L"AccessoryRedo"));
		m_testKey.SetKeyCode(L"C");

		m_undoBuffMB = m_ini.Int(L"UndoBuffMaxMB");
	}

	void start() override
	{
		m_mmdDataP = getMMDMainData();

		// �A���h�D/���h�D�N���X��������
		_checkRemakePtr();
	}

	void stop() override 
	{
		m_ini.Write();
	}

	void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
	{
		_checkRemakePtr();
		if (IsCameraMode())
		{
			if ((lParam & 0x40000000) &&	// ���O�ɃL�[��������Ă���
				(lParam & 0x80000000))		// �L�[��������Ă���Ȃ�
			{
				// �L�[�t���[���ύX���o
				m_bCheckFrame = true;
			}

			// �A���h�D
			if (m_camUndoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				m_camUndoP->Undo();
				m_bRepaint = m_bUpdateBtnState = true; // ���s������{�^���X�V
			}
			// ���h�D
			if (m_camUedoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				m_camUndoP->Redo();
				m_bRepaint = m_bUpdateBtnState = true; // ���s������{�^���X�V
			}

			// �A���h�D
			if (m_accUndoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				if (_accUndo() != nullptr)
				{
					_accUndo()->Undo();
					m_bRepaint = m_bUpdateBtnState = true; // ���s������{�^���X�V
				}
			}
			// ���h�D
			if (m_accUedoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				if (_accUndo() != nullptr)
				{
					_accUndo()->Redo();
					m_bRepaint = m_bUpdateBtnState = true; // ���s������{�^���X�V
				}
			}
		}

#ifdef USE_DEBUG
		if (m_testKey.IsTrg(wParam, lParam))
		{
			auto d = getMMDMainData();
			_dbgPrint("-----------------------------");
			CUnknownChecker::Print();
			_dbgPrint("-----------------------------");
			// CUnknownChecker::Copy();
			// CUnknownChecker::Check();
			// m_camUndoP->CreateDummyData();
		}
#endif // USE_DEBUG

		_update();
	}

	void MouseProc(WPARAM wParam, MOUSEHOOKSTRUCT* lParam) override
	{
		_checkRemakePtr();
		if (IsCameraMode())
		{
			// ���{�^���A�b�v���Ɍ��o
			if (wParam == WM_LBUTTONUP)
			{
				m_bCheckFrame = true;
			}
		}

		_update();
	}

	void MsgProc(int code, MSG* param) override 
	{
		/*
		char * codeName = "�s��";
		if( code == MSGF_DIALOGBOX ) { codeName = "�_�C�A���O"; }
		if( code == MSGF_MESSAGEBOX) { codeName = "���b�Z�[�W�{�b�N�X"; }
		if( code == MSGF_MENU      ) { codeName = "���j���["; }
		if( code == MSGF_SCROLLBAR ) { codeName = "�X�N���[���o�["; }
		if( code == MSGF_USER      ) { codeName = "���[�U�[��`"; }
		_dbgPrint("��������������MsgProc MSG:0x%08X code:%d[%s]", param->message, code, codeName);
		*/
	}

	void GetMsgProc(int code, MSG* param) override
	{
	}

	// MMD���̌Ăяo��������ł���v���V�[�W��
	// false : MMD�̃v���V�[�W�����Ă΂�܂��BLRESULT�͖�������܂�
	// true  : MMD�₻�̑��̃v���O�C���̃v���V�[�W���͌Ă΂�܂���BLRESULT���S�̂̃v���V�[�W���̖߂�l�Ƃ��ĕԂ���܂��B
	std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return { false, 0 };
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

		_checkRemakePtr();

		if (param->hwnd == m_modelComboH)
		{
			// �u���f������v�p�l���̃R���{�{�b�N�X���ύX���ꂽ�ꍇ(��Ƀv���O�������瑀�삳�ꂽ��)
			if (param->message == CB_SETCURSEL)
			{
				m_bUpdateBtnState = true;
			}
			// �R���{�{�b�N�X�̍��ڂ������� == ���f�����폜���ꂽ�ꍇ
			else if (param->message == CB_DELETESTRING)
			{
				m_bModelDelete = true;
			}
		}

		if (param->hwnd == m_accComboH)
		{
			// �u�A�N�Z�T������v�p�l���̃R���{�{�b�N�X���ύX���ꂽ�ꍇ(��Ƀv���O�������瑀�삳�ꂽ��)
			if (param->message == CB_SETCURSEL)
			{
				m_bUpdateBtnState = true;
			}
			// �R���{�{�b�N�X�̍��ڂ��������� == �A�N�Z�T�����ǉ�/�폜���ꂽ�ꍇ
			else if (param->message == CB_DELETESTRING ||
					 param->message == CB_ADDSTRING)
			{
				m_bAccAddDelete = true;
			}
		}

		if (param->message == WM_WINDOWPOSCHANGED)
		{
			auto posP = (WINDOWPOS*)param->lParam;
			// �ړ��Ȃ�
			if ((posP->flags & SWP_NOMOVE) == 0)
			{
				// MMD�́u���ɖ߂��v�{�^�����������玩��̃{�^�����Ǐ]����
				if (param->hwnd == m_undoBtnH)
				{
					int center = posP->x + 70 + 10;	// �u���ɖ߂��v�{�^���Ɓu��蒼���v�{�^���̐^��
					int w = 60;						// ����{�^���̕�

					SetWindowPos(m_myCamUndoBtnH, HWND_NOTOPMOST, center -10 -w -10 -w,	posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myCamRedoBtnH, HWND_NOTOPMOST, center -10 -w,		posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myAccUndoBtnH, HWND_NOTOPMOST, center +10,			posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myAccRedoBtnH, HWND_NOTOPMOST, center +10 +w +10,	posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
				}
			}
		}

		// MMD�́u��蒼���v�{�^�����\������鎞�A�J�������[�h�Ȃ�ēx����
		if (param->message == WM_SHOWWINDOW)
		{
			if (param->hwnd == m_undoBtnH)
			{
				if (param->wParam == TRUE)
				{
					if (IsCameraMode())
					{
						m_bUpdateBtnVisible = true;
					}
				}
			}
		}

		if (param->message == WM_COMMAND)
		{
			auto id = LOWORD(param->wParam);

			// �u�o�^�v�{�^���������ꂽ�ꍇ
			// �u�d�́v�̓_�C�A���O�Ȃ̂Ń_�C�A���O��OK�{�^��
			//  �d�͐ݒ�ȊO�̃��f���̐����A����O�̌x���Ȃǂł����邪�܂��ǂ���
			if (id == CAMERA_BTN_CTRL_ID ||
				id == LIGHT_BTN_CTRL_ID ||
				id == SELF_SHADOW_BTN_CTRL_ID ||
				id == ACCESSORY_BTN_CTRL_ID ||
				id == PASTE_BTN_CTRL_ID ||
				id == DELETE_BTN_CTRL_ID ||
				id == IDOK)
			{
				m_bCheckFrame = true;
			}
			else
			// ����u���ɖ߂��v�{�^���������ꂽ�ꍇ
			if (id == MY_CAM_UNDO_BTN_CTRL_ID)
			{
				if (m_camUndoP != nullptr)	{ m_camUndoP->Undo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// ����u��蒼���v�{�^���������ꂽ�ꍇ
			if (id == MY_CAM_REDO_BTN_CTRL_ID)
			{
				if (m_camUndoP != nullptr) { m_camUndoP->Redo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			// ����u���ɖ߂��v�{�^���������ꂽ�ꍇ
			if (id == MY_ACC_UNDO_BTN_CTRL_ID)
			{
				if (_accUndo() != nullptr)	{ _accUndo()->Undo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// ����u��蒼���v�{�^���������ꂽ�ꍇ
			if (id == MY_ACC_REDO_BTN_CTRL_ID)
			{
				if (_accUndo() != nullptr) { _accUndo()->Redo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// �u���f������v�p�l�����u�A�N�Z�T������v�p�l���̃R���{�{�b�N�X���ύX���ꂽ�ꍇ(���[�U�[�����삵����)
			if ((id == MODEL_COMBO_CTRL_ID || id == ACCESSORY_COMBO_CTRL_ID) &&
				HIWORD(param->wParam) == CBN_SELCHANGE)
			{
				m_bUpdateBtnState = true;
			}
		}
	}

private:
	std::shared_ptr<AccessoryModeKeyFrameUndoRedo>& _accUndo()
	{
		return m_accUndoP[m_mmdDataP->select_accessory];
	}

	// ����� WM_SHOWWINDOW �O�ɌĂ΂��
	// ���̎��_�ł̓{�^�����̃R���g���[�����o���Ă���͂�
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		HWND hMmd = getHWND();

		// �R���g���[���n���h���̏�����
		InitCtrl();

		// �{�^�������
		m_myCamUndoBtnH = CreateWindow(L"button", L"<-", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								116, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_CAM_UNDO_BTN_CTRL_ID, hInstance, NULL);
		m_myCamRedoBtnH = CreateWindow(L"button", L"->", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								206, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_CAM_REDO_BTN_CTRL_ID, hInstance, NULL);

		m_myAccUndoBtnH = CreateWindow(L"button", L"<-", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								116, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_ACC_UNDO_BTN_CTRL_ID, hInstance, NULL);
		m_myAccRedoBtnH = CreateWindow(L"button", L"->", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								206, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_ACC_REDO_BTN_CTRL_ID, hInstance, NULL);

		// �t�H���g�����낦��
		HFONT fontH = (HFONT)SendMessage(m_undoBtnH, WM_GETFONT, 0, 0);
		SendMessage(m_myCamUndoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myCamRedoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myAccUndoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myAccRedoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);

		// �\����Ԃ�ݒ�
		_checkRemakePtr();
		_updateBtnState();
	}

	// �{�^���Ȃǂ̍X�V
	void _update()
	{
		// ���f���폜���̏���
		if (m_bModelDelete)
		{
			m_camUndoP->OnModelDelete();
			for (auto &acc : m_accUndoP)
			{
				if (acc != nullptr) { acc->OnModelDelete(); }
			}
			m_bModelDelete = false;
		}

		// �A�N�Z�T���������̏���
		if (m_bAccAddDelete)
		{
			_checkAccUndoPtr();
			m_bAccAddDelete = false;
		}

		// �u�o�^�v�{�^���������ꂽ��J�����̃L�[�t���[���ω������o����
		if (m_bCheckFrame)
		{
			m_bUpdateBtnState |= m_camUndoP->CheckFrame();
			if (_accUndo() != nullptr) { m_bUpdateBtnState |= _accUndo()->CheckFrame(); }
			m_bCheckFrame = false;
		}

		// ����{�^���̍X�V
		if (m_bUpdateBtnState)
		{
			_updateBtnState();
			m_bUpdateBtnState = false;
		}

		if (m_bUpdateBtnVisible)
		{
			_updateBtnVisible();
			m_bUpdateBtnVisible = false;
		}

		// �ĕ`��
		if (m_bRepaint)
		{
			_setRepaintTimer();
			m_bRepaint = false;
		}
	}

	// �L�[�t���[���̃|�C���^���ς���Ă�����A���h�D/���h�D�N���X���ď���������
	void _checkRemakePtr()
	{
		// camera_key_frame �̃|�C���^�̓V�[�������[�h������ς��
		// vmd�̓ǂݍ��݂ł͕ς��Ȃ�
		if (m_cameraKeyFrameP != m_mmdDataP->camera_key_frame)
		{
			m_cameraKeyFrameP = m_mmdDataP->camera_key_frame;

			if (m_cameraKeyFrameP != nullptr)
			{
				m_camUndoP = std::make_shared<CameraModeKeyFrameUndoRedo>(0, m_undoBuffMB);

				for (auto &acc : m_accUndoP)
				{
					acc.reset();
				}
			}
		}
	}

	// �A�N�Z�T���̑����ɍ��킹�A�A���h�D/���h�D�N���X���������������肷��
	void _checkAccUndoPtr()
	{
		for (int cnt = 0; cnt < ARRAYSIZE(m_accUndoP); cnt++)
		{
			// �A�N�Z�T�����ǉ�����Ă�����
			if (m_accUndoP[cnt] == nullptr && m_mmdDataP->accessory_data[cnt] != nullptr)
			{
				m_accUndoP[cnt] = std::make_shared<AccessoryModeKeyFrameUndoRedo>(cnt, m_undoBuffMB);
				m_bUpdateBtnState = true;
			}
			else
			// �A�N�Z�T�����폜����Ă�����
			if(m_accUndoP[cnt] != nullptr && m_mmdDataP->accessory_data[cnt] == nullptr)
			{
				m_accUndoP[cnt].reset();
				m_bUpdateBtnState = true;
			}
		}
	}

	// �{�^���̏�Ԃ��X�V����
	void _updateBtnState()
	{
		_updateBtnVisible();
		if (m_camUndoP != nullptr)
		{
			auto undoNum = m_camUndoP->GetUndoNum();
			auto redoNum = m_camUndoP->GetRedoNum();
			TCHAR txt[32] = _T("");

			// �񐔂��{�^���ɕ\��
			_sntprintf(txt, ARRAYSIZE(txt), _T("<- (%zd)"), undoNum);
			SetWindowText(m_myCamUndoBtnH, txt);

			_sntprintf(txt, ARRAYSIZE(txt), _T("(%zd) ->"), redoNum);
			SetWindowText(m_myCamRedoBtnH, txt);

			// �A���h�D/���h�D���o����ꍇ�̂ݗL���ɂ���
			EnableWindow(m_myCamUndoBtnH, undoNum > 0);
			EnableWindow(m_myCamRedoBtnH, redoNum > 0);
		}
		if (_accUndo() != nullptr)
		{
			auto undoNum = _accUndo()->GetUndoNum();
			auto redoNum = _accUndo()->GetRedoNum();
			TCHAR txt[32] = _T("");

			// �񐔂��{�^���ɕ\��
			_sntprintf(txt, ARRAYSIZE(txt), _T("<- (%zd)"), undoNum);
			SetWindowText(m_myAccUndoBtnH, txt);

			_sntprintf(txt, ARRAYSIZE(txt), _T("(%zd) ->"), redoNum);
			SetWindowText(m_myAccRedoBtnH, txt);

			// �A���h�D/���h�D���o����ꍇ�̂ݗL���ɂ���
			EnableWindow(m_myAccUndoBtnH, undoNum > 0);
			EnableWindow(m_myAccRedoBtnH, redoNum > 0);
		}
		else
		{
			EnableWindow(m_myAccUndoBtnH, false);
			EnableWindow(m_myAccRedoBtnH, false);
		}
	}

	// �{�^���̕\����Ԃ��X�V����
	void _updateBtnVisible()
	{
		bool bCamMode = IsCameraMode();

		// MMD�́u���ɖ߂��v�{�^���Ɓu��蒼���v�{�^���̓J�������[�h�ł͔�\��
		ShowWindow(m_undoBtnH, !bCamMode);
		ShowWindow(m_redoBtnH, !bCamMode);
		// ����́u���ɖ߂��v�{�^���Ɓu��蒼���v�{�^���̓J�������[�h�ł͕\��
		ShowWindow(m_myCamUndoBtnH, bCamMode);
		ShowWindow(m_myCamRedoBtnH, bCamMode);
		ShowWindow(m_myAccUndoBtnH, bCamMode);
		ShowWindow(m_myAccRedoBtnH, bCamMode);
	}

	// �ĕ`��^�C�}�[���Z�b�g����
	// �}�E�X��������ĕ`����ĂԂƃ}�E�X�𓮂������ʒu�Ń}�E�X�A�b�v�Ɣ��肳��Ă��܂�
	// �Ȃ̂Ń^�C�}�[���Z�b�g���ă}�E�X�������I����Ă���ĕ`�悳���l�ɂ���
	// ���������ǁA���̓}�E�X�𓮂����Ȃ��l�ɂ����̂� Repaint() �Ă�ł����Ȃ������c
	void _setRepaintTimer()
	{
		// ���b�Z�[�W���|�X�g����������ƁA���̂��͒m��Ȃ���MMD���쒆�ɂ�WM_TIMER�������Ă��Ȃ�
		// (�^�C�g���o�[�Ƃ��𑀍삷��ƒx��ē͂��n�߂�)
		// ���ꂶ����ɗ����Ȃ��̂ŁA�R�[���o�b�N�ŏ�������
		SetTimer(getHWND(), (UINT_PTR)this, 1, [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			KillTimer(getHWND(), idEvent);

			CameraModeUndoPlugin * thisP = (CameraModeUndoPlugin *)idEvent;
			thisP->Repaint();
		});
	}

};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMD�̃o�[�W������ 9.31 �ł͂���܂���B\nCameraModeUndo�� Ver.9.31 �ȊO�ł͐���ɍ쓮���܂���B"), _T("CameraModeUndo"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return CameraModeUndoPlugin::GetInstance();
}