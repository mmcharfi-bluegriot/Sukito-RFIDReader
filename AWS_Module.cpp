#include <AWS_IOT.h>
#include "CaptivePortal_Module.h"

AWS_IOT hornbill;

char HOST_ADDRESS[] = "a1ot600sii5oea-ats.iot.eu-west-3.amazonaws.com";
char CLIENT_ID[] = "1";
char TOPIC_NAME[] = "$aws/things/SUKITO_1/shadow/update";

int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void AWS_connection()
{
    if (hornbill.connect(HOST_ADDRESS, CLIENT_ID) == 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
    }
}

void AWS_publish_RFID_state(bool state)
{
    int id = get_id().toInt();
    sprintf(TOPIC_NAME, "$aws/things/SUKITO_%d/shadow/update", id);
    sprintf(payload, "{\"RFID\" : %d}", state);
    if (hornbill.publish(TOPIC_NAME, payload) == 0)
    {
        Serial.print("Publish Message:");
        Serial.println(payload);
    }
    else
    {
        Serial.println("Publish failed");
    }
}
