#include "ING_Guard.h"

ING_Guard::ING_Guard(ING_Mutex &mutex)
:mutex_(mutex)
{
	mutex_.lock();
}

ING_Guard::~ING_Guard(void)
{
	mutex_.unlock();
}
