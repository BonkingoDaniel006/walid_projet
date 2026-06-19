const int trigPin = 5;
const int echoPin = 18;
const int alarmePin = 4;
const int buttonPin = 23; // Nouvelle broche de reset d'alarme

long duree;
float distance;
int buttonState = 0;

// Cette variable permet de mémoriser si l'alarme doit sonner ou non
bool alarmeActive = false; 

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(alarmePin, OUTPUT);
  pinMode(buttonPin, INPUT); // Entrée simple avec résistance Pull-Down externe
  
  // Au départ, l'alarme est éteinte
  digitalWrite(alarmePin, LOW);
}

void loop() {
  // 1. MESURE DE LA DISTANCE VIA L'ULTRASON
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duree = pulseIn(echoPin, HIGH);
  distance = (duree * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 2. LOGIQUE DE DÉCLENCHEMENT (Si < 50cm, on verrouille l'alarme sur ON)
  if (distance > 0 && distance < 50) { 
    alarmeActive = true;
  }

  // 3. LECTURE DU BOUTON D'ARRÊT (Logique HIGH à l'appui)
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == HIGH) {
    Serial.println("Bouton Arret presse : Desactivation de l'alarme.");
    alarmeActive = false; // L'alarme est coupée logiquement
  }

  // 4. APPLICATION DE L'ÉTAT SUR LA BROCHE PHYSIQUE
  if (alarmeActive == true) {
    digitalWrite(alarmePin, HIGH);
    // Ça sonne indéfiniment tant qu'on n'a pas appuyé
  } else {
    digitalWrite(alarmePin, LOW);  // Éteint si le bouton a remis la variable à false
  }

  delay(200); // Réduit à 200ms pour que le bouton soit beaucoup plus réactif au toucher
}
