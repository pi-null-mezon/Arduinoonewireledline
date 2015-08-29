/*-----------------------------------------------------------------------
 * A sketch for arduino controlled onewire addressable LED line
 * Function list:
 * TO DO... 
 *
 * Command set:
 * '1' - 
 *
 * Taranov Alex, 2015 
 *----------------------------------------------------------------------*/
#include <EEPROM.h>
#include <OneWire.h>

#define WIRE_PIN 9
#define BUTTON_PIN 2
#define ANALOG_PIN 0
#define BUTTON_DELAY_US 5000
#define SERIAL_PORT_SPEED 9600
#define IND_A 10
#define IND_B 11
#define IND_C 7
#define IND_D 13
#define IND_E 8
#define IND_F 5
#define IND_G 12
#define IND_H 6
#define COMMANDS_LOOP_LENGTH 32

OneWire wire_manager(WIRE_PIN);
byte v_addr[8];
byte m_devices = 0;
volatile int nextCommand = 1;
byte m_lineState = 0xFF;
int m_curpos;
volatile unsigned int m_delay;
boolean state = false;
boolean testFlag = false;
unsigned int cyclesPassed = 0;
byte v_map[ 8 * 16]; // enlarge vector size for you line length

byte v_ind[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 
                0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0xF7, 0xFC, 0xB9, 0xDE, 0xF9, 0xF1}; // '1', '2', '3', '4' and so on, overal 32 unique symbols

//----------------------------------
void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(SERIAL_PORT_SPEED);
  attachInterrupt(0, enrollPressure, FALLING); // 0 interrupt for Arduino Uno linked with 2 GPIO

  pinMode(IND_A, OUTPUT);
  pinMode(IND_B, OUTPUT);
  pinMode(IND_C, OUTPUT);
  pinMode(IND_D, OUTPUT);
  pinMode(IND_E, OUTPUT);
  pinMode(IND_F, OUTPUT);
  pinMode(IND_G, OUTPUT);
  pinMode(IND_H, OUTPUT);
  
  m_delay = 68;
  toogleLineOff(); // call once before loadAddresses() for m_devices count
  loadAddresses();
  toogleOscillatingLightOn();
}
//----------------------------------

//----------------------------------
void loop()
{      
  m_delay = 5 * analogRead(ANALOG_PIN);
  indicate(nextCommand);
  
  if(Serial.available()) {
    nextCommand = Serial.read();
    if(nextCommand == 's') {
      remapAddresses();
      nextCommand = 3;  
    } else {
      nextCommand -= '0';
    }  
  }
  
  switch(nextCommand) {
    case 0:
      toogleLineOff();
      break;
    case 1:
      toogleLineOn();
      break;
    case 2:
      toogleOverLine();
      break;
    case 3:
      toogleForwardLightOn();
      break;
    case 4:
      toogleBackwardLightOn();
      break;
    case 5:
      toogleOscillatingLightOn();
      break;
    case 6:
      toogleEvenLight();
      break;
    case 7:
      toogleOddLight();
      break;
    case 8:
      toogleRandomLight();
      break;
    case 9:
      toogleTwoForwardLight();
      break;
    case 10:
      toogleTwoBackwardLight();
      break;
    case 11:
      toogleEvenOddLight();
      break;
    case 12:
      toogleForwardStrip();
      break;
    case 13:
      toogleBackwardStrip();
      break;
    case 14:
      toogleForwardLightOff();
      break;
    case 15:
      toogleBackwardLightOff();
      break;
    case 16:
      lineBlinking();
      break;
    case 17:
      toogleCollapselight();
      break;
    case 18:
      toogleOscillatingLightOff();
      break;
    case (COMMANDS_LOOP_LENGTH - 1):
      testFlag = true;
      break; 
    default:
      toogleLineOn();
      break;    
  }
  
  if(testFlag) {
    cyclesPassed++;
    nextCommand = (++nextCommand) % COMMANDS_LOOP_LENGTH;
    if((cyclesPassed % (COMMANDS_LOOP_LENGTH - 2)) == 0) {
      testFlag = false;
      cyclesPassed = 0;
    }  
  }
}
//----------------------------------

