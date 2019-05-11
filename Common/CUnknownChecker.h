#include "stdafx.h"
#include <functional>
#include "../MMDPlugin/mmd_plugin.h"
using namespace mmp;

#pragma once

#ifdef _DEBUG
	#define	USE_DEBUG
#else
#endif // _DEBUG

//////////////////////////////////////////////////////////////////////
void _dbgPrint(const char *aFormatP, ...)
{
#ifdef USE_DEBUG
	static char msg[2048];
	va_list ap;
	va_start(ap, aFormatP);
	vsprintf(msg, aFormatP, ap);
	va_end(ap);

	OutputDebugStringA("■■■");
	OutputDebugStringA(msg);
	OutputDebugStringA("\n");
#endif // USE_DEBUG
}

//////////////////////////////////////////////////////////////////////
class CUnknownChecker
{
	static std::vector<BYTE> m_mmdDataVec;

public:
	static void Copy()
	{
		auto mmd_data = getMMDMainData();
		m_mmdDataVec.resize(sizeof(MMDMainData));
		CopyMemory(&m_mmdDataVec[0], mmd_data, m_mmdDataVec.size());
		_dbgPrint("★ MMDMainData をコピーしました。");
	}
	static void Check()
	{
		if (m_mmdDataVec.size() != sizeof(MMDMainData)) { _dbgPrint("★ MMDMainData がコピーされていません。");  return; }

		_dbgPrint("★ MMDMainData の unknown を比較開始します。");
		auto mmd_data = getMMDMainData();
		MMDMainData * copyP = (MMDMainData *)&m_mmdDataVec[0];
#define	ARY_CHECK(name, fmt)	\
		if (memcmp(copyP->name, mmd_data->name, sizeof(copyP->name)) != 0)	\
		{	\
			int __num = sizeof(copyP->name) / sizeof(copyP->name[0]);	\
			for (int __cnt = 0; __cnt < __num; __cnt++)	\
			{	\
				if (memcmp(&copyP->name[__cnt], &mmd_data->name[__cnt], sizeof(copyP->name[0])) != 0)	\
				{	\
					_dbgPrint( "違う " #name "[%d] " #fmt "!=" #fmt, __cnt, copyP->name[__cnt], mmd_data->name[__cnt]);	\
				}	\
			}	\
		}

		ARY_CHECK(__unknown10, %d);
		ARY_CHECK(__unknown20, %d);
		ARY_CHECK(__unknown40, %d);
		ARY_CHECK(__unknown41, %d);
		ARY_CHECK(__unknown49, %d);
		ARY_CHECK(__unknown50, %d);
		ARY_CHECK(__unknown60, %d);
		ARY_CHECK(__unknown70, %d);
		ARY_CHECK(__unknown80, %d);
		ARY_CHECK(__unknown100, %d);
		ARY_CHECK(__unknown103, %d);
		ARY_CHECK(__unknown104, %d);
		ARY_CHECK(__unknown105, %d);
		ARY_CHECK(__unknown106, %d);
		ARY_CHECK(__unknown109, %d);
		ARY_CHECK(__unknown110, %d);
		ARY_CHECK(__unknown120, %d);
		ARY_CHECK(__unknown130, %d);

#define	VAL_CHECK(name, fmt)	\
		if (copyP->name != mmd_data->name)	\
		{	\
			_dbgPrint( "違う " #name " " #fmt "!=" #fmt, copyP->name, mmd_data->name);	\
		}

		VAL_CHECK(__unknown30, %d);
		VAL_CHECK(__unknown71, %f);
		VAL_CHECK(__unknown90, %d);
		VAL_CHECK(left_frame, %d);
		VAL_CHECK(pre_left_frame, %d);
	}

	static void Print()
	{
		auto mmd_data = getMMDMainData();
		
		_dbgPrint("mmd_data:%p - %p size:%d", mmd_data, mmd_data +1, sizeof(*mmd_data));
		_dbgPrint("__unknown40:%p", mmd_data->__unknown40);
		_dbgPrint("__unknown49:%p", mmd_data->__unknown49);
		_dbgPrint("__unknown50:%p", mmd_data->__unknown50);
		_dbgPrint("__unknown60:%p", mmd_data->__unknown60);
		_dbgPrint("__unknown70:%p", mmd_data->__unknown70);
		_dbgPrint("__unknown100:%p", mmd_data->__unknown100);
		_dbgPrint("__unknown101:%p", &mmd_data->__unknown101);
		_dbgPrint("__unknown103:%p", mmd_data->__unknown103);
		_dbgPrint("__unknown104:%p", mmd_data->__unknown104);
		_dbgPrint("__unknown105:%p", mmd_data->__unknown105);
		_dbgPrint("__unknown106:%p", mmd_data->__unknown106);
		_dbgPrint("__unknown109:%p", mmd_data->__unknown109);
		_dbgPrint("__unknown110:%p", mmd_data->__unknown110);
		_dbgPrint("__unknown120:%p", mmd_data->__unknown120);
	}
};

std::vector<BYTE> CUnknownChecker::m_mmdDataVec;

//////////////////////////////////////////////////////////////////////
class DbgTimer
{
#ifdef USE_DEBUG
	std::chrono::high_resolution_clock::time_point			m_timePoint;
	std::function<void(float)>								m_printFunc;
#endif // USE_DEBUG

public:
	DbgTimer(std::function<void(float)> func)
	{
#ifdef USE_DEBUG
		m_printFunc = func;
		m_timePoint = std::chrono::high_resolution_clock::now();
#endif // USE_DEBUG
	}

	~DbgTimer()
	{
#ifdef USE_DEBUG
		auto duration = std::chrono::high_resolution_clock::now() - m_timePoint;
		typedef std::chrono::duration<float, std::milli> millisecondsF;
		auto count = std::chrono::duration_cast< millisecondsF >(duration).count();

		m_printFunc(count);
#endif // USE_DEBUG
	}
};
