#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "ipc_manage_bind_state.h"
#include "log_mx.h"
#include "cJSON.h"
#include "fw_env_para.h"
#ifdef WIN32
#include <windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

namespace maix {
	CIPCManageBindState::CIPCManageBindState(CIPCManageModule *objIPCManageModule, 
		std::string strMcuModuleGUID, std::string strMcuModuleRemoteServer)
		: m_objIPCManageModule(objIPCManageModule)
		, m_strMcuModuleGUID(strMcuModuleGUID)
		, m_strMcuModuleRemoteServer(strMcuModuleRemoteServer)
	{
		m_eBindState = E_BIND_DISCONNECT;
		m_bWait = mxfalse;
		m_sockClient = -1;
	}

	CIPCManageBindState::~CIPCManageBindState()
	{
	}

	mxbool CIPCManageBindState::init()
	{
		return mxtrue;
	}

	mxbool CIPCManageBindState::unInit()
	{
		disConnectOT();
		return mxtrue;
	}

	void CIPCManageBindState::run()
	{
        m_bRun = mxtrue;
		while (m_bRun)
		{
            if(m_eBindState != E_BIND_IDLE && m_eBindState != E_BIND_DISCONNECT && m_eBindState != E_BIND_TIMEOUT && m_eBindState != E_BIND_SUCCESS)	
            {
                mxbool bNeedHandle = false;
				if (getBindStateFromOT(m_strBindState))
				{
					if(m_strBindState == WIFI_AP_MODE)	
					{
                        if(m_eBindState == E_BIND_CONNECT || m_eBindState == E_BIND_WAITING)	
                        {
                            bNeedHandle = isHandleStep(E_BIND_WAITING);		
                        }
					}
					else if(m_strBindState == WIFI_CONNECTING)
					{
                        bNeedHandle = isHandleStep(E_BIND_CONNECTING);
					}
					else if(m_strBindState == WIFI_FAILED)
					{
                        bNeedHandle = isHandleStep(E_BIND_FAILED);
					}
                    else
                    {
                        usleep(200);
                        continue;
                    }
				}
                else
                {
                    if(m_objIPCManageModule)
                    {
                        m_objIPCManageModule->sendToIpcGetbindState();
                    }
                    sleep(3);
                    continue;
                }
                if(!bNeedHandle)
                {
#ifdef _WIN32
                    Sleep(1000);
#else
                    sleep(1);
#endif
                    continue;
                }
            }
            logPrint(MX_LOG_INFOR,"bind step = %d \n",m_eBindState);
			switch (m_eBindState)
			{
                case E_BIND_CONNECT:
                {
                    break;
                }
                case E_BIND_WAITING:
                {
                    if(!m_bWait)
                    {
                        m_threadWait = std::thread([this]() {
                            this->runWaiting();
                        });
                    }
                    break;
                }
                case E_BIND_CONNECTING:
                {
					//蓝色，闪烁 闪烁时间为：亮0.1s，灭0.4s
					sendLedStatus(LedInfo_Color_BLUE, LedInfo_State_FLASHING,1,4);
					sendVoicePrompts(PLAY_WIFI_CONNECTING); 
                    break;
                }
                case E_BIND_FAILED:
                {
					//（未超时）蓝色，闪烁 闪烁时间为：亮0.1s，灭0.4s
                    sendLedStatus(LedInfo_Color_BLUE, LedInfo_State_FLASHING,1,4);
                    sendVoicePrompts(PLAY_WIFI_CONNECT_ERR);  
                    m_eBindState = E_BIND_IDLE;
                    m_bRun = mxfalse;
                    break;
                }
                case E_BIND_TIMEOUT:
                {
					//橙色，常亮
                    sendLedStatus(LedInfo_Color_ORANGE, LedInfo_State_ON,0,0);
                    sendVoicePrompts(PLAY_WIFI_CONNECT_ERR);
                    m_eBindState = E_BIND_IDLE;
                    m_bRun = mxfalse;
                    break;
                }
                case E_BIND_SUCCESS:
                {
					//蓝色，亮20s后灯灭
                    sendLedStatus(LedInfo_Color_BLUE, LedInfo_State_ON,200,0);
                    sendVoicePrompts(PLAY_WIFI_CONNECT_SUCCESS);  
                    m_eBindState = E_BIND_IDLE;
                    m_bRun = mxfalse;
                    break;
                }
                case E_BIND_DISCONNECT:
                {
                    if (connectOT())
                    {
                        m_eBindState = E_BIND_CONNECT;
                        continue;
                    }
                    break;
                }
                case E_BIND_IDLE:
                {
					if (m_sockClient > 0)
					{
						disConnectOT();
					}
                    break;
                }
                default:
                    break;
			}

#ifdef _WIN32
					Sleep(1000);
#else
					sleep(1);
#endif
		}
	}

