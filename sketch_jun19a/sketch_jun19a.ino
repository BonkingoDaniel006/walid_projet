#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// Configuration du WiFi
const char* ssid = "Walid-Bot-WiFi";
const char* password = "Walid_Projet2026";

WebServer server(80);

// Définition des broches
#define SERVO_PIN 26
const int trigPin = 5;
const int echoPin = 18;
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
unsigned long precedentMillis = 0;
const int intervalServo = 30;    

// le code html

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Controle Walid</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap" rel="stylesheet">
  <style>
    /* Styles globaux et arrière-plan */
    body {
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
        background-color: #f3f4f6;
        font-family: 'Poppins', sans-serif;
    }

    /* Titre principal */
    h1 {
        color: #1f2937;
        margin-bottom: 25px;
        font-weight: 600;
        font-size: 1.8rem;
        text-align: center;
    }
    h1 span {
        color: #3b82f6;
    }

    /* Le boîtier de la télécommande */
    .remote-control {
        display: flex;
        flex-direction: column;
        background-color: #1e293b;
        border-radius: 35px;
        width: 450px;
        height: 500px;
        padding: 20px;
        box-shadow: 0 20px 25px -5px rgba(0, 0, 0, 0.3), 0 10px 10px -5px rgba(0, 0, 0, 0.2);
        gap: 15px;
        box-sizing: border-box;
    }

    /* 1. L'Écran / Header */
    .header {
        display: flex;
        flex-direction: row;
        background-color: #0f172a;
        border-radius: 20px;
        width: 100%;
        height: 60px;
        justify-content: center;
        align-items: center;
        gap: 10px;
    }
    .header p {
        color: #38bdf8;
        margin: 0;
        font-size: 0.95rem;
        font-weight: 600;
        letter-spacing: 0.5px;
    }
    .status-dot {
        width: 12px;
        height: 12px;
        background-color: #7f8c8d;
        border-radius: 50%;
        transition: background-color 0.3s, box-shadow 0.3s;
    }

    /* 2. Zone principale (Les boutons Gauche / Droite) */
    .main {
        display: flex;
        flex-direction: row;
        width: 100%;
        flex: 1;
        justify-content: center;
        align-items: center;
        gap: 20px;
    }

    /* Boutons directionnels */
    .btn-direction {
        display: flex;
        background-color: #334155;
        border: none;
        border-radius: 25px;
        width: 45%;
        height: 80%;
        justify-content: center;
        align-items: center;
        cursor: pointer;
        transition: all 0.2s ease;
    }
    .btn-direction i {
        color: #ffffff;
        font-size: 3rem;
        transition: transform 0.2s;
    }

    /* Effets au survol et au clic des directions */
    .btn-direction:hover {
        background-color: #3b82f6;
        box-shadow: 0 0 15px rgba(59, 130, 246, 0.5);
    }
    .btn-direction:hover i {
        transform: scale(1.15);
    }
    .btn-direction:active {
        transform: scale(0.95);
    }

    /* 3. Le Footer (Boutons d'action) */
    .footer {
        display: flex;
        flex-direction: row;
        width: 100%;
        height: 70px;
        justify-content: center;
        align-items: center;
        gap: 15px;
    }

    .btn-action {
        display: flex;
        justify-content: center;
        align-items: center;
        gap: 8px;
        width: 50%;
        height: 100%;
        border: none;
        border-radius: 18px;
        font-family: 'Poppins', sans-serif;
        font-weight: 600;
        color: white;
        cursor: pointer;
        transition: transform 0.2s, opacity 0.2s;
    }
    .btn-action.stop {
        background-color: #ef4444;
    }
    .btn-action.auto {
        background-color: #10b981;
    }
    .btn-action:hover {
        opacity: 0.9;
        transform: translateY(-2px);
    }
    .btn-action:active {
        transform: translateY(0);
    }

    /* ZONE D'ALERTE WEB CLIGNOTANTE */
    #web-alert { 
        display: none; 
        background: #ffeb3b; 
        color: #333; 
        padding: 15px; 
        border-radius: 10px; 
        font-weight: bold; 
        margin: 15px auto; 
        max-width: 410px; 
        width: 90%;
        box-shadow: 0 4px 10px rgba(0,0,0,0.1); 
        box-sizing: border-box;
        text-align: center;
    }
    .danger { 
        background: #ff3333 !important; 
        color: white !important; 
        animation: blink 1s infinite; 
    }
    @keyframes blink { 0% { opacity: 1; } 50% { opacity: 0.5; } 100% { opacity: 1; } }
  </style>
