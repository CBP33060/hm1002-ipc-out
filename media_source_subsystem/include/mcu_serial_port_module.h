#ifndef __MCU_SERIAL_PORT_MODULE_H__
#define __MCU_SERIAL_PORT_MODULE_H__
#include "module.h"
#include "b_queue.h"
#include "CSerialPort/SerialPort.h"
#include "stream.h"
#include "stream_hw.h"
#include "stream_link.h"
#include "stream_transfer.h"
#include "iot.pb.h"
#include "mcu_event_mange_remote_event.h"
#include "mcu_dev_spec_adapter.h"
#include "timer_mx.h"
namespace maix {
	typedef struct {
		uint64_t m_uTimeStamp;
		int		 m_iEventID;
		int		 m_iLen;
		unsigned char m_ucData[128];
	} T_SerialPortData;
	class CMcuDecSpecAdapter;

	class CMCUSerialPortModule : public CSerialPortListener, public CModule
	{
	public:
		CMCUSerialPortModule(std::string strGUID, std::string strName);
		~CMCUSerialPortModule();
		mxbool init();
		mxbool unInit();
		mxbool initServer();
		std::string remoteEventServerProc(
			std::string strEvent, std::string strParam);
		std::string procResult(std::string code,
			std::string strMsg, std::string strErr);
		void onReadEvent(const char *portName, unsigned int readBufferLen); 
		void onReadNoBufEvent(const char *portName, char * readBuffer, unsigned int readBufferLen);

		mxbool pushSendData(std::shared_ptr<T_SerialPortData> &data);
		void popSendData(std::shared_ptr<T_SerialPortData> &data);
		mxbool sendData(std::shared_ptr<T_SerialPortData> data);
		mxbool commSendData(char *data, unsigned int dataLen);
		int32_t uartSendCb(void *data, uint16_t len);
		mxbool commRecvSystemData(System_SystemID id, System *system);
		void commRecvIPCData(Ipc_IpcID id, Ipc *ipc);
		int32_t uartRecvCb(IotPacket *pkt);
		void run();
		mxbool initConnectModule();
		mxbool initMcuManageRemoteEvent();
		mxbool initDevSpecAdapter();

		std::string setMCUEnterLowPower(std::string strParam);
		std::string sendAesMessageToMCU(std::string strParam);
        std::string setLogManageOnlineState(std::string strParam);

		std::string setMcuLedSwitch(std::string strParam);
		std::string setMcuLedStatus(std::string strParam);
		std::string setMcuPirStatus(std::string strParam);
		std::string setSoundAndLightAlarm(std::string strParam);
		std::string sendSpeakerVoice(std::string strParam);
		std::string sendDevMsg(std::string strMsg);
		mxbool makeToReset();
		mxbool setAlarmLights(int ledState);

		std::string getBatteryChargingState();
		std::string getBatteryCapacity();

		int SendFactoryCmd(std::string strCommand);
		int FactoryReset();

		mxbool sendSpecDataVoiceLight();

	private:

        mxbool sendMessageToEvent(std::string strEventType,std::string value);
        mxbool sendMessageToDev(std::string strEventType,std::string value);
        mxbool sendMessageToOtherModule(std::string strGUID,std::string strServer,std::string strEventType,std::string value);
        mxbool playAudioFile(std::string fileId,int level,int playTime);
        mxbool stopAudioFile(std::string fileId);
        void forceEnterLowpower();
		void DelayLowPowerRun();
        std::string getPowerUpPlayEndState();

		CBQueue<std::shared_ptr<T_SerialPortData>> m_objSendQueue;
		std::shared_ptr<CMcuManageRemoteEvent> m_mcuManageRemoteEvent;
		std::shared_ptr<CMcuDecSpecAdapter> m_devSpecAdapter;
		std::string m_strPortName;
		CSerialPort m_cSerialPort;
		std::thread m_threadSendProc;
		KeepaliveConfig m_keepAliveConfig;
		char* m_pcRecvBuffer;
		int m_iRecvBufferLen;
		char m_acRecvLine[1024];
		char m_acRecvIndex;

        std::string m_strEventManageGUID;
        std::string m_strEventManageRemoteEventServer;
        std::string m_strDevManageGUID;
        std::string m_strDevManageServer;

		std::condition_variable m_conditionBatteryInfo;
		std::mutex m_mutexSendBatteryInfo;
		CTimer m_timeReset;

        std::thread m_threadForceEnterLowPower;
        int m_iForceLowpowerCount;

		std::thread m_threadDelayLowPower;

		mxbool m_keyPress;
		mxbool m_netWakeUp;

		int m_KeyCount;
		int m_NetCount;

		std::mutex m_mutexDelayLowPower;

		std::string m_strMcuVersion;
		int m_iCapacity;
		int m_iCurrent;
		int m_iVoltage;
		int m_iTemperature;
		int m_ichargeStatus;
		int m_iflag;
		int m_iaesFlag;

		int m_iledState;
	};
}
#endif //__MCU_SERIAL_PORT_MODULE_H__
