#include <Arduino.h>
#include <Wire.h> // Pour OLED
#include <Adafruit_GFX.h>//OLED
//#include <Adafruit_SSD1306.h>//OLED
#include <Adafruit_SH110X.h> // OLED
#include <WiFi.h>                                                               // Librairie Wi-Fi
#include <PubSubClient.h>  
#include <Adafruit_NeoPixel.h>
#include <Tone32.h>  // add note library

TaskHandle_t Task1;
TaskHandle_t Task2;

const char* ssid = "ISIBot";                                                     // SSID du réseau Wi-Fi auquel il faut se connecter
const char* password = "Robotix-ISIBot";                                            // Mot de passe du réseau Wi-Fi auquel il faut se connecter

int points = 0;
char charPoints[5];
bool finish = false;
bool flagFinishOnce1 = false;

#define PIN 19
#define NUMPIXELS 24
int pix;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* mqtt_server = "192.168.0.101"; 

WiFiClient espClient;                                                           // Création de "espClient" propre au client Wi-Fi
PubSubClient client(espClient);
#define MSG_BUFFER_SIZE  (50)                                                   // Défini la taille maximum des messages que l'on va envoyer
char msg[MSG_BUFFER_SIZE]; 

// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// OLED SSD1306
/*
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
*/
// OLED SH110X
#define OLED_RESET -1   //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//BUZZER
int PinBuzzer = 17;
int HeureFinish;
//notes in the melody
int melody[]={
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_B4, 0,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D5, NOTE_C5, 0,
  NOTE_G4, NOTE_G4, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_B4, NOTE_A4, 0,
  NOTE_F5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_C5
  };
//note durations. 4=quarter note / 8=eighth note
int noteDurations[]={4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4};

void setup_wifi() {                                                             // Fonction de connection au WiFi

  delay(10);                                                                    // Délai de 10ms
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);                                                         // Imprime dans la console le nom du WiFi

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);                                                   // Lancement de la connexion WiFi

  while (WiFi.status() != WL_CONNECTED) {                                       // Tant que le microcontrôleur n'est pas connecté au WiFi
    delay(500);                                                                 // Délai de 500ms
    Serial.print(".");                                                          // Imprimme un point
  }

  randomSeed(micros());                                                         // Création d'une "clef" aléatoire

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());                                               // Affichage dans la console que l'on est connecté avec cette adresse IP
}

void callback(char* topic, byte* payload, unsigned int length) {                // Fonction de Callback quand un message MQTT arrive sur un topic (subscribe)
  Serial.print("Message arrived [");
  Serial.print(topic);                                                          // On affiche de quel topic il s'agit
  Serial.print("] ");
  for (int i = 0; i < length; i++) {                                            // On boucle tous les caractères reçus sur le topic
    Serial.print((char)payload[i]);                                             // On affiche un à un tous les caractères reçus
  }
  Serial.println();
  
  char buffer1[length+1];                                                       // On crée une variable local de buffer
  for (int i = 0; i < length+1; i++) {                                          // On relis tous les caractères reçus sur le topic
    buffer1[i] = (char)payload[i];                                              // On enregistre tous les caractères que l'on a besoin (uniquement)
  }
  if (String(topic) == "main_points") {                                         // On vérifie si c'est le bon topic
    points = atoi(buffer1);           
  }
  if (String(topic) == "main_finish") {                                         // On vérifie si c'est le bon topic
    finish = bool(buffer1);           
  }  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {                                                 // Si le client se connecte
    Serial.print("Attempting MQTT connection...");                              // On attent la conexion MQTT
    if (client.connect("ESP32")) {                                              // Si le client se connecte en étant appelé "ESP32"
      Serial.println("connected");                                              // On prévient qu'il est connecté
      client.subscribe("main_points");                                          // On capture la donnée du topic "main_points"
      client.subscribe("main_finish");                                          // On capture la donnée du topic "main_finish"
    } else {                                                                    // Si la connexion rate
      Serial.print("failed, rc=");                                              // On affiche qu'il y a une erreur
      Serial.print(client.state());                                             // On affiche le numéro de l'erreur
      Serial.println(" try again in 5 seconds");                                // On affiche comme quoi on réessaye
      delay(5000);                                                              // On attend 5 secondes avant de réessayer
    }
  }
}

