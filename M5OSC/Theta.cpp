/*
 * Theta.h
 *
 *  Created on: 18 apr. 2016
 *      Author: m.vanturnhout
 */

#ifndef THETA_CPP_
#define THETA_CPP_

#include <WiFi.h>
#include <ArduinoJson.h>

#include "Theta.h"
#include "M5OSC.h"

//--- THETA API param lists ---

//#define   LIST_MAXSTRLEN_CAPMODE  7
//#define   LIST_NUM_CAPMODE        2
char Theta::sList_CaptureMode[LIST_NUM_CAPMODE][LIST_MAXSTRLEN_CAPMODE] = { "image", "_video" };

//#define   LIST_MAXSTRLEN_EXPPRG   2
//#define   LIST_NUM_EXPPRG         4
char Theta::sList_ExpProg[LIST_NUM_EXPPRG][LIST_MAXSTRLEN_EXPPRG] = { "2", // AUTO : Normal program
		"4",  // SS   : Shutter priority
		"9",  // ISO  : ISO priority
		"1"   // MANU : Manual
		};

//#define   LIST_MAXSTRLEN_EV       5
//#define   LIST_NUM_EV             13
char Theta::sList_Ev[LIST_NUM_EV][LIST_MAXSTRLEN_EV] = { "-2.0", "-1.7", "-1.3", "-1.0", "-0.7", "-0.3", "0.0", "0.3", "0.7", "1.0", "1.3", "1.7",
		"2.0" };

//#define   LIST_MAXSTRLEN_ISO      5
//#define   LIST_NUM_ISO            13
char Theta::sList_ISO[LIST_NUM_ISO][LIST_MAXSTRLEN_ISO] = { "100", "125", "160", "200", "250", "320", "400", "500", "640", "800", "1000", "1250",
		"1600" };

//#define   LIST_MAXSTRLEN_SS       11
//#define   LIST_NUM_SS_SSMODE      30
//#define   LIST_NUM_SS             55
char Theta::sList_SS[LIST_NUM_SS][LIST_MAXSTRLEN_SS] = { "0.00015625", "0.0002", "0.00025", "0.0003125", "0.0004", "0.0005", "0.000625", "0.0008",
		"0.001", "0.00125", "0.0015625", "0.002", "0.0025", "0.003125", "0.004", "0.005", "0.00625", "0.008", "0.01", "0.0125", "0.01666666", "0.02",
		"0.025", "0.03333333", "0.04", "0.05", "0.06666666", "0.07692307", "0.1", "0.125", "0.16666666", "0.2", "0.25", "0.33333333", "0.4", "0.5",
		"0.625", "0.76923076", "1", "1.3", "1.6", "2", "2.5", "3.2", "4", "5", "6", "8", "10", "13", "15", "20", "25", "30", "60" };
char Theta::sList_SS_Disp[LIST_NUM_SS][LIST_MAXSTRLEN_SS] = { "6400", "5000", "4000", "3200", "2500", "2000", "1600", "1250", "1000", "800 ", "640 ",
		"500 ", "400 ", "320 ", "250 ", "200 ", "160 ", "125 ", "100 ", "80  ", "60  ", "50  ", "40  ", "30  ", "25  ", "20  ", "15  ", "13  ",
		"10  ", "8   ", "6   ", "5   ", "4   ", "3   ", "2.5 ", "2   ", "1.6 ", "1.3 ", "1\"  ", "1.3\"", "1.6\"", "2\"  ", "2.5\"", "3.2\"", "4\"  ",
		"5\"  ", "6\"  ", "8\"  ", "10\" ", "13\" ", "15\" ", "20\" ", "25\" ", "30\" ", "60\" " };

//#define   LIST_MAXSTRLEN_WB       22
//#define   LIST_NUM_WB             10
char Theta::sList_WB[LIST_NUM_WB][LIST_MAXSTRLEN_WB] = { "auto", "daylight", "shade", "cloudy-daylight", "incandescent", "_warmWhiteFluorescent",
		"_dayLightFluorescent", "_dayWhiteFluorescent", "fluorescent", "_bulbFluorescent" };
char Theta::sList_WB_Disp[LIST_NUM_WB][25] = { "Auto", "Outdoor", "Shade", "Cloudy", "Incandescent", "Warmw hite Fluorescent", "Daylight Fluorescent",
		"natural whiteFluorescent", "Fluorescent", "Bulb Fluorescent" };

#define   LIST_MAXSTRLEN_OPT      16
#define   LIST_NUM_OPT            4
char Theta::sList_Opt[LIST_NUM_OPT][LIST_MAXSTRLEN_OPT] = { "off", "DR Comp", "Noise Reduction", "hdr" };
char Theta::sList_Opt_Disp[LIST_NUM_OPT][17] = { "Off", "DR Compensation", "Noise Reduction ", "HDR Rendering" };

