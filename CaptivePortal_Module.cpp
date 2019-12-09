#include "CaptivePortal_Module.h"


char c;
// Acces Point -> Captive Portal( comme pour les réseaux publiques )
String ssidAP = DEFAULT_AP_NAME;

char incoming_data;
String currentLine = "";

const byte DNS_PORT = 53;       // Capture DNS requests on port 53
IPAddress apIP(111, 222, 3, 4); //172.217.28.1  Private network for server

int indexStart = 0, indexEnd = 0;

DNSServer dnsServer; // objet DNS pour dire que peut importe la requete elle dirige vers l'adresse IP specifié juste au dessus
WiFiServer server(80);
WiFiClient client;


String password_user;
String ssid_user;
int ssid_size;


String get_ssid_user(){
  return ssid_user;
}

String get_pwd_user(){
  return password_user;
}


connection_status_t init_AP_com()
{
    Serial.println(WiFi.getMode());
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
        Serial.println("Wifi reset");
        WiFi.disconnect(true);
    }

    WiFi.mode(WIFI_AP);
    WiFi.persistent(false);

    WiFi.softAP(ssidAP.c_str());

    while (!(WiFi.softAPIP() == apIP))
    { // Sinon plante de manière aléatoire !!
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    }
    dnsServer.start(DNS_PORT, "*", apIP); // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
    server.begin();
    Serial.println("AP is init");

    return AP_MODE_INIT;
}

void end_AP_com()
{
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    //server.end(); // impossible se reconnecter a l'AP si on ferme le serveur durant le programme
    dnsServer.stop();
}

void parse_submit()
{
    indexStart = currentLine.lastIndexOf('&'); // le dernier & represente la fin de la partie qui nous interesse
    currentLine = currentLine.substring(0, indexStart + 1);
    Serial.print("currentLine cut with last & : ");
    Serial.println(currentLine);
    // récupérer le premier champs : //
    indexStart = currentLine.indexOf('=');
    indexEnd = currentLine.indexOf('&');
    ssid_user = currentLine.substring(indexStart + 1, indexEnd);
    // récupérer le second champs : //
    indexStart = currentLine.lastIndexOf('=');
    indexEnd = currentLine.lastIndexOf('&');
    password_user = currentLine.substring(indexStart + 1, indexEnd);
    ssid_size = strlen(ssid_user.c_str());

    Serial.println(ssid_user);
    Serial.println(password_user);
    currentLine = "";
}

int REQUEST = 0; // 1 = GET, 2 = POST

int handle_request()
{
    char c;
    c = client.read();

    if (((c >= 0x20) && (c < 0xFE) && (c != 0x7F)) || ((c == 13) || (c == 10)))
    {
        Serial.print(c);
    }
    else //si caractére non utilisable on s'arrête
    {
        Serial.print("\nOUT : ");
        Serial.println((int)c);
        return 0;
    }

    if (c == '\n')
    {
        return 1;
    }
    else if (c != '\r' && c >= 0x20 && c < 0xFE && c != 0x7F)
    {
        return 2;
    }
    return -1;
}

