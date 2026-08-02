#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>

const char* ssid = "SFR_05A8";
const char* password = "5hus35tudu6qcv98evnt";

// URL du serveur Render
const char* serverBaseUrl = "https://walid-api.onrender.com";

// Définition des broches
#define SERVO_PIN 5
const int trigPin = 15;
const int echoPin = 2;
const int alarmePin = 4;
const int buttonPin = 23;

// Variables globales
long duree;
float distance = 0.0;
int buttonState = 0;

// Logique du système
bool systemeActif = false;
bool alarmeActive = false;

// Variables Servo
Servo servoMotor;
int posServo = 0;
int directionServo = 4;
unsigned long precedentMillisServo = 0;
const int intervalServo = 20; // Fluidité du servo (20ms)

// Polling et requêtes HTTP
unsigned long precedentMillisSync = 0;
const int intervalSync = 1500; // Synchronisation serveur toutes les 1.5 sec

// Client TLS réutilisable
WiFiClientSecure client;

void synchroniserAvecFlask();

void setup() {
  Serial.begin(115200);

  // Configuration client SSL
  client.setInsecure(); // Ignore la vérification stricte du certificat SSL

  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi !");

  // Initialisation Matériel
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(posServo);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(alarmePin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  digitalWrite(alarmePin, LOW);
  Serial.println("Système ESP32 prêt !");
}

void loop() {
  unsigned long actuelMillis = millis();

  // --- 1. SYNCHRONISATION SERVEUR (Toutes les 1.5s) ---
  if (actuelMillis - precedentMillisSync >= intervalSync) {
    precedentMillisSync = actuelMillis;
    if (WiFi.status() == WL_CONNECTED) {
      synchroniserAvecFlask();
    }
  }

  // Si le système est éteint depuis le Web, on réinitialise les sorties
  if (!systemeActif) {
    digitalWrite(alarmePin, LOW);
    alarmeActive = false;
    delay(10);
    return;
  }

  // --- 2. MESURE ULTRA-SONS ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Timeout réduit à 15000 µs (~2.5m Max)
  duree = pulseIn(echoPin, HIGH, 15000); 
  
  if (duree > 0) {
    distance = (duree * 0.034) / 2.0;
  } else {
    distance = 400.0; // Aucune détection
  }

  // Logique d'intrusion (< 50 cm)
  if (distance > 0 && distance < 50) {
    if (!alarmeActive) {
      Serial.printf("⚠️ INTRUSION DÉTECTÉE ! Distance: %.1f cm -> Arrêt Servo\n", distance);
      alarmeActive = true;
      // Synchronisation immédiate avec Render lors d'un évènement d'intrusion
      if (WiFi.status() == WL_CONNECTED) {
        synchroniserAvecFlask();
      }
    }
  }

  // --- 3. BOUTON PHYSIQUE RESET ---
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && alarmeActive) {
    Serial.println("🔓 Bouton Physique : Reset Alarme.");
    alarmeActive = false;
  }

  // --- 4. GESTION ALARME ET MOTEUR ---
  if (alarmeActive) {
    digitalWrite(alarmePin, HIGH);
    // LE SERVOMOTEUR RESTE SANS MOUVEMENT ICI (BLOQUÉ SUR SA POSITION ACTUELLE)
  } else {
    digitalWrite(alarmePin, LOW);
    
    // Déplacement fluide du servomoteur uniquement si aucune alarme n'est active
    if (actuelMillis - precedentMillisServo >= intervalServo) {
      precedentMillisServo = actuelMillis;
      posServo += directionServo;
      servoMotor.write(posServo);
      
      if (posServo >= 180) directionServo = -4;
      if (posServo <= 0) directionServo = 4;
    }
  }

  delay(5);
}

// Fonction combinée : Envoie l'intrusion + distance ET récupère l'état 'actif'
void synchroniserAvecFlask() {
  HTTPClient http;
  String url = String(serverBaseUrl) + "/api/data";
  
  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"intrusion\":" + String(alarmeActive ? "true" : "false") + 
                         ",\"actif\":" + String(systemeActif ? "true" : "false") + 
                         ",\"distance\":" + String(distance, 1) + "}";

    int httpCode = http.POST(jsonPayload);
    
    if (httpCode == HTTP_CODE_OK || httpCode == 201) {
      String payload = http.getString();
      
      // Mise à jour de l'état 'actif' d'après le serveur
      if (payload.indexOf("\"actif\":true") != -1 || payload.indexOf("\"actif\": true") != -1) {
        systemeActif = true;
      } else if (payload.indexOf("\"actif\":false") != -1 || payload.indexOf("\"actif\": false") != -1) {
        systemeActif = false;
        alarmeActive = false;
      }
    } else {
      Serial.printf("Erreur HTTP Sync : %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