char Theta::sList_BatState_Disp[3][11] = { "charge", "charged", "discon" };

Theta::Theta(void) {
	recordedTime = 0;
	recordableTime = 0;
	bateryLevel = 0;
	batteryState = 2;
}

void Theta::dumpJson(String label, String request, String response) {
#ifdef DUMPJSON
	Serial.println(label + " - REQUEST\n" + request);
	Serial.println(label + " - RESPONSE\n" + response);
#endif
}

String Theta::httpProtocol(const String sPostGet, String sUrl, String sHost, int iPort, String strData) {
	int iHttpCode = 0;
	String strPayload = "";

	http.begin(sHost, iPort, sUrl);
	http.addHeader("Content-Type", "application/json;charset=utf-8");
	http.addHeader("Accept", "application/json");
	http.addHeader("Connection", "close");
	http.addHeader("Content-Length", String(strData.length()), false, true);

	if (sPostGet.equals("GET"))
		iHttpCode = http.GET();
	else
		iHttpCode = http.POST(strData);

	// httpCode will be negative on error
	if (iHttpCode > 0) {
		// HTTP header has been send and Server response header has been handled
		//Serial.printf("[HTTP] GET... code: %d\n", httpCode);
		// file found at server
		if (iHttpCode == HTTP_CODE_OK) {
			strPayload = http.getString();
			//Serial.println(payload);
		}
	} else {
		Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(iHttpCode).c_str());
	}

	http.end();
	//Serial.println("httpResponse: " + payload);
	return strPayload;
}

bool Theta::isBusy() {
	return iTakePicStat == TAKE_PIC_STAT_BUSY;
}

void Theta::resetState(){
	iTakePicStat = TAKE_PIC_STAT_DONE;
}
//-------------------------------------------
// THETA API functions
//-------------------------------------------
String Theta::getInfo(void) {
	int iRet = 0;

	String strSendData = String("");
	String strJson = httpProtocol("GET", strUrlInfo, strHost, iHttpPort, strSendData);
	dumpJson("getInfo", strSendData, strJson);

	//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );
	//Serial.println("Theta::getInfo Host: "+strHost);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<350> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("Theta.getInfo() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sModel = root["model"];
			const char* sSN = root["serialNumber"];
			const char* sFwVer = root["firmwareVersion"];
			fwVersion = String(sFwVer);
#ifdef DEBUG
			Serial.println(
					"ThetaAPI_Get_Info() : Model[" + String(sModel) + "], S/N["
					+ String(sSN) + "], FW Ver[" + String(sFwVer)
					+ "]");
#endif
			iRet = 1;
		}
	}
	return strJson;
}

//----------------
int Theta::postGetState(void) {
	int iRet = 0;

	String strSendData = String("");
	String strJson = httpProtocol("POST", strUrlState, strHost, iHttpPort, strSendData);
	dumpJson("getState", strSendData, strJson);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<350> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("theta.postGetState() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sessionId = root["state"]["sessionId"];
			const char* batteryLevel = root["state"]["batteryLevel"];
			const char* _captureStatus = root["state"]["_captureStatus"];
			const char* _recordedTime = root["state"]["_recordedTime"];
			const char* _recordableTime = root["state"]["_recordableTime"];
			const char* _batteryState = root["state"]["_batteryState"];
#ifdef DEBUG
			Serial.println(
					"post_State() : sessionId[" + String(sessionId)
					+ "], batteryLevel[" + String(batteryLevel)
					+ "], _captureStatus[" + String(_captureStatus)
					+ "], _recordedTime[" + String(_recordedTime)
					+ "], _recordableTime[" + String(_recordableTime)
					+ "], _batteryState[" + String(_batteryState)
					+ "]");
#endif
			strSesId = String(sessionId);

			String strCaptureStatus = String(_captureStatus);
			recordedTime = atoi(_recordedTime);
			recordableTime = atoi(_recordableTime);
			bateryLevel = (int) (atof(batteryLevel) * 100);
			String strBatteryState = String(_batteryState);
			if (strBatteryState.equals("charging"))
				batteryState = 0;
			else if (strBatteryState.equals("charged"))
				batteryState = 1;
			else
				batteryState = 3;

			if (strCaptureStatus.equals("idle")) {
				iMoveStat = MOVE_STAT_STOP;
				iIntExpStat = INT_EXP_STAT_STOP;
			} else {
				if ((recordedTime == 0) && (recordableTime == 0)) {
					iMoveStat = MOVE_STAT_STOP;
					iIntExpOnOff = INT_EXP_ON;
					iIntExpStat = INT_EXP_STAT_RUN;
				} else {
					iMoveStat = MOVE_STAT_REC;
					iIntExpStat = INT_EXP_STAT_STOP;
				}
			}
			iRet = 1;
		}
	}
	return iRet;
}
//----------------
int Theta::postStartSession(void) {
	int iRet = 0;

	String strSendData = String("{\"name\": \"camera.startSession\",  \"parameters\": {\"timeout\": 180}}");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);

	dumpJson("startSession", strSendData, strJson);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("postStartSession() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sSessionId = root["results"]["sessionId"];
			strSesId = String(sSessionId);
#ifdef DEBUG
			Serial.println(
					"postStartSession() : sessionId[" + strSesId
					+ "]");
#endif
			iRet = 1;
		}
	}
	return iRet;
}