int captive_portale_home()
{
    dnsServer.processNextRequest();
    client = server.available(); // listen for incoming clients
    if (client)
    {                                 // if you get a client,
        Serial.println("New Client"); // print a message out the serial port
        currentLine = "";             // make a String to hold incoming data from the client
        while (client.connected())
        {                           // loop while the client's connected
            if (client.available()) //bytes to read from the client
            {
                switch (handle_request())
                {
                case 0: //caractére non interprétable
                    client.stop();
                    currentLine = "";
                    break;
                case 1: //new line caractere
                    // Si on voit un \n et qu'en plus la ligne qui était en lecture est nul, c'est qu'on est à la fin de la requête GET
                    if (currentLine.length() == 0 && REQUEST == 0)
                    { // Fin de requete (seul cas ou on a une nouvelle ligne et rien d'écrit dans celle d'avant)
                        currentLine = "";
                        sendHTMLLoginPage(&client);
                        REQUEST = 0;
                        client.stop();
                        Serial.println("\nGET REQUEST : Client Disconnected.");
                        return 0;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                case 2:
                    // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                    break;
                default:
                    break;
                }
            }

            if (currentLine.endsWith("SUBMIT=Valider"))
            { // la ligne avec la requete POST est la dernière ligne
                sendHTMLReplyPage(&client);
                REQUEST = 0;
                client.stop();
                Serial.println("SUBMIT : Client Disconnected.");
            }
            if (currentLine.startsWith("POST"))
            {
                Serial.println("POST REQUEST");
                REQUEST = 1;
            }
            else
            {
                Serial.println("client not available");
            }
        }
        client.stop();
        Serial.println("Client Disconnected.");
    }
    currentLine = "";
    return -1;
}



void display_login(WiFiClient client) {
  sendHTMLLoginPage(&client);
}


int get_SSID_password(unsigned long timeout_ms) { 
    unsigned long start_time = millis();
    dnsServer.processNextRequest();
    WiFiClient client = server.available();   // listen for incoming clients
    if (client) {                             // if you get a client,
        Serial.println("New Client");  // print a message out the serial port
        currentLine = "";                     // make a String to hold incoming data from the client

        while (client.connected()) {          // loop while the client's connected
            if (client.available()) {         // if there's bytes to read from the client,
                c = client.read();            // read a byte, then
                if( ((c >= 0x20) && (c < 0xFE) && (c != 0x7F)) || ((c == 13) || (c == 10))){
                    Serial.print(c);
                }
                else{
                    Serial.print("\nOUT : ");
                    Serial.println((int)c);
                    client.stop();
                    currentLine = "";
                    return 0;
                }
                if (c == '\n') {              // if the byte is a newline character
                    // Si on voit un \n et qu'en plus la ligne qui était en lecture est nul, c'est qu'on est à la fin de la requête GET
                    if (currentLine.length() == 0 && REQUEST == 0) { // Fin de requete (seul cas ou on a une nouvelle ligne et rien d'écrit dans celle d'avant)
                        //display_login(client);
                        currentLine = "";
                        sendHTMLLoginPage(&client);
                        REQUEST = 0;
                        client.stop();
                        Serial.println("GET REQUEST : Client Disconnected.");
                        return 0;
                    }
                    else {    // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r' && c >= 0x20 && c < 0xFE && c != 0x7F) {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }
                if (currentLine.endsWith("SUBMIT=Valider")) { // la ligne avec la requete POST est la dernière ligne
                    sendHTMLReplyPage(&client);
                    //display_login(client);
                    REQUEST = 0;
                    client.stop();
                    Serial.println("SUBMIT : Client Disconnected.");
                    return 1;
                }
                if (currentLine.startsWith("POST")) {
                    Serial.println("POST REQUEST");
                    REQUEST = 1;
                }
            }
            else{
                Serial.println("client not available");
            }
            if(((millis() - start_time) > timeout_ms) && (timeout_ms != 0)){
                Serial.println("Timeout");
                return -1;
            }
        }
        client.stop();
        Serial.println("Client Disconnected.");
        //return 0;
    }
    currentLine = "";
    return -1;
}

int get_SSID_password_try(unsigned long timeout_ms){
    REQUEST = 0;
    unsigned long start_time = millis();
    while(1){
        if(get_SSID_password(timeout_ms) == 1){
            Serial.println("ssid et pwd validé");
            return 1;
        }
        if(((millis() - start_time) > timeout_ms) && (timeout_ms != 0)){
            Serial.println("Timeout");
            return -1;
        }
    }
}


connection_status_t connect_to_ssid(){
  int cpt_try = 0;
  String ssid = get_ssid_user();
  String pwd = get_pwd_user();
  Serial.print("SSID:");
  Serial.println(ssid);

  Serial.print("PWD:");
  Serial.println(pwd);

  WiFi.begin(ssid.c_str(), pwd.c_str());
  while (WiFi.status() != WL_CONNECTED && cpt_try < 5) {
      delay(500);
      Serial.print(".");
      cpt_try++;
  }
  Serial.println(".");
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Impossible de se connecter au réseau");
    return STA_MODE_ERROR;
  }
  else{
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return STA_MODE;
  }
}


connection_status_t send_request(String id_tag, int id_lecteur_rfid)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String Request_1 = "http://admin.sukito.fr/webservice/postjson/idtag/";
    String Request_2 = Request_1 + id_tag;
    String Request_3 = Request_2 +  "/idreader/";
    String Request_4 = Request_3 +  id_lecteur_rfid;
    http.begin(Request_4); //Specify the URL
    int httpCode = http.GET(); //Make the request

    if (httpCode == 200) { //Check for the returning code
      return HTTP_POST_SUCCESS;
    }
    else {
      return HTTP_POST_FAILED;
    }
    
    http.end(); //Free the resources
    
  } else {
    return STA_MODE_ERROR;
  }

}