	void CIPCManageBindState::runWaiting()
    {
		int iCount = 0;
		m_bWait = mxtrue;

		setFWParaConfig(LOWPOWER_MODE,"0",1);
		saveFWParaConfig();

		while (1)
		{
			if(m_eBindState == E_BIND_WAITING)
			{
                //橙色，闪烁 闪烁时间为：亮0.1s，灭0.4s
                sendLedStatus(LedInfo_Color_ORANGE, LedInfo_State_FLASHING, 1, 4);
                sendVoicePrompts(PLAY_WAITING_CONNECT);
            }
			
			sleep(30);//循环播报"等待连接"，间隔30s

            if(m_eBindState == E_BIND_SUCCESS)
            {
                break;
            }
			if(++iCount == 6)
			{
				m_eBindState = E_BIND_TIMEOUT;
				system("touch /tmp/enter_shipmode");
				setFWParaConfig(LOWPOWER_MODE, "1", 1);
				saveFWParaConfig();
				return;
			}
        }

		m_bWait = mxfalse;
    }
	bool CIPCManageBindState::isHandleStep(E_BIND_STATE eState)
    {
        if(m_eBindState != eState)
        {
            m_eBindState = eState;
            return true;
        }
        return false;     
    }

	mxbool CIPCManageBindState::connectOT()
	{
		int ret = -1;
		if (m_sockClient > 0)
		{
			disConnectOT();
		}
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
		server.sin_port = htons(54322);
		server.sin_addr.s_addr = inet_addr("127.0.0.1");

		m_sockClient = socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
		int nTimeout = 1000;
		setsockopt(m_sockClient, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeout, sizeof(int));
		setsockopt(m_sockClient, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeout, sizeof(int));

		ret = connect(m_sockClient, (SOCKADDR*)&server, sizeof(SOCKADDR));
#else
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		setsockopt(m_sockClient, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
		setsockopt(m_sockClient, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

		ret = connect(m_sockClient, (struct sockaddr*)&server, sizeof(struct sockaddr));
#endif

		if (ret)
		{
#ifdef _WIN32
			m_connetErrno = WSAGetLastError();
#else
			m_connetErrno = errno;
#endif
			logPrint(MX_LOG_ERROR, "connectOT error: %s", strerror(m_connetErrno));
			return mxfalse;
		}
		else
		{
			logPrint(MX_LOG_DEBUG, "connectOT success");
		}

		return mxtrue;
	}

	mxbool CIPCManageBindState::disConnectOT()
	{
#ifdef _WIN32
		closesocket(m_sockClient);
		WSACleanup();
#else
		close(m_sockClient);
#endif
		m_sockClient = -1;
		logPrint(MX_LOG_DEBUG, "diconnect ot success");

		return mxtrue;
	}

	mxbool CIPCManageBindState::getBindStateFromOT(std::string &strBindState)
	{
		std::string strCommond = "{\"method\":\"local.query_status\",\"params\":\"\",\"id\":123456}";

		int ret = send(m_sockClient, strCommond.c_str(), strCommond.length(), 0);
		if (ret == -1)
		{
			m_connetErrno = errno;
			logPrint(MX_LOG_ERROR, "send to OT error: %s", strerror(m_connetErrno));
			return mxfalse;
		}

		char recvbuf[1024] = { 0 };
		ret = recv(m_sockClient, recvbuf, sizeof(recvbuf), 0);
		if (ret == -1)
		{
			m_connetErrno = errno;
			logPrint(MX_LOG_ERROR, "recv from OT error: %s", strerror(m_connetErrno));
			return mxfalse;
		}
		//{"id":12345,"method":"local.status","params":"cloud_connected"}
		logPrint(MX_LOG_DEBUG, "recv from OT: %s", recvbuf);
		if (strstr(recvbuf, "wifi_ap_mode"))
			strBindState = WIFI_AP_MODE;

		if (strstr(recvbuf, "wifi_connecting"))
			strBindState = WIFI_CONNECTING;

		if (strstr(recvbuf, "wifi_failed"))
			strBindState = WIFI_FAILED;

		logPrint(MX_LOG_DEBUG, "strBindState: %s", strBindState.c_str());
		return mxtrue;
	}
	
	/**
     * @brief 
     * 
     * @param iLedColor     E_COLOR_RED:0,E_COLOR_GREEN:1,E_COLOR_BLUE:2,E_COLOR_ORANGE:3
     * @param iLevel        E_LED_SHOW_LEVEL
     * @param iLedState     E_LED_STATE 0:点亮，1:闪烁，2:熄灭
     * @param showTime      led亮灯时长,单位s， <=0时则认为不定时，一直展示
     * @return mxbool 
     */
	mxbool CIPCManageBindState::sendLedStatus(int iColor, int iState, int iOnTime, int iOffTime)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "SetLedStatus");
		cJSON_AddNumberToObject(jsonParam, "led_Color", iColor);
		cJSON_AddNumberToObject(jsonParam, "led_State", iState);
		cJSON_AddNumberToObject(jsonParam, "on_time_ms", iOnTime);
		cJSON_AddNumberToObject(jsonParam, "off_time_ms", iOffTime);
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strLedStatus = std::string(out);
		logPrint(MX_LOG_DEBUG, "sendLedStatus strLedStatus=[%s]", strLedStatus.c_str());
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		std::string strResult = m_objIPCManageModule->output(m_strMcuModuleGUID,
				m_strMcuModuleRemoteServer,
				(unsigned char*)strLedStatus.c_str(), strLedStatus.length());
		logPrint(MX_LOG_DEBUG, "sendLedStatus strResult=[%s]", strResult.c_str());

		return mxtrue;
	}

