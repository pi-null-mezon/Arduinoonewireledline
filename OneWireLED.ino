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
#define BUTTON_DELAY_US 2048
#define SERIAL_PORT_SPEED 9600
#define COMMANDS_LOOP_LENGTH 9
#define DO_NOTHING -1

OneWire wire_manager(WIRE_PIN);
byte v_addr[8];
byte m_devices = 0;
volatile int nextCommand = '0';
byte m_lineState = 0xFF;
int m_curpos;
unsigned int m_delay = 1; // in ms


//----------------------------------
void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(SERIAL_PORT_SPEED);
  attachInterrupt(0, enrollPressure, FALLING); // 0 interrupt for Arduino Uno linked with 2 GIO  
}
//----------------------------------

//----------------------------------
void loop()
{      
  if(Serial.available()) {
    nextCommand = Serial.read();  
  }

  m_delay = 1 + analogRead(ANALOG_PIN);
  
  switch(nextCommand) {
    case 's':
      remapAddresses();
      nextCommand = '3';
      break;
    case '0':
      toogleLineOff();
      break;
    case '1':
      toogleLineOn();
      break;
    case '2':
      toogleOverLine();
      break;
    case '3':
      toogleForwardLight();
      break;
    case '4':
      toogleBackwardLight();
      break;
    case '5':
      toogleOscillatingLight();
      break;
    case '6':
      toogleEvenLight();
      break;
    case '7':
      toogleOddLight();
      break;
    case '8':
      toogleRandomLight();
      break;      
  }
}
//----------------------------------

//----------------------------------
void enrollPressure()
{
  delayMicroseconds(BUTTON_DELAY_US);
  if(digitalRead(BUTTON_PIN) == LOW)
  nextCommand = (((nextCommand - '0') + 1) % COMMANDS_LOOP_LENGTH) + '0'; // + '0' because '0' read from Serial.read() have decimal value 48 in int 
}
//----------------------------------

//----------------------------------
/*boolean checkCRC(byte *ad)
{
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
  return wire_manager.read_bit(); // 0x00 means key toogled off and 0x01 means key toogled on      
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
    }
  }
  m_lineState = 0x00;  
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
    } 
  }
  m_lineState = 0xFF;  
}
//----------------------------------

//----------------------------------
void remapAddresses()
{
  Serial.println("Mapping procedure was initiated");  
  noInterrupts(); 
  int pos = 0;
  toogleLineOff();
  wire_manager.reset_search();
  while(wire_manager.search(v_addr)) {
    
    switchKey(v_addr);  // toggle LED ON
    Serial.print("Set position: ");
    while(true) {
      if(Serial.available()) {
        pos = Serial.parseInt();
        if((pos > 0) && (pos < 129)) { // overal 128 because v_addr contains 8 byte and ATMega328 have 1024 EEPROM bytes
          Serial.print(pos);
          Serial.println(" ok");
          break;
        } else {
          Serial.print(pos);
          Serial.println(" invalid, set another: ");
        }  
      }      
    }
    pos = (pos - 1) * 8;   // shift address
    for(byte i = 0; i < 8; i++) {
      EEPROM.update(pos + i, v_addr[i]);
    }
    switchKey(v_addr);  // toogle LED OFF    
  }
   
 interrupts();
 Serial.println("Success, ready to light");  
}
//----------------------------------

//----------------------------------
void toogleForwardLight()
{
  toogleLineOff();
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * j;    
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = EEPROM.read(i + m_curpos);
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleBackwardLight()
{
  toogleLineOff(); // m_devices updates in toogleLineOff
  for(byte j = 0; j < m_devices; j++) {
    m_curpos = 8 * (m_devices - 1 - j);     
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = EEPROM.read(i + m_curpos);
    }
    switchKey(v_addr);
    delay(m_delay);
    switchKey(v_addr);
  }
} 
//----------------------------------

//----------------------------------
void toogleOscillatingLight()
{
  toogleForwardLight();
  toogleBackwardLight();
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
          v_addr[j] = EEPROM.read(j + m_curpos);
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
          v_addr[j] = EEPROM.read(j + m_curpos);
        }
        switchKey(v_addr);
      }      
    }
  }
  m_lineState = 0x07;  
}
//----------------------------------

//----------------------------------
void toogleRandomLight()
{
    toogleLineOn();
    long randomKey = random(m_devices) * 8;
    for(byte i = 0; i < 8; i++) {
      v_addr[i] = EEPROM.read(randomKey + i);
    }
    switchKey(v_addr);
    delay(m_delay);
}
//----------------------------------

