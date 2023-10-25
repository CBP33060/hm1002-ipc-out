#ifndef __CLI_AGENT_API_HPP__
#define __CLI_AGENT_API_HPP__
#include "cli/pch.h"
#include "cli/common.h"

void SysSHIPMode(const cli::OutputDevice& CLI_Out);
void BurnInStart(const cli::OutputDevice& CLI_Out,const char* time);
void BurnInStop(const cli::OutputDevice& CLI_Out);
void GetBurnInResult(const cli::OutputDevice& CLI_Out);
void FacReset(const cli::OutputDevice& CLI_Out);
void SetPID(const cli::OutputDevice& CLI_Out,const char* pid);
void GetPID(const cli::OutputDevice& CLI_Out);
void GenOOB(const cli::OutputDevice& CLI_Out);
void SetKey(const cli::OutputDevice& CLI_Out,const char* key);
void GetKey(const cli::OutputDevice& CLI_Out);
void SetDID(const cli::OutputDevice& CLI_Out,const char* did);
void GetDID(const cli::OutputDevice& CLI_Out);
void SetMAC(const cli::OutputDevice& CLI_Out,const char* mac);
void GetMAC(const cli::OutputDevice& CLI_Out);
void SetPSN(const cli::OutputDevice& CLI_Out,const char* psn);
void GetPSN(const cli::OutputDevice& CLI_Out);
void SetSN(const cli::OutputDevice& CLI_Out,const char* sn);
void GetSN(const cli::OutputDevice& CLI_Out);
void GetVersion(const cli::OutputDevice& CLI_Out);
void GetQRCode(const cli::OutputDevice& CLI_Out);
void MICTest(const cli::OutputDevice& CLI_Out);
void MICFileUpload(const cli::OutputDevice& CLI_Out);
void SPKTest(const cli::OutputDevice& CLI_Out);
void GetAlsValue(const cli::OutputDevice& CLI_Out);
void GetAlsRaw(const cli::OutputDevice& CLI_Out);
void GetAlsInterrupt(const cli::OutputDevice& CLI_Out);
void CalibrationAls(const cli::OutputDevice& CLI_Out);
void GravitySensorStart(const cli::OutputDevice& CLI_Out);
void Wled(const cli::OutputDevice& CLI_Out,int flag);
void Irled(const cli::OutputDevice& CLI_Out,int flag);
void GetPirTriggerNum(const cli::OutputDevice& CLI_Out);
void StartPirTriggerNum(const cli::OutputDevice& CLI_Out);
void GetPirSignal(const cli::OutputDevice& CLI_Out);
void RGBLEDSet(const cli::OutputDevice& CLI_Out,int index,int flag);
void IrCutSet(const cli::OutputDevice& CLI_Out,int flag);
void NightModeSet(const cli::OutputDevice& CLI_Out,int flag);
void GetBattery(const cli::OutputDevice& CLI_Out);
void ButtonClick(const cli::OutputDevice& CLI_Out);
void SaveInfo(const cli::OutputDevice& CLI_Out,const char* mkey,const char* mvalue);
void GetInfo(const cli::OutputDevice& CLI_Out,const char* mkey);
void MCUUartTest(const cli::OutputDevice& CLI_Out, const char* pcData);
void MCUFactoryTest(const cli::OutputDevice& CLI_Out, int iCmdCode);
void EnterSleepMode(const cli::OutputDevice& CLI_Out);
void BurnOtp(const cli::OutputDevice& CLI_Out);
void SolarTestStart(const cli::OutputDevice& CLI_Out);
void SolarTestGetResult(const cli::OutputDevice& CLI_Out);
void MCUPhotoresistancecConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUBatteryConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUADCConfig(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUInfraredConfig(const cli::OutputDevice& CLI_Out, int iCmdCode, int iPWM, int iFrequency);
void MCUTest(const cli::OutputDevice& CLI_Out, int iCmdCode);
void MCUUpgrade(const cli::OutputDevice& CLI_Out);
void MICSpkTest(const cli::OutputDevice& CLI_Out);
void MICSpkAec(const cli::OutputDevice& CLI_Out);
void WIFICMD(const cli::OutputDevice& CLI_Out,const char* chAT);
void GetButtonLevel(const cli::OutputDevice& CLI_Out);
void GetRxInfo(const cli::OutputDevice& CLI_Out);
void SaveConfig(const cli::OutputDevice& CLI_Out);
void GetMcuVersion(const cli::OutputDevice& CLI_Out);
void RGBLEDStart(const cli::OutputDevice& CLI_Out);
void RGBLEDStop(const cli::OutputDevice& CLI_Out);
#endif /* __CLI_AGENT_API_HPP__ */
