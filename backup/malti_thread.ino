#include <BleKeyboard.h>
#include "esp32-hal-log.h"
#include "freertos/task.h"
#define RSIZE 5
#define LSIZE 5
#define LED_BUILTIN 2

BleKeyboard bleKeyboard;

const int PULSE_1=15;
const int sensor1=4;
const int sensor2=5;
const int sensor3=18;
const int sensor4=19;
const int sensor5=21;

const int PULSE_2=23;
const int sensor6=13;
const int sensor7=12;
const int sensor8=14;
const int sensor9=27;
const int sensor10=26;

const int threshold=250;

long int touch1(int pulse,int sensor){
	long int t=0;
	long int start;

	// Pulse up
	digitalWrite(pulse, HIGH);

	start=micros();
	while (digitalRead(sensor)!=HIGH);
	t=micros()-start;

	digitalWrite(pulse, LOW);  
	delay(1);

	return t;
}

long int touch2(int pulse,int sensor){
	long int t=0;
	long int start;

	// Pulse up
	digitalWrite(pulse, HIGH);

	start=micros();
	while (digitalRead(sensor)!=HIGH);
	t=micros()-start;

	digitalWrite(pulse, LOW);  
	delay(1);

	return t;
}

void Right_hand(void *pvParameters){
	int i;
	char buf=0;
	char out=-1;
	unsigned long int prev=0;
	long int Right_times[RSIZE]={0};
	char Right_keymap[RSIZE]={'j','k','l',';','h'};
	const int Right_sensors[RSIZE]={sensor1,sensor2,sensor3,sensor4,sensor5};

	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	
	while(1){
		//Right_times={};
		memset(Right_times,0,sizeof(Right_times));

		out=-1;

		for(i=0;i<RSIZE;i++){
			Right_times[i]=touch1(PULSE_1,Right_sensors[i]);
		}

		//Serial.printf("%d,%d,%d,%d,%d\n",Right_times[0],Right_times[1],Right_times[2],Right_times[3],Right_times[4]);

		for(i=0;i<RSIZE;i++){
			if(Right_times[i]>threshold) out=Right_keymap[i];
		}

		if(out!=-1 && (millis()-prev)>((out==buf)?200:100)){
			bleKeyboard.print(out);
			prev=millis();
			buf=out;
		}
	}
}

void Left_hand(void *pvParameters){
	int i;
	char buf=0;
	char out=-1;
	unsigned long int prev=0;
	long int Left_times[LSIZE]={0};
	char Left_keymap[LSIZE]={'a','s','d','f','g'};
	const int Left_sensors[LSIZE]={sensor6,sensor7,sensor8,sensor9,sensor10};

	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
	
	while(1){
		//Left_flags={};
		memset(Left_times,0,sizeof(Left_times));

		out=-1;

		for(i=0;i<LSIZE;i++){
			Left_times[i]=touch2(PULSE_2,Left_sensors[i]);
		}

		Serial.printf("%ld,%ld,%ld,%ld,%ld\n",Left_times[0],Left_times[1],Left_times[2],Left_times[3],Left_times[4]);

		for(i=0;i<LSIZE;i++){
			if(Left_times[i]>threshold) out=Left_keymap[i];
		}

		if(out!=-1 && (millis()-prev)>((out==buf)?200:100)){
			bleKeyboard.print(out);
			prev=millis();
			buf=out;
		}
	}
}

void setup() {
	Serial.begin(115200); 
	bleKeyboard.begin();

	pinMode(PULSE_1,OUTPUT);
	pinMode(sensor1,INPUT);
	pinMode(sensor2,INPUT);
	pinMode(sensor3,INPUT);
	pinMode(sensor4,INPUT);
	pinMode(sensor5,INPUT);

	pinMode(PULSE_2,OUTPUT);
	pinMode(sensor6,INPUT);
	pinMode(sensor7,INPUT);
	pinMode(sensor8,INPUT);
	pinMode(sensor9,INPUT);
	pinMode(sensor10,INPUT);
	
	pinMode(LED_BUILTIN,OUTPUT);

	delay(100);
	xTaskCreatePinnedToCore(Right_hand, "Right_hand", 8192, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(Left_hand, "Left_hand", 8192, NULL, 1, NULL, 1);
}

void loop(){}
