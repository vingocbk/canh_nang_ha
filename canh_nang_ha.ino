#include "tubep_test_2.h"

void handleOk() {
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\"}");
}

void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
    ECHOLN("done!");
}


void GoUp(){
    if(clickbutton == false){
        server.send(200, "text/html", "{\"status\":\"success\"}");
    }
    
    ECHOLN("go_up");
    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L2, LOW);
    digitalWrite(R1, LOW);
    delay(200);
    
    digitalWrite(L1, HIGH);
    digitalWrite(R2, HIGH);
    delay(300);
    digitalWrite(PIN_LOA, LOW);
    forward = false;
    statusStop = false;
    
}

void GoDown(){
    if(clickbutton == false){
        server.send(200, "text/html", "{\"status\":\"success\"}");
    }
    ECHOLN("go_down!");
    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L1, LOW);
    digitalWrite(R2, LOW);
    delay(200);
    digitalWrite(L2, HIGH);
    digitalWrite(R1, HIGH);
    
    delay(300);
    digitalWrite(PIN_LOA, LOW);
    forward = true;
    statusStop = false;
}

void Stop(){
    if(clickbutton == false){
        server.send(200, "text/html", "{\"status\":\"success\"}");
    }
    digitalWrite(PIN_LOA, HIGH);
    digitalWrite(L1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(L2, LOW);
    digitalWrite(R1, LOW);
    ECHOLN("Stop");
    statusStop = true;
    delay(500);
    digitalWrite(PIN_LOA, LOW);
    //tickerSetMotor.stop();
}



void clearEeprom(){
    ECHOLN("clearing eeprom");
    for (int i = 0; i < EEPROM_WIFI_MAX_CLEAR; ++i){
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    ECHOLN("Done writing!");
}

void detachIP(String ip){
    String first_octetStr = "";
    String second_octetStr = "";
    String third_octetStr = "";
    String fourth_octetStr = "";
    for(int i = 0, j = 0; i <= sizeof(ip)+1; i++){
        char c = ip[i];
        if(c == '.'){
            j++;
            continue;
        }
        switch(j){
            case 0:
                first_octetStr += c;
                break;
            case 1:
                second_octetStr += c;
                break;
            case 2:
                third_octetStr += c;
                break;
            case 3:
                fourth_octetStr += c;
                break;
        }

    }

    first_octet = atoi(first_octetStr.c_str());
    second_octet = atoi(second_octetStr.c_str());
    third_octet = atoi(third_octetStr.c_str());
    fourth_octet = atoi(fourth_octetStr.c_str());
}

void ConfigMode(){
    StaticJsonBuffer<RESPONSE_LENGTH> jsonBuffer;
    ECHOLN(server.arg("plain"));
    JsonObject& rootData = jsonBuffer.parseObject(server.arg("plain"));
    ECHOLN("--------------");
    tickerSetApMode.stop();
    if (rootData.success()) {
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\"}");
        //server.stop();
        String nssid = rootData["ssid"];
        String npass = rootData["password"];
        String ip = rootData["set_ip"];

        detachIP(ip); 
        ECHO("Wifi new name: ");
        ECHOLN(nssid);
        ECHO("Wifi new password: ");
        ECHOLN(npass);
        ECHO("Wifi new IP: ");
        ECHO(first_octet);
        ECHO(".");
        ECHO(second_octet);
        ECHO(".");
        ECHO(third_octet);
        ECHO(".");
        ECHOLN(fourth_octet);
        if (connectToWifi(nssid, npass, ip)) {
            esid = nssid;
            epass = npass;
            StartNormalSever();
            Flag_Normal_Mode = true;
            return;
        }

        ECHOLN("Wrong wifi!!!");
        SetupConfigMode();
        StartConfigServer();
        return;
    }
    ECHOLN("Wrong data!!!");
}

void setupIP(){
    // config static IP
    IPAddress ip(first_octet, second_octet, third_octet, fourth_octet); // where xx is the desired IP Address
    IPAddress gateway(first_octet, second_octet, third_octet, 1); // set gateway to match your network
    IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
    WiFi.config(ip, gateway, subnet);
}

bool connectToWifi(String nssid, String npass, String ip) {
    ECHOLN("Open STA....");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);
    setupIP();
    //WiFi.begin(nssid.c_str(), npass.c_str());

    if (testWifi(nssid, npass)) {
        ECHOLN("clearing eeprom");
        for (int i = 0; i < EEPROM_WIFI_MAX_CLEAR; ++i){ 
            EEPROM.write(i, 0); 
        }
        ECHOLN("writing eeprom ssid:");
        ECHO("Wrote: ");
        for (int i = 0; i < nssid.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_SSID_START, nssid[i]);             
            ECHO(nssid[i]);
        }
        ECHOLN("");
        ECHOLN("writing eeprom pass:"); 
        ECHO("Wrote: ");
        for (int i = 0; i < npass.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_PASS_START, npass[i]);
            ECHO(npass[i]);
        }
        ECHOLN("");
        ECHOLN("writing eeprom IP:"); 
        ECHO("Wrote: ");
        for (int i = 0; i < ip.length(); ++i){
            EEPROM.write(i+EEPROM_WIFI_IP_START, ip[i]);             
            ECHO(ip[i]);
        }
        ECHOLN("");
        EEPROM.commit();
        ECHOLN("Done writing!");
        return true;
    }

    return false;
}

