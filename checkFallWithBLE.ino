/********************************************************************************************************************************************************************/
//  checkFallWithBLE.ino pour Arduino v1.8.19 et versions antérieures                                                                                               //
//  Fonctionne avec la carte "Seeed nRF52840 Sense"                                                                                                                 //
//                                                                                                                                                                  //
//  Mettre le lien https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json dans Préférences -> URL de gestionnaire de cartes supplémentaires      //
//  Installer la carte "Seeed nRF52 Boards" dans Outils -> Type de carte -> Gestionnaire de carte                                                                   //
//  Installer la bibliothèque "Seeed Arduino LSM6DS3" dans Croquis -> Inclure une bibliothèque -> Gérer les bibliothèques                                           //
//  Sélectionner la carte "Seeed XIAO nRF52840 Sense" dans Outils -> Type de carte                                                                                  //
//  Sélectionner le bon port pour la carte "Seeed XIAO nRF52840 Sense" dans Outils -> Port                                                                          //
//                                                                                                                                                                  //
//  Téléverser le script sur la carte, aller dans le Moniteur série (logo loupe en haut à droite), envoyer n'importe quelle information via le Moniteur série       //
//  Connecter le téléphone à la carte "XIAO nRF52840 Sense" via Bluetooth, aller dans un environnement où le téléphone peut écrire à l'aide du clavier              //
//                                                                                                                                                                  //
//  Permet de détecter une accélération en combinant l'accélération sur les 3 axes repéré par l'appareil : dans le cas d'un mouvement rectiligne                    //
//  ATTENTION : Les 3 axes changent avec l'orientation de la carte, l'utilisation du gyroscope peut permettre de retracer chacun des axes mais il est très imprécis //
//  Changer la valeur de g pour détecter une accélération différente de 9.8m/s²                                                                                     //
//                                                                                                                                                                  //
//  Créé par Baptiste Duchosal, TG4 en 2023-2024                                                                                                                    //
/********************************************************************************************************************************************************************/

#include <bluefruit.h>
#include "LSM6DS3.h"
#include "Wire.h"

LSM6DS3 myIMU(I2C_MODE, 0x6A);
BLEDis bledis;
BLEHidAdafruit blehid;

bool    Fall = false;             // En descente?
bool    hasKeyPressed = false;    // Artéfact pour l'envoi d'informations
char    ch;                       // Caractère du message
float   Accel;                    // Accélération calculée
float   Debut;                    // Temps au début de la descente
float   Fin;                      // Temps à la fin de la descente
float   resetTime;                // Temps entre fin et présent (combien de temps doit-on attendre pour recapturer une accélération?)
float   g = 9.8;                  // Constante gravitationnelle sur Terre -- sera comparée
int     i;                        // Emplacement des caractères dans Temps
String  Temps;                    // Temps entre début en fin (en string)



void setup() {
  
    Serial.begin(115200);         // Initialisation Arduino

//***************************************************//
// Mise en place fonctionnalité Bluetooth côté carte //
//                                                   //
    while (!Serial);                                 //
    delay(10);                                       //                                                                     
    Bluefruit.begin();                               //
    Bluefruit.setTxPower(4);                         //
    bledis.setManufacturer("SYSTEME ERLB");          //
    bledis.setModel("ballFall infoQuery");           //
    bledis.begin();                                  //
    blehid.begin();                                  //
    startAdv();                                      //
//***************************************************//

//***************************************//
// Regarde si l'accéléromètre fonctionne //
//                                       //
    if (myIMU.begin() != 0) {            //
        Serial.println("ERREUR");        //
    } else {                             //
        Serial.println("OK");            //
    }                                    //
//***************************************//
}


//*************************************************************************************************************************************************************************************//
// Mise en place émission Bluetooth                                                                                                                                                    //
//                                                                                                                                                                                     //
  void startAdv(void) {                                                                                                                                                                //                                                                                                                                                                                    //
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);                                                                                                       //
    Bluefruit.Advertising.addTxPower();                                                                                                                                                //
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);                                                                                                                  //
    Bluefruit.Advertising.addService(blehid);                                                                                                                                          //
    Bluefruit.Advertising.addName();                                                                                                                                                   //
    Bluefruit.Advertising.restartOnDisconnect(true);                                                                                                                                   //
    Bluefruit.Advertising.setInterval(32, 244);                                     // par unité de 0.625ms                                                                            //
    Bluefruit.Advertising.setFastTimeout(30);                                       // nombre de secondes en Fast Mode                                                                 //
    Bluefruit.Advertising.start(0);                                                 // 0 = Ne pas arrêter l'émission ; n = Arrêter au bout de n secondes                               //
  }                                                                                                                                                                                    //
