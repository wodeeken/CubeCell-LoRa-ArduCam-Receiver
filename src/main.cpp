#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "Constants.cpp"

// Lora definitions.
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 128 // Max payload size.

int16_t rssi,rxSize;
bool lora_idle = true;
bool WaitingForResponse = false;
bool bRxTimeout = false;
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
//char* rxpacket_trunc;
static RadioEvents_t RadioEvents;
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnTxDone(void);
void OnTxTimeout();
void OnRxTimeout();
Constants::ReceiverState CurrentReceiverState = Constants::Wait;
//const uint8_t 
void setup() {
    // Setup serial.
    Serial.begin(38400);
    Serial.flush();
    // Setup Radio.
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, false);
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
}



void loop()
{
  delay(100);
  switch (CurrentReceiverState)
  {
  case Constants::Wait:
   turnOffRGB();
    // Read in from serial.
    if(Serial.available() == 1){
        // First, read in.
        String input = Serial.readString();
         Serial.println("Echoing: " + input);
        delay(500);
        input.trim();
        if(input.compareTo(Constants::FromHost_Ping_Command) == 0){
            CurrentReceiverState = Constants::ReceiverPingRequest;
        }else if(input.compareTo(Constants::FromHost_Ping_Camera_Command) == 0){
            CurrentReceiverState = Constants::CameraPingRequest;
        
        }
      }
    break;
  case Constants::ReceiverPingRequest:
    Serial.flush();
    Serial.println(Constants::ToHost_Ping_Command_Response);
    // Change state back to wait.
    CurrentReceiverState = Constants::Wait;
  break;
  case Constants::CameraPingRequest:
    // Send message via radio.
    if(lora_idle){
      sprintf(txpacket, Constants::ToTrans_Ping_Camera_Command);
      lora_idle = false;
      turnOnRGB(COLOR_SEND,0);
      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
      // Wait 1/2 sec before entering Rx mode.
      while(!lora_idle)
        delay(10);
      turnOffRGB();
      lora_idle = false;
      Serial.println("into RX mode");
      WaitingForResponse = true;
      bRxTimeout = false;
      Radio.Rx(Constants::CameraPingTimeoutMS);
      // Set state to Wait.
      CurrentReceiverState= Constants::CameraPingWait;
    }else 
      // Stay in PingRequestState until we can send the ping.
    
  break;
  case Constants::CameraPingWait:
    // Enter into RX mode.
    // Check if message has been received back, or timeout has been hit.
    if(!WaitingForResponse)
    {
      if(bRxTimeout){
        Serial.println(Constants::ToHost_Ping_Camera_Timeout);
      }else{
        // Check the response. Convert to string for ease of comparison.
        String str = String(rxpacket);
        str.trim();
        if(str.compareTo(Constants::FromTrans_Ping_Camera_Response) == 0)
          Serial.println(Constants::ToHost_Ping_Camera_Response);
          else
            Serial.println("INVALID PING RESPONSE");
      }
      //Serial.println(Constants::ToHost_Ping_Camera_Timeout);
      // Set state back to wiat.
      CurrentReceiverState = Constants::Wait;
    }else{
      Serial.println("Still waiting for response or timeout.");
    }
   
  break;
  default:
    break;
  }
}
  void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
  {
      WaitingForResponse = false;
      bRxTimeout = false;
      rssi=rssi;
      rxSize=size;
      memcpy(rxpacket, payload, size );
      // Copy to trunced point for ease of comparison.
      //memcpy(rxpacket_trunc,payload,size );
      rxpacket[size]='\0';
      turnOnRGB(COLOR_RECEIVED,0);
      Radio.Sleep( );
      Serial.printf("\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
      lora_idle = true;
  }
  void OnTxDone( void )
  {
    turnOffRGB();
    Serial.println("TX done......");
    lora_idle = true;
  }

  void OnTxTimeout( void )
  {
    turnOffRGB();
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    lora_idle = true;
  }
  void OnRxTimeout( void )
  {
    turnOffRGB();
    Radio.Sleep( );
    WaitingForResponse = false;
    bRxTimeout = true;
    lora_idle = true;
  }
  
  


