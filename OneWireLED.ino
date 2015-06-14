#include <OneWire.h>

#define DIGITAL_PIN_NAME 2
#define SWITCH_KEY_ROM_COMMAND 0x55

OneWire wire_manager(DIGITAL_PIN_NAME);
byte v_addr[8];
unsigned int devices_counter = 0;

void setup()
{
  Serial.begin(115200);  
}

void loop()
{    
  if( !wire_manager.search(v_addr) )
  {
    Serial.println("No more addresses");
    wire_manager.reset_search();
    delay(100);
    return;  
  }
  
  Serial.print(devices_counter++);
  Serial.print(" device address: ");
  for(byte i = 0; i < 8; i++)
  {
    Serial.print(v_addr[i], HEX);
    Serial.print(" ");
  }
   
 if( OneWire::crc8(v_addr, 7) != v_addr[7])
  {
    Serial.println("CRC is not VALID");
    return;
  }
  Serial.print("\n");
  
  wire_manager.reset();
  wire_manager.select(v_addr);
  wire_manager.write(SWITCH_KEY_ROM_COMMAND);
  delay(200);
}
