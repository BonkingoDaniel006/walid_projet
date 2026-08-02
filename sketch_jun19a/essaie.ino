#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// Configuration du Wi-Fi (mettez vos identifiants d'accès Internet)
const char* ssid = "VOTRE_WIFI_SSID";
const char* password = "VOTRE_WIFI_MOT_DE_PASSE";

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
float distance;
int buttonState = 0;

// Logique du système
bool systemeActif = false; 
bool alarmeActive = false; 

// Variables Servo
Servo servoMotor;
int posServo = 0;
int directionServo = 4;          
unsigned long precedentMillisServo = 0;
const int intervalServo = 30;    

// Polling et requêtes HTTP
unsigned long precedentMillisSync = 0;
const int intervalSync = 1000; // Synchronisation avec le serveur toutes les secondes

void setup() {
  Serial.begin(115200);

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

  // Synchronisation régulière avec le serveur Cloud Flask
  if (actuelMillis - precedentMillisSync >= intervalSync) {
    precedentMillisSync = actuelMillis;
    
    if (WiFi.status() == WL_CONNECTED) {
      // 1. Envoyer les données du capteur au serveur
      envoyerDonneesFlask();
      
      // 2. Synchroniser l'état (actif/inactif) depuis le serveur
      lireStatutFlask();
    }
  }

  // Si le système n'est pas actif côté serveur, on stoppe la boucle matérielle
  if (!systemeActif) {
    digitalWrite(alarmePin, LOW);
    return; 
  }

  // --- 1. MESURE DE LA DISTANCE ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duree = pulseIn(echoPin, HIGH, 30000); 
  distance = (duree * 0.034) / 2;

  if (distance > 0 && distance < 400) {
    if (distance < 50) { 
      if (!alarmeActive) {
        Serial.println("⚠️ INTRUSION DÉTECTÉE !");
        alarmeActive = true;
      }
    }
  }

  // --- 2. BOUTON PHYSIQUE RESET ---
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && alarmeActive) {
    Serial.println("🔓 Bouton Physique : Désactivation de l'alarme.");
    alarmeActive = false; 
  }

  // --- 3. GESTION ACTIONS (ALARME ET SERVO) ---
  if (alarmeActive) {
    digitalWrite(alarmePin, HIGH); 
  } 
  else {
    digitalWrite(alarmePin, LOW);  
    
    if (actuelMillis - precedentMillisServo >= intervalServo) {
      precedentMillisServo = actuelMillis;
      posServo += directionServo;     
      servoMotor.write(posServo);     
      
      if (posServo >= 180) directionServo = -4; 
      if (posServo <= 0) directionServo = 4;   
    }
  }

  delay(10); 
}

// Fonction d'envoi POST de l'état vers l'API Flask
void envoyerDonneesFlask() {
  HTTPClient http;
  String url = String(serverBaseUrl) + "/api/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"intrusion\":" + String(alarmeActive ? "true" : "false") + 
                       ",\"actif\":" + String(systemeActif ? "true" : "false") + "}";

  int httpCode = http.POST(jsonPayload);
  if (httpCode <= 0) {
    Serial.printf("Erreur POST HTTP : %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

// Fonction GET pour lire si l'utilisateur a démarré/arrêté le système via le Web
void lireStatutFlask() {
  HTTPClient http;
  String url = String(serverBaseUrl) + "/status";
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    // Analyse simplifiée du champ "actif" dans le JSON
    if (payload.indexOf("\"actif\":true") != -1 || payload.indexOf("\"actif\": true") != -1) {
      systemeActif = true;
    } else if (payload.indexOf("\"actif\":false") != -1 || payload.indexOf("\"actif\": false") != -1) {
      systemeActif = false;
      alarmeActive = false;
    }
  }
  http.end();
}