#include <ssd1306.h>
#include <EEPROM.h>


#define PWR_PIN_1     3
#define PWR_PIN_2     4
#define SERVO1PIN     5
#define SERVO2PIN     6
#define OLED_SCL_PIN  8
#define OLED_SDA_PIN  7

/* Servo pulse duration 1000us to 2000us */
#define MIN_ANGLE   1200
#define MAX_ANGLE   1800
#define MAX_SPEED   100
#define EEPROM_ADDR 2

#define STATE_OFF           0
#define STATE_UP_CONTRACT   1
#define STATE_UP_EXPAND     2
#define STATE_DOWN_CONTRACT 3
#define STATE_DOWN_EXPAND   4
#define SCREEN_OFFSET       32
#define IMG_START           40
#define IMG_TRAIL           70
#define CMD_TIMEOUT         100
#define CMD_START           0xA5

uint32_t angle1 = MIN_ANGLE;
uint32_t angle2 = MIN_ANGLE;
uint8_t speed = 5;    
bool pwr_flag = true;
uint8_t h_state = STATE_OFF;
uint8_t cursorx = 0;
uint8_t yoffset;
uint32_t cmd_timestamp = 0;
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

void draw_ecg(){
  uint8_t y;
  uint8_t x = cursorx & 127; 
  uint8_t d = (cursorx - IMG_TRAIL) & 127;

  if(x < IMG_START){     
    yoffset = 0;
  }else if(x < (IMG_START+2)){
    yoffset += 2;   
  }else if(x < (IMG_START+3)){
    yoffset += 1;   
  }else if(x < (IMG_START+5)){ 

  }else if(x < (IMG_START+6)){
    yoffset -= 1;     
  }else if(x < (IMG_START+8)){
    yoffset -= 2;  
  }else if(x < (IMG_START+14)){

  }else if(x < (IMG_START+15)){
    yoffset -= 3;  
  }else if(x < (IMG_START+20)){
    yoffset += 6;  
  }else if(x < (IMG_START+26)){
    yoffset -= 6;  
  }else if(x < (IMG_START+27)){
    yoffset += 6;  
  }else if(x < (IMG_START+28)){
    yoffset += 3;  
  }else if(x < (IMG_START+32)){ 

  }else if(x < (IMG_START+34)){
    yoffset += 2;   
  }else if(x < (IMG_START+35)){
    yoffset += 1;   
  }else if(x < (IMG_START+37)){ 

  }else if(x < (IMG_START+38)){
    yoffset -= 1;     
  }else if(x < (IMG_START+40)){
    yoffset -= 2;  
  }

  y = SCREEN_OFFSET + yoffset;
  uint8_t clear_buff[16] = {0};

  ssd1306_drawVLine(x, y, y+3);
  ssd1306_drawBuffer(d, 0, 2, 64, clear_buff);   
  cursorx++; 
}

void draw_screensaver(){
  ssd1306_fillScreen(0x00); 
  int x, y, r, dist;
  r = 16;
  
  for (x = 0; x <= r*2; x++){
    for (y = 0; y <= r * 2; y++){
      dist = sqrt((r-x) * (r-x) + (r-y) * (r-y));
    
      if (dist==r){
        ssd1306_drawVLine(x+64-r, y+SCREEN_OFFSET-r, y+SCREEN_OFFSET-r+2);
      }          
    }   
    
  }
  ssd1306_drawHLine(64-r, SCREEN_OFFSET, 64);
  ssd1306_drawHLine(64-r, SCREEN_OFFSET+1, 64);
  ssd1306_drawHLine(64-r, SCREEN_OFFSET-1, 64);
}

uint8_t cmd_rx(){  
  // uint8_t rx = 0;
  // bool rx_received = false;  
  // uint8_t last_rx = 0;

  // while(mySerial.available()) {
  //   rx = mySerial.read();

  //   if(rx == CMD_START){
  //     last_rx = rx;
  //   }else if(last_rx == CMD_START){
  //     rx_received = true;
  //     last_rx = rx;
  //   }         
  // }

  // if(rx_received){ 
  //   speed = rx;

  //   if(speed > MAX_SPEED){
  //     speed = MAX_SPEED;
  //   }

  //   EEPROM.begin();
  //   EEPROM.update(EEPROM_ADDR, speed);
  //   EEPROM.end();  
  //   // mySerial.write(speed);        
  // }    
}
   
void setup() {
  pinMode(PWR_PIN_1, INPUT_PULLUP); 
  pinMode(PWR_PIN_2, OUTPUT); 
  pinMode(SERVO1PIN, OUTPUT); 
  pinMode(SERVO2PIN, OUTPUT); 
  digitalWrite(PWR_PIN_2, LOW);
  digitalWrite(SERVO1PIN, LOW);
  digitalWrite(SERVO2PIN, LOW);   

  ssd1306_128x64_i2c_initEx(3,4,0);
  ssd1306_fillScreen(0x00);  
  draw_screensaver();

  EEPROM.begin();
  speed = EEPROM.read(EEPROM_ADDR);
  EEPROM.end(); 

  if(speed > MAX_SPEED ){
    // Not saved. Use low speed.
    speed = 5;
  }  
}

void loop()  { 
  // pwr_flag = (digitalRead(UART_RX_PIN) == HIGH);  

  if(!pwr_flag){ 
    if(h_state != STATE_OFF){
      h_state = STATE_OFF;
      draw_screensaver();     
    }
  }else{
    if(h_state == STATE_OFF){
      h_state = STATE_UP_CONTRACT;
      ssd1306_fillScreen(0x00); 
    }else{  
      draw_ecg();    

      if(h_state == STATE_UP_CONTRACT){
        if(angle1 < MAX_ANGLE){
          angle1 += speed;              
        }else{
          h_state = STATE_UP_EXPAND;
        }
      }else if(h_state == STATE_UP_EXPAND){
        if(angle1 > MIN_ANGLE){
          angle1 -= speed;
        }else{
          h_state = STATE_DOWN_CONTRACT;
        }
      }else if(h_state == STATE_DOWN_CONTRACT){
        if(angle2 < MAX_ANGLE){
          angle2 += speed;
        }else{
          h_state = STATE_DOWN_EXPAND;
        }
      }else if(h_state == STATE_DOWN_EXPAND){
        if(angle2 > MIN_ANGLE){
          angle2 -= speed;          
        }else{
          h_state = STATE_UP_CONTRACT;            
        }
      }

      update_servos();
    }
  }
}
