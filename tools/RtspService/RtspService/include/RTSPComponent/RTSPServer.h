#ifndef __CR_RTSPSERVER_H__
#define __CR_RTSPSERVER_H__

#include "RTSPCommon.h"
#include "RTSPObj.h"
#include "RTSPSession.h"
#include "RTSPSocket.h"

#include <vector>

class RTSPServer
{
public:
	typedef std::vector<RTSPSession*> CRSESSIONARR;

	RTSPServer(unsigned short nListenPort, unsigned short nMediaPortMin, unsigned short nMediaPortMax, int nSetTrackNumTimeOutInterval,
		int nSDPTimeOutInterval);

	~RTSPServer();

	int Run();

	SESSION_HANDLE GetNewSession();
	unsigned short GetPort();

	int DeleteSession(SESSION_HANDLE hSession);

protected:

	int Accept();

protected:
	SOCKET server_;
	bool bSockFlag_;

	unsigned short nMediaPortMin_;
	unsigned short nMediaPortMax_;

	int nSetTrackNumTimeOutInterval_;
	int nSDPTimeOutInterval_;

	CRSESSIONARR ready_session_;
	CRSESSIONARR established_session_;
};

#endif/*__CR_RTSPSERVER_H__*/