void loadAddresses() {
  for(byte i = 0 ; i < (m_devices * 8); i++) {
    v_map[i] = EEPROM.read(i);
  }     
}

//----------------------------------
void indicate(int symbolNumber) {
  byte symbolCode = ~v_ind[symbolNumber]; // inversion for shared anode indicator 
  digitalWrite(IND_A, symbolCode & 0x01);
  digitalWrite(IND_B, (symbolCode >> 1) & 0x01);
  digitalWrite(IND_C, (symbolCode >> 2) & 0x01);
  digitalWrite(IND_D, (symbolCode >> 3) & 0x01);
  digitalWrite(IND_E, (symbolCode >> 4) & 0x01);
  digitalWrite(IND_F, (symbolCode >> 5) & 0x01);
  digitalWrite(IND_G, (symbolCode >> 6) & 0x01);
  digitalWrite(IND_H, (symbolCode >> 7) & 0x01); 
}

//----------------------------------

//----------------------------------
void enrollPressure()
{
  delayMicroseconds(BUTTON_DELAY_US);
  if(digitalRead(BUTTON_PIN) == LOW)  {  
    nextCommand = (++nextCommand) % COMMANDS_LOOP_LENGTH;
    m_delay = 1;
    testFlag = false;
  }
}
//----------------------------------

//----------------------------------
/*boolean checkCRC(byte *ad) {
 if( OneWire::crc8(ad, 7) != ad[7])  {
    Serial.println("CRC is not VALID");
    return false;
  } else {
    Serial.println("valid CRC");
    return true;
  }
}*/
//----------------------------------

//----------------------------------
byte switchKey(byte *ad)
{ 
  wire_manager.reset();
  wire_manager.select(ad); // this function automatically toogles DS2405 key state, look at OneWire.cpp and device specification
  return (wire_manager.read_bit() & 0x01); // 0x00 means key toogled off and 0x01 means key toogled on      
}
//----------------------------------

//----------------------------------
void toogleOverLine()
{
  if(wire_manager.search(v_addr)) {
    switchKey(v_addr);
    delay(m_delay);  
  } else {
    wire_manager.reset_search();
  }
  m_lineState = 0x02;  
}
//----------------------------------

//----------------------------------
void toogleLineOff()
{
  if(m_lineState != 0x00) {
    m_devices = 0;  
    wire_manager.reset_search();
    while(wire_manager.search(v_addr)) {
      m_devices++;
      if(switchKey(v_addr) == 0x01)
        switchKey(v_addr);
      delay(33);
    }
    m_lineState = 0x00;
  }
}
//----------------------------------

//----------------------------------
void toogleLineOn()
{
  if(m_lineState != 0xFF) {
    m_devices = 0;
    wire_manager.reset_search();
    while(wire_manager.search(v_addr)) {
      m_devices++;
      if(switchKey(v_addr) == 0x00)
        switchKey(v_addr);
      delay(33);
    }
    m_lineState = 0xFF; 
  }
}
//----------------------------------

//----------------------------------
void remapAddresses()
{
  Serial.println("Mapping was initiated...");  
  noInterrupts(); 
  int pos = 0;
  toogleLineOff();
  wire_manager.reset_search();
  while(wire_manager.search(v_addr)) {
    
    switchKey(v_addr);  // toggle LED ON
    Serial.print("Set LED position on line: ");
    while(true) {
      if(Serial.available()) {
        pos = Serial.parseInt();
        if((pos > 0) && (pos < 129)) { // overal 128 because v_addr contains 8 byte and ATMega328 have 1024 EEPROM bytes
          Serial.print(pos);
          Serial.println(" is ok");
          break;
        } else {
          Serial.print(pos);
          Serial.print(" invalid, set another: ");
        }  
      }      
    }
    pos = (pos - 1) * 8;   // shift address
    for(byte i = 0; i < 8; i++) {
      EEPROM.update(pos + i, v_addr[i]);
    }
    switchKey(v_addr);  // toogle LED OFF    
  }
 loadAddresses();
   
 interrupts();
 Serial.println("Mapping finished");  
}
//----------------------------------

