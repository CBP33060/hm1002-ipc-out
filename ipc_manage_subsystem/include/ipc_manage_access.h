#ifndef __IPC_MANAGE_ACCESS_H__
#define __IPC_MANAGE_ACCESS_H__
#include "ipc_manage_module.h"
#ifdef _WIN32
#include <windows.h>
#include <time.h>
#include <Ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace maix {
	enum E_ACCESS_STATE
	{
		E_ACCESS_CONNECT,
		E_ACCESS_AUTHENTICATION,
		E_ACCESS_EXCHANGE,
		E_ACCESS_DISCONNECT,
		E_ACCESS_SEND_CONFIG,
		E_ACCESS_EXIT,
	};

	class CIPCManageModule;

	class CIPCManageAccess
	{
	public:
		CIPCManageAccess(CIPCManageModule *objIPCManageModule);
		~CIPCManageAccess();
		mxbool init();
		mxbool connectCenter();
		mxbool disConnectCenter();
		mxbool ipcDeviceAuth();
		mxbool sendIPCAuthInfo(char * pcbuf, int iLen);
		mxbool gwAuthInfoCheck(char* pcbuf, int iLen);

		mxbool exchangePassword();
		mxbool sendConfig();
		mxbool receiveMsg(char* pcbuf, int iLen, int &iRecvLen);
		mxbool gwAuthentication(char* pcbuf, int iBufLen);
		mxbool ipcAuthentication();
		mxbool gwAuthentication2(char* pcbuf, int iBufLen);
		void setAccessState(E_ACCESS_STATE state);
		E_ACCESS_STATE getAccessState();
		void run();
	private:
		CIPCManageModule* m_objIPCManageModule;
#ifdef _WIN32
		SOCKET m_sockClient;
#else
		int m_sockClient;
#endif
		int m_connetErrno;

		E_ACCESS_STATE m_eAccessState;
		std::string m_strDID;
		std::string m_strKEY;
		std::string m_strMAC;
		std::string m_strFWVER;
		mxbool m_bBind;
		std::string m_strDevPublicKey;
		std::string m_strDevPrivateKey;
		std::string m_strGUID;
		std::string m_strTimeStamp;
		int m_iEvent;
		int m_iType;
		std::string m_strIP;
		int m_iPort;
		std::string m_strUnix;
		int m_iLen;

		std::string m_strGWVersion;
		std::string m_strGWPublicKey;
		unsigned char m_acAESKey[16];
		unsigned int  m_iRelayIndex;
		unsigned int  m_iIPCIndex;
		std::string m_strRandomData;
		std::string m_strRandomKey;
	};
}
#endif //__IPC_MANAGE_ACCESS_H__
