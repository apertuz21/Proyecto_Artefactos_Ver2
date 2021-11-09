#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


/************************* WiFi Access Point *********************************/


#define WLAN_SSID       "iPhone de Rodrigo"     //your SSID
#define WLAN_PASS       "123456789"         //your password

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                                // use 8883 for SSL
#define AIO_USERNAME  "apertuz"                             //your AIO username at ADAFRUIT
#define AIO_KEY       "aio_GDIe90Sn6eh5TWSvCfvnifx1G611"                                 //your AIO Key at ADAFRUIT

/******************* para declarar pines y variables *************************/
#define ON 1
#define OFF 2


#define LED_VERDE D1    // Pin al que LED está conectado

//Sensor:
#define DHTPIN D0
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int ldr_input;                                //sirve para guardar el dato leído de la LDR
int pulsador;

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'indicador' for publishing. led verde
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");

// Setup a feed called 'indicador2' for publishing.led rojo
Adafruit_MQTT_Publish humedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

//// Setup a feed called 'onoff' for subscribing to changes to the button
//Adafruit_MQTT_Subscribe boton_led = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/boton_led", MQTT_QOS_1);

/*************************** Sketch Code ************************************/


//*********************************************** SetUp *****************************/
void setup() {
  //para declarar si son entradas o salidas
  
  Serial.begin(115200); //inicio de comunicacion serie
  dht.begin();
  delay(10);
  //****************************************** Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  //********************************************* Callback Functions
  
//  slider.setCallback(slidercallback);
 // bomba_riego.setCallback(bomba_riego_callback);
  
  // Setup MQTT subscription for feed's.
//  mqtt.subscribe(&slider);
 // mqtt.subscribe(&bomba_riego);
 pinMode(LED_VERDE, OUTPUT);

}

uint32_t x=0;
//***************************************************** Loop ********************************/
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();
  }

  digitalWrite(LED_VERDE, HIGH);

  /********* Capturar temperatura por medio del sensor *********/

  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  float h = dht.readHumidity();
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(t) || isnan(h)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
  //Imprimimos la temperatura
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("  Humedad: ");
  Serial.println(h);

  //publicamos la temperatura en el gauge
  temperatura.publish(t);
  humedad.publish(h);

  //validamos que si la temperatura es mayor que 38 grados, se encienda la alerta
  /*if (t > 38) {
    alerta.publish(ON);
  }*/
}


// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000);  // wait 10 seconds
    retries--;
    if (retries == 0) {       // basically die and wait for WDT to reset me
     while (1);
    }
  }
  
  Serial.println("MQTT Connected!");  
}
