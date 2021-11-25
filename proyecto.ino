/***********************************

Proyecto de Programación de Artefactos - Ciclo 02/21

Presentado por:
JOSE ENRIQUE GARCIA AREVALO 00093619
RODRIGO ANIBAL HERNANDEZ GARCIA 00050519
ANTONIO ALEXIS PERTUZ ARÉVALO 00267016

Sistema de control de temperatura, humedad y luminosidad para cultivos con envió de datos a la nube
Cultivo elegido: Verdolaga

Recomendaciones para el cultivo:
Temperatura adecuada: 18ªC a 25ºC
Humedad adecuada: 70% a 80%
Luminosidad: Necesita abundante luz, al menos que el sol sea muy intenso, colocar en lugares de semisobra
Riego:
Regar 1 o 2 veces a la semana abundantemente pero sin encharcar la maceta, si la temperatura es mayor 
de la adecuada, regar más frecuentemente.


***********************************/



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

/******************* Declarar pines y variables *************************/

/********** Sensor **********/
#define DHTPIN D0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/********** LDR **********/
#define LDR A0
int lum; // guardaremos el valor que nos de el sensor (0 a 1023)
float luminosidad_porcentaje; // guardaremos el valor que nos da el sensor convertido a porcentaje
float coeficiente_porcentaje = 100.0 / 1023.0; // factor para convertir el valor del sensor a porcentaje

/********** LEDS: **********/
#define LED_TEMP_BAJA D1
#define LED_TEMP_ALTA D2
#define LED_HUM_BAJA D3
#define LED_HUM_ALTA D4
#define LED_LUM_BAJA D5
#define LED_LUM_ALTA D6

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'temperatura' for publishing
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");

// Setup a feed called 'humedad' for publishing
Adafruit_MQTT_Publish humedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

// Setup a feed called 'luminosidad' for publishing
Adafruit_MQTT_Publish luminosidad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/luminosidad");

// Setup a feed called 'frecuencia_riego' for publishing
Adafruit_MQTT_Publish frecuencia_riego = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/frecuencia_riego");

/*************************** Sketch Code ************************************/


//*********************************************** SetUp *****************************/

void setup() {
  
  /******** Declaramos dispositivos de entrada o salida ********/
  
  pinMode(LDR, INPUT);
  pinMode(LED_TEMP_BAJA, OUTPUT);
  pinMode(LED_TEMP_ALTA, OUTPUT);
  pinMode(LED_HUM_BAJA, OUTPUT);
  pinMode(LED_HUM_ALTA, OUTPUT);
  pinMode(LED_LUM_BAJA, OUTPUT);
  pinMode(LED_LUM_ALTA, OUTPUT);
  
  Serial.begin(115200); //inicio de comunicacion serie
  
  dht.begin(); // inicio de comunicación con el sensor dht11
  
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
}


//***************************************************** Loop ********************************/
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();
  }

  //********* Capturar temperatura y humedad por medio del sensor *********/

  float t = dht.readTemperature(); // Leemos la temperatura en grados centígrados (por defecto)
  float h = dht.readHumidity(); 
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(t) || isnan(h)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
  //Imprimimos la temperatura y humedad
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print("  Humedad: ");
  Serial.println(h);

  //*********** Publicamos la temperatura y humedad en sus gauge y gráficos respectivos ***********/
  
  temperatura.publish(t); // Si la temperatura es menor de 18 o mayor de 25, se encienden alertas en adafruit
  humedad.publish(h); // Si la humedad es menor de 70 o mayor de 80, se encienden alertas en adafruit

  
  //*********** Leemos la luminosidad del LDR y la convertimos a porcentaje ***********/
  
  lum = analogRead(LDR);
  luminosidad_porcentaje = lum * coeficiente_porcentaje;
  Serial.print("luminosidad: ");
  Serial.print(lum);
  Serial.print(" luminosidad_porcentaje: ");
  Serial.println(luminosidad_porcentaje);


  //*********** Publicamos la luminosidad (procentaje) en su gauge y gráfico ***********/
  
  luminosidad.publish(luminosidad_porcentaje); // Si la luminosidad es menor de 45 o mayor de 90, se encienden alertas en adafruit

  
  //*********** Dependiendo del valor de la temperatura, se encenderán leds como alertas y
  // se publicará la frecuencia con la que se recomienda regar la Dalia ***********/

  // Temperatura
  if (t < 25) {
    frecuencia_riego.publish("Regar 1 vez a la semana"); // Si la temp es menor a 25, se recomienda regar 1 vez a la semana
    digitalWrite(LED_TEMP_BAJA, HIGH); // Se enciende alerta por temperatura muy baja
    digitalWrite(LED_TEMP_ALTA, LOW);
  } else if (t >= 25 && t <= 30) {
    frecuencia_riego.publish("Regar 2 veces a la semana"); // Si la temp es ideal, se recomienda regar 2 veces a la semana
  } else if (t > 30){
    frecuencia_riego.publish("Regar 4 veces a la semana"); // Si la temp es mayor a 30, se recomienda regar 4 veces a la semana
    digitalWrite(LED_TEMP_BAJA, LOW);
    digitalWrite(LED_TEMP_ALTA, HIGH); // Se enciende alerta por temperatura muy alta (regar más abundantemente)
  }

  // Humedad
  if (h < 70) {
    digitalWrite(LED_HUM_BAJA, HIGH); // Se enciende alerta por humedad muy baja
    digitalWrite(LED_HUM_ALTA, LOW);
  } else if (h > 80) {
    digitalWrite(LED_HUM_BAJA, LOW);
    digitalWrite(LED_HUM_ALTA, HIGH); // Se enciende alerta por humedad muy alta
  }
  
  // Luminosidad
  if (luminosidad_porcentaje < 45) {
    digitalWrite(LED_LUM_BAJA, HIGH); // Se enciende por muy poca luz
    digitalWrite(LED_LUM_ALTA, LOW);
  } else if (luminosidad_porcentaje > 90) {
    digitalWrite(LED_LUM_BAJA, LOW);
    digitalWrite(LED_LUM_ALTA, HIGH); // Se enciende por mucha
  }
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
