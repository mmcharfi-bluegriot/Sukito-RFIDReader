#include <AWS_IOT.h>
#include "CaptivePortal_Module.h"

AWS_IOT hornbill;

char HOST_ADDRESS[] = "a1ot600sii5oea-ats.iot.eu-west-3.amazonaws.com";
char CLIENT_ID[] = "4";
char TOPIC_NAME[] = "$aws/things/SUKITO_1/shadow/update";

int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

bool AWS_connection(int retry)
{
    int cpt_retry = 0;
    while (1)
    {
        if (hornbill.connect(HOST_ADDRESS, CLIENT_ID) == 0)
        {
            Serial.println("Connected to AWS");
            return true;
        }
        else
        {
            Serial.println("AWS connection failed, Check the HOST Address");
            cpt_retry++;
        }
        if (cpt_retry >= retry)
        {
            return false;
        }
    }
}

bool AWS_publish_RFID(bool state, int nb_epc, int rssi, int freq)
{
    int id = get_id().toInt();
    sprintf(TOPIC_NAME, "$aws/things/SUKITO_%d/shadow/update", id);
    sprintf(payload, "{\"RFID\" : %d,\"nb_epc\" : %d,\"rssi\" : %d,\"freq\" : %d}", state, nb_epc, rssi, freq);
    if (hornbill.publish(TOPIC_NAME, payload) == 0)
    {
        Serial.print("Publish Message:");
        Serial.println(payload);
        Serial.print("id:");
        Serial.println(id);
        return true;
    }
    else
    {
        Serial.println("Publish failed");
        return false;
    }
}
