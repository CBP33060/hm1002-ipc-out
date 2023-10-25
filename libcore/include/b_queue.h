#ifndef __B_QUEUE_H__
#define __B_QUEUE_H__
#include "type_def.h"
#include "global_export.h"
#include <time.h>
#include <queue>

namespace maix {
	template <class T>
	class MAIX_EXPORT CBQueue
	{
	public:
		mxbool init(int iSize, int iTimeOut);
		mxbool unInit();
		mxbool isEmpty();
		mxbool push(T &elem);
		void pop(T &elem);

	private:
		std::mutex m_mutexQueue;
		std::condition_variable m_conditionQueue;

		int m_iMaxSize;
		int  m_iTimeOut;
		std::queue<T> m_qQueue;
		mxbool m_bStop;
	};

	template<class T>
	mxbool CBQueue<T>::init(int iSize, int iTimeOut)
	{
		m_iMaxSize = iSize;
		m_iTimeOut = iTimeOut;
		m_bStop = mxfalse;
		return mxtrue;
	}

	template<class T>
	mxbool CBQueue<T>::unInit()
	{
		return mxfalse;
	}

	template<class T>
	inline mxbool CBQueue<T>::isEmpty()
	{
		mxbool ret;
		std::unique_lock<std::mutex> lock(m_mutexQueue);
		ret = m_qQueue.empty();
		return ret;
	}

	template<class T>
	inline mxbool CBQueue<T>::push(T & elem)
	{
		mxbool ret = mxfalse;

		std::unique_lock<std::mutex> lock(m_mutexQueue);
		if ((int)m_qQueue.size() >= (int)m_iMaxSize)
		{
			ret = mxfalse;
		}
		else
		{
			m_qQueue.push(elem);
			ret = mxtrue;
		}

		m_conditionQueue.notify_one();
		return ret;
	}

	template<class T>
	void CBQueue<T>::pop(T & elem)
	{
		if (m_iTimeOut)
		{
			std::unique_lock<std::mutex> lock(m_mutexQueue);
			if (m_qQueue.empty())
			{
				m_conditionQueue.wait_until(lock,
					std::chrono::steady_clock::now() +
					std::chrono::milliseconds(m_iTimeOut),
					[this] { return this->m_bStop || !m_qQueue.empty(); });
			}

			if (!m_qQueue.empty())
			{
				elem = m_qQueue.front();
				m_qQueue.pop();
			}
		}
		else
		{
			std::unique_lock<std::mutex> lock(m_mutexQueue);
			if (m_qQueue.empty())
			{
				m_conditionQueue.wait(lock,
					[this] { return this->m_bStop || !m_qQueue.empty(); });
			}

			if (!m_qQueue.empty())
			{
				elem = m_qQueue.front();
				m_qQueue.pop();
			}
		}
	}

}
#endif //__B_QUEUE_H__