// CameraUndo.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include <atltime.h>
#include <deque>
#include <map>
#include <windows.h>
#include <WindowsX.h>
#include "../MMDPlugin/mmd_plugin.h"
using namespace mmp;

#include "../Common/CUnknownChecker.h"
#include "../Common/CInifile.h"
#include "../Common/CKeyState.h"


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
		// デバッガでの確認用
		struct
		{
			DataUnion	from;	// 変更前の状態
			DataUnion	to;		// 変更後の状態
		};
		DataUnion ary[Max];		// 配列アクセス用[From / To]
	};

	// 先頭が用途を記憶するビット その後 DataFromTo がそのビット数分続く
	struct DataFormat
	{
		uint32_t	use_bit;
		DataFromTo	data[32];		// 実際に記憶するデータ [x].ary[From / To]
	};
	
protected:
	// ベクタを初期化
	static void Init(std::vector<DataUnion> & out)
	{
		out.resize(1);
		out[0].use_bit = 0;
	}

	// int型をデータベクタに追加する outは先頭の use_bit が確保済の前提
	static void PushInt(std::vector<DataUnion> & out, int32_t from, int32_t to, int32_t flg)
	{
		if (from != to)
		{
			out.emplace_back(from);
			out.emplace_back(to);
			out[0].use_bit |= flg;
		}
	}

	// float型をデータベクタに追加する outは先頭の use_bit が確保済の前提
	static void PushFloat(std::vector<DataUnion> & out, float from, float to, int32_t flg)
	{
		if (from != to)
		{
			out.emplace_back(from);
			out.emplace_back(to);
			out[0].use_bit |= flg;
		}
	}

	// short型2つをデータベクタに追加する outは先頭の use_bit が確保済の前提
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

	// char型4つをデータベクタに追加する outは先頭の use_bit が確保済の前提
	static void PushChar(std::vector<DataUnion> & out, int8_t f1, int8_t f2, int8_t f3, int8_t f4, int8_t t1, int8_t t2, int8_t t3, int8_t t4, int32_t flg)
	{
		if ((f1 != t1) || (f2 != t2) || (f3 != t3) || (f4 != t4))
		{
			out.emplace_back(f1, f2, f3, f4);
			out.emplace_back(t1, t2, t3, t4);
			out[0].use_bit |= flg;
		}
	}

	// int型をデータベクタから取り出す 取り出せた場合は idx が進む
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

	// float型をデータベクタから取り出す 取り出せた場合は idx が進む
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

	// short型をデータベクタから取り出す 取り出せた場合は idx が進む
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

	// char型をデータベクタから取り出す 取り出せた場合は idx が進む
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
// カメラのキーフレーム差分情報
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
		UseHokanX			= 1<< 10,	// cVal[0]～[4]
		UseHokanY			= 1<< 11,	// cVal[0]～[4]
		UseHokanZ			= 1<< 12,	// cVal[0]～[4]
		UseHokanR			= 1<< 13,	// cVal[0]～[4]
		UseHokanL			= 1<< 14,	// cVal[0]～[4]
		UseHokanA			= 1<< 15,	// cVal[0]～[4]
		UsePerspective		= 1<< 16,	// iVal
		UseViewAngle		= 1<< 17,	// iVal
		UseLookingModel		= 1<< 18,	// sVal[0][1]
	};

	// 差を探して差分情報のベクタを作る
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
	
	// 差分情報の先頭ポインタからキーフレームを復帰する
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

		// モデル追従がある場合、モデルが消えていないか調べる
		// 一応、消えた時に ResetModel() で修正しているはずだが念のため
		if (out.looking_model_index >= 0)
		{
			auto mmdDataP = getMMDMainData();
			if (mmdDataP->model_data[out.looking_model_index] == nullptr)
			{
				// 消えていたので、モデル追従を解除する
				out.looking_model_index = -1;
				out.looking_bone_index = 0;
			}
		}
	}

	// 差分情報のモデル追従でモデルが消えている差分を修正する
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
					// モデル追従で
					for (int fot = 0; fot < Max; fot++)
					{
						auto &modelIdx = myP->data[idx].ary[fot].sVal[0];
						if (modelIdx >= 0)
						{
							// モデルが無ければ
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
				// UseLookingModel 以下のビットを数える
				if(bit & myP->use_bit)
				{
					idx++;
				}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////
// ライトのキーフレーム差分情報
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

	// 差を探して差分情報のベクタを作る
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
	
	// 差分情報の先頭ポインタからキーフレームを復帰する
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
// セルフ影のキーフレーム差分情報
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

	// 差を探して差分情報のベクタを作る
	static void Push(std::vector<DataUnion> & out, const SelfShadowKeyFrameData & from, const SelfShadowKeyFrameData & to)
	{
		Init(out);

		PushInt(out, from.frame_no, to.frame_no, UseFrameNo);
		PushShort(out, from.pre_index, from.next_index, to.pre_index, to.next_index, UsePreIndex, UseNextIndex);
		PushInt(out, (int32_t)from.mode, (int32_t)to.mode, UseMode);
		PushFloat(out, from.range, to.range, UseRange);
	}
	
	// 差分情報の先頭ポインタからキーフレームを復帰する
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
// 重力のキーフレーム差分情報
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

	// 差を探して差分情報のベクタを作る
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
	
	// 差分情報の先頭ポインタからキーフレームを復帰する
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
// アクセサリのキーフレーム差分情報
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

	// 差を探して差分情報のベクタを作る
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

	// 差分情報の先頭ポインタからキーフレームを復帰する
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

		// モデル追従がある場合、モデルが消えていないか調べる
		// 一応、消えた時に ResetModel() で修正しているはずだが念のため
		if (out.looking_model_index >= 0)
		{
			auto mmdDataP = getMMDMainData();
			if (mmdDataP->model_data[out.looking_model_index] == nullptr)
			{
				// 消えていたので、モデル追従を解除する
				out.looking_model_index = -1;
				out.looking_bone_index = 0;
			}
		}
	}

	// 差分情報のモデル追従でモデルが消えている差分を修正する
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
					// モデル追従で
					for (int fot = 0; fot < Max; fot++)
					{
						auto &modelIdx = myP->data[idx].ary[fot].sVal[0];
						if (modelIdx >= 0)
						{
							// モデルが無ければ
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
					// UseLookingModel 以下のビットを数える
					if (bit & myP->use_bit)
					{
						idx++;
					}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////
// キーフレームの比較用コピー管理クラス
template<class FrameDataType>
class KeyFrameCopy
{
	FrameDataType					m_keyFrameCopy[10000];		// フレームデータのコピー 変化検出用
	int								m_useMax;					// m_keyFrameCopy の最大使用要素
	
public:
	// 比較用コピーを更新する
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

	// キーフレーム配列の使用している最大要素を調べる
	static int GetMaxFrameIndex(FrameDataType(&frameAry)[10000])
	{
		int maxIdx = -1;

		for (int idx = 0; idx < 10000;)
		{
			auto &c = frameAry[idx];

			maxIdx = std::max(maxIdx, c.next_index);
			// 次が無ければ終了
			if (c.next_index == 0) { break; }
			// 次のキーフレームへ
			idx = c.next_index;
		}

		return maxIdx;
	}
};

//////////////////////////////////////////////////////////////////////
// カメラ/照明/セルフ影/重力 のキーフレームのコピー
struct CameraModeCopy
{
public:
	KeyFrameCopy<CameraKeyFrameData>		cameraCopy;
	KeyFrameCopy<LightKeyFrameData>			lightCopy;
	KeyFrameCopy<SelfShadowKeyFrameData>	shadowCopy;
	KeyFrameCopy<GravityKeyFrameData>		gravityCopy;

	// 比較用コピーを更新する
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
// アクセサリ のキーフレームのコピー
struct AccessoryModeCopy
{
public:
	KeyFrameCopy<AccessoryKeyFrameData>		accessoryCopy;

	// 比較用コピーを更新する
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
	std::vector< std::pair<int, int>>		m_indexTable;	// キーフレームの要素番号のデータが↓の何処から開始するかのテーブル
	std::vector<KeyFrameChange::DataUnion>	m_changeData;	// 変化したキーフレームを保存するためのベクタ
	int										m_size;			// 上2つのデータのサイズ

public:
	typedef std::shared_ptr<UndoRedo>	SP;

	static SP Create(KeyFrameCopy<FrameDataType> const & copy, FrameDataType(&nowAry)[10000] )
	{
		std::vector<std::pair<int, int> >		indexTable;
		std::vector<KeyFrameChange::DataUnion>	changeData;

		// コピーと現在の最大要素で大きい方まで調べる
		int nowUseMax = KeyFrameCopy<FrameDataType>::GetMaxFrameIndex(nowAry);
		int useMax = std::max(copy.UseMax(), nowUseMax);
		// 念のため
		useMax = std::min(useMax, 10000-1);

		// 最大使用箇所までの変化をチェックする
		std::vector<KeyFrameChange::DataUnion> out;
		for (int idx = 0; idx <= useMax; idx++)
		{
			CChange::Push(out, copy.GetFrame(idx), nowAry[idx]);
			if (out[0].use_bit)
			{
				// 変化したフレームを記憶
				indexTable.emplace_back(idx, (int)changeData.size()); // 現在のサイズ(==最後尾+1)が↓で追加する先頭になる
				changeData.insert(changeData.cend(), out.cbegin(), out.cend());
			}
		}

		// 変化していなければnullptr
		if (changeData.empty()) { return nullptr; }

		auto sp = std::make_shared<UndoRedo>();
		sp->m_indexTable.swap(indexTable);
		sp->m_changeData.swap(changeData);
		// 無駄領域を削除
		std::vector<std::pair<int, int>>(sp->m_indexTable).swap(sp->m_indexTable);
		std::vector<KeyFrameChange::DataUnion>(sp->m_changeData).swap(sp->m_changeData);
		sp->m_size = (int)sp->m_indexTable.size() * sizeof(std::pair<int, int>) +
					 (int)sp->m_changeData.size() * sizeof(KeyFrameChange::DataUnion);
		return sp;
	}


	//////////////////////////////////////////////////////
	// アンドゥ
	void Undo(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			auto& idx = s.first;
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			CChange::Pop(nowAry[idx], topP, KeyFrameChange::From);
		}
	}

	// リドゥ
	void Redo(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			auto& idx = s.first;
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			CChange::Pop(nowAry[idx], topP, KeyFrameChange::To);
		}
	}

	// モデルが削除されたときのコールバック
	void OnModelDelete(FrameDataType(&nowAry)[10000])
	{
		for (auto &s : m_indexTable)
		{
			KeyFrameChange::DataUnion *topP = &m_changeData[s.second];

			// モデル追従の修正
			CChange::ResetModel(topP);
		}
	}

	// 使用サイズを返す
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
typedef UndoRedo<CameraKeyFrameChange, CameraKeyFrameData>				CameraUndoRedo;
typedef UndoRedo<LightKeyFrameChange, LightKeyFrameData>				LightUndoRedo;
typedef UndoRedo<SelfShadowKeyFrameChange, SelfShadowKeyFrameData>		SelfShadowUndoRedo;
typedef UndoRedo<GravityKeyFrameChange, GravityKeyFrameData>			GravityUndoRedo;
typedef UndoRedo<AccessoryKeyFrameChange, AccessoryKeyFrameData>		AccessoryUndoRedo;


//////////////////////////////////////////////////////////////////////
// カメラ/照明/セルフ影/重力 のアンドゥ/リドゥを合わせて行う
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
	// アンドゥ
	void Undo(int idx)
	{
		if (m_cameraP)	{ m_cameraP->	Undo(m_mmdDataP->camera_key_frame); }
		if (m_lightP)	{ m_lightP->	Undo(m_mmdDataP->light_key_frame); }
		if (m_shadowP)	{ m_shadowP->	Undo(m_mmdDataP->self_shadow_key_frame); }
		if (m_gravityP)	{ m_gravityP->	Undo(m_mmdDataP->gravity_key_frame); }
	}

	// リドゥ
	void Redo(int idx)
	{
		if (m_cameraP)	{ m_cameraP->	Redo(m_mmdDataP->camera_key_frame); }
		if (m_lightP)	{ m_lightP->	Redo(m_mmdDataP->light_key_frame); }
		if (m_shadowP)	{ m_shadowP->	Redo(m_mmdDataP->self_shadow_key_frame); }
		if (m_gravityP)	{ m_gravityP->	Redo(m_mmdDataP->gravity_key_frame); }
	}

	// モデルが削除されたときのコールバック
	void OnModelDelete(int idx)
	{
		// モデルが関係するのはカメラのみ
		if (m_cameraP)	{ m_cameraP->	OnModelDelete(m_mmdDataP->camera_key_frame); }
	}

	// 使用サイズを返す
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
// アクセサリ のアンドゥ/リドゥを行う
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
	// アンドゥ
	void Undo(int idx)
	{
		if (m_accessoryP)	{ m_accessoryP->Undo(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// リドゥ
	void Redo(int idx)
	{
		if (m_accessoryP)	{ m_accessoryP->Redo(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// モデルが削除されたときのコールバック
	void OnModelDelete(int idx)
	{
		// モデルが関係するのはカメラのみ
		if (m_accessoryP)	{ m_accessoryP->OnModelDelete(*m_mmdDataP->accessory_key_frames[idx]); }
	}

	// 使用サイズを返す
	int GetSize() { return m_size; }
};

//////////////////////////////////////////////////////////////////////
template<class CCopy, class CUndo>
class KeyFrameUndoRedo
{
private:
	MMDMainData*						m_mmdDataP;

	int									m_idx;						// コピーする要素番号(主にアクセサリ用)
	CCopy								m_copy;
	std::deque<typename CUndo::SP>		m_undoDec;					// アンドゥ用のキュー
	std::deque<typename CUndo::SP>		m_redoDec;					// リドゥ用のキュー
	int									m_buffSize;					// 現在のバッファのサイズ合計
	int									m_dataMaxSize;				// 最大バッファサイズ
	
	// バッファオーバー時の削除要素数
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

		_dbgPrint("アンドゥ/リドゥ 初期化");
	}

	// アンドゥ
	void Undo()
	{
		if (m_undoDec.empty()) { return; }
		
		DbgTimer dbgTimer([this](float count) { _dbgPrint("処理時間:%f ms Buff:%9.2f KB キュー:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); } );

		// アンドゥキューがあれば、取り出して戻す
		m_undoDec.back()->Undo(m_idx);
		// リドゥキューに積む
		m_redoDec.push_back(m_undoDec.back());
		m_undoDec.pop_back();
		// 比較用コピーを更新
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

	// リドゥ
	void Redo()
	{
		if (m_redoDec.empty()) { return; }
		
		DbgTimer dbgTimer([this](float count) { _dbgPrint("処理時間:%f ms Buff:%9.2f KB キュー:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); });

		// リドゥキューがあれば、取り出して戻す
		m_redoDec.back()->Redo(m_idx);
		// アンドゥキューに積む
		m_undoDec.push_back(m_redoDec.back());
		m_redoDec.pop_back();
		// 比較用コピーを更新
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

	// キーフレーム変化を検出する
	bool CheckFrame()
	{
		int beforeSize = m_buffSize;
		DbgTimer dbgTimer([this, beforeSize](float count) {if (beforeSize != m_buffSize) { _dbgPrint("検出時処理時間:%f ms Buff:%9.2f KB キュー:%d", count, (m_buffSize / 1024.0f), m_undoDec.size()); }});

		bool bDo = false;

		// 前の状態と現在の状態で変化していればクラスが作られて返る
		auto camModeUR = CUndo::Create(m_copy, m_idx);
		if (camModeUR != nullptr)
		{
			// 変化していたら、変化情報をアンドゥキューに追加
			bDo = true;
			m_undoDec.emplace_back(camModeUR);
			m_buffSize += camModeUR->GetSize();

			// リドゥキューはクリア
			m_redoDec.clear();

			// 比較用コピーを更新
			m_copy.UpdateKeyFrameCopy(m_idx);

			// バッファサイズを超えたら、古いデータを消しておく
			if (m_buffSize > m_dataMaxSize)
			{
				// サイズ的に10個消せない事はあまりないと思うが、念のためキューに最低半分は残る様にしておく
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

	// 現在のアンドゥ/リドゥ可能回数
	size_t GetUndoNum() { return m_undoDec.size(); }
	size_t GetRedoNum() { return m_redoDec.size(); }

	// モデルが削除されたときのコールバック
	void OnModelDelete()
	{
		for (auto& d : m_undoDec) { d->OnModelDelete(m_idx); }
		for (auto& d : m_redoDec) { d->OnModelDelete(m_idx); }

		// モデル削除をキーフレーム変更で検出させない為に比較用コピーを更新
		m_copy.UpdateKeyFrameCopy(m_idx);
	}

#ifdef USE_DEBUG
	// ダミーデータ生成
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

		_dbgPrint("アンドゥキュー:%d リドゥキュー:%d", m_undoDec.size(), m_redoDec.size());
		for (int cnt = 0; cnt < 10; cnt++)
		{
			auto &c = mmdDataP->camera_key_frame[cnt];
			_dbgPrint(" %3d 前:%3d 次:%3d 選:%d %f %f %f", c.frame_no, c.pre_index, c.next_index, c.is_selected, c.xyz.x, c.xyz.y, c.xyz.z);
		}
		_dbgPrint("------------------------");
	}
#endif // USE_DEBUG
};

typedef KeyFrameUndoRedo<CameraModeCopy, CameraModeUndoRedo>			CameraModeKeyFrameUndoRedo;
typedef KeyFrameUndoRedo<AccessoryModeCopy, AccessoryModeUndoRedo>		AccessoryModeKeyFrameUndoRedo;

//////////////////////////////////////////////////////////////////////
class CameraModeUndoPlugin : public MMDPluginDLL3
{
private:
	MMDMainData*									m_mmdDataP;
	CInifile										m_ini;
	bool											m_bShowWindow;		// 初回の WM_SHOWWINDOW 判定用
	CameraKeyFrameData *							m_cameraKeyFrameP;	// カメラのキーフレーム配列の先頭ポインタ
	std::shared_ptr<CameraModeKeyFrameUndoRedo>		m_camUndoP;
	std::shared_ptr<AccessoryModeKeyFrameUndoRedo>	m_accUndoP[255];

	CKeyState										m_camUndoKey;		// アンドゥのキー
	CKeyState										m_camUedoKey;		// リドゥのキー
	CKeyState										m_accUndoKey;		// アクセサリアンドゥのキー
	CKeyState										m_accUedoKey;		// アクセサリリドゥのキー
	CKeyState										m_testKey;			// テストのキー
	int												m_undoBuffMB;		// アンドゥバッファサイズ(MB)

	HWND											m_frameNoEditH;		// MMDの「フレーム番号」のエディットボックスのハンドル
	HWND											m_hscrollH;			// MMDのタイムラインの水平スクロールバーのハンドル
	HWND											m_curveComboH;		// MMDの「補間曲線操作」パネルのコンボボックスのハンドル
	HWND											m_modelComboH;		// MMDの「モデル操作」パネルのコンボボックスのハンドル
	HWND											m_accComboH;		// MMDの「アクセサリ操作」パネルのコンボボックスのハンドル
	HWND											m_undoBtnH;			// MMDの「元に戻す」ボタン
	HWND											m_redoBtnH;			// MMDの「やり直し」ボタン
	HWND											m_myCamUndoBtnH;	// カメラ用に自作する「元に戻す」ボタン
	HWND											m_myCamRedoBtnH;	// カメラ用に自作する「やり直し」ボタン
	HWND											m_myAccUndoBtnH;	// アクセサリ用に自作する「元に戻す」ボタン
	HWND											m_myAccRedoBtnH;	// アクセサリ用に自作する「やり直し」ボタン
	bool											m_bCheckFrame;		// キーフレームの変更検出
	bool											m_bRepaint;			// 再描画する
	bool											m_bUpdateBtnState;	// 「元に戻す」「やり直し」ボタンを更新する
	bool											m_bUpdateBtnVisible;// 「元に戻す」「やり直し」ボタンの表示状態を更新する
	bool											m_bModelDelete;		// モデルが削除された
	bool											m_bAccAddDelete;	// アクセサリが追加/削除された
	
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
	// カメラ用に自作する「元に戻す」ボタンと「やり直し」ボタンのコントロールID
	static constexpr int MY_CAM_UNDO_BTN_CTRL_ID = 0xA190;
	static constexpr int MY_CAM_REDO_BTN_CTRL_ID = 0xA191;
	// アクセサリ用に自作する「元に戻す」ボタンと「やり直し」ボタンのコントロールID
	static constexpr int MY_ACC_UNDO_BTN_CTRL_ID = 0xB190;
	static constexpr int MY_ACC_REDO_BTN_CTRL_ID = 0xB191;
public:
	const char* getPluginTitle() const override { return "MMDUtility_CameraUndo"; }

	CameraModeUndoPlugin()
		: m_mmdDataP(nullptr)
		, m_bShowWindow(false)
		, m_cameraKeyFrameP(nullptr)
		, m_curveComboH(nullptr)
		, m_modelComboH(nullptr)
		, m_accComboH(nullptr)
		, m_undoBtnH(nullptr)
		, m_redoBtnH(nullptr)
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
			ini.Str(L"CameraUndo") = L"Ctrl + Z";
			ini.Str(L"CameraRedo") = L"Ctrl + X";
			ini.Str(L"AccessoryUndo") = L"Ctrl + Shift + Z";
			ini.Str(L"AccessoryRedo") = L"Ctrl + Shift + X";
			ini.Int(L"UndoBuffMaxMB") = 100;
		});

		// 入力判定を初期化
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

		// アンドゥ/リドゥクラスを初期化
		_checkRemakePtr();
	}

	void stop() override 
	{
		m_ini.Write();
	}

	void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
	{
		_checkRemakePtr();
		if (_isCameraMode())
		{
			if ((lParam & 0x40000000) &&	// 直前にキーが押されていて
				(lParam & 0x80000000))		// キーが離されているなら
			{
				// キーフレーム変更検出
				m_bCheckFrame = true;
			}

			// アンドゥ
			if (m_camUndoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				m_camUndoP->Undo();
				m_bRepaint = m_bUpdateBtnState = true; // 実行したらボタン更新
			}
			// リドゥ
			if (m_camUedoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				m_camUndoP->Redo();
				m_bRepaint = m_bUpdateBtnState = true; // 実行したらボタン更新
			}

			// アンドゥ
			if (m_accUndoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				if (_accUndo() != nullptr)
				{
					_accUndo()->Undo();
					m_bRepaint = m_bUpdateBtnState = true; // 実行したらボタン更新
				}
			}
			// リドゥ
			if (m_accUedoKey.IsRepeat(wParam, lParam, 10, 2, 2))
			{
				if (_accUndo() != nullptr)
				{
					_accUndo()->Redo();
					m_bRepaint = m_bUpdateBtnState = true; // 実行したらボタン更新
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
			m_camUndoP->CreateDummyData();
		}
#endif // USE_DEBUG

		_update();
	}

	void MouseProc(WPARAM wParam, MOUSEHOOKSTRUCT* lParam) override
	{
		_checkRemakePtr();
		if (_isCameraMode())
		{
			// 左ボタンアップ時に検出
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
		char * codeName = "不明";
		if( code == MSGF_DIALOGBOX ) { codeName = "ダイアログ"; }
		if( code == MSGF_MESSAGEBOX) { codeName = "メッセージボックス"; }
		if( code == MSGF_MENU      ) { codeName = "メニュー"; }
		if( code == MSGF_SCROLLBAR ) { codeName = "スクロールバー"; }
		if( code == MSGF_USER      ) { codeName = "ユーザー定義"; }
		_dbgPrint("■■■■■■■MsgProc MSG:0x%08X code:%d[%s]", param->message, code, codeName);
		*/
	}

	void GetMsgProc(int code, MSG* param) override
	{
	}

	// MMD側の呼び出しが制御できるプロシージャ
	// false : MMDのプロシージャも呼ばれます。LRESULTは無視されます
	// true  : MMDやその他のプラグインのプロシージャは呼ばれません。LRESULTが全体のプロシージャの戻り値として返されます。
	std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		return { false, 0 };
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

		_checkRemakePtr();

		if (param->hwnd == m_modelComboH)
		{
			// 「モデル操作」パネルのコンボボックスが変更された場合(主にプログラムから操作された時)
			if (param->message == CB_SETCURSEL)
			{
				m_bUpdateBtnState = true;
			}
			// コンボボックスの項目が減った == モデルが削除された場合
			else if (param->message == CB_DELETESTRING)
			{
				m_bModelDelete = true;
			}
		}

		if (param->hwnd == m_accComboH)
		{
			// 「アクセサリ操作」パネルのコンボボックスが変更された場合(主にプログラムから操作された時)
			if (param->message == CB_SETCURSEL)
			{
				m_bUpdateBtnState = true;
			}
			// コンボボックスの項目が増減した == アクセサリが追加/削除された場合
			else if (param->message == CB_DELETESTRING ||
					 param->message == CB_ADDSTRING)
			{
				m_bAccAddDelete = true;
			}
		}

		if (param->message == WM_WINDOWPOSCHANGED)
		{
			auto posP = (WINDOWPOS*)param->lParam;
			// 移動なら
			if ((posP->flags & SWP_NOMOVE) == 0)
			{
				// MMDの「元に戻す」ボタンが動いたら自作のボタンも追従する
				if (param->hwnd == m_undoBtnH)
				{
					int center = posP->x + 70 + 10;	// 「元に戻す」ボタンと「やり直し」ボタンの真ん中
					int w = 60;						// 自作ボタンの幅

					SetWindowPos(m_myCamUndoBtnH, HWND_NOTOPMOST, center -10 -w -10 -w,	posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myCamRedoBtnH, HWND_NOTOPMOST, center -10 -w,		posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myAccUndoBtnH, HWND_NOTOPMOST, center +10,			posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
					SetWindowPos(m_myAccRedoBtnH, HWND_NOTOPMOST, center +10 +w +10,	posP->y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
				}
			}
		}

		// MMDの「やり直し」ボタンが表示される時、カメラモードなら再度消す
		if (param->message == WM_SHOWWINDOW)
		{
			if (param->hwnd == m_undoBtnH)
			{
				if (param->wParam == TRUE)
				{
					if (_isCameraMode())
					{
						m_bUpdateBtnVisible = true;
					}
				}
			}
		}

		if (param->message == WM_COMMAND)
		{
			auto id = LOWORD(param->wParam);

			// 「登録」ボタンが押された場合
			// 「重力」はダイアログなのでダイアログのOKボタン
			//  重力設定以外のモデルの説明、閉じる前の警告などでも来るがまぁ良いか
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
			// 自作「元に戻す」ボタンが押された場合
			if (id == MY_CAM_UNDO_BTN_CTRL_ID)
			{
				if (m_camUndoP != nullptr)	{ m_camUndoP->Undo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// 自作「やり直し」ボタンが押された場合
			if (id == MY_CAM_REDO_BTN_CTRL_ID)
			{
				if (m_camUndoP != nullptr) { m_camUndoP->Redo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			// 自作「元に戻す」ボタンが押された場合
			if (id == MY_ACC_UNDO_BTN_CTRL_ID)
			{
				if (_accUndo() != nullptr)	{ _accUndo()->Undo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// 自作「やり直し」ボタンが押された場合
			if (id == MY_ACC_REDO_BTN_CTRL_ID)
			{
				if (_accUndo() != nullptr) { _accUndo()->Redo(); m_bRepaint = m_bUpdateBtnState = true; }
			}
			else
			// 「モデル操作」パネルか「アクセサリ操作」パネルのコンボボックスが変更された場合(ユーザーが操作した時)
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

	// 初回の WM_SHOWWINDOW 前に呼ばれる
	// この時点ではボタン等のコントロールが出来ているはず
	void _beforeShowWindow()
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
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
		// MMDの「元に戻す」ボタンと「やり直し」ボタンを取得しておく
		m_undoBtnH = GetDlgItem(hMmd, UNDO_BTN_CTRL_ID);
		m_redoBtnH = GetDlgItem(hMmd, REDO_BTN_CTRL_ID);

		// ボタンを作る
		m_myCamUndoBtnH = CreateWindow(L"button", L"<-", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								116, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_CAM_UNDO_BTN_CTRL_ID, hInstance, NULL);
		m_myCamRedoBtnH = CreateWindow(L"button", L"->", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								206, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_CAM_REDO_BTN_CTRL_ID, hInstance, NULL);

		m_myAccUndoBtnH = CreateWindow(L"button", L"<-", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								116, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_ACC_UNDO_BTN_CTRL_ID, hInstance, NULL);
		m_myAccRedoBtnH = CreateWindow(L"button", L"->", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
								206, 21, 60, 24, hMmd, (HMENU)(UINT_PTR)MY_ACC_REDO_BTN_CTRL_ID, hInstance, NULL);

		// フォントをそろえる
		HFONT fontH = (HFONT)SendMessage(m_undoBtnH, WM_GETFONT, 0, 0);
		SendMessage(m_myCamUndoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myCamRedoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myAccUndoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);
		SendMessage(m_myAccRedoBtnH, WM_SETFONT, (WPARAM)fontH, TRUE);

		// 表示状態を設定
		_checkRemakePtr();
		_updateBtnState();
	}

	// ボタンなどの更新
	void _update()
	{
		// モデル削除時の処理
		if (m_bModelDelete)
		{
			m_camUndoP->OnModelDelete();
			for (auto &acc : m_accUndoP)
			{
				if (acc != nullptr) { acc->OnModelDelete(); }
			}
			m_bModelDelete = false;
		}

		// アクセサリ増減時の処理
		if (m_bAccAddDelete)
		{
			_checkAccUndoPtr();
			m_bAccAddDelete = false;
		}

		// 「登録」ボタンが押されたらカメラのキーフレーム変化を検出する
		if (m_bCheckFrame)
		{
			m_bUpdateBtnState |= m_camUndoP->CheckFrame();
			if (_accUndo() != nullptr) { m_bUpdateBtnState |= _accUndo()->CheckFrame(); }
			m_bCheckFrame = false;
		}

		// 自作ボタンの更新
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

		// 再描画
		if (m_bRepaint)
		{
			_setRepaintTimer();
			m_bRepaint = false;
		}
	}

	// キーフレームのポインタが変わっていたらアンドゥ/リドゥクラスを再初期化する
	void _checkRemakePtr()
	{
		// camera_key_frame のポインタはシーンをロードしたら変わる
		// vmdの読み込みでは変わらない
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

	// アクセサリの増減に合わせ、アンドゥ/リドゥクラスも作ったり消したりする
	void _checkAccUndoPtr()
	{
		for (int cnt = 0; cnt < ARRAYSIZE(m_accUndoP); cnt++)
		{
			// アクセサリが追加されていたら
			if (m_accUndoP[cnt] == nullptr && m_mmdDataP->accessory_data[cnt] != nullptr)
			{
				m_accUndoP[cnt] = std::make_shared<AccessoryModeKeyFrameUndoRedo>(cnt, m_undoBuffMB);
				m_bUpdateBtnState = true;
			}
			else
			// アクセサリが削除されていたら
			if(m_accUndoP[cnt] != nullptr && m_mmdDataP->accessory_data[cnt] == nullptr)
			{
				m_accUndoP[cnt].reset();
				m_bUpdateBtnState = true;
			}
		}
	}

	// 現在カメラモードかどうかコンボボックスの状態から判定する
	// select_bone_type はモデルモードの未選択時でも「2:カメラモード」になっているので使えない
	bool _isCameraMode()
	{
		if (m_modelComboH != nullptr)
		{
			// 0番目の「ｶﾒﾗ･照明･ｱｸｾｻﾘ」が選択されているか?
			if (ComboBox_GetCurSel(m_modelComboH) == 0)
			{
				return true;
			}
		}
		return false;
	}

	// ボタンの状態を更新する
	void _updateBtnState()
	{
		_updateBtnVisible();
		if (m_camUndoP != nullptr)
		{
			auto undoNum = m_camUndoP->GetUndoNum();
			auto redoNum = m_camUndoP->GetRedoNum();
			TCHAR txt[32] = _T("");

			// 回数をボタンに表示
			_sntprintf(txt, ARRAYSIZE(txt), _T("<- (%zd)"), undoNum);
			SetWindowText(m_myCamUndoBtnH, txt);

			_sntprintf(txt, ARRAYSIZE(txt), _T("(%zd) ->"), redoNum);
			SetWindowText(m_myCamRedoBtnH, txt);

			// アンドゥ/リドゥが出来る場合のみ有効にする
			EnableWindow(m_myCamUndoBtnH, undoNum > 0);
			EnableWindow(m_myCamRedoBtnH, redoNum > 0);
		}
		if (_accUndo() != nullptr)
		{
			auto undoNum = _accUndo()->GetUndoNum();
			auto redoNum = _accUndo()->GetRedoNum();
			TCHAR txt[32] = _T("");

			// 回数をボタンに表示
			_sntprintf(txt, ARRAYSIZE(txt), _T("<- (%zd)"), undoNum);
			SetWindowText(m_myAccUndoBtnH, txt);

			_sntprintf(txt, ARRAYSIZE(txt), _T("(%zd) ->"), redoNum);
			SetWindowText(m_myAccRedoBtnH, txt);

			// アンドゥ/リドゥが出来る場合のみ有効にする
			EnableWindow(m_myAccUndoBtnH, undoNum > 0);
			EnableWindow(m_myAccRedoBtnH, redoNum > 0);
		}
		else
		{
			EnableWindow(m_myAccUndoBtnH, false);
			EnableWindow(m_myAccRedoBtnH, false);
		}
	}

	// ボタンの表示状態を更新する
	void _updateBtnVisible()
	{
		bool bCamMode = _isCameraMode();

		// MMDの「元に戻す」ボタンと「やり直し」ボタンはカメラモードでは非表示
		ShowWindow(m_undoBtnH, !bCamMode);
		ShowWindow(m_redoBtnH, !bCamMode);
		// 自作の「元に戻す」ボタンと「やり直し」ボタンはカメラモードでは表示
		ShowWindow(m_myCamUndoBtnH, bCamMode);
		ShowWindow(m_myCamRedoBtnH, bCamMode);
		ShowWindow(m_myAccUndoBtnH, bCamMode);
		ShowWindow(m_myAccRedoBtnH, bCamMode);
	}

	// 再描画タイマーをセットする
	// マウス処理から再描画を呼ぶとマウスを動かした位置でマウスアップと判定されてしまう
	// なのでタイマーをセットしてマウス処理が終わってから再描画される様にする
	// だったけど、今はマウスを動かさない様にしたので _repaint() 呼んでも問題ないかも…
	void _setRepaintTimer()
	{
		// メッセージをポストするやり方だと、何故かは知らないがMMD操作中にはWM_TIMERが送られてこない
		// (タイトルバーとかを操作すると遅れて届き始める)
		// それじゃ役に立たないので、コールバックで処理する
		SetTimer(getHWND(), (UINT_PTR)this, 1, [](HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			KillTimer(getHWND(), idEvent);

			CameraModeUndoPlugin * thisP = (CameraModeUndoPlugin *)idEvent;
			thisP->_repaint();
		});
	}

	// タイムラインと補間曲線の再描画
	void _repaint()
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
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
	if (!MMDVersionOk())
	{
		MessageBox(getHWND(), _T("MMDのバージョンが 9.31 ではありません。\nCameraModeUndoは Ver.9.31 以外では正常に作動しません。"), _T("CameraModeUndo"), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return new CameraModeUndoPlugin();
}