bool testWifi(String esid, String epass) {
    ECHO("Connecting to: ");
    ECHOLN(esid);
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    server.close();
    delay(1000);
    setupIP();      //cai dat ip theo quy dinh
    WiFi.mode(WIFI_STA);        //bat che do station
    WiFi.begin(esid.c_str(), epass.c_str());
    int c = 0;
    ECHOLN("Waiting for Wifi to connect");
    while (c < 20) {
        if (WiFi.status() == WL_CONNECTED) {
            ECHOLN("\rWifi connected!");
            ECHO("Local IP: ");
            ECHOLN(WiFi.localIP());
            digitalWrite(LEDTEST, LOW);
            return true;
        }
        delay(500);
        ECHO(".");
        c++;
        if(digitalRead(PIN_CONFIG) == LOW){
            break;
        }
    }
    ECHOLN("");
    ECHOLN("Connect timed out");
    return false;
}

void setLedApMode() {
    digitalWrite(LED_TEST_AP, !digitalRead(LED_TEST_AP));
}



String GetFullSSID() {
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    String macID;
    WiFi.mode(WIFI_AP);
    WiFi.softAPmacAddress(mac);
    macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    macID = SSID_PRE_AP_MODE + macID;
    ECHO("[Helper][getIdentify] Identify: ");
    ECHOLN(macID);
    return macID;
}

void checkButtonConfigClick(){
    if (digitalRead(PIN_CONFIG) == LOW && (ConfigTimeout + CONFIG_HOLD_TIME) <= millis()) { // Khi an nut
        ConfigTimeout = millis();
        //tickerSetMotor.attach(0.2, setLedApMode);  //every 0.2s
        Flag_Normal_Mode = false;
        tickerSetApMode.start();
        SetupConfigMode();
        StartConfigServer();
    } else if(digitalRead(PIN_CONFIG) == HIGH) {
        ConfigTimeout = millis();
    }
}


