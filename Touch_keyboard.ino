/**
 * @date		2019 05/17-
 * @development code	Marionette
 * @author		Takana Norimasa <j17423@kisarazu.kosen-ac.jp> <seigenkousya@outlook.jp>
 * @brief		secure I/O Unit
 */ 

#include <BleKeyboard.h>
#include "esp32-hal-log.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define SENSORS 7
#define DICTSIZE 50
#define LED_BUILTIN 2

#define END_R (1<<0)
#define END_L (1<<1)
#define START_R (1<<2)
#define START_L (1<<3)
#define START (START_R | START_L)
#define ALL_SYNC (END_R | END_L)

BleKeyboard bleKeyboard;
EventGroupHandle_t eg_handle;

//yellow wire
const int PULSE_R=15;
const int sensor1=4;
const int sensor2=5;
const int sensor3=18;
const int sensor4=19;
const int sensor5=21;
const int ex_key1=34;
const int ex_key2=22;

//green wire
const int PULSE_L=23;
const int sensor6=13;
const int sensor7=12;
const int sensor8=14;
const int sensor9=27;
const int sensor10=26;
const int ex_key3=25;
const int ex_key4=33;

const int threshold=50;

bool Left_flags[SENSORS]={0};
bool Right_flags[SENSORS]={0};
long int Right_times[SENSORS]={0};
long int Left_times[SENSORS]={0};

void Right_hand(void *pvParameters){
	int i;
	long int t=0;
	long int start;
	const int Right_sensors[SENSORS]={sensor1,sensor2,sensor3,sensor4,sensor5,ex_key1,ex_key2};

	EventBits_t eg_bit;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;

	while(1){
		eg_bit=xEventGroupWaitBits(eg_handle,START_R,pdTRUE,pdFALSE,portMAX_DELAY);

		for(i=0;i<SENSORS;i++){
			digitalWrite(PULSE_R, HIGH);

			start=micros();
			while (digitalRead(Right_sensors[i])!=HIGH);
			if(threshold<micros()-start) Right_flags[i]=true;
			//Right_times[i]=micros()-start;

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
	const int Left_sensors[SENSORS]={sensor6,sensor7,sensor8,sensor9,sensor10,ex_key3,ex_key4};
	EventBits_t eg_bit;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken=pdFALSE;

	while(1){
		eg_bit=xEventGroupWaitBits(eg_handle,START_L,pdTRUE,pdTRUE,portMAX_DELAY);

		for(i=0;i<SENSORS;i++){
			digitalWrite(PULSE_L, HIGH);

			start=micros();
			while (digitalRead(Left_sensors[i])!=HIGH);
			if(threshold<micros()-start) Left_flags[i]=true;
			//Left_times[i]=micros()-start;

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
	pinMode(ex_key1,INPUT);
	pinMode(ex_key2,INPUT);

	pinMode(PULSE_L,OUTPUT);
	pinMode(sensor6,INPUT);
	pinMode(sensor7,INPUT);
	pinMode(sensor8,INPUT);
	pinMode(sensor9,INPUT);
	pinMode(sensor10,INPUT);
	pinMode(ex_key3,INPUT);
	pinMode(ex_key4,INPUT);

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
	char keymap[3][10]={	{'q','w','e','r','t','y','u','i','o','p'},
				{'a','s','d','f','g','h','j','k','l',';'},
				{'z','x','c','v','b','n','m',',','.','/'}};


	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);

	while(1){
		memset(Left_flags,false,sizeof(Left_flags));
		memset(Right_flags,false,sizeof(Right_flags));

		out=-1;

		//1100 -> 0011
		eg_bit=xEventGroupSync(eg_handle,START,ALL_SYNC,portMAX_DELAY);

		//Serial.printf("Left :%ld,%ld,%ld,%ld,%ld\n",Left_times[0],Left_times[1],Left_times[2],Left_times[3],Left_times[4]);
		//Serial.printf("Right:%ld,%ld,%ld,%ld,%ld\n",Right_times[0],Right_times[1],Right_times[2],Right_times[3],Right_times[4]);
		//Serial.printf("%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",Left_times[0],Left_times[1],Left_times[2],Left_times[3],Left_times[4],Left_times[5],Left_times[6],Right_times[0],Right_times[1],Right_times[2],Right_times[3],Right_times[4],Right_times[5],Right_times[6]);
		
		if(Left_flags[5]){bleKeyboard.print(' ');delay(100);}
		if(Left_flags[6]){bleKeyboard.write(KEY_TAB);delay(100);}
		if(Right_flags[5]){bleKeyboard.write(KEY_BACKSPACE);delay(100);}
		if(Right_flags[6]){bleKeyboard.write(KEY_RETURN);delay(100);}

		for(i=0;i<SENSORS-3;i++){
			if(Left_flags[i]){
				if(Left_flags[4]){
					out=keymap[2][i];
				}else if(Right_flags[4]){
					out=keymap[0][i];
				}else{
					out=keymap[1][i];
				}
				break;
			}
			if(Right_flags[i]){
				if(Left_flags[4]){
					out=keymap[2][i+SENSORS+1];
				}else if(Right_flags[4]){
					out=keymap[0][i+SENSORS+1];
				}else{
					out=keymap[1][i+SENSORS+1];
				}
				break;
			}
		}

		if(out!=-1 && (millis()-prev)>((out==buf)?200:100)){
			bleKeyboard.print(out);
			prev=millis();
			buf=out;
		}
	}
}
