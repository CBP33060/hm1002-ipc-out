#pragma once
#include "ING_Mutex.h"
class ING_Guard
{
public:
	ING_Guard(ING_Mutex &mutex);
	~ING_Guard(void);
private:
	ING_Mutex& mutex_;
};
