#include "stdafx.h"
#include <regex>

#pragma once

//////////////////////////////////////////////////////////////////////
// Csv入出力のユティリティクラス
class CCsv
{

public:
	CCsv() {}
	~CCsv() {}

	static inline const char*	DQuot	= R"(")";			// ダブルクォーテーション1つ
	static inline const char*	DDQuot	= R"("")";			// ダブルクォーテーション2つ
	static inline const char*	Comma		= ",";			// コンマ

	// ダブルクォーテーション1つを判定するための正規表現
	static inline std::regex _reg_DQuot{ CCsv::DQuot };
	// ダブルクォーテーション2つを判定するための正規表現
	static inline std::regex _reg_DDQuot{ CCsv::DDQuot };
	// 「,」以降に「"」が無い、または偶数個ある場合にマッチする正規表現式
	static inline std::regex _reg_CSV{ R"(,(?=(?:[^"]*"[^"]*")*[^"]*$))"};
	// 先頭と末尾に「"」があり、その途中に「,」含む場合にマッチする正規表現式
	static inline std::regex _reg_DQuotSand{ R"***(("+).*,.*("+))***" };
	
	// ダブルクォーテーションを付ける
	static std::string& AddDQuot(std::string &str)
	{
		// 既存の「"」を「""」にすべて置換
		if (str.find(DQuot) != std::string::npos)
		{
			str = std::regex_replace(str, _reg_DQuot, DDQuot);
		}

		// ,がある時は前後を"で挟む
		if (str.find(Comma) != std::string::npos)
		{
			str = DQuot + str + DQuot;
		}

		return str;
	}

	// ダブルクォーテーションを取り除く
	static std::string& RemoveDQuot(std::string &str)
	{
		std::smatch		results;
		// 前後に"があって、間に<があるか
		if (std::regex_match(str, results, _reg_DQuotSand))
		{
			// 前後の「"」の数がどちらも奇数個なら
			if ((results[1].str().length() & 1) == 1 &&
				(results[2].str().length() & 1) == 1)
			{
				str = str.substr(1, str.size() - 2);
			}
		}

		// 既存の「""」を「"」にすべて置換
		if (str.find(DDQuot) != std::string::npos)
		{
			str = std::regex_replace(str, _reg_DDQuot, DQuot);
		}

		return str;
	}
	
	// 文字列フォーマット関数
	template <typename... Args>
	static std::string Format(const std::string &fmt, Args... args)
	{
		size_t len = std::snprintf( nullptr, 0, fmt.c_str(), args... );
		std::vector<char> buf(len + 1);
		std::snprintf(&buf[0], len + 1, fmt.c_str(), args... );
		return std::string(&buf[0], &buf[0] + len);
	}
	
	// CSVの1行を分解する
	// 戻り値は分解した要素数 分解出来ない場合(コメント/空行等)は0が返る
	static size_t Split(std::string csv, std::vector<std::string> &result)
	{
		result.clear();

		// 行頭#はコメントとして nullptr を返す
		if (csv.empty() || csv[0] == '#')	{ return result.size(); }
		
		// Split
        std::copy
		(
            std::sregex_token_iterator{ csv.begin(), csv.end(), _reg_CSV, -1},
            std::sregex_token_iterator{},
            std::back_inserter(result)
		);

		return result.size();
	}
};
