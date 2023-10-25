#include "ING_Mutex.h"

ING_Mutex::ING_Mutex(void)
{

}

ING_Mutex::~ING_Mutex(void)
{

}

void ING_Mutex::lock()
{
	mutex_.lock();
}
void ING_Mutex::unlock()
{
	mutex_.unlock();
}
