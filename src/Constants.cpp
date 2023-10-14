#include "Arduino.h"
struct Constants{
    
    static constexpr char* FromHost_Ping_Command = "<PING>";
    static constexpr char* ToHost_Ping_Command_Response = "<PING_RESPONSE>";
    static constexpr char* FromHost_Ping_Camera_Command = "<PING_CAMERA>";
    static constexpr char* ToHost_Ping_Camera_Timeout = "<PING_CAMERA_TIMEOUT>";
    static constexpr char* ToHost_Ping_Camera_Response = "<PING_CAMERA_RESPONSE>";
    static constexpr char* ToTrans_Ping_Camera_Command = "<PING_CAMERA>";
    static constexpr char* FromTrans_Ping_Camera_Response = "<PING_CAMERA_RESPONSE>";
    static const int CameraPingTimeoutMS = 3000;

    enum ReceiverState{
    Wait, // Doing nothing, waiting for commands over serial.
    ReceiverPingRequest, // Ping command over serial received. Respond back.
    CameraPingRequest, // Camera ping command over serial, attempt to send message through radio and then enter receive state.
    CameraPingWait, // Ping send to camera, now waiting for response or timeout.
    };
};
