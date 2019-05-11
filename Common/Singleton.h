#include "stdafx.h"
#include <memory>

#pragma once

//////////////////////////////////////////////////////////////////////
// シングルトンテンプレート
template <class T>
class Singleton
{
public:
	static T* GetInstance()
	{
		static std::unique_ptr<T> obj(new T());

		return obj.get();
	}

protected:
	Singleton() {}

private:
	Singleton(const Singleton &) = delete;
	Singleton& operator=(const Singleton &) = delete;
	Singleton(Singleton &&) = delete;
	Singleton& operator=(Singleton &&) = delete;
};
