#include "media_frame_packet.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

namespace maix {

	CMediaFramePacket::CMediaFramePacket()
		: m_pcPacketData(NULL)
	{

	}

	CMediaFramePacket::~CMediaFramePacket()
	{
		if (m_pcPacketData)
		{
			free(m_pcPacketData);
			m_pcPacketData = NULL;
			//std::cout << "frame packet free" << std::endl;
		}
	}

	void CMediaFramePacket::setPacketType(E_P_TYPE ePacketType)
	{
		m_packetHeader.ePacketType = ePacketType;
	}

	E_P_TYPE CMediaFramePacket::getPacketType()
	{
		return m_packetHeader.ePacketType;
	}

	void CMediaFramePacket::setFrameType(E_F_TYPE eFrameType)
	{
		m_packetHeader.eFrameType = eFrameType;
	}

	E_F_TYPE CMediaFramePacket::getFrameType()
	{
		return m_packetHeader.eFrameType;
	}

    void CMediaFramePacket::setReserve_1(int32_t iReserve_1)
    {
        m_packetHeader.iReserve_1 = iReserve_1;
    }

    int32_t CMediaFramePacket::getReserve_1()
    {
        return m_packetHeader.iReserve_1;
    }     

    void CMediaFramePacket::setReserve_2(int32_t iReserve_2)
    {
        m_packetHeader.iReserve_2 = iReserve_2;
    }

    int32_t CMediaFramePacket::getReserve_2()
    {
        return m_packetHeader.iReserve_2;
    }

    void CMediaFramePacket::setReserve_3(int32_t iReserve_3)
    {
        m_packetHeader.iReserve_3 = iReserve_3;
    }

    int32_t CMediaFramePacket::getReserve_3()
    {
        return m_packetHeader.iReserve_3;
    }   

    void CMediaFramePacket::setReserve_4(int32_t iReserve_4)
    {
        m_packetHeader.iReserve_4 = iReserve_4;
    }

    int32_t CMediaFramePacket::getReserve_4()
    {
        return m_packetHeader.iReserve_4;
    }

    void CMediaFramePacket::setReserve_5(int32_t iReserve_5)
    {
         m_packetHeader.iReserve_5 = iReserve_5;
    }

    int32_t CMediaFramePacket::getReserve_5()
    {
        return m_packetHeader.iReserve_5;
    } 

	bool CMediaFramePacket::setFrameData(unsigned char * pcPacketData, 
		int nPacketSize, int64_t nTimeStamp, int32_t iFrameSeq)
	{
		if (nPacketSize <= 0)
			return false;

		m_packetHeader.nPacketSize = nPacketSize;
		m_packetHeader.lTimeStamp = nTimeStamp;
		m_packetHeader.iFrameSeq = iFrameSeq;

		m_pcPacketData = (unsigned char*)malloc(
				nPacketSize + sizeof(T_MediaFramePacketHeader));

		if (m_pcPacketData == NULL)
		{
			return false;
		}
		else
		{
			memcpy(m_pcPacketData, &m_packetHeader, 
				sizeof(T_MediaFramePacketHeader));
			memcpy(m_pcPacketData + sizeof(T_MediaFramePacketHeader), 
				pcPacketData, nPacketSize);
		}

		return true;
	}

	int64_t CMediaFramePacket::getFrameTimeStamp()
	{
		return m_packetHeader.lTimeStamp;
	}

	int32_t CMediaFramePacket::getFrameSeq()
	{
		return m_packetHeader.iFrameSeq;
	}

	unsigned char * CMediaFramePacket::getFrameData()
	{
		return m_pcPacketData;
	}

	int CMediaFramePacket::getFrameDataLen()
	{
		return m_packetHeader.nPacketSize + sizeof(T_MediaFramePacketHeader);
	}

	void CMediaFramePacket::setPacketMark(int iMark)
	{
		m_packetHeader.iMark = iMark;
	}

	int CMediaFramePacket::getPacketMark()
	{
		return m_packetHeader.iMark;
	}
}
