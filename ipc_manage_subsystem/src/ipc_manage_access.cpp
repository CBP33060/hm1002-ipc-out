#include "ipc_manage_access.h"
#include "ipc_manage_bind_state.h"
#include "common.h"
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/types.h>         
#include <sys/socket.h>
#endif
#include <time.h>
#include "cJSON.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "crypt_api_mx.h"
#include "fw_env_para.h"
#include "common.h"
#include "mbedtls/base64.h"

#ifdef TIME_DEBUG
#include <sys/time.h>
#endif

namespace maix {
	CIPCManageAccess::CIPCManageAccess(CIPCManageModule *objIPCManageModule)
		: m_objIPCManageModule(objIPCManageModule)
	{
		m_eAccessState = E_ACCESS_DISCONNECT;
		m_iRelayIndex = 0;
		m_iIPCIndex = 0;
		m_iEvent = 0;
        m_bBind = mxfalse;
		m_sockClient = -1;
	}

	CIPCManageAccess::~CIPCManageAccess()
	{
	}

	mxbool CIPCManageAccess::init()
	{
		m_strDID = getDID();
		m_strKEY = getPSK();
		m_strMAC = getMACWithColon();


#ifdef _INI_CONFIG
        INI::CINIFile iniVersionConfig; 
        try
        {
            iniVersionConfig.load(FW_VERSION_PATH);
            m_strFWVER = 
                iniVersionConfig["APP_FW_CONFIG"]["VERSION"].as<std::string>();
        }
        catch (std::invalid_argument &ia)
        {
            logPrint(MX_LOG_ERROR, "parse version file failed");
            return mxfalse;
        }
#endif
		logPrint(MX_LOG_DEBUG, "m_strFWVER: %s", m_strFWVER.c_str());

		char *info = getFWParaConfig(ENV_BIND_STATUS);
		if(info == NULL){
			m_bBind = mxfalse;
		}
		else
        {
            m_bBind = mxtrue;
		}
		logPrint(MX_LOG_DEBUG, "m_bBind: %d", m_bBind);

		memset(m_acAESKey, 0, sizeof(m_acAESKey));
		int ret = crypto_ecdh_gen_public(m_strDevPublicKey,
			m_strDevPrivateKey);
		if (ret != 0)
		{
			return mxfalse;
		}

		if (!m_objIPCManageModule->
			getConfig("COM_SERVER_CONFIG_0", "TYPE", m_iType))
		{
			return mxfalse;
		}
		else
		{
			if (m_iType == 5)
				m_iType = 1;
		}

		if (!m_objIPCManageModule->
			getConfig("COM_SERVER_CONFIG_0", "IP", m_strIP))
		{
			return mxfalse;
		}

		if (!m_objIPCManageModule->
			getConfig("COM_SERVER_CONFIG_0", "PORT", m_iPort))
		{
			return mxfalse;
		}

		if (!m_objIPCManageModule->
			getConfig("COM_SERVER_CONFIG_0", "UNIX", m_strUnix))
		{
			return mxfalse;
		}

		if (!m_objIPCManageModule->
			getConfig("COM_SERVER_CONFIG_0", "LEN", m_iLen))
		{
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageAccess::connectCenter()
	{
		int ret = -1;

#ifdef _WIN32
		WSADATA wsaData;
		DWORD dwRet = -1;
		if ((dwRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
		{
			WSACleanup();
			return mxfalse;
		}
#endif
		struct sockaddr_in server;
		memset(&server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(55000);

		//char* cValue = getFWParaConfig("BASE_STATION_IP");
		//if(cValue != NULL)
		//	server.sin_addr.s_addr = inet_addr(cValue); 
		//else
			server.sin_addr.s_addr = inet_addr(BASE_STATION_IP); 

		m_sockClient = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
		ret = connect(m_sockClient, (SOCKADDR*)&server, sizeof(SOCKADDR));

		int nTimeout = 1000;
		setsockopt(m_sockClient, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeout, sizeof(int));
		setsockopt(m_sockClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeout, sizeof(int));
#else

		// struct timeval tv;
		// tv.tv_sec = 1;
		// tv.tv_usec = 0;
		// setsockopt(m_sockClient, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
		// setsockopt(m_sockClient, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
		#ifdef TIME_DEBUG
			struct timeval tv; 
			uint64_t time_last;
			double time_ms;
			gettimeofday(&tv, NULL);
			time_last = tv.tv_sec*1000000 + tv.tv_usec;
		#endif

		ret = connect(m_sockClient, (struct sockaddr*)&server, sizeof(struct sockaddr));
		
		#ifdef TIME_DEBUG
			gettimeofday(&tv, NULL);
			time_last = tv.tv_sec*1000000 + tv.tv_usec - time_last;
			time_ms = time_last*1.0/1000;
			printf("load bin model time_ms: %fms\n", time_ms);
		#endif
#endif

		if (ret)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "connectCenter error: %s", strerror(m_connetErrno));
			return mxfalse;
		}
		else
		{
			m_iRelayIndex = 0;
			m_iIPCIndex = 0;
			logPrint(MX_LOG_DEBUG, "connectCenter success");
		}

		return mxtrue;
	}

	mxbool maix::CIPCManageAccess::disConnectCenter()
	{
#ifdef _WIN32
		closesocket(m_sockClient);
		WSACleanup();
#else
		if(m_sockClient > 0)
			close(m_sockClient);
		
		m_sockClient = -1;
#endif
		return mxtrue;
	}

	mxbool CIPCManageAccess::exchangePassword()
	{
		int iRecvLen = 0;
		char recvbuf[1024] = { 0 };
		if (!receiveMsg(recvbuf, sizeof(recvbuf), iRecvLen))
		{
			return mxfalse;
		}

		if (!gwAuthentication(recvbuf, iRecvLen))
		{
			return mxfalse;
		}

		if (!ipcAuthentication())
		{
			return mxfalse;
		}
	
		memset(recvbuf, 0, sizeof(recvbuf));
		if (!receiveMsg(recvbuf, sizeof(recvbuf), iRecvLen))
		{
			return mxfalse;
		}

		if (!gwAuthentication2(recvbuf, iRecvLen))
		{
			return mxfalse;
		}

		m_connetErrno = 0;
		return mxtrue;
	}

	mxbool CIPCManageAccess::sendConfig()
	{
		std::string strModel = std::string(getFWParaConfig("factory", "model"));
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "Config");
		cJSON_AddStringToObject(jsonRoot, "guid", m_strGUID.c_str());
		cJSON_AddNumberToObject(jsonRoot, "ipcIndex", ++m_iIPCIndex);
		cJSON_AddStringToObject(jsonParam, "did", m_strDID.c_str());
		cJSON_AddStringToObject(jsonParam, "key", m_strKEY.c_str());
		cJSON_AddStringToObject(jsonParam, "mac", m_strMAC.c_str());
		cJSON_AddStringToObject(jsonParam, "fwver", m_strFWVER.c_str());
		cJSON_AddStringToObject(jsonParam, "model", strModel.c_str());

		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
		
		if(!m_bBind)
		{
			m_iEvent = 1;
		}
		cJSON_AddNumberToObject(jsonParam, "event", m_iEvent);

		cJSON_AddNumberToObject(jsonParam, "type", m_iType);
		if ((m_strIP.compare("0.0.0.0") == 0) || (m_strIP.find("192.168.10.") != 0))
		{
			std::string strIP = m_strIP;
			getLocalIPByName(ETH0, strIP);
			cJSON_AddStringToObject(jsonParam, "ip", strIP.c_str());
		}
		else
		{
			cJSON_AddStringToObject(jsonParam, "ip", m_strIP.c_str());
		}

		cJSON_AddNumberToObject(jsonParam, "port", m_iPort);
		cJSON_AddStringToObject(jsonParam, "unix", m_strUnix.c_str());
		cJSON_AddNumberToObject(jsonParam, "len", m_iLen);

		char *out = cJSON_Print(jsonRoot);
		std::string strDevAuth = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);
		
		logPrint(MX_LOG_DEBUG, "send config: %s", out);
		unsigned char pcEncryptData[2048] = { 0 };
		size_t iEncryptDataLen = 0;
		if (crypto_aes128_encrypt_base64((unsigned char*)m_strRandomKey.c_str(),
						(unsigned char*)strDevAuth.c_str(), strDevAuth.length(),
						pcEncryptData,
						sizeof(pcEncryptData),
						&iEncryptDataLen) != 0)
		{	
			
			return mxfalse; 			
		}
							
		std::string strConfig(m_strGUID);
		strConfig.append("|");
		strConfig.append((char*)pcEncryptData, iEncryptDataLen);
	
		int ret = send(m_sockClient, strConfig.c_str(), strConfig.length(), 0);
		if (ret == -1)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
			std::cout << strerror(m_connetErrno) << std::endl;
#else
			m_connetErrno = errno;
#endif
			return mxfalse;
		}

		char recvbuf[1024] = { 0 };
		ret = recv(m_sockClient, recvbuf, sizeof(recvbuf), 0);
		if (ret == -1)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
			std::cout << strerror(m_connetErrno) << std::endl;
#else
			m_connetErrno = errno;
#endif
			return mxfalse;
		}

