#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include <DNSServer.h>
#include "config.h"
#include "html_page.h"

#define DEFAULT_AP_NAME "Sukito_AP"
#define DEFAULT_SSID "BLUEGRioT2"
#define DEFAULT_MDP "bluegri2t"


typedef enum {
    AP_MODE_INIT,
    AP_MODE,
    AP_MODE_ERROR,
    STA_MODE,
    STA_MODE_ERROR,
    HTTP_POST_FAILED,
    HTTP_POST_SUCCESS,
} connection_status_t;

String get_ssid_user();
String get_pwd_user();
connection_status_t init_AP_com();
void end_AP_com();
void parse_submit();
int handle_request();
int captive_portale_home();

void display_login(WiFiClient client);
int get_SSID_password(unsigned long timeout_ms);
int get_SSID_password_try(unsigned long timeout_ms);


connection_status_t connect_to_ssid();
connection_status_t send_request(String id_tag, int id_lecteur_rfid);
