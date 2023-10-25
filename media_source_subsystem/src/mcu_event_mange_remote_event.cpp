#include "mcu_event_mange_remote_event.h"
#include "cJSON.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "log_mx.h"

namespace maix
{
	CMcuManageRemoteEvent::CMcuManageRemoteEvent(CModule *module)
		: m_module(module)
	{

	}

	CMcuManageRemoteEvent::~CMcuManageRemoteEvent()
	{
	}

	mxbool CMcuManageRemoteEvent::init()
	{
		if (!m_objMcuQueue.init(10, 0))
			return mxfalse;
        
        m_OutPutThread = std::thread([this](){
            this->run();
        });

		return mxtrue;
	}

	mxbool CMcuManageRemoteEvent::unInit()
	{
		return mxbool();
	}

	void CMcuManageRemoteEvent::run()
	{
		while (1)
		{
            std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT> outPut = NULL;
			popRecvData(outPut);
            if(!outPut)
            {
                continue;
            }
			cJSON *jsonRoot = cJSON_CreateObject();
            cJSON *jsonParam = cJSON_Parse(outPut->strJsonParam.c_str());

			cJSON_AddStringToObject(jsonRoot, "event", outPut->strEvent.c_str());
			cJSON_AddItemToObject(jsonRoot, "param", jsonParam);

			char *out = cJSON_Print(jsonRoot);
			std::string strAIEvent = std::string(out);
			logPrint(MX_LOG_INFOR,"strMcuEvent=[%s]\n", strAIEvent.c_str());
			cJSON_Delete(jsonRoot);
			if (out)
				free(out);

			std::string ret = m_module->output(outPut->strGUid, outPut->strServer, (unsigned char *)strAIEvent.c_str(),
								strAIEvent.length());
            logPrint(MX_LOG_INFOR,"ret=[%s]\n", ret.c_str());
            if((0 == outPut->strEvent.compare(OUT_PUT_EVENT_MANAGE_EVENT)) && (ret.length() <= 0))
            {
                logPrint(MX_LOG_INFOR,"event not online = [%s]\n", outPut->strJsonParam.c_str());
                pushRecvData(outPut);
                usleep(500 * 1000);
                continue;
            }
		}
	}

	mxbool CMcuManageRemoteEvent::pushRecvData(std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT> &outPutData)
	{
        // logPrint(MX_LOG_INFOR,"outPutData=[%s]\n", outPutData->strEvent.c_str());
		return m_objMcuQueue.push(outPutData);
	}

	void CMcuManageRemoteEvent::popRecvData(std::shared_ptr<T_MCU_MANAGE_RENOTE_OUTPUT> &outPutData)
	{
		m_objMcuQueue.pop(outPutData);
	}

}