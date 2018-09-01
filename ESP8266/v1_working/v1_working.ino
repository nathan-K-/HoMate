#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>


#define wifi_ssid "Livebox-4A58"
#define wifi_password "MojitOrhUmgIN99"
 
#define mqtt_server "192.168.1.18"
 
#define temperature_topic "sensor/temperature"  //Topic température
#define humidity_topic "sensor/humidity"        //Topic humidité
#define general_topic "/sensors/test"
 
//Buffer qui permet de décoder les messages MQTT reçus
char message_buff[100];
 
long lastMsg = 0;   //Horodatage du dernier message publié sur MQTT
long lastRecu = 0;
bool debug = true;  //Affiche sur la console si True
 
#define DHTPIN D4    // Pin sur lequel est branché le DHT
 
#define DHTTYPE DHT22         // DHT 22  (AM2302)
 
//Création des objets
DHT dht(DHTPIN, DHTTYPE);     
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
  Serial.begin(115200);     //Facultatif pour le debug
  pinMode(D2,OUTPUT);     //Pin 2 
  setup_wifi();           //On se connecte au réseau wifi
  client.setServer(mqtt_server, 1883);    //Configuration de la connexion au serveur MQTT
  client.setCallback(callback);  //La fonction de callback qui est executée à chaque réception de message   
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
 
//Reconnexion
void reconnect() {
  //Boucle jusqu'à obtenur une reconnexion
  while (!client.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("OK");
    } else {
      Serial.print("KO, erreur : ");
      Serial.print(client.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(5000);
    }
  }
}
 
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
 
  long now = millis();
  //Envoi d'un message par minute
  if (now - lastMsg > 1000 * 60) {
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

     StaticJsonBuffer<200> jsonBuffer;
     JsonObject& root = jsonBuffer.createObject();

    float heatIndex = dht.computeHeatIndex(t, h, false);

    root["temp"] = t;
    root["hum"] = h;
    root["hIdx"] = heatIndex;
    root.printTo(Serial);
    Serial.println();
  
    if ( debug ) {
      Serial.print("Temperature : ");
      Serial.print(t);
      Serial.print(" | Humidite : ");
      Serial.print(h);
      Serial.print(" | Heat index : ");
      Serial.println(heatIndex);
    }  
    String output;
    root.printTo(output);
    client.publish(general_topic, output.c_str(), true);
    //client.publish(temperature_topic, String(t).c_str(), true);   //Publie la température sur le topic temperature_topic
    //client.publish(humidity_topic, String(h).c_str(), true);      //Et l'humidité
  }
  if (now - lastRecu > 100 ) {
    lastRecu = now;
    client.subscribe("homeassistant/switch1");
  }
}
 
// Déclenche les actions à la réception d'un message
// D'après http://m2mio.tumblr.com/post/30048662088/a-simple-example-arduino-mqtt-m2mio
void callback(char* topic, byte* payload, unsigned int length) {
 
  int i = 0;
  if ( debug ) {
    Serial.println("Message recu =>  topic: " + String(topic));
    Serial.print(" | longueur: " + String(length,DEC));
  }
  // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  if ( debug ) {
    Serial.println("Payload: " + msgString);
  }
  
  if ( msgString == "ON" ) {
    digitalWrite(D2,HIGH);  
  } else {
    digitalWrite(D2,LOW);  
  }
}



    

