#include <EEPROM.h>


#define SERVO1PIN     5
#define SERVO2PIN     6
#define BTNPIN        8
#define LEDPIN        7

/* Servo pulse duration 1000us to 2000us */
#define MIN_ANGLE       1200
#define MAX_ANGLE       1800
#define MAX_SPEED       50
#define EEPROM_ADDR     2
#define SHUTDOWN_COUNT  30

#define STATE_OFF           0
#define STATE_UP_CONTRACT   1
#define STATE_UP_EXPAND     2
#define STATE_DOWN_CONTRACT 3
#define STATE_DOWN_EXPAND   4

uint32_t angle1 = MIN_ANGLE;
uint32_t angle2 = MIN_ANGLE;
uint8_t speed = 5;    
bool pwr_flag = false;
uint8_t h_state = STATE_OFF;
uint32_t cmd_timestamp = 0;
uint32_t led_timestamp = 0;
uint8_t cmd_count = 0;
uint32_t servo_sync_timestamp = 0;


void servo_delay_short(uint32_t duration_us){ 
  while(duration_us > 500){
    duration_us -= 500;
    delayMicroseconds(500);
  }
  delayMicroseconds(duration_us); 
  
}

void update_servos(){  
  uint32_t next_sync = servo_sync_timestamp + 20;  

  while(millis() < next_sync){ /* Wait for 20ms to pass since last servo update */ }
  servo_sync_timestamp = millis();

  /* Servo command pulse is up to 2ms long */
  digitalWrite(SERVO1PIN, HIGH);
  servo_delay_short(angle1);
  digitalWrite(SERVO1PIN, LOW);
  
  next_sync = servo_sync_timestamp + 3;
  while(millis() < next_sync){ /* wait 3ms since sync to update second servo */ }

  /* Update second servo */
  digitalWrite(SERVO2PIN, HIGH);
  servo_delay_short(angle2);
  digitalWrite(SERVO2PIN, LOW);
} 

void LED_set(){
  led_timestamp = millis();
  digitalWrite(LEDPIN, HIGH);
}

void LED_process(){
  if((led_timestamp > 0) && ((millis() - led_timestamp) > 100)){
    led_timestamp = 0;
    digitalWrite(LEDPIN, LOW);
  }  
}

void btn_cmd(){

  if(digitalRead(BTNPIN) == LOW){  
    cmd_timestamp = millis();

    delay(50);
    if(cmd_count <= SHUTDOWN_COUNT){
      cmd_count++;
      digitalWrite(LEDPIN, HIGH);     
      led_timestamp = millis();
    }else{
      pwr_flag = false;
      digitalWrite(LEDPIN, LOW);
    } 
  }else{
    if(cmd_count >= SHUTDOWN_COUNT){
      
      EEPROM.begin();
      EEPROM.update(EEPROM_ADDR, speed);
      EEPROM.end();
    }else{
      uint32_t btn_press_timespan = millis() - cmd_timestamp;

      if(cmd_timestamp > 0){
        if(btn_press_timespan < (SHUTDOWN_COUNT * 50)){
          if(pwr_flag){
            speed+=5;
            if(speed > MAX_SPEED ){
              speed = 5;
            }  
          }else{        
            pwr_flag = true;     
          }     
        }      
      }  
    }
    
    cmd_count = 0;
    cmd_timestamp = 0;  
  }
}

void setup() {
  pinMode(SERVO1PIN, OUTPUT); 
  pinMode(SERVO2PIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT); 
  pinMode(BTNPIN, INPUT_PULLUP);
  digitalWrite(SERVO1PIN, LOW);
  digitalWrite(SERVO2PIN, LOW);   
  digitalWrite(LEDPIN, LOW);
  

  EEPROM.begin();
  speed = EEPROM.read(EEPROM_ADDR);
  EEPROM.end(); 

  if(speed > MAX_SPEED ){
    // Not saved. Use low speed.
    speed = 5;
  }  

  Serial.begin(9600);
  Serial.print("\n\nSpeed:");
  Serial.println(speed);
}

void loop()  { 
  btn_cmd();
  LED_process();


  if(!pwr_flag){ 
    if(h_state != STATE_OFF){
      h_state = STATE_OFF;
      Serial.println("Stop");
    }
  }else{
    if(h_state == STATE_OFF){
      h_state = STATE_UP_CONTRACT;

      LED_set();
      Serial.println("Start");

      EEPROM.begin();
      speed = EEPROM.read(EEPROM_ADDR);
      EEPROM.end(); 

      if(speed > MAX_SPEED ){
        // Not saved. Use low speed.
        speed = 5;
      }  
    }else{  
      if(h_state == STATE_UP_CONTRACT){
        if(angle1 < MAX_ANGLE){
          angle1 += speed;              
        }else{
          h_state = STATE_UP_EXPAND;
          LED_set();
        }
      }else if(h_state == STATE_UP_EXPAND){
        if(angle1 > MIN_ANGLE){
          angle1 -= speed;
        }else{
          h_state = STATE_DOWN_CONTRACT;
          LED_set();
        }
      }else if(h_state == STATE_DOWN_CONTRACT){
        if(angle2 < MAX_ANGLE){
          angle2 += speed;
        }else{
          h_state = STATE_DOWN_EXPAND;
          LED_set();
        }
      }else if(h_state == STATE_DOWN_EXPAND){
        if(angle2 > MIN_ANGLE){
          angle2 -= speed;          
        }else{
          h_state = STATE_UP_CONTRACT;  
          LED_set();          
        }
      }

      update_servos();
    }
  }
}
