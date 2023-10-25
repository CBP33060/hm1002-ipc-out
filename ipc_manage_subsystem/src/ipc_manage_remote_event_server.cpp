#include "ipc_manage_remote_event_server.h"
#include "cJSON.h"
#include "crypt_api_mx.h"

namespace maix {
	CIPCManageRemoteEventServer::CIPCManageRemoteEventServer(
		CIPCManageModule * objIPCManageModule)
		: m_objIPCManageModule(objIPCManageModule)
	{
	}

	CIPCManageRemoteEventServer::~CIPCManageRemoteEventServer()
	{
	}

	mxbool CIPCManageRemoteEventServer::init()
	{
		return mxbool();
	}

	mxbool CIPCManageRemoteEventServer::unInit()
	{
		return mxbool();
	}

	std::string CIPCManageRemoteEventServer::eventProc(
		std::string strMsg)
	{
		std::string strResult;
		std::string strInput;
		unsigned char acKey[16] = { 0 };
		RCF::TransportType type =
			RCF::getCurrentRcfSession().getTransportType();
		if (type == RCF::Tt_Tcp ||
			type == RCF::Tt_Udp)
		{
			unsigned char pcDecryptData[strMsg.length() * 2] = {0};
			int iDecryptDataLen = 0;
			m_objIPCManageModule->getAES128Key(acKey);
			if (crypto_aes128_decrypt_base64(acKey,
				(unsigned char*)strMsg.c_str(),
				strMsg.length(),
				pcDecryptData, &iDecryptDataLen) != 0)
				return std::string("");

			strInput = std::string((char*)pcDecryptData, iDecryptDataLen);
		}
		else if (type == RCF::Tt_UnixNamedPipe ||
			type == RCF::Tt_Win32NamedPipe)
		{
			strInput = strMsg;
		}
		else
		{
			return std::string("");
		}

		std::string strEvent;
		cJSON *jsonRoot = cJSON_Parse(strInput.c_str());

		if (jsonRoot)
		{
			cJSON *jsonEvent = cJSON_GetObjectItem(jsonRoot, "event");
			if (jsonEvent)
			{
				strEvent = std::string(jsonEvent->valuestring);
			}
			cJSON_Delete(jsonRoot);
		}

		if (m_objIPCManageModule)
		{
			strResult = m_objIPCManageModule->remoteEventServerProc(
				strEvent, strInput);
		}
		else
		{
			strResult = procResult("500","",
				std::string("module no exist"));
		}
		
		if (type == RCF::Tt_Tcp ||
			type == RCF::Tt_Udp)
		{
			unsigned char pcEncryptData[2048] = { 0 };
			size_t iEncryptDataLen = 0;
			if (crypto_aes128_encrypt_base64(acKey,
				(unsigned char*)strResult.c_str(), strResult.length(),
				pcEncryptData,
				sizeof(pcEncryptData),
				&iEncryptDataLen
				) != 0)
				return  procResult("500", "",
					std::string("encrypt no exist"));

			return (char*)pcEncryptData;
		}
		else if (type == RCF::Tt_UnixNamedPipe ||
			type == RCF::Tt_Win32NamedPipe)
		{
			return strResult;
		}
		return procResult("500", "",
                    std::string("parse err"));
	}

	std::string CIPCManageRemoteEventServer::procResult(std::string code, 
		std::string strMsg, std::string strErr)
	{
		std::string strResult;
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonMsg = cJSON_Parse(strMsg.c_str());
		cJSON_AddStringToObject(jsonRoot, "code", code.c_str());
		cJSON_AddItemToObject(jsonRoot, "msg", jsonMsg);
		cJSON_AddStringToObject(jsonRoot, "errMsg", strErr.c_str());
		char *pcResult = cJSON_Print(jsonRoot);
		strResult = std::string(pcResult);
		cJSON_Delete(jsonRoot);
		if (pcResult)
			free(pcResult);

		return strResult;
	}
}