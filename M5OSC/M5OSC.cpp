// Do not remove the include below
#include "M5OSC.h"

#ifdef TTGO
#include <M5StackTTGO.h>
#else
#include <M5Stack.h>
#endif
#include <Preferences.h>

#include "Theta.h"

#include "3901.h"
#include "batteryfull.h"

#include "nvs_flash.h"
#include "nvs.h"

void thetaTask(void *pvParameters);
void displayTask(void *pvParameters);
//void soundTask(void *pvParameters);

int ConnectTHETA(void);
void shoot(void);

Preferences preferences;
Theta theta;

xQueueHandle event_queue;

//#define DEFAUT_SSID "YOURSSID"
//#define DEFAULT_PWD "PASSWORD"
#define EMU_SSID "YOUR_SSID"
#define EMU_PWD "PASSWORD"

#define DEFAULT_SSID "THETAXS00130913.OSC"
#define DEFAULT_PWD "00130913"

String strThetaSSID = DEFAULT_SSID;
String strThetaPWD = DEFAULT_PWD;

#define GFXFF 1
#define FF18 &FreeSans12pt7b
#define CF_OL24 &Orbitron_Light_24

bool buttnApressed = false;
//bool play_music = false;
bool inprogress = false;

#define MENU_BUT_W	100
#define MENU_BUT_H	25

long debouncing_time = 200; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
volatile bool mEmulate = false;

//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handlePinInterrupt() {
	uint32_t event = 0;
	if ((long) (micros() - last_micros) >= debouncing_time * 1000) {
		if (digitalRead(BUTTON_A_PIN) == LOW)
			event = 1;
		else if (digitalRead(BUTTON_B_PIN) == LOW)
			event = 2;
		else if (digitalRead(BUTTON_C_PIN) == LOW)
			event = 3;
		xQueueSendToBack(event_queue, &event, 0);
		last_micros = micros();
		Serial.println(event);
	}

}

void setup() {
#ifdef INITNVS
	int err;
	err=nvs_flash_init();
	Serial.println("nvs_flash_init: " + err);
	err=nvs_flash_erase();
	Serial.println("nvs_flash_erase: " + err);
#endif INITNVS

	//M5.startupLogo();

	Serial.begin(115200);         // SERIAL
	M5.begin();                   // M5STACK INITIALIZE
	M5.Lcd.setBrightness(50);    // BRIGHTNESS = MAX 255
	M5.Lcd.fillScreen(WHITE);     // CLEAR SCREEN
	//M5.Lcd.setRotation(0);        // SCREEN ROTATION = 0
	M5.Lcd.setCursor(10, 10);
	M5.Lcd.setTextColor(BLACK);
	M5.Lcd.setTextSize(1);
	M5.Lcd.drawBitmap(0, 0, 320, 240, (uint16_t *) gImage_logoM5);
	M5.Lcd.printf("Theta Remote!");

	pinMode(BUTTON_A_PIN, INPUT);
	digitalWrite(BUTTON_A_PIN, HIGH);
	pinMode(BUTTON_B_PIN, INPUT);
	digitalWrite(BUTTON_B_PIN, HIGH);
	pinMode(BUTTON_C_PIN, INPUT);
	digitalWrite(BUTTON_C_PIN, HIGH);

	if (digitalRead(BUTTON_A_PIN) == LOW) {
		mEmulate = true;
		strThetaSSID = EMU_SSID;
		strThetaPWD = EMU_PWD;
		M5.Lcd.printf(" EMULATE ");
	}

	preferences.begin("theta-config");

//	strThetaSSID = preferences.getString("WIFI_SSID", DEFAULT_SSID);
//	strThetaPWD = preferences.getString("WIFI_PASSWD", DEFAULT_PWD);

	Serial.println("");
	Serial.println("");
	Serial.println("-----------------------------------------");
	Serial.println("  RICOH THETA S Remote Control Software  ");
	Serial.println("           Full Control Edition          ");
	Serial.print("WIFI-SSID: ");
	Serial.println(strThetaSSID);

	event_queue = xQueueCreate(10, sizeof(uint32_t));

	TaskHandle_t xHandle = NULL;
	xTaskCreatePinnedToCore(thetaTask, "thetaTask", 4096, (void *) 1, 1, NULL, 0);
	xTaskCreatePinnedToCore(displayTask, "displayTask", 12288, (void *) 1, 1, NULL, 1);
//	xTaskCreatePinnedToCore(soundTask, "soundTask", 4096, (void *) 1, 1, &xHandle, 1);

	attachInterrupt(BUTTON_A_PIN, handlePinInterrupt, FALLING);
	attachInterrupt(BUTTON_B_PIN, handlePinInterrupt, FALLING);
	attachInterrupt(BUTTON_C_PIN, handlePinInterrupt, FALLING);
//	configASSERT(xHandle);
	//Speaker.playMusic(m5stack_startup_music, 25000);
	//play_music = true;
}