		char pcDecryptData[2048] = {0};
		int iDecryptDataLen = 0;
	
		if (crypto_aes128_decrypt_base64((unsigned char*)m_strKEY.c_str(),
				(unsigned char*)recvbuf,
				ret,
				(unsigned char*)pcDecryptData, &iDecryptDataLen) != 0)
			return mxfalse;

		std::string strEvent;
		std::string strGUID;
		unsigned int iRelayIndex = 0;
		std::string strCode;
		jsonRoot = cJSON_Parse((char*)pcDecryptData);
		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonGUID = cJSON_GetObjectItem(jsonRoot, "guid");
			if (jsonGUID)
			{
				strGUID = std::string(jsonGUID->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonRelayIndex = cJSON_GetObjectItem(jsonRoot, "relayIndex");
			if (jsonRelayIndex)
			{
				iRelayIndex = jsonRelayIndex->valueint;
				if (iRelayIndex != ++m_iRelayIndex)
				{
					return mxfalse;
				}
				else
				{
					//std::cout << "3---iRelayIndex: " << iRelayIndex << std::endl;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			if (strEvent.compare("Authentication3") != 0)
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

		
			
			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				cJSON *jsonCode = cJSON_GetObjectItem(jsonParam, "result");
				if (jsonCode)
				{
					strCode = std::string(jsonCode->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			cJSON_Delete(jsonRoot);
		}

		if (strCode.compare("200") != 0)
		{
			return mxfalse;
		}
		else
		{
			//std::cout << "send config success" <<std::endl;
		}
		
		m_connetErrno = 0;
		
		return mxtrue;
	}

	mxbool CIPCManageAccess::receiveMsg(char * pcbuf, int iLen, int &iRecvLen)
	{
		iRecvLen = 0;
		iRecvLen = recv(m_sockClient, pcbuf, iLen, 0);
		if (iRecvLen == -1)
		{

#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "receiveMsg error: %s", strerror(m_connetErrno));
			return mxfalse;
		}

		if(iRecvLen == 0)
			return mxfalse;

		logPrint(MX_LOG_DEBUG, "receiveMsg: iRecvLen: %d", iRecvLen);
		return mxtrue;
	}

	mxbool CIPCManageAccess::gwAuthentication(char * pcbuf, int iBufLen)
	{
		if (pcbuf == NULL)
			return mxfalse;

		unsigned char pcDecryptData[2048] = {0};
		int iDecryptDataLen = 0;
		unsigned int iRelayIndex = 0;
		if (crypto_aes128_decrypt_base64((unsigned char*)m_strKEY.c_str(),
				(unsigned char*)pcbuf,
				iBufLen,
				pcDecryptData, &iDecryptDataLen) != 0)
			return mxfalse;

		int ret = 0;
		std::string strEvent;
		cJSON *jsonRoot = cJSON_Parse((char*)pcDecryptData);
		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonGUID = cJSON_GetObjectItem(jsonRoot, "guid");
			if (jsonGUID)
			{
				m_strGUID = std::string(jsonGUID->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonRelayIndex = cJSON_GetObjectItem(jsonRoot, "relayIndex");
			if (jsonRelayIndex)
			{
				iRelayIndex = jsonRelayIndex->valueint;
				if((++m_iRelayIndex) !=  iRelayIndex)
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}
				else
				{
					//std::cout << "2---iRelayIndex: " << iRelayIndex << std::endl;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			
			if (strEvent.compare("Authentication") != 0)
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				cJSON *jsonVersion = cJSON_GetObjectItem(jsonParam, "version");
				if (jsonVersion)
				{
					m_strGWVersion = std::string(jsonVersion->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}

				cJSON *jsonPublicKey = cJSON_GetObjectItem(jsonParam, "key");
				if (jsonPublicKey)
				{
					m_strGWPublicKey = std::string(jsonPublicKey->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			cJSON_Delete(jsonRoot);
		}

		ret = crypto_ecdh_compute_shared(m_strGWPublicKey,
			m_strDevPrivateKey, m_acAESKey);
		if (ret != 0)
			return mxfalse;

		for (int i = 0; i < (int)sizeof(m_acAESKey); i++)
		{
			printf("%d ", m_acAESKey[i]);
		}
		printf("\n");
		m_objIPCManageModule->setAES128Key(m_acAESKey);
		return mxtrue;
	}

	mxbool CIPCManageAccess::ipcAuthentication()
	{
		int ret = 0;
		char acAESGWPublicKey[256] = { 0 };
		size_t iAESGWPublicKeyLen = 0;

		ret = crypto_aes128_encrypt_base64(m_acAESKey,
			(unsigned char*)m_strGWPublicKey.c_str(),
			m_strGWPublicKey.length(), 
			(unsigned char*)acAESGWPublicKey,
			sizeof(acAESGWPublicKey),
			&iAESGWPublicKeyLen);

		if (ret != 0)
			return mxfalse;

		std::string strEvent;
		cJSON * jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "Authentication");
		cJSON_AddStringToObject(jsonRoot, "guid", m_strGUID.c_str());
		cJSON_AddNumberToObject(jsonRoot, "ipcIndex", ++m_iIPCIndex);
		cJSON_AddStringToObject(jsonParam, "version", "0.0.0.1");
		cJSON_AddStringToObject(jsonParam, "key", m_strDevPublicKey.c_str());
		cJSON_AddStringToObject(jsonParam, "ekey", acAESGWPublicKey);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);
		
		char *out = cJSON_Print(jsonRoot);
		std::string strDevAuth = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		logPrint(MX_LOG_DEBUG, "ipcAuthentication send strDevAuth: %s", out);
		unsigned char pcEncryptData[2048] = { 0 };
		size_t iEncryptDataLen = 0;
		if (crypto_aes128_encrypt_base64((unsigned char*)m_strRandomKey.c_str(),
				(unsigned char*)strDevAuth.c_str(), strDevAuth.length(),
				pcEncryptData,
				sizeof(pcEncryptData),
				&iEncryptDataLen) != 0)
		{	
			
			return mxfalse;				
		}
					
		std::string strSendMsg(m_strGUID);
		strSendMsg.append("|");
		strSendMsg.append((char*)pcEncryptData, iEncryptDataLen);
		ret = send(m_sockClient, strSendMsg.c_str(), strSendMsg.length(), 0);

		if (ret == -1)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "ipcAuthentication error: %s", strerror(m_connetErrno));
			return mxfalse;
		}

		return mxtrue;
	}

	mxbool CIPCManageAccess::gwAuthentication2(char * pcbuf, int iBufLen)
	{
		int ret = 0;
		std::string strEvent;
		std::string strGUID;
		std::string strDevPublicKey;
		unsigned int iRelayIndex = 0;
		unsigned char pcDecryptData[2048] = {0};
		int iDecryptDataLen = 0;
	
		if (crypto_aes128_decrypt_base64((unsigned char*)m_strKEY.c_str(),
				(unsigned char*)pcbuf,
				iBufLen,
				pcDecryptData, &iDecryptDataLen) != 0)
			return mxfalse;

		cJSON *jsonRoot = cJSON_Parse((char*)pcDecryptData);
		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonGUID = cJSON_GetObjectItem(jsonRoot, "guid");
			if (jsonGUID)
			{
				strGUID = std::string(jsonGUID->valuestring);
				if (m_strGUID.compare(strGUID) != 0)
					return mxfalse;
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonRelayIndex = cJSON_GetObjectItem(jsonRoot, "relayIndex");
			if (jsonRelayIndex)
			{
				iRelayIndex = jsonRelayIndex->valueint;
				if((++m_iRelayIndex) !=  iRelayIndex)
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}
				else
				{
					//std::cout << "3----iRelayIndex: " << iRelayIndex << std::endl;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			//std::cout << "iRelayIndex: " << iRelayIndex << std::endl;

			if (strEvent.compare("Authentication2") != 0)
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				cJSON *jsonPublicKey = cJSON_GetObjectItem(jsonParam, "ekey");
				if (jsonPublicKey)
				{
					strDevPublicKey = std::string(jsonPublicKey->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}

			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			cJSON_Delete(jsonRoot);
		}

		unsigned char acDevPublicKey[256] = { 0 };
		int iDevPublicKeyLen = 0;

		ret = crypto_aes128_decrypt_base64(m_acAESKey,
			(unsigned char*)strDevPublicKey.c_str(),
			strDevPublicKey.length(),
			acDevPublicKey, &iDevPublicKeyLen);
		if (ret != 0)
		{
			return mxfalse;
		}

		if (m_strDevPublicKey.compare((char*)acDevPublicKey) != 0)
		{
			return mxfalse;
		}
		return mxtrue;
	}

	mxbool CIPCManageAccess::ipcDeviceAuth()
	{
		int iRecvLen = 0;
		char recvbuf[1024] = { 0 };

		if (!receiveMsg(recvbuf, sizeof(recvbuf), iRecvLen))
		{
			return mxfalse;
		}
		
		if (!sendIPCAuthInfo(recvbuf, iRecvLen))
		{
			return mxfalse;
		}

		if (!receiveMsg(recvbuf, sizeof(recvbuf), iRecvLen))
		{
			return mxfalse;
		}

		if (!gwAuthInfoCheck(recvbuf, iRecvLen))
		{
			return mxfalse;
		}

		m_connetErrno = 0;
		return mxtrue;
	}

	mxbool CIPCManageAccess::gwAuthInfoCheck(char* pcbuf, int iLen)
	{
		if (pcbuf == NULL)
			return mxfalse;

		unsigned char pcDecryptData[2048] = {0};
		int iDecryptDataLen = 0;
		if (crypto_aes128_decrypt_base64((unsigned char*)m_strRandomKey.c_str(),
				(unsigned char*)pcbuf,
				iLen,
				pcDecryptData, &iDecryptDataLen) != 0)
			return mxfalse;
		
		int ret = 0;
		std::string strEvent;
		std::string strGWAuthInfo;
		std::string strGWTimeStamp;
		unsigned int iRelayIndex = 0;
		cJSON *jsonRoot = cJSON_Parse((char*)pcDecryptData);
		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonGUID = cJSON_GetObjectItem(jsonRoot, "guid");
			if (jsonGUID)
			{
				m_strGUID = std::string(jsonGUID->valuestring);
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonRelayIndex = cJSON_GetObjectItem(jsonRoot, "relayIndex");
			if (jsonRelayIndex)
			{
				iRelayIndex = jsonRelayIndex->valueint;
				if((++m_iRelayIndex) !=  iRelayIndex)
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}
				else
				{
					//std::cout << "1---iRelayIndex: " << iRelayIndex << std::endl;
				}
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			
			if (strEvent.compare("GWDeviceAuth") != 0)
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}

			cJSON *jsonParam = cJSON_GetObjectItem(jsonRoot, "param");
			if (jsonParam)
			{
				cJSON *jsonAuthInfo = cJSON_GetObjectItem(jsonParam, "authinfo");
				if (jsonAuthInfo)
				{
					strGWAuthInfo = std::string(jsonAuthInfo->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}

				cJSON *jsonTimeStamp = cJSON_GetObjectItem(jsonParam, "timestamp");
				if (jsonTimeStamp)
				{
					strGWTimeStamp = std::string(jsonTimeStamp->valuestring);
				}
				else
				{
					cJSON_Delete(jsonRoot);
					return mxfalse;
				}

				// if( abs(atol(strGWTimeStamp.c_str()) - atol(m_strTimeStamp.c_str())) > 10 ) //10s time out
				// {
				// 	logPrint(MX_LOG_ERROR, "auth time out: %s %s", strGWTimeStamp.c_str(), m_strTimeStamp.c_str());
				// 	cJSON_Delete(jsonRoot);
				// 	return mxfalse;
				// }
			}
			else
			{
				cJSON_Delete(jsonRoot);
				return mxfalse;
			}
			cJSON_Delete(jsonRoot);
		}
		else
		{
			return mxfalse;
		}

		std::string strIPCKey = strGWAuthInfo.substr(8, 8) + strGWAuthInfo.substr(strGWAuthInfo.length()-15, 8);
		if (strIPCKey.compare(m_strKEY) != 0)//key error
		{
			logPrint(MX_LOG_DEBUG, "gwAuthInfoCheck key error, m_strKEY:%s, strIPCKey:%s", m_strKEY.c_str(), strIPCKey.c_str());
			return mxfalse;
		}
				
		strGWAuthInfo = strGWAuthInfo.substr(0, 8) + strGWAuthInfo.substr(16, strGWAuthInfo.length()-16-15) + strGWAuthInfo.substr(strGWAuthInfo.length()-7);
		//logPrint(MX_LOG_DEBUG, "gwAuthInfoCheck strGWAuthInfo: %s", strGWAuthInfo.c_str());

		std::string strAuthInfo = crypto_sha256_base64(strIPCKey.substr(0, 8) + strGWTimeStamp);
		//logPrint(MX_LOG_DEBUG, "gwAuthInfoCheck strAuthInfo: %s", strAuthInfo.c_str());

		if (strAuthInfo.compare(strGWAuthInfo) != 0)//auth failed
		{
			logPrint(MX_LOG_ERROR, "gwAuthInfoCheck auth failed!");
			return mxfalse;
		}

		jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "GWDeviceAuth");
		cJSON_AddStringToObject(jsonRoot, "guid", m_strGUID.c_str());
		cJSON_AddNumberToObject(jsonRoot, "ipcIndex", ++m_iIPCIndex);
		cJSON_AddStringToObject(jsonParam, "authinfo", "ok");
		cJSON_AddStringToObject(jsonParam, "timestamp", std::to_string(time(NULL)).c_str());
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strDevAuth = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		logPrint(MX_LOG_DEBUG, "gwAuthInfoCheck send strDevAuth: %s", strDevAuth.c_str());
		unsigned char pcEncryptData[2048] = { 0 };
		size_t iEncryptDataLen = 0;
		if (crypto_aes128_encrypt_base64((unsigned char*)m_strRandomKey.c_str(),
				(unsigned char*)strDevAuth.c_str(), strDevAuth.length(),
				pcEncryptData,
				sizeof(pcEncryptData),
				&iEncryptDataLen) != 0)
		{	
			
			return mxfalse;				
		}
					
		std::string strSendMsg(m_strGUID);
		strSendMsg.append("|");
		strSendMsg.append((char*)pcEncryptData, iEncryptDataLen);
		ret = send(m_sockClient, strSendMsg.c_str(), strSendMsg.length(), 0);
		
		if (ret == -1)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "gwAuthInfoCheck send error: %s", strerror(m_connetErrno));
			return mxfalse;
		}

		return mxtrue;
	}
	
	mxbool CIPCManageAccess::sendIPCAuthInfo(char * pcbuf, int iLen)
	{
		int ret = 0;
		
		if(iLen <=0 || pcbuf == NULL)
			return mxfalse;

		//std::cout << "recv key: " << pcbuf <<" len: " << iLen << std::endl;

		if(m_strRandomData.length() != 0 || iLen == 0 || pcbuf == NULL)
		{
			m_strRandomData=std::string();
			return mxfalse;
		}

		unsigned char base64_output_buffer[256] = {0};
		
		size_t base64_output_len = 0;
		ret = mbedtls_base64_decode(base64_output_buffer,
					sizeof(base64_output_buffer),
					&base64_output_len,
					(unsigned char*)pcbuf, iLen);

		if(ret != 0 || base64_output_len == 0)
			return mxfalse;
		
		m_strRandomData = std::string((char*)base64_output_buffer, base64_output_len);
		std::string::size_type pos = m_strRandomData.find("|");
		m_strRandomKey = m_strRandomData.substr(0, pos);
		m_strGUID = m_strRandomData.substr(pos+1, base64_output_len - pos);

		//std::cout << "randomkey: " << m_strRandomKey << std::endl;
		//std::cout <<"guid: " << m_strGUID << std::endl;

		
		m_strTimeStamp = std::to_string(time(NULL));
		std::string strAuthInfo = crypto_sha256_base64(m_strKEY.substr(8) + m_strTimeStamp);
		if(strAuthInfo.length() == 0)
			return mxfalse;

		//logPrint(MX_LOG_DEBUG, "ipcDeviceAuth before strAuthInfo: %s", strAuthInfo.c_str());

		strAuthInfo = strAuthInfo.substr(0,7) + m_strKEY.substr(0, 8) + 
				strAuthInfo.substr(7,strAuthInfo.length()-7-8) + m_strKEY.substr(8)  + 
				strAuthInfo.substr(strAuthInfo.length()-8);
		//logPrint(MX_LOG_DEBUG, "ipcDeviceAuth after strAuthInfo: %s", strAuthInfo.c_str());

		m_iIPCIndex = getRandomDigit(2);
		m_iRelayIndex = getRandomDigit(2);

		//std::cout << "m_iIPCIndex: " << m_iIPCIndex << std::endl;
		//std::cout << "m_iRelayIndex: " << m_iRelayIndex << std::endl;
		
		cJSON * jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();
		cJSON_AddStringToObject(jsonRoot, "event", "IPCDeviceAuth");
		cJSON_AddStringToObject(jsonRoot, "guid", m_strGUID.c_str());
		cJSON_AddNumberToObject(jsonRoot, "ipcIndex", m_iIPCIndex);
		cJSON_AddStringToObject(jsonParam, "authinfo", strAuthInfo.c_str());
		cJSON_AddStringToObject(jsonParam, "timestamp", m_strTimeStamp.c_str());
		cJSON_AddNumberToObject(jsonParam, "relayIndex", m_iRelayIndex);
		cJSON_AddNumberToObject(jsonParam, "ipcIndex", m_iIPCIndex);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strDevAuth = std::string(out);
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		logPrint(MX_LOG_DEBUG, "ipcDeviceAuth send strDevAuth: %s", strDevAuth.c_str());
		
		unsigned char pcEncryptData[2048] = { 0 };
		size_t iEncryptDataLen = 0;
		if (crypto_aes128_encrypt_base64((unsigned char*)m_strRandomKey.c_str(),
				(unsigned char*)strDevAuth.c_str(), strDevAuth.length(),
				pcEncryptData,
				sizeof(pcEncryptData),
				&iEncryptDataLen) != 0)
		{	
			
			return mxfalse;				
		}

		//std::cout << m_strGUID << "  " << (char*)pcEncryptData << std::endl;
				
		std::string strSendMsg(m_strGUID);
		strSendMsg.append("|");
		strSendMsg.append(std::string((char*)pcEncryptData, iEncryptDataLen));

		//std::cout << "strSendMsg: " << strSendMsg << std::endl;
		
		ret = send(m_sockClient, strSendMsg.c_str(), strSendMsg.length(), 0);
				
		if (ret == -1)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "ipcDeviceAuth error: %s", strerror(m_connetErrno));
			return mxfalse;
		}

		return mxtrue;
	}

	void CIPCManageAccess::run()
	{
		while (1)
		{
			switch (m_eAccessState)
			{
			case E_ACCESS_CONNECT:
			{
				if (ipcDeviceAuth())
				{
					m_eAccessState = E_ACCESS_AUTHENTICATION;
					continue;
				}
				else
				{
#ifdef _WIN32
					if (m_connetErrno == WSAECONNRESET ||
						m_connetErrno == WSAECONNABORTED)
					{
						disConnectCenter();
						m_eAccessState = E_ACCESS_DISCONNECT;
					}
#else
					disConnectCenter();
					m_eAccessState = E_ACCESS_DISCONNECT;
#endif
				}

				break;
			}
			case E_ACCESS_AUTHENTICATION:
			{
				if (exchangePassword())
				{
					m_eAccessState = E_ACCESS_EXCHANGE;
					continue;
				}
				else
				{
#ifdef _WIN32
					if (m_connetErrno == WSAECONNRESET ||
						m_connetErrno == WSAECONNABORTED ||
						m_connetErrno == WSAETIMEDOUT)
					{
						disConnectCenter();
						m_eAccessState = E_ACCESS_DISCONNECT;
					}
#else
					disConnectCenter();
					m_eAccessState = E_ACCESS_DISCONNECT;
#endif
				}

				break;
			}
			case E_ACCESS_EXCHANGE:
			{
				if (sendConfig())
				{
					m_eAccessState = E_ACCESS_SEND_CONFIG;
					continue;
				}
				else
				{
#ifdef _WIN32
					if (m_connetErrno == WSAECONNRESET ||
						m_connetErrno == WSAECONNABORTED)
					{
						disConnectCenter();
						m_eAccessState = E_ACCESS_DISCONNECT;
					}
#else
					disConnectCenter();
					m_eAccessState = E_ACCESS_DISCONNECT;
#endif
				}

				break;
			}
			case E_ACCESS_DISCONNECT:
			{
				if(!getLocalIPByName("wlan0", m_strIP) || !m_strIP.compare("192.168.43.1") || !m_strIP.compare("0.0.0.0"))
				{
					usleep(300 * 1000);
					continue;
				}
				
				if (connectCenter())
				{
					m_eAccessState = E_ACCESS_CONNECT;
					continue;
				}
				else
				{
					disConnectCenter();
					m_eAccessState = E_ACCESS_DISCONNECT;
				}
				break;
			}
			case E_ACCESS_SEND_CONFIG:
			{
				disConnectCenter();
				sleep(3);
				m_eAccessState = E_ACCESS_EXIT;
				break;
			}
            case E_ACCESS_EXIT:
            {
                break;
            }
			}
#ifdef _WIN32
			Sleep(3000);
#else
			sleep(3);
#endif
		}
	}

	E_ACCESS_STATE CIPCManageAccess::getAccessState()
	{
		return m_eAccessState;
	}

	void CIPCManageAccess::setAccessState(E_ACCESS_STATE state)
	{
		m_eAccessState = state;
	}
}

