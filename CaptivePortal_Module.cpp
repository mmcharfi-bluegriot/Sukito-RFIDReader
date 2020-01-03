#include "CaptivePortal_Module.h"

char c;
// Acces Point -> Captive Portal( comme pour les réseaux publiques )
String ssidAP = DEFAULT_AP_NAME;

char incoming_data;
String currentLine = "";

const byte DNS_PORT = 53; // Capture DNS requests on port 53
IPAddress apIP(111, 222, 3, 4);

int indexStart = 0, indexEnd = 0;

DNSServer dnsServer; // objet DNS pour dire que peut importe la requete elle dirige vers l'adresse IP specifié juste au dessus
WiFiServer server(80);

static String password_user = DEFAULT_MDP;
static String ssid_user = DEFAULT_SSID;
static int ssid_size;

String module_id = DEFAULT_READER_ID;

String get_ssid_user()
{
    return ssid_user;
}

String get_pwd_user()
{
    return password_user;
}

String get_id()
{
    return module_id;
}

void set_id(String data)
{
    module_id = data;
}

void set_ssid_user(String data)
{
    ssid_user = data;
}

void set_pwd_user(String data)
{
    password_user = data;
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

    return AP_INIT;
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
    Serial.print("fist cute: ");
    Serial.println(currentLine);
    // récupérer le premier champs : //
    indexStart = currentLine.lastIndexOf('=');
    indexEnd = currentLine.lastIndexOf('&');
    password_user = currentLine.substring(indexStart + 1, indexEnd);

    currentLine = currentLine.substring(0, indexStart);
    Serial.print("seconde cute ");
    Serial.println(currentLine);
    // récupérer le 2 champs : //
    indexStart = currentLine.lastIndexOf('=');
    indexEnd = currentLine.lastIndexOf('&');
    ssid_user = currentLine.substring(indexStart + 1, indexEnd);

    currentLine = currentLine.substring(0, indexStart);
    Serial.print("third cute");
    Serial.println(currentLine);
    // récupérer le 3 champs : //
    indexStart = currentLine.lastIndexOf('=');
    indexEnd = currentLine.lastIndexOf('&');
    module_id = currentLine.substring(indexStart + 1, indexEnd);

    ssid_size = strlen(ssid_user.c_str());

    Serial.print("ssid user");
    Serial.println(ssid_user);
    Serial.print("pwd user:");
    Serial.println(password_user);
    Serial.print("id: ");
    Serial.println(module_id);
    currentLine = "";
}

int handle_request(WiFiClient client)
{
    int REQUEST = 0; // 1 = GET, 2 = POST
    //currentLine = ""; // make a String to hold incoming data from the client

    while (1)
    {
        currentLine = "";
        c = 0;
        //traitement caractères
        while (c != '\n')
        {
            c = client.read();
            if (((c >= ASCII_SPACE) && (c < ASCII_MAX_VALUE) && (c != ASCII_DEL)) || ((c == ASCII_NL) || (c == ASCII_CR)))
            {
                Serial.print(c);

                if (c != '\r')
                {
                    currentLine += c;
                }
            }
            else
            { //caractére non interprétable
                Serial.print("\nData not usefull : ");
                Serial.println((int)c);
                break;
            }
        }

        if (currentLine.startsWith("GET"))
        {
            Serial.println("GET REQUEST");
            REQUEST = 1;
        }
        else if (currentLine.startsWith("POST"))
        {
            Serial.println("POST REQUEST");
            REQUEST = 2;
        }
        
        if (currentLine.length() == 0 && REQUEST == 1) //END of request GET
        {

            Serial.println("GET REQUEST :Ligne null");
            return 1;
        }
        if (currentLine.length() == 0 && REQUEST == 2) //END of request POST but empty
        {

            Serial.println("POST REQUEST :Ligne null");
            return 1;
        }

        if (currentLine.endsWith("SUBMIT=Save+Configuration") && REQUEST == 2) //END of request POST
        {
            Serial.println("\nPOST SUBMIT: New configuration");
            return 2;
        }
        if (currentLine.endsWith("SUBMIT=Continue") && REQUEST == 2)
        {
            Serial.println("\nPOST SUBMIT: Continue");
            return 3;
        }
    }
    Serial.print("REQUEST: ");
    Serial.println(REQUEST);
    return REQUEST;
}

int captive_portale_home(bool configured)
{
    dnsServer.processNextRequest();
    WiFiClient client = server.available(); // listen for incoming clients

    if (client)
    {                                 // if you get a client,
        Serial.println("New Client"); // print a message out the serial port
        while (client.connected())
        {                           // loop while the client's connected
            if (client.available()) //bytes to read from the client
            {
                switch (handle_request(client))
                {
                case 0: //ligne
                    client.stop();
                    return 0;
                case 1:
                    if (configured)
                    {
                        sendHTMLLoginPage_new_configuration(&client, ssid_user, module_id);
                    }
                    else
                    {
                        sendHTMLLoginPage_first_configuration(&client);
                    }
                    client.stop();
                    break;
                case 2:
                    client.stop();
                    parse_submit();
                    return 1;
                    break;
                case 3:
                    client.stop();
                    return 2;
                default:
                    break;
                }
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
    return 0;
}

connection_status_t connect_to_ssid(int nb_try)
{
    int cpt_try = 0;

    WiFi.begin(ssid_user.c_str(), password_user.c_str());
    while (WiFi.status() != WL_CONNECTED && cpt_try < nb_try)
    {
        delay(500);
        Serial.print(".");
        cpt_try++;
    }
    Serial.println(".");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Impossible de se connecter au réseau");
        return STA_ERROR;
    }
    else
    {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        return STA_CONNECTED;
    }
}

connection_status_t send_request(String id_tag)
{
    int cpt_try = 0;
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String Request_1 = "http://admin.sukito.fr/webservice/postjson/idtag/";
        String Request_2 = Request_1 + id_tag;
        String Request_3 = Request_2 + "/idreader/";
        String Request_4 = Request_3 + module_id;
        http.begin(Request_4); //Specify the URL
        int httpCode = 0;      //Make the request

        while (1)
        {
            httpCode = http.GET();
            if (httpCode == 200)
            { //Check for the returning code
                return HTTP_POST_SUCCESS;
            }
            else
            {
                cpt_try ++;
            }

            if(cpt_try > 4)
            {
               return HTTP_POST_FAILED;
            }
        }
        http.end(); //Free the resource
    }
    else
    {
        return STA_ERROR;
    }
}
