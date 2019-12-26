#include <bitset>
#include <iostream>
#include <BleKeyboard.h>
#include "esp32-hal-log.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define RSIZE 5
#define LSIZE 5
#define LED_BUILTIN 2

#define END_R (1<<0)
#define END_L (1<<1)
#define START_R (1<<2)
#define START_L (1<<3)
#define START (START_R | START_L)
#define ALL_SYNC (END_R | END_L)

BleKeyboard bleKeyboard;
EventGroupHandle_t eg_handle;

const int PULSE_R=15;
const int sensor1=4;
const int sensor2=5;
const int sensor3=18;
const int sensor4=19;
const int sensor5=21;

const int PULSE_L=23;
const int sensor6=13;
const int sensor7=12;
const int sensor8=14;
const int sensor9=27;
const int sensor10=26;

const int threshold=100;

long int Right_times[RSIZE]={0};
long int Left_times[LSIZE]={0};

void Right_hand(void *pvParameters){
	int i;
	long int t=0;
	long int start;
	const int Right_sensors[RSIZE]={sensor1,sensor2,sensor3,sensor4,sensor5};
	
	EventBits_t eg_bit;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;

	while(1){
		eg_bit=xEventGroupWaitBits(eg_handle,START_R,pdTRUE,pdFALSE,portMAX_DELAY);

		for(i=0;i<RSIZE;i++){
			digitalWrite(PULSE_R, HIGH);

			start=micros();
			while (digitalRead(Right_sensors[i])!=HIGH);
			Right_times[i]=micros()-start;

			digitalWrite(PULSE_R, LOW);  
			delay(1);
		}

		eg_bit=xEventGroupSetBits(eg_handle,END_R);
	}
}

void Left_hand(void *pvParameters){
	int i;
	long int t=0;
	long int start;
	const int Left_sensors[LSIZE]={sensor6,sensor7,sensor8,sensor9,sensor10};
	EventBits_t eg_bit;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;
	
	while(1){
		eg_bit=xEventGroupWaitBits(eg_handle,START_L,pdTRUE,pdTRUE,portMAX_DELAY);
		
		for(i=0;i<LSIZE;i++){
			digitalWrite(PULSE_L, HIGH);

			start=micros();
			while (digitalRead(Left_sensors[i])!=HIGH);
			Left_times[i]=micros()-start;

			digitalWrite(PULSE_L, LOW);  
			delay(1);
		}
	
		eg_bit=xEventGroupSetBits(eg_handle,END_L);
	}
}

void setup() {
	Serial.begin(115200); 
	bleKeyboard.begin();
	eg_handle=xEventGroupCreate();

	pinMode(PULSE_R,OUTPUT);
	pinMode(sensor1,INPUT);
	pinMode(sensor2,INPUT);
	pinMode(sensor3,INPUT);
	pinMode(sensor4,INPUT);
	pinMode(sensor5,INPUT);

	pinMode(PULSE_L,OUTPUT);
	pinMode(sensor6,INPUT);
	pinMode(sensor7,INPUT);
	pinMode(sensor8,INPUT);
	pinMode(sensor9,INPUT);
	pinMode(sensor10,INPUT);

	pinMode(LED_BUILTIN,OUTPUT);

	delay(100);

	xTaskCreatePinnedToCore(Right_hand, "Right_hand", 8192, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(Left_hand, "Left_hand", 8192, NULL, 1, NULL, 1);
	delay(100);
	
}


void loop(){
	int i;
	char buf=0;
	char out=-1;
	EventBits_t eg_bit;
	unsigned long int prev=0;
	char Right_keymap[RSIZE]={'j','k','l',';','h'};
	char Left_keymap[LSIZE]={'a','s','d','f','g'};

	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);

	while(1){
		memset(Left_times,0,sizeof(Left_times));
		memset(Right_times,0,sizeof(Right_times));

		out=-1;

		//1100 -> 0011
		eg_bit=xEventGroupSync(eg_handle,START,ALL_SYNC,portMAX_DELAY);
		
		//Serial.printf("Left :%ld,%ld,%ld,%ld,%ld\n",Left_times[0],Left_times[1],Left_times[2],Left_times[3],Left_times[4]);
		//Serial.printf("Right:%ld,%ld,%ld,%ld,%ld\n",Right_times[0],Right_times[1],Right_times[2],Right_times[3],Right_times[4]);
		Serial.printf("%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",Left_times[0],Left_times[1],Left_times[2],Left_times[3],Left_times[4],Right_times[0],Right_times[1],Right_times[2],Right_times[3],Right_times[4]);
/*	
		for(i=0;i<LSIZE;i++){
			if(Right_times[i]>threshold) out=Right_keymap[i];
			if(Left_times[i]>threshold) out=Left_keymap[i];
		}

		if(out!=-1 && (millis()-prev)>((out==buf)?200:100)){
			bleKeyboard.print(out);
			prev=millis();
			buf=out;
		}
*/
	}
}

