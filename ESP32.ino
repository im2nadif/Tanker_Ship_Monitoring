#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "WiFiClientSecure.h"

#define TRIG_PIN 13
#define ECHO_PIN 12

RTC_DS1307 rtc;
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 20, 4);

float duration_us, distance_cm;
String xtime, stime, slevel, slocation;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char *mqtt_broker = "0.tcp.ap.ngrok.io";
const int mqtt_port     = 19436;
const char *mqtt_user   = "tigapagi";
const char *mqtt_pass   = "pwdTigaPagi";
char *device_id  = "91026";
char *data = "/data";

// https://drive.google.com/drive/folders/1J7iK9Zogsrj8LtZMf8wWa9Z7bF75cBK4?usp=share_link
// LINK CERT
const char* CA_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDDTCCAfWgAwIBAgIUO0n6qYw2n9P/txK+FoQJUHpoqdwwDQYJKoZIhvcNAQEL\n" \
"BQAwFjEUMBIGA1UEAwwLVGlnYVBhZ2kuY2MwHhcNMjIxMjIxMjA1ODQyWhcNMjMx\n" \
"MjIxMjA1ODQyWjAWMRQwEgYDVQQDDAtUaWdhUGFnaS5jYzCCASIwDQYJKoZIhvcN\n" \
"AQEBBQADggEPADCCAQoCggEBANUB06ZUAa4uTKaTCvGZsRcPCq+d9eSPssuiYM3X\n" \
"wJ4C8vqt75IVXKFNowfvB4gmY9DShhPIeCUMmSVJpdVYFZYmmuwyaA4chMIJmfMe\n" \
"ZoQ3ywjhSTJaNqx7FqSCULOt21ttcb9qsrLn8MGyctB6kX8O/6s3S8R5EMFcAWx4\n" \
"R12SONVwCMoA5qbibq4wug5QbfSI7Q73BoDezDQmC59pClziqOGrlPP4NLu9g/fA\n" \
"1Wv5iZohCTjUNFe2oyxPxKQ7lWfO02MIhp9YN54X3cm/TvuZKjziHdJbMtv27KZi\n" \
"SSRpdSv/iDoB7o7/H6drgt1sgGRryKWM7HOh/0RHIzpdBMcCAwEAAaNTMFEwHQYD\n" \
"VR0OBBYEFEkLAX2mQgCCuJduKyqvYyxFKbOKMB8GA1UdIwQYMBaAFEkLAX2mQgCC\n" \
"uJduKyqvYyxFKbOKMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEB\n" \
"AHmlzCY1yNK1QesivRNh4i+IkLdEIjKwQjvC5kOeZeYE9iZGqw2tgQza2SfvDgdj\n" \
"HlNznrfU6uyWoqwvnKYkxDIcT7CPPgHrtjH5ccUkJPh5JSDE7vg8J9bx4aGL7Usp\n" \
"g1ZPaa5ef90U+iqN6OU4YPemnqSCT8VugbyVmf7XjY2zrv2hfOkBO7IEapVlQhif\n" \
"mT3JMaalsO0ei068u1KWnq36xxtVAYEFRhxR5KqUUDFwFEaugpsjw0rb5xtHJRdL\n" \
"HB/GH8xve0gzXleuY6O0aZ1IiRBOjHYNqiQg4EYvAyjhqPFEz7S1Ri8TdDd+IttN\n" \
"QoFq4FLBNxqKrR+rwR56EeM=\n" \
"-----END CERTIFICATE-----";

const char* ESP_CA_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIICsTCCAZkCFCD5dW+boPuhWvaRDJkVmKBNhfTHMA0GCSqGSIb3DQEBCwUAMBYx\n" \
"FDASBgNVBAMMC1RpZ2FQYWdpLmNjMB4XDTIyMTIyMTIxMDAzMFoXDTIzMTIyMTIx\n" \
"MDAzMFowFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
"AQ8AMIIBCgKCAQEAiRIMezDHd2nr2tp0elGqe72ROgIsT44rZwFXlXBtW9WvGsBS\n" \
"qh+bM8WpllXiR9cVxDl0OItdOTINhobQ5OKPMjBtuMC7S2XfPkvGl8YpPpGolZfm\n" \
"cb6iZi1ka+OBVGKeQ69hCHAE55tihNqLSY9oPy3Cc6cDx/A+iUcvqz1mpcsjl+xY\n" \
"dx/jq+Jb2ni5TP60Bov3qkYgPIjCM8CCUseU6PMV6t0Gq7FVXB+H+fOTnU5ublKd\n" \
"IH4Lz1nBM1yjxCK3ssKYEGBbtN/SCy17k1DYNRrcHIDXYq25ZcEYvBYSaLgB29ah\n" \
"WvKWIblexlQA8Nh5qAq5VGBRFtm+cTv1dnjLvwIDAQABMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQABZXGSQLRNvs/3IKgawN9qu6x0ep7TOdzR5VHw8v3LhVsmcqgnjV4gJqBi\n" \
"HKEwAP0Re6MzhMTfmJO2n+wsNb1/bnOQUW41xfufZKTuzFIXE913yFMZLfLpB0X9\n" \
"RATNUa6qUObvy9pnQ+f/RNsxETTqSmz5oDOayK8do6o4EhVsgt1q8BjlT2cgzJsC\n" \
"rZxs03LumqMPO9DqwEjjxNPLit96mwoc1REdpIg7VTxfqPrKv4SDm9QSTzAzM5xF\n" \
"K4CAp70Ebg+EhpTWqhyRAU6qJR7wOu0hkW9+K4AUmI8ZPuEPPL6uiy6W2kKSRrJI\n" \
"5rC6uzKFjgFkcfcUEfMi8DtPH+yN\n" \
"-----END CERTIFICATE-----";

