#include <ESP8266WiFi.h>
#include <FS.h>

String gl_ssid;
String gl_psw;
const char * ssid_fname = "/config/ssid.txt";
const char * pwd_fname = "/config/passwd.txt";
const int time_out = 30;
int port = 8086;
WiFiServer server(port);

void setup() {
    Serial.begin(115200);
    pinMode(16, OUTPUT);
    WiFi.mode(WIFI_STA);
    Serial.println("Connecting to Wifi");
    if (file_exists()) {
        gl_ssid = read_from_file(ssid_fname);
        gl_psw = read_from_file(pwd_fname);
    }
    if (!gl_ssid.isEmpty() && !gl_psw.isEmpty()) {
        gl_ssid.trim();
        gl_psw.trim();
        wifi_init(gl_ssid.c_str(), gl_psw.c_str());
        Serial.println("TCP uploaded WiFi init.");
    } else {
        wifi_init("zbzju", "Zb3928255");
        Serial.println("General WiFi init.");
    }
}

void loop() {
    processing();
}

void wifi_init(const char *ssid, const char *password) {
    WiFi.begin(ssid, password);
    unsigned long connection_count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("Connecting ssid: ");
        Serial.print(ssid);
        Serial.println("");
        Serial.print("Password: ");
        Serial.print(password);
        Serial.println("");
        delay(500);
        connection_count += 1;
        if (connection_count == time_out) {
            wifi_init("zbzju", "Zb3928255");
        }
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("port: ");
    Serial.println(port);
    server.begin();
    server.setNoDelay(true);
}

void processing() {
    WiFiClient client = server.available();
    if (client) {
        if(client.connected())
        {
            Serial.print("Client Connected: ");
            Serial.print(client.remoteIP());
            Serial.print(":");
            Serial.print(client.remotePort());
            Serial.println("");
        }
        String rec = client.readStringUntil('\n');
        if (rec == "STD") {
            client.write("Disconnected.");
            client.stop();
            Serial.println("Client disconnected");
        } else {
            if (rec == "RELAYON") {
                digitalWrite(16, HIGH);
                client.write("PIN16HIGH\n");
                Serial.println("PIN16HIGH");
            } else if (rec == "RELAYOFF") {
                digitalWrite(16, LOW);
                client.write("PIN16LOW\n");
                Serial.println("PIN16LOW");
            } else if (rec == "SETTINGSCLEAR") {
                remove_files();
                client.write("Config files clear successed.\n");
                Serial.println("Config files clear successed.");
            } else if (rec == "REBOOT") {
                Serial.println("Device rebooting...");
                ESP.restart();
            } else {
                tcp_update_wifi_config(rec);
                client.write("SSID, PASSWD saved seccussed.");
            }
            Serial.print("received data = ");
            Serial.println(rec);
        }
    }
}

void tcp_update_wifi_config(String command) {
    if (!command.isEmpty() && command.startsWith("SETUP+")) {
        int plus_index = command.lastIndexOf('+');
        int un_index = command.lastIndexOf('#');
        String tmp_ssid = command.substring(plus_index + 1, un_index);
        String tmp_psw = command.substring(un_index + 1);
        write_to_file(ssid_fname, tmp_ssid);
        Serial.print("WIFI SSID changed to: ");
        Serial.print(tmp_ssid);
        Serial.println("");
        write_to_file(pwd_fname, tmp_psw);
        Serial.print("WIFI PWD changed to: ");
        Serial.print(tmp_psw);
        Serial.println("");
    }
}

void write_to_file(const char * f_name, String content) {
    if (SPIFFS.begin()) {
        Serial.println("FS boot successed.");
    } else {
        Serial.println("FS boot failed.");
    }
    File f_bundle = SPIFFS.open(f_name, "w");
    f_bundle.println(content);
    f_bundle.flush();
    f_bundle.close();
}

String read_from_file(const char * f_name) {
    if (SPIFFS.begin()) {
        Serial.println("FS boot successed.");
    } else {
        Serial.println("FS boot failed.");
    }
    File f_bundle = SPIFFS.open(f_name, "r");
    String tmp_str = f_bundle.readStringUntil('\n');
    Serial.print("Content = ");
    Serial.print(tmp_str);
    Serial.println("");
    f_bundle.close();
    return tmp_str;
}

void remove_files() {
    if (SPIFFS.format()) {
        Serial.println("FS formated seccussed.");
    } else {
        Serial.println("FS formated failed.");
    }
}

bool file_exists() {
    if (SPIFFS.exists(ssid_fname) && SPIFFS.exists(pwd_fname)) {
        Serial.print(ssid_fname);
        Serial.print("\t");
        Serial.print(pwd_fname);
        Serial.print("Files exists.");
        return true;
    } else {
        Serial.print(ssid_fname);
        Serial.print("\t");
        Serial.print(pwd_fname);
        Serial.print("Files doesnt exists.");
    }
}