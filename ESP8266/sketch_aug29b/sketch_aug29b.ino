#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>


#define wifi_ssid "Livebox-4A58"
#define wifi_password "MojitOrhUmgIN99"

#define mqtt_server "ipMOSQUITTO"
#define mqtt_user "guest"  //s'il a été configuré sur Mosquitto
#define mqtt_password "guest" //idem

#define temperature_topic "sensor/temperature"  //Topic température
#define humidity_topic "sensor/humidity"        //Topic humidité

//Buffer qui permet de décoder les messages MQTT reçus
char message_buff[100];

long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;
bool debug = true;  //Affiche sur la console si True

#define DHTPIN D4    // Pin sur lequel est branché le DHT

// Dé-commentez la ligne qui correspond à votre capteur
//#define DHTTYPE DHT11       // DHT 11
#define DHTTYPE DHT22         // DHT 22  (AM2302)

//Création des objets
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;

void setup() {
  Serial.begin(115200);     //Facultatif pour le debug
  pinMode(D2, OUTPUT);    //Pin 2
  setup_wifi();           //On se connecte au réseau wifi
  dht.begin();
}

//Connexion au réseau WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connexion WiFi etablie ");
  Serial.print("=> Addresse IP : ");
  Serial.print(WiFi.localIP());
}


void loop() {

  long now = millis();
  //Envoi d'un message par minute
  if (now - lastMsg > 1000 * 10) {
    lastMsg = now;
    //Lecture de l'humidité ambiante
    float h = dht.readHumidity();
    // Lecture de la température en Celcius
    float t = dht.readTemperature();

    //Inutile d'aller plus loin si le capteur ne renvoi rien
    if ( isnan(t) || isnan(h)) {
      Serial.println("Echec de lecture ! Verifiez votre capteur DHT");
      return;
    }

    float heatIndex = dht.computeHeatIndex(t, h, false);

    Serial.print("Temperature : ");
    Serial.print(t);
    Serial.print(" | Humidite : ");
    Serial.print(h);
    Serial.print(" | Heat index : ");
    Serial.println(heatIndex);
    

  }
}

