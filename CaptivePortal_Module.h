#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include <DNSServer.h>
#include "config.h"
#include "html_page.h"

#define DEFAULT_AP_NAME "Sukito_AP"
#define DEFAULT_SSID "BLUEGRioT"
#define DEFAULT_MDP "bluegri2t"
#define DEFAULT_READER_ID "0"

//ASCII CHARACTERE
#define ASCII_SPACE 0x20
#define ASCII_DEL 0x7F
#define ASCII_NL 0x0A
#define ASCII_CR 0x0D
#define ASCII_MAX_VALUE 0xFF


typedef enum {
    AP_INIT,
    AP_OK,
    AP_ERROR,
    STA_CONNECTED,
    STA_DISCONNECTED,
    STA_ERROR,
    HTTP_POST_FAILED,
    HTTP_POST_SUCCESS,
} connection_status_t;

String get_ssid_user();
String get_pwd_user();
String get_id();
void set_ssid_user(String data);
void set_pwd_user(String data);

connection_status_t init_AP_com();

void end_AP_com();
void parse_submit();
int handle_request();
int captive_portale_home(unsigned long timeout_ms, bool configured);

void display_login(WiFiClient client);

connection_status_t connect_to_ssid(int cpt_try);
connection_status_t send_request(String id_tag);