void SetupConfigMode(){
    ECHOLN("[WifiService][setupAP] Open AP....");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    server.close();
    delay(1000);
    WiFi.mode(WIFI_AP_STA);
    IPAddress APIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(APIP, gateway, subnet);
    String SSID_AP_MODE = GetFullSSID();
    WiFi.softAP(SSID_AP_MODE.c_str(), PASSWORD_AP_MODE);
    ECHOLN(SSID_AP_MODE);

    ECHOLN("[WifiService][setupAP] Softap is running!");
    IPAddress myIP = WiFi.softAPIP();
    ECHO("[WifiService][setupAP] IP address: ");
    ECHOLN(myIP);
}
void StartConfigServer(){    
    ECHOLN("[HttpServerH][startConfigServer] Begin create new server...");
//    server = new ESP8266WebServer(HTTP_PORT);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/cleareeprom", HTTP_GET, clearEeprom);
    server.on("/config", HTTP_POST, ConfigMode);
    server.on("/", HTTP_OPTIONS, handleOk);
    server.on("/cleareeprom", HTTP_OPTIONS, handleOk);
    server.on("/config", HTTP_OPTIONS, handleOk);
    server.begin();
    ECHOLN("[HttpServerH][startConfigServer] HTTP server started");
}



void SetupNomalMode(){
    SetupNetwork();
    if (WiFi.status() == WL_CONNECTED){
        StartNormalSever();
    }    
}


void SetupNetwork() {
    ECHOLN("Reading EEPROM ssid");
    esid = "";
    for (int i = EEPROM_WIFI_SSID_START; i < EEPROM_WIFI_SSID_END; ++i){
        esid += char(EEPROM.read(i));
    }
    ECHO("SSID: ");
    ECHOLN(esid);
    ECHOLN("Reading EEPROM pass");
    epass = "";
    for (int i = EEPROM_WIFI_PASS_START; i < EEPROM_WIFI_PASS_END; ++i){
        epass += char(EEPROM.read(i));
    }
    ECHO("PASS: ");
    ECHOLN(epass);
    ECHOLN("Reading EEPROM IP");
    eip = "";
    for (int i = EEPROM_WIFI_IP_START; i < EEPROM_WIFI_IP_END; ++i){
        eip += char(EEPROM.read(i));
    }
    ECHO("IP: ");
    ECHOLN(eip);
    detachIP(eip);  //tach ip thanh 4 kieu uint8_t
    testWifi(esid, epass);
}

void StartNormalSever(){
    server.on("/", HTTP_GET, handleRoot);
    server.on("/go_up", HTTP_GET, GoUp);
    server.on("/go_down", HTTP_GET, GoDown);
    server.on("/stop", HTTP_GET, Stop);
    server.on("/", HTTP_OPTIONS, handleOk);
    server.on("/go_up", HTTP_OPTIONS, handleOk);
    server.on("/go_down", HTTP_OPTIONS, handleOk);
    server.on("/stop", HTTP_OPTIONS, handleOk);
    server.begin();
    ECHOLN("HTTP server started");
}

void buttonClick(){  
    if(statusStop == false){
        Stop();
    }
    else if(forward == false ){
      GoDown();
    }else {
      GoUp();
    }
    delay(500);
}

void tickerupdate(){
    tickerSetApMode.update();
}

void setup() {
    pinMode(LED_TEST_AP, OUTPUT);
    pinMode(PIN_CONFIG, INPUT);
    pinMode(PIN_LOA, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(L1, OUTPUT);
    pinMode(L2, OUTPUT);
    pinMode(LEDTEST, OUTPUT);
    digitalWrite(L1, LOW);
    digitalWrite(L2, LOW);
    digitalWrite(R1, LOW);
    digitalWrite(R2, LOW);
    digitalWrite(LEDTEST, HIGH);
    // pinMode(PIN_LOA, INPUT_PULLUP);
    Serial.begin(115200);
    EEPROM.begin(512);
    SetupNomalMode();     //khi hoat dong binh thuong

    //external interrupt doc tin hieu encoder

}


void loop() {
    if (Flag_Normal_Mode == true && WiFi.status() != WL_CONNECTED){
        digitalWrite(LEDTEST, HIGH);
        if (testWifi(esid, epass)){
            StartNormalSever();
        } 
    } 
//    if(digitalRead(BUTTON) == LOW){
//        clickbutton = true;
//        buttonClick();
//        clickbutton = false;
//    }
    checkButtonConfigClick();
    tickerupdate();
    server.handleClient();
}