int Theta::postGetFileList(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.listFiles\", \"parameters\": { \"fileType\": \"all\",\"entryCount\": \"3\" ,\"maxThumbSize\": \"0\"} }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
	dumpJson("getFileList", strSendData, strJson);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("postGetFiles() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG
			Serial.print(
					"postGetFiles() : state[" + strState + "], ");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println("Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iRet = -1;
			} else {  //inProgress

				iRet = 1;
			}
		}
	}
	return iRet;
}

//----------------
int Theta::postTakePicture(void) {
	int iRet = 0;

	iTakePicStat = TAKE_PIC_STAT_BUSY;

	String strSendData = String("{\"name\": \"camera.takePicture\", \"parameters\": { \"sessionId\": \"" + strSesId + "\" } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
	dumpJson("takePicture", strSendData, strJson);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("postTakePicture() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG
			Serial.print(
					"postTakePicture() : state[" + strState + "], ");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println("Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			} else {  //inProgress
				const char* sId = root["id"];
				strTakePicLastId = String(sId);
#ifdef DEBUG
				Serial.println("id[" + strTakePicLastId + "]");
#endif
				iRet = 1;
			}
		}
	}
	return iRet;
}
//----------------
int Theta::postStartCapture(void) {
	int iRet = 0;

	String strSendData = String("{\"name\": \"camera._startCapture\", \"parameters\": { \"sessionId\": \"" + strSesId + "\" } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("postStartCapture() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG

			Serial.print(
					"postStartCapture() : state[" + strState + "]");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println(", Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			} else {  //done
				Serial.println("");
				iRet = 1;
			}
		}
	}

	return iRet;
}
//----------------
int Theta::postStopCapture(void) {
	int iRet = 0;

	String strSendData = String("{\"name\": \"camera._stopCapture\", \"parameters\": { \"sessionId\": \"" + strSesId + "\" } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("post__stopCapture() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG

			Serial.print(
					"post__stopCapture() : state[" + strState + "]");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println(", Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			} else {  //done
				Serial.println("");
				iRet = 1;
			}
		}
	}

	return iRet;
}
//----------------
int Theta::postGetOptions(void) {
	int iRet = 0;

//String strSendData = String("{\"name\": \"camera.getOptions\", \"parameters\": { \"sessionId\": \"" + strSSID + "\", \"optionNames\": [\"captureMode\"] } }");
	String strSendData =
			String(
					"{\"name\": \"camera.getOptions\", \"parameters\": { \"sessionId\": \"" + strSesId
							+ "\", \"optionNames\":[\"captureMode\",\"exposureProgram\",\"exposureCompensation\",\"iso\",\"shutterSpeed\",\"whiteBalance\",\"_filter\",\"_captureInterval\",\"_captureNumber\",\"_shutterVolume\"] } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
	dumpJson("getOptions", strSendData, strJson);

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<300> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("post_getOptions() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG
			Serial.print(
					"post_getOptions() : state[" + strState + "], ");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println("Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			} else {  //done
				const char* sCaptureMode = root["results"]["options"]["captureMode"];
				strCaptureMode = String(sCaptureMode);

				const char* sExposureProgram = root["results"]["options"]["exposureProgram"];
				strExpProg = String(sExposureProgram);

				const char* sExposureCompensation = root["results"]["options"]["exposureCompensation"];
				String strEv = String(sExposureCompensation);
				for (int iCnt = 0; iCnt < LIST_NUM_EV; iCnt++) {
					if (strEv.equals(sList_Ev[iCnt])) {
						iCurEv = iCnt;
						break;
					}
				}

				const char* sIso = root["results"]["options"]["iso"];
				String strIso = String(sIso);
				for (int iCnt = 0; iCnt < LIST_NUM_ISO; iCnt++) {
					if (strIso.equals(sList_ISO[iCnt])) {
						iCurISO = iCnt;
						break;
					}
				}

				const char* sShutterSpeed = root["results"]["options"]["shutterSpeed"];
				String strShutterSpeed = String(sShutterSpeed);
				for (int iCnt = 0; iCnt < LIST_NUM_SS; iCnt++) {
					if (strShutterSpeed.equals(sList_SS[iCnt])) {
						iCurSS = iCnt;
						break;
					}
				}

				const char* sWhiteBalance = root["results"]["options"]["whiteBalance"];
				String strWhiteBalance = String(sWhiteBalance);
				for (int iCnt = 0; iCnt < LIST_NUM_WB; iCnt++) {
					if (strWhiteBalance.equals(sList_WB[iCnt])) {
						iCurWB = iCnt;
						break;
					}
				}

				const char* sOpt = root["results"]["options"]["_filter"];
				String strOpt = String(sOpt);
				for (int iCnt = 0; iCnt < LIST_NUM_OPT; iCnt++) {
					if (strOpt.equals(sList_Opt[iCnt])) {
						iCurOpt = iCnt;
						break;
					}
				}

				const char* sIntExpSec = root["results"]["options"]["_captureInterval"];
				iIntExpSec = atoi(sIntExpSec);

				const char* sIntExpNum = root["results"]["options"]["_captureNumber"];
				iIntExpNum = atoi(sIntExpNum);

				const char* sShutterVol = root["results"]["options"]["_shutterVolume"];
				iBeepVol = atoi(sShutterVol);

#ifdef DEBUG
				Serial.println(
						"captureMod[" + strCaptureMode + "], exposureProgram["
						+ strExpProg + "]");
#endif
				iRet = 1;
			}
		}
	}

	return iRet;
}
//----------------
int Theta::postCommandsStatus(void) {
	int iRet = 0;

	String strSendData = String("{\"id\":\"" + strTakePicLastId + "\"}");
	String strJson = httpProtocol("POST", strUrlCmdStat, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );
	this->dumpJson("postCommandsStatus", strSendData, strJson);
	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("post_commnads_status() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG

			Serial.print(
					"post_commnads_status() : state[" + strState
					+ "]");
#endif
			if (strState.equals("error")) {
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println(", Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			} else if (strState.equals("done")) {
				const char* sFileUri = root["results"]["fileUri"];
				Serial.println(", fileUri[" + String(sFileUri) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = 1;
			} else {  // inProgress
				const char* sId = root["id"];
				Serial.println(", id[" + String(sId) + "]");
				iRet = 1;
			}
		}
	}

	return iRet;
}

//----------------
int Theta::resSetOptions(String strJson) {
	int iRet;

	iRet = strJson.length();
	if (iRet != 0) {
		char sJson[iRet + 1];
		strJson.toCharArray(sJson, iRet + 1);
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.parseObject(sJson);
		if (!root.success()) {
			Serial.println("post_commnads_status() : parseObject() failed.");
			iRet = -1;
		} else {
			const char* sState = root["state"];
			String strState = String(sState);
#ifdef DEBUG
			Serial.print("ThetaAPI_Res_setOptions() : state[" + strState + "]" );
#endif
			if (strState.equals("done")) {
				iRet = 1;
			} else {  //error
				Serial.print("setOptions : state is not done. [" + strState + "]");
				const char* sErrorCode = root["error"]["code"];
				const char* sErrorMessage = root["error"]["message"];
				Serial.println(", Code[" + String(sErrorCode) + "], Message[" + String(sErrorMessage) + "]");
				iTakePicStat = TAKE_PIC_STAT_DONE;
				iRet = -1;
			}
		}
	}
	return iRet;
}

//----------------
int Theta::postSetOptionsCaptureMode(String psCapMode) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"captureMode\":\"" + psCapMode
					+ "\"} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsExposureProgram(String psExpProg) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"exposureProgram\":" + psExpProg
					+ "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsExposureCompensation(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"exposureCompensation\":"
					+ String(sList_Ev[iCurEv]) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsIso(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"iso\":"
					+ String(sList_ISO[iCurISO]) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsShutterSpeed(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"shutterSpeed\":"
					+ String(sList_SS[iCurSS]) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsWhiteBalance(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"whiteBalance\":\""
					+ String(sList_WB[iCurWB]) + "\"} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsFilter(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"_filter\":\""
					+ String(sList_Opt[iCurOpt]) + "\"} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsCaptureInterval(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"_captureInterval\":"
					+ String(iIntExpSec) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsCaptureNumber(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"_captureNumber\":"
					+ String(iIntExpNum) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}
//----------------
int Theta::postSetOptionsShutterVolume(void) {
	int iRet = 0;

	String strSendData = String(
			"{\"name\": \"camera.setOptions\", \"parameters\": { \"sessionId\": \"" + strSesId + "\", \"options\":{\"_shutterVolume\":"
					+ String(iBeepVol) + "} } }");
	String strJson = httpProtocol("POST", strUrlCmdExe, strHost, iHttpPort, strSendData);
//Serial.println("JSON:[" + strJson + "], len=" + strJson.length() );

	iRet = resSetOptions(strJson);

	return iRet;
}

#endif /* THETA_CPP_ */
