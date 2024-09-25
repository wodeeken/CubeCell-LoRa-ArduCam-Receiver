#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "Constants.cpp"
#include <Regexp.h>

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
int packetTimeouts;
int16_t rssi,rxSize;
bool lora_idle = true;
bool WaitingForResponse = false;
bool bRxTimeout = false;
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
char fullCameraImageData[6000];
//char* rxpacket_trunc;
static RadioEvents_t RadioEvents;
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnTxDone(void);
void OnTxTimeout();
void OnRxTimeout();
Constants::ReceiverState CurrentReceiverState = Constants::Wait;
// variables for packet transfer.
int currentPacketNumber;

//const uint8_t 
void setup() {
    packetTimeouts = 0;
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
  delay(10);
  String input;
  switch (CurrentReceiverState)
  {
  case Constants::Wait:
   turnOffRGB();
    // First, read in.
    input = Serial.readString();
    //Serial.println("Echoing: " + input);
    if(input != ""){
      turnOnRGB(COLOR_JOINED, 10);
      input.trim();
      if(input.compareTo(Constants::FromHost_Ping_Command) == 0){
          CurrentReceiverState = Constants::ReceiverPingRequest;
      }else if(input.compareTo(Constants::FromHost_Ping_Camera_Command) == 0){
          CurrentReceiverState = Constants::CameraPingRequest;
      
      }else if(input.compareTo(Constants::FromHost_Capture_Camera_Command) == 0)
        CurrentReceiverState = Constants::CameraCaptureRequest;
      else{
        // Parse the data packet we are requesting.
        MatchState ms;
        ms.Target((char*)input.c_str());
        char result = ms.Match(Constants::FromHost_Data_Transfer_Request);
        if(result == REGEXP_MATCHED){
          char packetNumberLocation = ms.Match("%d+");
          if(packetNumberLocation == REGEXP_MATCHED){
              int packetNumber = input.substring(ms.MatchStart, ms.MatchStart + ms.MatchLength).toInt();
              currentPacketNumber = packetNumber;
              CurrentReceiverState = Constants::DataTransferRequest;
          }else{
            Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "1");
          }
        }else{
          Serial.println(String(Constants::ToHost_Data_Transfer_Error) + 2);
        }
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
      //Serial.println("into RX mode");
      WaitingForResponse = true;
      bRxTimeout = false;
      Radio.Rx(Constants::CameraPingTimeoutMS);
      // Set state to Wait.
      CurrentReceiverState= Constants::CameraPingWait;
    }else ;
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
          else;
      }
      // Set state back to wiat.
      CurrentReceiverState = Constants::Wait;
    }else{
    }
   
  break;
  case Constants::CameraCaptureRequest:
  // Send message via radio.
    if(lora_idle){
      sprintf(txpacket, Constants::ToTrans_Capture_Camera_Command);
      lora_idle = false;
      turnOnRGB(COLOR_SEND,0);
      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
      // Wait 1/2 sec before entering Rx mode.
      while(!lora_idle)
        delay(10);
      turnOffRGB();
      lora_idle = false;
      //Serial.println("into RX mode");
      WaitingForResponse = true;
      bRxTimeout = false;
      Radio.Rx(Constants::CameraCaptureTimeoutMS);
      // Set state to Wait.
      CurrentReceiverState= Constants::CameraCaptureWait;
    }else; 
      // Stay in CameraCaptureRequest until we can send the ping.
  break;
  case Constants::CameraCaptureWait:
  // Enter into RX mode.
    // Check if message has been received back, or timeout has been hit.
    if(!WaitingForResponse)
    {
      if(bRxTimeout){
        Serial.println(Constants::ToHost_Capture_Camera_Timeout);
        CurrentReceiverState = Constants::Wait;
      }else{
        // Check the response. Convert to string for ease of comparison.
        String str = String(rxpacket);
        // use regex
        MatchState ms;
        ms.Target(rxpacket);
        char result = ms.Match(Constants::FromTrans_Capture_Camera_Response);
        if(result == REGEXP_MATCHED){
          char packetNumberLocation = ms.Match("%d+");
          if(packetNumberLocation == REGEXP_MATCHED){
            
            int packetNumber = str.substring(ms.MatchStart, ms.MatchStart + ms.MatchLength).toInt();

            if(packetNumber > 0){
              // Write back to host.
              
              String toHostResponse = String(Constants::ToHost_Capture_Camera_Response);
              toHostResponse.replace("PACKETS", String(packetNumber));
              Serial.println(toHostResponse);
              

              CurrentReceiverState = Constants::Wait;
            }else{
              
              Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "3");
              CurrentReceiverState = Constants::Wait;
            }
          }else{
            Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "4");
            CurrentReceiverState = Constants::Wait;
          }
          
          
        }
        else{
          Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "5");
          CurrentReceiverState = Constants::Wait;
        }
            
      }
      
    }else{
    }
  break;
  case Constants::DataTransferRequest:
      // // Send to transmitter begin transfer request for the current packet.
      if(lora_idle){
        String txPacketString = String(Constants::ToTrans_Data_Transfer_Command);
        txPacketString.replace("PACKET_NUM", String(currentPacketNumber));
        sprintf(txpacket, txPacketString.c_str());
        lora_idle = false;
        turnOnRGB(COLOR_SEND,0);
        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
        // Wait 1/2 sec before entering Rx mode.
        while(!lora_idle)
          delay(10);
        turnOffRGB();
        lora_idle = false;
        //Serial.println("into RX mode");
        WaitingForResponse = true;
        bRxTimeout = false;
        Radio.Rx(Constants::DataTransferTimeoutMS);
        // Set state to Wait.
        CurrentReceiverState= Constants::DataTransferWait;
    }

    break;
  case Constants::DataTransferWait:
    if(!WaitingForResponse)
      {
        if(bRxTimeout){
          // Allow 20 timeouts each transfer process
          packetTimeouts++;
          if(packetTimeouts >= 20){
            Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "6");
            CurrentReceiverState = Constants::Wait;
          }else{
            CurrentReceiverState = Constants::DataTransferRequest;
          }
          
        }else{
          // Check the response. Convert to string for ease of comparison.
          String str = String(rxpacket);
          MatchState ms;
          ms.Target(rxpacket);
          char result = ms.Match(Constants::FromTrans_Data_Transfer_Response);
          if(result > 0){
            char packetNumberLocation = ms.Match("%d+");
            if(packetNumberLocation == REGEXP_MATCHED){
              int packetNumber = str.substring(ms.MatchStart, ms.MatchStart + ms.MatchLength).toInt();
              if(packetNumber == currentPacketNumber){
                packetTimeouts = 0;
                for(int i = 0; i < sizeof(rxpacket); i++){
                  Serial.write(rxpacket[i]);
                }
                
                // Enter back into Wait.
                CurrentReceiverState = Constants::Wait;
              }else{
                Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "7");
                CurrentReceiverState = Constants::Wait;
              }
            }else{
              Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "8");
              CurrentReceiverState = Constants::Wait;
            }
            
            
          }
          else{
            Serial.println(String(Constants::ToHost_Data_Transfer_Error) + "9" + rxpacket);
            CurrentReceiverState = Constants::Wait;
          }
              
        }
        
      }else{
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
      rxpacket[size]='\0';
      turnOnRGB(COLOR_RECEIVED,0);
      lora_idle = true;
  }
  void OnTxDone( void )
  {
    turnOffRGB();
    lora_idle = true;
  }

  void OnTxTimeout( void )
  {
    turnOffRGB();
    lora_idle = true;
  }
  void OnRxTimeout( void )
  {
    turnOffRGB();
    WaitingForResponse = false;
    bRxTimeout = true;
    lora_idle = true;
  }
  
  