void printToOLED(int x, int y,  char *message){
  display.setCursor(x, y); 
  //display.setTextColor(WHITE,BLACK); //Superposer les texte
  display.print(message);
  display.display();
}

void playMusic(){
  //play the notes of the melody. The 28 reresent the number of notes played in the song.
  for (int thisNote=0; thisNote <28; thisNote++){

    //to calculate the note duration, take one second. Divided by the note type
    int noteDuration = 1000 / noteDurations [thisNote];
    tone(PinBuzzer, melody [thisNote], noteDuration, 0);
    noTone(PinBuzzer, 0);

    //to set the speed of the song set a minimum time between notes
    int pauseBetweenNotes = int(noteDuration / 4);
    delay(pauseBetweenNotes);      
  }
}

void setup() {
  Serial.begin(115200); 

  // LEDs
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.setBrightness(200);
  
  client.setServer(mqtt_server, 1883);                                          // On se connecte au serveur MQTT
  client.setCallback(callback);  

  setup_wifi();
  
 
  
  // OLED SSD1306
  /*if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }*/
  // OLED SH110X
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  delay(250); // wait for the OLED to power up
  /* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
  #define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
  //#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
  display.begin(i2c_Address, true); // Address 0x3C default
  display.display();
  // END this type OLED

  delay(2000);
  display.clearDisplay();                                                       // Clear the buffer.
  // OLED SSD1306 => WHITE
  // OLED SH110X => SH110X_WHITE
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 3, SH110X_WHITE); //x1, y1, largeur, hauteur, rayon
  display.setTextSize(2);  
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);  
  printToOLED(5, 5, "Points :");
  //printToOLED(30, 17,"0");

  //BUZZER
  pinMode(PinBuzzer, OUTPUT);
  
  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,&Task1,0);
  delay(500); 

  xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,1,&Task2,1);
  delay(500); 
}

void Task1code( void * parameter ){
  Serial.print("Task1 is running on core ");
  Serial.println(xPortGetCoreID());
  const TickType_t xDelayTask1 = 1 / portTICK_PERIOD_MS ;
  for(;;){
    if (!client.connected()) {                                                    // Si le client pour le MQTT en WiFi n'est pas connecté
      reconnect();                                                                // On appelle la fonction qui demande une reconnexion
    }
    client.loop();
    
    vTaskDelay( xDelayTask1 );

    //BUZZER
    if(finish && !flagFinishOnce1) {
      flagFinishOnce1 = true;
      // Joue MUSIQUE
      Serial.println("Go musique");
      playMusic();
    }
  } 
}

void Task2code( void * parameter ){
  Serial.print("Task2 is running on core ");
  Serial.println(xPortGetCoreID());
  const TickType_t xDelayTask2 = 500 / portTICK_PERIOD_MS ;
  for(;;){ 

    vTaskDelay( xDelayTask2 );

    sprintf(charPoints, "%5u", points);
    printToOLED(17, 35, charPoints);  

    //BUZZER
    if(finish) {
      // Affiche LED
      Serial.println("Go LED");
      pix ++;
      if (pix%3 == 0){ 
        for (int i = 0; i < NUMPIXELS; i++){
          pixels.setPixelColor(i, 255, 100, 100);
        }
      }
      if (pix%3 == 1){ 
        for (int i = 0; i < NUMPIXELS; i++){
          pixels.setPixelColor(i, 255, 100, 0);
        }
      }
      else {
        for (int i = 0; i < NUMPIXELS; i++){
          pixels.setPixelColor(i, 255, 250, 0);
        }
      }
      pixels.show(); // Send the updated pixel colors to the hardware.  
    }
  }
}

void loop() {

}
