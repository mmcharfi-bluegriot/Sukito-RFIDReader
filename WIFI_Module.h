#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include "config.h"

typedef enum {
    AP_MODE,
    AP_MODE_ERROR,
    STA_MODE,
    STA_MODE_ERROR,
    HTTP_POST_FAILED,
    HTTP_POST_SUCCESS,
} connection_status_t;


String get_ssid_user();

connection_status_t init_AP_com();

void end_AP_com();

connection_status_t connect_to_ssid();

void parse_submit();

void display_login(WiFiClient client);

int get_SSID_password(unsigned long timeout_ms);

int get_SSID_password_try(unsigned long timeout_ms);

connection_status_t send_request(String id_tag, int id_lecteur_rfid);