//*************************************************************************************************************************************************************************************//


void loop() {
  
    Accel = sqrt(sq(myIMU.readFloatAccelX()) + sq(myIMU.readFloatAccelY()) + sq(myIMU.readFloatAccelZ()));        // Composé des accélérations sur les 3 axes (Pythagore en 3 dimensions)
    resetTime = (micros() - Fin) / 1000000;                                                                       // Temps écoulé entre la dernière fin d'accélération et le présent

//******************************************************************************************************************************************************************************************************//
// Module pour éviter de recevoir chacune des accélérations, modifier le 3.0 pour le temps d'attente en secondes                                                                                        //
//                                                                                                                                                                                                      //
    if (Accel > g and Fall == false and resetTime < 3.0) {                                                        // Capture dans une boucle une accélération avant que les 3s se soient écoulées       //
      while(resetTime < 3.0 or Accel > 1) {                                                                                                                                                             //
        resetTime = (micros() - Fin) / 1000000;                                                                   // Calculé lors de la boucle pour savoir quand le temps est dépassé                   //
        Accel = sqrt(sq(myIMU.readFloatAccelX()) + sq(myIMU.readFloatAccelY()) + sq(myIMU.readFloatAccelZ()));    // Calculé lors de la boucle pour savoir quand la balle s'arrête                      //
        delay(1);                                                                                                                                                                                       //
      }                                                                                                                                                                                                 //
    }                                                                                                                                                                                                   //
//******************************************************************************************************************************************************************************************************//
    
//******************************************************************************************************************************************************************************************************//
// On vérifie si la carte est en accélération, capture l'instant où l'accélération débute et on informe avec "Fall" que l'accélération a débuté                                                         //
//                                                                                                                                                                                                      //
    if (Accel > g and Fall == false and resetTime > 3.0) {                                                                                                                                              //
      Debut = micros();                                         // Temps avec micros() car plus précis que millis()                                                                                     //
      Serial.print("\n\n\nEn descente\n");                                                                                                                                                              //
      Fall = true;                                                                                                                                                                                      //
    }                                                                                                                                                                                                   //
//******************************************************************************************************************************************************************************************************//

//******************************************************************************************************************************************************************************************************//
// On vérifie si la carte est à un arrêt arbitraire (1 m/s²) et si elle a été en descente. Calcule le temps de l'accélération de la balle et on informe avec "Fall" que l'accélération est finie        //
//                                                                                                                                                                                                      //
    if (Accel < 1 and Fall == true) {                                                                                                                                                                   //
      Fin = micros();                                                                                                                                                                                   //
      Serial.print("\nEn arrêt");                                                                                                                                                                       //
      Fall = false;                                                                                                                                                                                     //
//                                                                                                                                                                                                      //
      Serial.print("\nTemps de descente : ");                                                                                                                                                           //
      Temps = String(0.20+(Fin-Debut)/1000000);         // Ajout de 0.20s car le capteur met environ 0.2s à se rendre compte qu'il est en accélération, converti en str pour pouvoir l'écrire           //
//******************************************************************************************************************************************************************************************************//

//************************************************************************************************//
// Module pour écrire le texte "Temps" via Bluetooth sur le téléphone connecté                    //
//                                                                                                //
      if (Serial.available()) {                                                                   //
        if (hasKeyPressed) {                                                                      //
          blehid.keyRelease();      // Éviter qu'une touche reste appuyée                         //
          hasKeyPressed = false;    // Informe qu'aucune touche est appuyée                       //
//                                                                                                //
          delay(5);                 // Délai de sécurité                                          //
        }                                                                                         //
//                                                                                                //
        for (i=0; i < Temps.length(); i+=1) {                                                     //
          ch = Temps.charAt(i);     // Prend le caractère à l'emplacement i du str Temps          //
//                                                                                                //
          Serial.write(ch);         // Écho de l'envoi par Bluetooth                              //
          blehid.keyPress(ch);      // Appuie sur le caractère ch du clavier du téléphone         //
          hasKeyPressed = true;     // Informe qu'une touche a été appuyée                        //
//                                                                                                //
          delay(5);                 // Délai de sécurité                                          //
        }                                                                                         //
//                                                                                                //
        blehid.keyPress(char(13));  // Touche entrée                                              //
        delay(5);                   // Délai de sécurité                                          //
        blehid.keyRelease();        // Lâche prise sur toutes les touches                         //
      }                                                                                           //
    }                                                                                             //
//************************************************************************************************//

    delay(0);   // Délai de sécurité
}