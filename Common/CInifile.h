#include "stdafx.h"
#include <map>
#include <list>
#include <string>
#include <functional>
#include <shlwapi.h>

#pragma once

//////////////////////////////////////////////////////////////////////
// キー入力を管理するクラス
class CInifile
{
	std::wstring							m_iniFile;
	std::map<std::wstring, std::wstring>	m_strTable;
	std::map<std::wstring, int32_t>			m_intTable;
	std::map<std::wstring, float>			m_floatTable;
public:
	CInifile() {}
	CInifile(std::wstring iniFile)
		: m_iniFile(iniFile)
	{
	}

	// セッター&ゲッター
	std::wstring &	Str  (const std::wstring &key)	{ return m_strTable[key]; }
	int32_t &		Int  (const std::wstring &key)	{ return m_intTable[key]; }
	float &			Float(const std::wstring &key)	{ return m_floatTable[key]; }


	void SetIniFile(std::wstring iniFile)
	{
		if (m_iniFile.empty()) { m_iniFile = iniFile; }
	}

	// Iniファイルを読み込む
	// デフォルト値をセットしたい場合は、引数に渡す関数内でセッターを使ってセットする
	void Read(std::function<void(CInifile &ini)> setDefault = nullptr)
	{
		// デフォルト値を設定したい場合はこの関数内でセットする
		if (setDefault) { setDefault(*this); }

		if (PathFileExists(m_iniFile.c_str()))
		{
			_readSection<std::wstring>(L"String", m_strTable,   [](const std::wstring &s) { return s; });
			_readSection<int32_t>     (L"Int",    m_intTable,   [](const std::wstring &s) { return std::stoi(s); });
			_readSection<float>       (L"Float",  m_floatTable, [](const std::wstring &s) { return std::stof(s); });
		}
		else
		{
			// iniファイルが存在していない場合はデフォルト値で書き込む
			Write();
		}
	}

	// Iniファイルに書き込む
	void Write()
	{
		for (auto &val : m_strTable)
		{
			WritePrivateProfileString(L"String", val.first.c_str(), val.second.c_str(), m_iniFile.c_str());
		}
		for (auto &val : m_intTable)
		{
			WritePrivateProfileString(L"Int", val.first.c_str(), std::to_wstring(val.second).c_str(), m_iniFile.c_str());
		}
		for (auto &val : m_floatTable)
		{
			WritePrivateProfileString(L"Float", val.first.c_str(), std::to_wstring(val.second).c_str(), m_iniFile.c_str());
		}
	}

private:
	// セクションの全データを読み込む
	template <class ValueType>
	void _readSection(	const std::wstring &section,
						std::map<std::wstring, ValueType> &table,
						std::function< const ValueType(const std::wstring &) > strToValue )
	{
		size_t					readSize = 0;
		std::vector<wchar_t>	buffer;

		// セクション内のすべてのキーと値を読み出す
		do
		{
			buffer.resize(buffer.size() + 1024);
			readSize = GetPrivateProfileSection(section.c_str(), &buffer[0], (DWORD)buffer.size(), m_iniFile.c_str());
		} while (readSize + 2 == buffer.size());

		if (readSize == 0) { return; }
		
		size_t					pos = 0;
		std::wstring			str;
		while (true)
		{
			str = &buffer[pos];
			// 終端が\0\0なので、0文字なら終了
			if (str.length() == 0) { break; }

			// とりあえず = は使用不可で1つのみ
			if (std::count(str.cbegin(), str.cend(), '=') == 1)
			{
				auto eqPos = str.find('=');
				if (eqPos != std::wstring::npos)
				{
					try
					{
						table[str.substr(0, eqPos)] = strToValue(str.substr(eqPos + 1));
					}
					catch (...) {}
				}
			}

			// 文字数+\0で次の文字列の先頭にくる
			pos += str.length() + 1;
		}
	}
};