const char* ESP_RSA_key = \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCJEgx7MMd3aeva\n" \
"2nR6Uap7vZE6AixPjitnAVeVcG1b1a8awFKqH5szxamWVeJH1xXEOXQ4i105Mg2G\n" \
"htDk4o8yMG24wLtLZd8+S8aXxik+kaiVl+ZxvqJmLWRr44FUYp5Dr2EIcATnm2KE\n" \
"2otJj2g/LcJzpwPH8D6JRy+rPWalyyOX7Fh3H+Or4lvaeLlM/rQGi/eqRiA8iMIz\n" \
"wIJSx5To8xXq3QarsVVcH4f585OdTm5uUp0gfgvPWcEzXKPEIreywpgQYFu039IL\n" \
"LXuTUNg1GtwcgNdirbllwRi8FhJouAHb1qFa8pYhuV7GVADw2HmoCrlUYFEW2b5x\n" \
"O/V2eMu/AgMBAAECggEAJqnWp3EM8pwIEUWJkW8zvY+tRUWTpiUeDQIN6HM8Baly\n" \
"qQMSzjX4G1RoD6XOHCGJXD4wkDqPYzDBK+8fQVC+v5N1f6WtrwjfPb02GV2mCKYh\n" \
"fYDJWdBCErw/FUepFVpz9fekUTL3zzS/R2Y139v5tch8jw7PPDiCC0rvIUhCigUn\n" \
"HhBJK3nTkeJCUpYfeSZ8RlTGRTQmm4yNYUi4vMQU0sroz570l6Hnll5Nt0QHkMjf\n" \
"1XKfwl1KcWcv8LDDh7+6MCRFLtLjhSFVXeTlTGMC8IjUqRBKMEB8bNV1Dw/cCJcX\n" \
"yqgky4vVNglepxXr9g0mbXPrqHMIl+rYwfnI3lunYQKBgQC19SQnmZKwgyqU5g/q\n" \
"dxyP4En8Goy4qypRtM2y09SXvQBoDNKD9b6dBSjm71ARVGcBuAvLxlCksoqh0MPp\n" \
"iYsmRSDEXlktpNRhy2zHbNU4sG3ysvFHn4ahIIhRJHS7zbMq7B1NS52U5QNDv1ov\n" \
"X7S3fYRGuRrvW4IEezfN+BrYGQKBgQDA2O/rIOx2N3RxnBQjdXG6gM+rmf5tXFBU\n" \
"OEr/DuCTjWNwCoZsITzuH27i/kMHsC2846pCerTUAY+KS7yzy9DYeNCy8V6HxZWV\n" \
"GKQ3DfeTK/52OTeNUUhSRs1YjJ832yYR/TaNbIXEo4NiLHECrGfNbifSElti2qzi\n" \
"W+eWXHydlwKBgQCUkYemyc8rTr4K+9pNAFqS43ilXuw9sIyKUE2QUUQJbWoSiz2s\n" \
"P5AW12qxPP+sVrUQgz0mclPQVnO5ciM04Xh7lAAQNlUsXqSHATldj/B9LBlqDE/T\n" \
"wl6D23NShajG20sKpeW7Ah21wFQtEeXKPqPpGpRRxrxGafo8PHuxMn6AqQKBgQCl\n" \
"p7Qv3hBBXFGCZfhGnVQcSRoa5iVP+8RlFBYYEseG+CU60U+3ArY8NnQiOReLOyOO\n" \
"5mISySUcGFsdCQIH0ESOLvnXPM9fAbMEaC5nvG+21Eu5clzt36yoQHmUXvdGG9IQ\n" \
"EdsSkO43g94QLuzExyRDqyQ5p9soh3+K0jeVSc0h8QKBgBKXoHt4mwI9UQLW7V9A\n" \
"xWc43ng86FBMzIjPFd9m12OlKX/DK5v80ou/x4SgYSeu0yWlmsNsjN7aoEELcw8U\n" \
"8iLgUPGK1bzODeA919GOqWVU1v2843t6Y2pupw/vkwKCNsaDDpE1QG4rGEmXM3BA\n" \
"qEVnHxS977vH21dcgSvoXTyr\n" \
"-----END PRIVATE KEY-----";