//----------------------------------
void toogleForwardLightOn()
{
  toogleLineOff();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleBackwardLightOn()
{
  toogleLineOff(); // m_devices updates in toogleLineOff
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * (m_devices - 1 - j);     
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleForwardLightOff()
{
  toogleLineOn();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleBackwardLightOff()
{
  toogleLineOn(); // m_devices updates in toogleLineOff
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * (m_devices - 1 - j);     
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleOscillatingLightOn()
{
  toogleForwardLightOn();
  toogleBackwardLightOn();
} 
//----------------------------------

//----------------------------------
void toogleOscillatingLightOff()
{
  toogleForwardLightOff();
  toogleBackwardLightOff();
} 
//----------------------------------


//----------------------------------
void toogleEvenLight()
{
  if(m_lineState != 0x06) {
    toogleLineOff();
    for(byte i = 0; i < m_devices; i++) {
      if((i % 2) == 0) {
        m_curpos = i * 8;
        for(byte j = 0; j < 8; j++) {
          v_addr[j] = v_map[j + m_curpos];
        }
        switchKey(v_addr);
      }      
    }
  }
  m_lineState = 0x06;
}
//----------------------------------

//----------------------------------
void toogleOddLight()
{
  if(m_lineState != 0x07) {
    toogleLineOff();
    for(byte i = 0; i < m_devices; i++) {
      if((i % 2) == 1) {
        m_curpos = i * 8;
        for(byte j = 0; j < 8; j++) {
          v_addr[j] = v_map[j + m_curpos];
        }
        switchKey(v_addr);
      }      
    }
  }
  m_lineState = 0x07;  
}
//----------------------------------

//----------------------------------
void toogleEvenOddLight() {
  state = !state;
  if(state) 
    toogleEvenLight();
  else
    toogleOddLight();
  delay(m_delay);  
}
//----------------------------------

//----------------------------------
void toogleRandomLight()
{
    long randomKey = random(m_devices) * 8;
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[randomKey + i];
    }
    switchKey(v_addr);
    delay(m_delay);
    m_lineState = 0xAA;
}
//----------------------------------

//----------------------------------
void toogleTwoBackwardLight()  { 
  toogleLineOff();
  for(byte j = 0; j < m_devices + 1; j++) {
    m_curpos = 8 * (m_devices - 1 - j);     
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos + 8];
    }
    switchKey(v_addr);
    delay(m_delay);    
  }
}
//----------------------------------

void toogleTwoForwardLight()  { 
  toogleLineOff(); // m_devices updates in toogleLineOff
  for(byte j = 0; j < m_devices + 1; j++) {
    m_curpos = 8 * j;     
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos - 8];
    }
    switchKey(v_addr);
    delay(m_delay);    
  }
}

//--------------------------------
void toogleForwardStrip()
{
  toogleLineOff();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
  }
} 

//-------------------------------
void toogleBackwardStrip()
{
  toogleLineOff();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    delay(m_delay);
  }
}

//------------------------------

void lineBlinking()  {
  state = !state;
  if(state)
    toogleLineOn();
  else
    toogleLineOff();
  delay(m_delay);  
}

//------------------------------

void toogleCollapselight()  {
  toogleLineOff();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);
    m_curpos = 8 * (m_devices - 1 - j);    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = v_map[i + m_curpos];
    }
    switchKey(v_addr);    
    delay(m_delay);
  }
  
}


