#pragma once

#include "INGCommon.h"

class ING_Mutex
{
public:
	ING_Mutex(void);
	~ING_Mutex(void);
	void lock();
	void unlock();
private:
	std::mutex mutex_;
};
