#include <DeviceHiveEngine.h>
#include <Rainbowduino.h>

#define CMD_STATUS_SUCCESS      "Success"
#define CMD_STATUS_FAIL         "Failed"

#define ERR_OK                  "OK"
#define ERR_UNKNOWN_CMD         "Unknown command"
#define ERR_INVALID_VALUE       "Invalid data"
#define ERR_TIMEOUT             "Failed to retrieve command due to timeout"
#define ERR_CHECKSUM            "Invalid checksum"
#define ERR_UNREG               "Is not registered"

static const char *REG_INFO = "{"
        "id:\"b125698d-61bd-40d7-b65e-e1f86852a166\","
        "key:\"LED_qube\","
        "name:\"LED Qube\","
        "deviceClass:{"
            "name:\"LED_qube\","
            "version:\"1.0\"},"
        "equipment:[{code:\"qube\",name:\"qube\",type:\"LED_Qube\"}],"
        "commands:["
            "{intent:257,name:\"pixel\",params:{X:u8,Y:u8,Z:u8,R:u8,G:u8,B:u8}},"
            "{intent:258,name:\"pixels\",params:[{X:u8,Y:u8,Z:u8,R:u8,G:u8,B:u8}]},"
            "{intent:259,name:\"fill\",params:{R:u8,G:u8,B:u8}}"
            "],"
        "notifications:[]"
    "}";


/**
 * Write data into USART
 */
static int transmit_data(const uint8_t * buff, int len)
{
    return Serial.write(buff, len);
}

/**
 * Read characters from USART module
 */
static int receive_data(uint8_t * buff, int len)
{
    for (int i = 0; i < len; ++i)
    {
        do { /* nothing */ }
        while (!Serial.available());

        buff[i] = Serial.read();
    }
    return len;
}


/**
 * Handles registration request
 */
void do_reqreg_command(void)
{
    if (RecvAndValidateChecksum())
    {
        SendRegistration2Data(REG_INFO);
    }
    else
        SendNotificationData(0, CMD_STATUS_FAIL, ERR_CHECKSUM);
}


struct Pixel
{
    BYTE X;
    BYTE Y;
    BYTE Z;
    BYTE R;
    BYTE G;
    BYTE B;
};

//static int Pixel_size_check[(sizeof(Pixel)==6) ? 1 : -1];

/**
 * Handles PIXEL command
 */
void do_pixel_command(void)
{
    DWORD code = 0;
    if (sizeof(code) == RecvBytes((BYTE*)&code, sizeof(code)))
    {
        Pixel body;
        if (sizeof(body) == RecvBytes((BYTE*)&body, sizeof(body)))
        {
            if (RecvAndValidateChecksum())
            {
                Rb.setPixelZXY(body.Z, body.X, body.Y,
                    body.R, body.G, body.B);

                SendNotificationData(code, CMD_STATUS_SUCCESS, ERR_OK);
            }
            else
                SendNotificationData(code, CMD_STATUS_FAIL, ERR_CHECKSUM);
        }
        else
            SendNotificationData(code, CMD_STATUS_FAIL, ERR_INVALID_VALUE);
    }
    else
        SendNotificationData(0, CMD_STATUS_FAIL, ERR_TIMEOUT);
}


/**
 * Handles PIXELS command
 */
void do_pixels_command(void)
{
    DWORD code = 0;
    if (sizeof(code) == RecvBytes((BYTE*)&code, sizeof(code)))
    {
        // TODO: read body - array of pixels
        WORD i, count = 0;
        if (sizeof(count) == RecvBytes((BYTE*)&count, sizeof(count)))
        {
            for (i = 0; i < count; ++i)
            {
                Pixel body;
                if (sizeof(body) == RecvBytes((BYTE*)&body, sizeof(body)))
                {
                    // execute before checksum verification???
                    Rb.setPixelZXY(body.Z, body.X, body.Y,
                        body.R, body.G, body.B);
                }
                else
                    SendNotificationData(code, CMD_STATUS_FAIL, ERR_INVALID_VALUE);
            }

            if (RecvAndValidateChecksum())
            {
                SendNotificationData(code, CMD_STATUS_SUCCESS, ERR_OK);
            }
            else
                SendNotificationData(code, CMD_STATUS_FAIL, ERR_CHECKSUM);
        }
        else
            SendNotificationData(0, CMD_STATUS_FAIL, ERR_TIMEOUT);
    } else
        SendNotificationData(0, CMD_STATUS_FAIL, ERR_TIMEOUT);
}


/**
 * Handles FILL command
 */
void do_fill_command(void)
{
    DWORD code = 0;
    if (sizeof(code) == RecvBytes((BYTE*)&code, sizeof(code)))
    {
        BYTE body[3];
        if (sizeof(body) == RecvBytes(body, sizeof(body)))
        {
            if (RecvAndValidateChecksum())
            {
                const BYTE R = body[0];
                const BYTE G = body[1];
                const BYTE B = body[2];

                for (int x = 0; x < 4; ++x)
                    for (int y = 0; y < 4; ++y)
                        for (int z = 0; z < 4; ++z)
                {
                    Rb.setPixelZXY(z, x, y, R, G, B);
                }

                SendNotificationData(code, CMD_STATUS_SUCCESS, ERR_OK);
            }
            else
                SendNotificationData(code, CMD_STATUS_FAIL, ERR_CHECKSUM);
        }
        else
            SendNotificationData(code, CMD_STATUS_FAIL, ERR_INVALID_VALUE);
    }
    else
        SendNotificationData(0, CMD_STATUS_FAIL, ERR_TIMEOUT);
}


/**
Initializes the Arduino firmware.
*/
void setup(void)
{
    Rb.init(); // initialize Rainbowduino driver
    Rb.blankDisplay();

    Serial.begin(9600);
    InitializeDeviceHive(&transmit_data, &receive_data);

    SendRegistration2Data(REG_INFO);
}


/**
Loop procedure is called continuously.
*/
void loop(void)
{
    if (0 < Serial.available())
    {
        MessageHeader msg_hdr;
        if (ReceiveMessageHeader(&msg_hdr))
        {
            switch (msg_hdr.Intent)
            {
                case G2D_REQREG:
                    do_reqreg_command();
                    break;

                case 257:
                    do_pixel_command();
                    break;

                case 258:
                    do_pixels_command();
                    break;

                case 259:
                    do_fill_command();
                    break;

                default: // unknownw message, just skip it
                    SkipMessage(msg_hdr.Length);
                    SendNotificationData(0, CMD_STATUS_FAIL, ERR_UNKNOWN_CMD);
                    break;
            }
        }
    }
}
