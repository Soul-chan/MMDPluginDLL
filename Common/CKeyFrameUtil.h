#include "stdafx.h"
#include <windows.h>
#include <windowsX.h>

#pragma once

//////////////////////////////////////////////////////////////////////
// MMDのキーフレーム関連のユティリティクラス
class CKeyFrameUtil
{
private:
	CKeyFrameUtil() {}
	~CKeyFrameUtil() {}

public:
	// カメラのキーフレームを検索する
	static CameraKeyFrameData* FindCameraKeyFrame(int frame, bool bAdd)
	{
		auto mmdDataP = getMMDMainData();
		return _findKeyFrame(mmdDataP->camera_key_frame, frame, 0, bAdd, 1);
	}

	// 照明のキーフレームを検索する
	static LightKeyFrameData* FindLightKeyFrame(int frame, bool bAdd)
	{
		auto mmdDataP = getMMDMainData();
		return _findKeyFrame(mmdDataP->light_key_frame, frame, 0, bAdd, 1);
	}

	// セルフ影のキーフレームを検索する
	static SelfShadowKeyFrameData* FindSelfShadowKeyFrame(int frame, bool bAdd)
	{
		auto mmdDataP = getMMDMainData();
		return _findKeyFrame(mmdDataP->self_shadow_key_frame, frame, 0, bAdd, 1);
	}

	// 重力のキーフレームを検索する
	static GravityKeyFrameData* FindGravityKeyFrame(int frame, bool bAdd)
	{
		auto mmdDataP = getMMDMainData();
		return _findKeyFrame(mmdDataP->gravity_key_frame, frame, 0, bAdd, 1);
	}

	// アクセサリのキーフレームを検索する
	static AccessoryKeyFrameData* FindAccessoryeyFrame(int accNo, int frame, bool bAdd)
	{
		auto mmdDataP = getMMDMainData();
		return _findKeyFrame(*mmdDataP->accessory_key_frames[accNo], frame, 0, bAdd, 1);
	}

	// モデルボーンのキーフレームを検索する
	static MMDModelData::BoneKeyFrame* FindBoneKeyFrame(MMDModelData* modelP, int boneNo, int frame, bool bAdd)
	{
		return _findKeyFrame(modelP->bone_keyframe, frame, boneNo, bAdd, modelP->bone_count);
	}

	// モデルモーフのキーフレームを検索する
	static MMDModelData::MorphKeyFrame* FindMorphKeyFrame(MMDModelData* modelP, int morphNo, int frame, bool bAdd)
	{
		return _findKeyFrame(modelP->morph_keyframe, frame, morphNo, bAdd, modelP->morph_count);
	}

	// frame のキーフレームを検索する
	// bAdd 検索してなかった場合、空いているフレームを探して追加して返すかどうか
	// 見つからなければ nullptr を返す
	template<class T, int aryCount> static T* _findKeyFrame( T(&keyFrameAry)[aryCount], int frame, int topIdx, bool bAdd, int addStartIdx )
	{
		T *f = &keyFrameAry[topIdx];
		T *pre = nullptr;	// 探すキーフレームの1つ前のキーフレーム

		if (frame < 0 || frame >= aryCount)	{ return nullptr; }

		// 最後のキーフレームになるまでループ
		while (true)
		{
			if (f->frame_no <  frame)	{ pre = f; }			// frameより小さければ1つ前のキーフレームを更新
			if (f->frame_no == frame)	{ return f; }			// 探しているフレームがあれば終了
			if (f->next_index == 0)		{ break; }				// 次が無ければ終了
			f = &keyFrameAry[f->next_index];					// 次へ
		}

		// ここに来るのは frame のキーフレームが見つからなかったとき

		// 追加するなら
		if (bAdd && pre != nullptr)
		{
			T *empty = nullptr;

			// 空いているフレームを探して
			for (int cnt = addStartIdx; cnt < aryCount; cnt++)
			{
				empty = &keyFrameAry[cnt];

				if (empty->frame_no == 0)
				{
					T *top = &keyFrameAry[0];
					T *next = (pre->next_index != 0) ? &keyFrameAry[pre->next_index] : nullptr;

					// 見つかれば、フレームを初期化して
					empty->frame_no = frame;
					empty->pre_index = (int)(pre - top);
					empty->next_index = (next != nullptr) ? ((int)(next - top)) : 0;

					// 挿入して
					pre->next_index = cnt;
					if (next != nullptr) { next->pre_index = cnt; }

					// 返す
					return empty;
				}
			}
		}

		return nullptr;
	}
};

