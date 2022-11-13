#include "stdafx.h"
#include <vector>
#include <windows.h>
#include <windowsX.h>

#pragma once

//////////////////////////////////////////////////////////////////////
// MMDのモデルやアクセサリ関連のユティリティクラス
class CMMDDataUtil
{
private:
	CMMDDataUtil() {}
	~CMMDDataUtil() {}

public:
	// 描画順でソートされたモデルベクタを返す
	static std::vector<MMDModelData *> RenderOrderModels()
	{
		auto mmdDataP = getMMDMainData();
		return _makeRenderOrder(&mmdDataP->model_data[0], &mmdDataP->model_data[255]);
	}

	// 描画順でソートされたアクセサリベクタを返す
	static std::vector<MMDAccessoryData *> RenderOrderAccessorys()
	{
		auto mmdDataP = getMMDMainData();
		return _makeRenderOrder(&mmdDataP->accessory_data[0], &mmdDataP->accessory_data[255]);
	}

	// コンテナに描画順でソートされたデータポインタをセットする
	template<class T> static std::vector<T*> _makeRenderOrder( T **begin, T **end )
	{
		std::vector<T*>	vec;

		// nullptr以外でコンテナを作る
		std::copy_if(begin, end, std::back_inserter(vec), [](T *m){ return m != nullptr; });
		// 描画順でソート
		std::sort(vec.begin(), vec.end(), [](T *a, T *b) { return a->render_order < b->render_order; });

		return vec;
	}
	


	// 描画順でソートされたモデルデータのインデックスベクタを返す
	static std::vector<int> RenderOrderModelsIndex()
	{
		auto mmdDataP = getMMDMainData();
		return _makeRenderOrderIndex(&mmdDataP->model_data[0], &mmdDataP->model_data[255]);
	}

	// 描画順でソートされたアクセサリデータのインデックスベクタを返す
	static std::vector<int> RenderOrderAccessorysIndex()
	{
		auto mmdDataP = getMMDMainData();
		return _makeRenderOrderIndex(&mmdDataP->accessory_data[0], &mmdDataP->accessory_data[255]);
	}

	// コンテナに描画順でソートされたデータのインデックスをセットする
	template<class T> static std::vector<int> _makeRenderOrderIndex( T **begin, T **end )
	{
		std::vector<std::tuple<int, int>>	sv;
		std::vector<int>					vec;
		auto								first = begin;
		auto								last = end;

		for (int idx = 0; first != last; ++first, ++idx)
		{
			if (*first != nullptr)
			{
				sv.emplace_back((*first)->render_order, idx);
			}
		}

		// 描画順でソート
		std::sort(sv.begin(), sv.end(), [](std::tuple<int, int> &a, std::tuple<int, int> &b) { return std::get<0>(a) < std::get <0>(b); });

		for (auto a : sv)
		{
			vec.emplace_back(std::get<1>(a));
		}

		return vec;
	}
};

