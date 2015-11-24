#include <OneWire.h> // Inclusion de la librairie OneWire
 
// Definition de la temperature souhaite
int Temp = 20;
int TempMin = Temp - 1;
int TempMax = Temp + 1;

// Definition des delais
int VerifTemp = 10; //delais pour eviter un arret relance du chauffage tout les 16MHz

// Definition des broches
#define BROCHE_ONEWIRE 7 // Broche utilisee pour le bus 1-Wire
#define BROCHE_RELAIS 2 //Broche utilisee pour le relais

// configuration de la sonde de temperature
#define DS18B20 0x28     // Adresse 1-Wire du DS18B20 
OneWire ds(BROCHE_ONEWIRE); // Création de l'objet OneWire ds
 
 
// comandes executees uniquement au demarrage de l'arduino
void setup() {
  Serial.begin(9600); // Initialisation du port série
  // ---- configuration des broches
   pinMode(BROCHE_RELAIS, OUTPUT);
   digitalWrite(BROCHE_RELAIS, LOW);
}
 
// boucle du principale de l'arduino
void loop() {
  float ReadTemp;
   
  // Lit la température ambiante
  if(getTemperature(&ReadTemp)) {   
    // Affiche la température
    Serial.print("Temperature : ");
    Serial.print(ReadTemp);
    Serial.write(176); // caractère °
    Serial.write('C');
    Serial.println();

    if (ReadTemp >=  Temp + 1) // si temp mesuree superieur a consigne
    {
     digitalWrite(BROCHE_RELAIS,LOW);   // J'eteinds le chauffage
    };
    
    if (ReadTemp <=  Temp - 1) // si temp mesuree inferieur a consigne
    {
     digitalWrite(BROCHE_RELAIS,HIGH);   // j'allume le chauffage
    }
    else{ 
     digitalWrite(BROCHE_RELAIS,LOW);  //sinon j'eteinds le chauffage
    };
  }
  delay(VerifTemp); // pause avant de relire la temperature dans la piece
}


// Fonction récupérant la température depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
boolean getTemperature(float *temp){
  byte data[9], addr[8];
  // data : Données lues depuis le scratchpad
  // addr : adresse du module 1-Wire détecté
 
  if (!ds.search(addr)) { // Recherche un module 1-Wire
    ds.reset_search();    // Réinitialise la recherche de module
    return false;         // Retourne une erreur
  }
   
  if (OneWire::crc8(addr, 7) != addr[7]) // Vérifie que l'adresse a été correctement reçue
    return false;                        // Si le message est corrompu on retourne une erreur
 
  if (addr[0] != DS18B20) // Vérifie qu'il s'agit bien d'un DS18B20
    return false;         // Si ce n'est pas le cas on retourne une erreur
 
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
   
  ds.write(0x44, 1);      // On lance une prise de mesure de température
  delay(800);             // Et on attend la fin de la mesure
   
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
  ds.write(0xBE);         // On envoie une demande de lecture du scratchpad
 
  for (byte i = 0; i < 9; i++) // On lit le scratchpad
    data[i] = ds.read();       // Et on stock les octets reçus
   
  // Calcul de la température en degré Celsius
  *temp = ((data[1] << 8) | data[0]) * 0.0625; 
   
  // Pas d'erreur
  return true;
}

