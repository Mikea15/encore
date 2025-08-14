#pragma once

#include "core/core_minimal.h"

#define DECLARE_SINGLETON(ClassName) \
public: \
    static ClassName& GetInstance() \
    { \
        static ClassName instance; \
        return instance; \
    } \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete; \
private: \
    ClassName() = default; \
    ~ClassName() = default;

template<typename T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

protected:
	Singleton() = default;
	virtual ~Singleton() = default;

public:
	// Delete copy constructor and assignment
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;
};