long lastReconnectAttempt = 0;

WiFiClientSecure client;
PubSubClient mqtt_client(client); 


void setup() {
  Serial.begin(115200);
  
  LCD.init();
  LCD.backlight();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LCD.setCursor(0, 0);
    LCD.print("Connecting to WiFi..");
  }
  LCD.clear();
  while (!Serial);

  rtc.begin();
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  client.setCACert(CA_cert);
  client.setCertificate(ESP_CA_cert);
  client.setPrivateKey(ESP_RSA_key);
  mqtt_client.setServer(mqtt_broker, mqtt_port);
  lastReconnectAttempt = 0;

  LCD.setCursor(0, 0);
  LCD.print("Connecting MQTT...");
  mqtt_connect();
}

void mqtt_connect(){
  if(mqtt_client.connect(device_id, mqtt_user , mqtt_pass)) {
    LCD.setCursor(0, 0);
    LCD.print("[M2SHIP TECHNOLOGY]");
    LCD.setCursor(0, 1);
    LCD.print("Status:");
    LCD.setCursor(8, 1);
    LCD.print("Connected   ");
    Serial.print("connected, mqtt_client state:");
    Serial.println(mqtt_client.state());
  }else{
    mqtt_connect();
  }
}

boolean reconnect() {
  LCD.setCursor(0, 1);
  LCD.print("Status:");
  LCD.setCursor(8, 1);
  LCD.print("Reconnecting");

  if(mqtt_client.connect(device_id, mqtt_user , mqtt_pass)) {
    LCD.setCursor(0, 1);
    LCD.print("Status:");
    LCD.setCursor(8, 1);
    LCD.print("Connected   ");
  }
  return mqtt_client.connected();
}

void level(){
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration_us = pulseIn(ECHO_PIN, HIGH);
  distance_cm = 0.017 * duration_us;
  
  double D = 400;
  double L = 1000;
  double R = D/2;
  // float zero = 609837.50; 
  double H = D-distance_cm;
  double level = (((((R*R)*acos((R-H)/R))-((R-H)*( pow(((2*R*H)-(H*H)),(1/2)) )))*L+((22/7)*((H*H)/3)*(3*R-H))));

  slevel = String(level);
  Serial.print("Level: ");
  Serial.print(String(distance_cm));
  Serial.println(" cm");
  Serial.print(slevel);
  Serial.println(" cubic cm");
}

void time(){
 DateTime now = rtc.now();
 char buf1[] = "YYYY/MM/DD#hh|mm|ss";
 char buf2[] = "hh.mm DD/MM/YY";
 stime = now.toString(buf1);
 xtime = now.toString(buf2);
 Serial.println("Time: " + stime);
}

void location(){
  int longitude = 1502;
  int latitude = 1321;
  slocation =  String(longitude) + "@" + String(latitude);
  Serial.println("Longitude: " + String(latitude));
  Serial.println("Latitude: " + String(latitude));
}

void clearLCDLine(int line){
  for(int n = 6; n < 20; n++) {
    LCD.setCursor(n,line);
    LCD.print(" ");
  }
  LCD.setCursor(0,line);
}
void loop() {
  time();
  level();
  location();
  Serial.println();
  LCD.setCursor(0, 2);
  clearLCDLine(2);
  LCD.print("Level: " + slevel);
  LCD.setCursor(0, 3);
  LCD.print("Date: " + xtime);
  delay(3000);
  Serial.println(String(strcat(device_id, data)));
  String data = String(device_id) + ":" + slevel + ":" + stime + ":" + slocation;

  int str_len = data.length() + 1; 
  char msg[str_len];
  data.toCharArray(msg, str_len);

  if (!mqtt_client.connected()) {
    LCD.setCursor(0, 1);
    LCD.print("Status:");
    LCD.setCursor(8, 1);
    LCD.print("Disconnected");

    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
      mqtt_client.publish(device_id, msg);
      mqtt_client.loop();
  }

  delay(500);
}