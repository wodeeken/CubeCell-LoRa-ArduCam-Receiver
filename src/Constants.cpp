#include "Arduino.h"
struct Constants{
    
    static constexpr char* FromHost_Ping_Command = "<PING>";
    static constexpr char* ToHost_Ping_Command_Response = "<PING_RESPONSE>";
    static constexpr char* FromHost_Ping_Camera_Command = "<PING_CAMERA>";
    static constexpr char* ToHost_Ping_Camera_Timeout = "<PING_CAMERA_TIMEOUT>";
    static constexpr char* ToHost_Ping_Camera_Response = "<PING_CAMERA_RESPONSE>";
    static constexpr char* ToTrans_Ping_Camera_Command = "<PING_CAMERA>";
    static constexpr char* FromTrans_Ping_Camera_Response = "<PING_CAMERA_RESPONSE>";
    static constexpr char* FromHost_Capture_Camera_Command = "<CAPTURE_CAMERA>";
    static constexpr char* ToTrans_Capture_Camera_Command = "<CAPTURE_CAMERA_REQUEST>";
    static constexpr char* FromTrans_Capture_Camera_Response = "<CAPTURE_CAMERA_RESPONSE {%d+}>";
    static constexpr char* ToHost_Capture_Camera_Response = "<CAPTURE_CAMERA_RESPONSE {PACKETS}>";
    static constexpr char* ToHost_Capture_Camera_Timeout = "<CAPTURE_CAMERA_TIMEOUT>";
    static constexpr char* FromHost_Data_Transfer_Request = "<DATA_TRANSFER_REQUEST {%d+}>";
    static constexpr char* ToTrans_Data_Transfer_Command = "<DATA_TRANSFER_REQUEST {PACKET_NUM}>";
    static constexpr char* FromTrans_Data_Transfer_Response = "^<DATA_TRANSFER_RESPONSE {%d+}>";
    static constexpr char* ToHost_Data_Transfer_Response_Header = "<DATA_TRANSFER_RESPONSE {PACKET_NUM}>";
    static constexpr char* ToHost_Data_Transfer_Response_Footer = "</DATA_TRANSFER_RESPONSE>";
    static constexpr char* ToHost_Data_Transfer_Error = "<DATA_TRANSFER_ERROR>";
    static constexpr char* FromTrans_Camera_Malfunction = "<CAMERA_ERROR>";
    static constexpr char* ToRec_Camera_Malfunction = "<CAMERA_ERROR>";
    static const int CameraPingTimeoutMS = 3000;
    static const int CameraCaptureTimeoutMS = 7000;
    static const int DataTransferTimeoutMS = 5000;
    enum ReceiverState{
    Wait, // Doing nothing, waiting for commands over serial.
    ReceiverPingRequest, // Ping command over serial received. Respond back.
    CameraPingRequest, // Camera ping command over serial, attempt to send message through radio and then enter receive state.
    CameraPingWait, // Ping send to camera, now waiting for response or timeout.
    CameraCaptureRequest, // Camera capture request received over serial, attempt to send message through radio and then enter receive state.
    CameraCaptureWait, // Camera capture request sent to camera, now waiting for response or timeout.
    DataTransferRequest, // Camera capture response received with total packet number, now send request to begin data transfer for the current packet number.
    DataTransferWait // Request sent to camera for current packet number, waiting on response.
    };
};