</head>
<body>

    <h1>Dirigez <span>Walid</span> depuis n'importe où</h1>

    <div id="web-alert">⚠️ INTRUSION DÉTECTÉE !</div>

    <div class="remote-control">
        <div class="header">
            <div class="status-dot" id="dot"></div>
            <p id="status-text">Walid-Bot : En attente</p>
        </div>

        <div class="main">
            <button class="btn-direction gauche" onclick="sendCommand('gauche')">
                <i class="fa-solid fa-chevron-left"></i>
            </button>
            <button class="btn-direction droite" onclick="sendCommand('droite')">
                <i class="fa-solid fa-chevron-right"></i>
            </button>
        </div>

        <div class="footer">
            <button class="btn-action stop" onclick="sendCommand('stop')"><i class="fa-solid fa-circle-stop"></i> STOP RADAR</button>
            <button class="btn-action auto" onclick="sendCommand('start')"><i class="fa-solid fa-robot"></i> START RADAR</button>
        </div>
    </div>

    <script>
        function sendCommand(action) {
            fetch('/' + action);
        }

        setInterval(function() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    const alertBox = document.getElementById('web-alert');
                    const dot = document.getElementById('dot');
                    const txt = document.getElementById('status-text');

                    if(data.actif) {
                        txt.innerText = "Walid-Bot : Radar Actif";
                        dot.style.backgroundColor = "#4ade80";
                        dot.style.boxShadow = "0 0 8px #4ade80";
                    } else {
                        txt.innerText = "Walid-Bot : Radar Arrêté";
                        dot.style.backgroundColor = "#7f8c8d";
                        dot.style.boxShadow = "none";
                    }

                    if (data.alarme) {
                        alertBox.style.display = "block";
                        alertBox.classList.add('danger');
                        alertBox.innerText = "⚠️ INTRUSION : " + data.distance + " cm !";
                    } else {
                        alertBox.style.display = "none";
                        alertBox.classList.remove('danger');
                    }
                });
        }, 1000);
    </script>
</body>
</html>
)=====";


void setup() {
  Serial.begin(115200);

  // Configuration WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Serveur démarré. Connectez-vous à : ");
  Serial.println(WiFi.softAPIP());

  // --- INTERACTION SERVEUR WEB ---
  
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", INDEX_HTML);
  });

  server.on("/start", HTTP_GET, []() {
    systemeActif = true;
    Serial.println("▶️ Système activé depuis le Web.");
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", HTTP_GET, []() {
    systemeActif = false;
    alarmeActive = false; 
    digitalWrite(alarmePin, LOW);
    Serial.println("⏸️ Système arrêté depuis le Web.");
    server.send(200, "text/plain", "OK");
  });

  // Routes pour les flèches plus tard :
  server.on("/gauche", HTTP_GET, []() { Serial.println("⬅️ Bouton Gauche pressé !"); server.send(200, "text/plain", "OK"); });
  server.on("/droite", HTTP_GET, []() { Serial.println("➡️ Bouton Droite pressé !"); server.send(200, "text/plain", "OK"); });

  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"actif\":" + String(systemeActif ? "true" : "false") + ",";
    json += "\"alarme\":" + String(alarmeActive ? "true" : "false") + ",";
    json += "\"distance\":" + String(distance);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();

  // Initialisation Matériel
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(posServo);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(alarmePin, OUTPUT);
  pinMode(buttonPin, INPUT); 
  
  digitalWrite(alarmePin, LOW);
  Serial.println("Système prêt !");
}

void loop() {
  server.handleClient(); 

  if (!systemeActif) {
    return; 
  }

  // 1. MESURE DE LA DISTANCE
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

  // 2. BOUTON PHYSIQUE RESET
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && alarmeActive) {
    Serial.println("🔓 Bouton Physique : Désactivation.");
    alarmeActive = false; 
  }

  // 3. GESTION ACTIONS (ALARME ET SERVO)
  if (alarmeActive) {
    digitalWrite(alarmePin, HIGH); 
  } 
  else {
    digitalWrite(alarmePin, LOW);  
    
    unsigned long actuelMillis = millis();
    if (actuelMillis - precedentMillis >= intervalServo) {
      precedentMillis = actuelMillis;
      posServo += directionServo;     
      servoMotor.write(posServo);     
      
      if (posServo >= 180) directionServo = -4; 
      if (posServo <= 0) directionServo = 4;   
    }
  }

  delay(10); 
}
