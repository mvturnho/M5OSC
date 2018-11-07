/*
 * theta.h
 *
 *  Created on: 3 jul. 2016
 *      Author: michi_000
 */

#ifndef THETA_H_
#define THETA_H_

#include <WiFiClient.h>
#include <HTTPClient.h>

//#include "utility/Config.h"

#define   HTTP_TIMEOUT_DISABLE  0 // never times out during transfer.
#define   HTTP_TIMEOUT_NORMAL   1
#define   HTTP_TIMEOUT_STATE    2
#define   HTTP_TIMEOUT_STARTCAP 5
#define   HTTP_TIMEOUT_STOPCAP  70
#define   HTTP_TIMEOUT_CMDSTAT  2

//--- Internal Info  ---
#define   TAKE_PIC_STAT_DONE  0
#define   TAKE_PIC_STAT_BUSY  1

#define   INT_EXP_OFF         0
#define   INT_EXP_ON          1

#define   PUSH_CMD_CNT_INTEXP 3

#define   INT_EXP_STAT_STOP   0
#define   INT_EXP_STAT_RUN    1

#define   MOVE_STAT_STOP      0
#define   MOVE_STAT_REC       1

#define   INT_EXP_MIN_SEC     8
#define   INT_EXP_MAX_SEC     3600

#define   INT_EXP_MIN_NUM     0
#define   INT_EXP_MAX_NUM     9999

#define   BEEP_VOL_MIN        0
#define   BEEP_VOL_MAX        100

#define   LIST_NUM_EV             13
#define   LIST_NUM_SS             55
#define   LIST_NUM_SS_SSMODE      30
#define   LIST_NUM_OPT            4
#define   LIST_NUM_ISO            13
#define   LIST_NUM_WB             10

class Theta {
private:
	String strSesId = "SID_0000";
	String strTakePicLastId = "0";
	String strCaptureMode = "";
	String strExpProg = "2";

	//--- THETA API URLs ---
	const String strUrlInfo = "/osc/info";
	const String strUrlState = "/osc/state";
	const String strUrlChkForUp = "/osc/checkForUpdates";
	const String strUrlCmdExe = "/osc/commands/execute";
	const String strUrlCmdStat = "/osc/commands/status";
	WiFiClient client;
	HTTPClient http;

public:
	String strHost = "192.168.1.1";
	int iHttpPort = 80;

	int iTakePicStat = TAKE_PIC_STAT_DONE;
	int iIntExpOnOff = INT_EXP_OFF;          //For expansion
	int iIntExpStat = INT_EXP_STAT_STOP;
	int iMoveStat = MOVE_STAT_STOP;

	int iCurEv = 6;
	int iCurISO = 0;
	int iCurSS = 14;
	int iCurWB = 0;
	int iCurOpt = 0;
	int iIntExpSec = INT_EXP_MIN_SEC;       // 8-3600
	int iIntExpNum = INT_EXP_MIN_NUM;       // 0:NoLimit or  2-9999
	int iBeepVol = BEEP_VOL_MAX;          // 0-100;

	int batteryState;
	int bateryLevel;
	int recordedTime;
	int recordableTime;

	String fwVersion;

#define   LIST_MAXSTRLEN_EXPPRG   2
#define   LIST_NUM_EXPPRG         4
	static char sList_ExpProg[LIST_NUM_EXPPRG][LIST_MAXSTRLEN_EXPPRG];
#define   LIST_MAXSTRLEN_CAPMODE  7
#define   LIST_NUM_CAPMODE        2
	static char sList_CaptureMode[LIST_NUM_CAPMODE][LIST_MAXSTRLEN_CAPMODE];
#define   LIST_MAXSTRLEN_EV       5
#define   LIST_NUM_EV             13
	static char sList_Ev[LIST_NUM_EV][LIST_MAXSTRLEN_EV];
#define   LIST_MAXSTRLEN_SS       11
#define   LIST_NUM_SS_SSMODE      30
#define   LIST_NUM_SS             55
	static char sList_SS[LIST_NUM_SS][LIST_MAXSTRLEN_SS];
	static char sList_SS_Disp[LIST_NUM_SS][LIST_MAXSTRLEN_SS];
#define   LIST_MAXSTRLEN_OPT      16
#define   LIST_NUM_OPT            4
	static char sList_Opt[LIST_NUM_OPT][LIST_MAXSTRLEN_OPT];
	static char sList_Opt_Disp[LIST_NUM_OPT][17];
#define   LIST_MAXSTRLEN_ISO      5
#define   LIST_NUM_ISO            13
	static char sList_ISO[LIST_NUM_ISO][LIST_MAXSTRLEN_ISO];
#define   LIST_MAXSTRLEN_WB       22
#define   LIST_NUM_WB             10
	static char sList_WB[LIST_NUM_WB][LIST_MAXSTRLEN_WB];
	static char sList_WB_Disp[LIST_NUM_WB][25];
	static char sList_BatState_Disp[3][11];

public:
	Theta(void);
	String getInfo(void);
	bool isBusy(void);
	void resetState(void);
	int postGetState(void);
	int postStartSession(void);
	int postTakePicture(void);
	int postStartCapture(void);
	int postStopCapture(void);
	int postCommandsStatus(void);
	int postGetFileList(void);
	int postGetOptions(void);
	int postSetOptionsCaptureMode(String psCapMode);
	int postSetOptionsExposureProgram(String psExpProg);
	int postSetOptionsExposureCompensation(void);
	int postSetOptionsShutterSpeed(void);
	int postSetOptionsWhiteBalance(void);
	int postSetOptionsFilter(void);
	int postSetOptionsCaptureInterval(void);
	int postSetOptionsCaptureNumber(void);
	int postSetOptionsShutterVolume(void);
	int postSetOptionsIso(void);
private:
	String httpProtocol(const String sPostGet, String sUrl, String sHost, int iPort, String strData);
	int resSetOptions(String strJson);
	void dumpJson(String label, String request, String response);

};

#endif /* THETA_H_ */