	mxbool CIPCManageBindState::sendVoicePrompts(std::string strVoiceType)
	{
		cJSON *jsonRoot = cJSON_CreateObject();
		cJSON *jsonParam = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "event", "SendSpeakerBroadcast");
		cJSON_AddStringToObject(jsonParam, "command", strVoiceType.c_str());
		cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

		char *out = cJSON_Print(jsonRoot);
		std::string strVoice = std::string(out);
		logPrint(MX_LOG_DEBUG, "sendVoicePrompts strVoice=[%s]", strVoice.c_str());
		cJSON_Delete(jsonRoot);
		if (out)
			free(out);

		std::string strResult = m_objIPCManageModule->output(m_strMcuModuleGUID,
				m_strMcuModuleRemoteServer,
				(unsigned char*)strVoice.c_str(), strVoice.length());
		logPrint(MX_LOG_DEBUG, "sendVoicePrompts strResult=[%s]", strResult.c_str());

		return mxtrue;
	}

	mxbool CIPCManageBindState::setBindState(E_BIND_STATE bindState)
	{

		m_eBindState = bindState;
        logPrint(MX_LOG_INFOR, "ssetBindState=[%d]", bindState);
		if(bindState == E_BIND_SUCCESS)
		{
			setFWParaConfig(ENV_BIND_STATUS, "ok", 1);
			sleep(10);
			setFWParaConfig(LOWPOWER_MODE, "1", 1);
			saveFWParaConfig();
			system("rm /tmp/enter_shipmode");
		}
		else if (bindState == E_BIND_DISCONNECT)
		{
			unsetFWParaConfig(ENV_BIND_STATUS);
			setFWParaConfig(LOWPOWER_MODE, "0", 1);
			saveFWParaConfig();
		}
		return mxtrue;
	}

	mxbool CIPCManageBindState::getBindState()
	{
		if(NULL != getFWParaConfig(ENV_BIND_STATUS))
		{
			return mxtrue;
		}
		return mxfalse;
	}

}