void thetaTask(void *pvParameters) {
	bool init = true;
	int taskno = (int) pvParameters;
	uint32_t event = 0;
	while (1) {
		if (!WiFi.isConnected()) {
			init = true;
			theta.resetState();
			ConnectTHETA();
		} else {
			theta.postGetState();
			if (theta.isBusy()) {
				theta.postCommandsStatus();
			}
			if (init) {
				theta.getInfo();
				theta.postStartSession();
				theta.postGetState();
				theta.postGetOptions();
				init = false;
			}
			if (xQueueReceive(event_queue,&event,1000/portTICK_RATE_MS) == pdTRUE) {
				if (event == 2)
					theta.postTakePicture();
			}
		}
//		vTaskDelay(1000);
	}
}

void drawBatteryLevel(int x, int y) {
	if (theta.batteryState == 0) {
		//M5.Lcd.drawBitmap(x, y, 39, 20, image_data_battery00);
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_batterych);
	} else if (theta.batteryState == 1) {
		//if (blink())
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_batteryfc);
	} else if (theta.bateryLevel > 70)
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_batteryfull);
	else if (theta.bateryLevel > 40)
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_battery67);
	else if (theta.bateryLevel > 20) {
		//if (blink())
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_battery33);
	} else
		//if (blink())
		M5.Lcd.drawBitmap(x, y, 39, 20, image_data_battery00);

}

void drawWifiState(int x, int y) {
	if (WiFi.isConnected()) {
		M5.Lcd.fillCircle(x, y, 10, DARKGREEN);
		M5.Lcd.fillCircle(x, y, 6, GREEN);
	} else {
		M5.Lcd.fillCircle(x, y, 10, ORANGE);
		M5.Lcd.fillCircle(x, y, 6, RED);
	}
}

void drawCameraBusyState(int x, int y) {
	if (theta.isBusy()) {
		M5.Lcd.setTextColor(RED);
		M5.Lcd.drawString("BUSY", x, y, GFXFF);
	} else
		M5.Lcd.fillRect(x, y, 120, 20, WHITE);
}

void displayTask(void *pvParameters) {
	int taskno = (int) pvParameters;

	//M5.Lcd.setTextColor(RED);
	//M5.Lcd.drawCentreString("Snap", 160, 220, 2);
	M5.Lcd.drawBitmap(139, 197, 52, 35, (uint16_t *) image_data_3901);

	while (1) {
		M5.update();
		M5.Lcd.setFreeFont(FF18);
		drawWifiState(240, 12);
		drawCameraBusyState(200, 40);
		drawBatteryLevel(280, 2);

		M5.Lcd.setFreeFont(CF_OL24);
		M5.Lcd.setTextColor(BLACK);
		M5.Lcd.setTextFont(1);

		vTaskDelay(1000);
	}
}

//void soundTask(void *pvParameters) {
//	bool pic = true;
//	int taskno = (int) pvParameters;
//	while (1) {
//		if (play_music) {
//			Speaker.playMusic(m5stack_startup_music, 30000);
//			play_music = false;
//		}
//		vTaskDelay(500);
//	}
//}

void loop() {
	vTaskDelay(portMAX_DELAY);
}

//-------------------------------------------
// Wi-Fi Connect functions
//-------------------------------------------
int ConnectTHETA(void) {
	WiFi.disconnect(true);

	int retval = WiFi.begin(strThetaSSID.c_str(), strThetaPWD.c_str());
	Serial.print("result WiFi.begin : ");
	Serial.println(retval);

	Serial.print(WiFi.status());
	Serial.print("WIFI-SSID: ");
	Serial.println(WiFi.SSID());
	while (!WiFi.isConnected()) {
		M5.Lcd.printf(".");
		Serial.print(WiFi.status());
		Serial.print("WIFI-SSID: ");
		Serial.println(WiFi.SSID());
		if (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_CONNECTION_LOST) {
			M5.Lcd.printf("o");
			WiFi.disconnect(true);
			WiFi.begin(strThetaSSID.c_str(), strThetaPWD.c_str());
		}

		vTaskDelay(500);
	}

	M5.Lcd.printf("\n\nWiFI connected\n");
	M5.Lcd.printf("Mac: %s\n", WiFi.macAddress().c_str());
	M5.Lcd.printf("Ip:  %s\n", WiFi.localIP().toString().c_str());
	M5.Lcd.printf("Gw:  %s\n", WiFi.gatewayIP().toString().c_str());

	Serial.println("");
	Serial.println("WiFi connected : ");
	Serial.println("Mac            : " + WiFi.macAddress());
	Serial.print("IP address     : ");
	Serial.println(WiFi.localIP());
	Serial.print("GW address     : ");
	Serial.println(WiFi.gatewayIP());
//set the gateway as connection host
//theta.strHost = WiFi.gatewayIP().toString();
	if (mEmulate) {
		theta.iHttpPort = 8080;
		theta.strHost = "192.168.43.1";
	}
	return 0;
}

