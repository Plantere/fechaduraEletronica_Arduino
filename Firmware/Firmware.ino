#include <WiFi.h>
#include "IOXhop_FirebaseESP32.h"
#include <ArduinoJson.h>
#include <cstring> 
#include "Utils.h"
#include "Keypad.h"
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#define WIFI_SSID "Plantere"
#define WIFI_PASSWORD "benficatorres"
#define FIREBASE_HOST "https://door-security-43ae7-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Fq4ggji0Q5oTVPcJSQQLwMuPMG7d7CALKutu9chN"

Usuario usuarios[10];
Keypad keypad(KEYPAD_PIN_COL1, KEYPAD_PIN_COL2, KEYPAD_PIN_COL3, KEYPAD_PIN_ROW1, KEYPAD_PIN_ROW2, KEYPAD_PIN_ROW3, KEYPAD_PIN_ROW4);
Utilitario utilitarioSistema = {false, false, true, false, 0, 0};
Usuario usuarioLogado;

char keypadkeys[ROWS][COLS] = {
    {
        '1',
        '2',
        '3'
    },
    {
        '4',
        '5',
        '6'
    },
    {
        '7',
        '8',
        '9'
    },
    {
        '*',
        '0',
        '#'
    }
};


void beginKeypad();
void beginLCD();
void beginRelay();
void connectToWifi();
void connectToFirebase();

char* readData(const char *title, char *data, int maxLength);
void handleRelay(bool isActive);
void showMessage(const char* message);
void configurateMenu(bool isActive);
void showMenu();
char readKeypadEntry();
void updateOptionActual(char character);
void executeActionMenu(char character);
void configurateMenu(bool isActive);
void showOptionsMenu();
void enableDoor();

bool storeUser(const char *username, const char *password,int tipoUsuario);
bool checkUsername(const char* username);
bool checkPassword(const char* username, const char* password);
bool createUser(int type);
void removeAllUsers();
void getAllUsers(StaticJsonDocument<768> doc);
bool loginUser();


LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println();

  beginRelay();
  beginLCD();
  beginKeypad();

  connectToWifi();
  connectToFirebase();
}

void loop() {
  utilitarioSistema.usuarioLogado = false;

  if(!utilitarioSistema.sistemaIniciado){
    showMessage("< Pressione >");
    startSystem();
  }

  if(utilitarioSistema.numeroUsuario <= 0) {
    showMessage("< Registro >");
      delay(500);
      createUser(1);
      if(utilitarioSistema.numeroUsuario > 0){
        showMenu();
      }
  }else{
    while(!loginUser());
  }

  if(utilitarioSistema.usuarioLogado == true && usuarioLogado.tipoUsuario == 0){
    enableDoor();
  }else if(utilitarioSistema.usuarioLogado == true && usuarioLogado.tipoUsuario == 1){
    showMenu();
  }
  delay(2000);
}


void startSystem(){
   do{
      if(readKeypadEntry()){
         utilitarioSistema.sistemaIniciado = true;
      }
   }while(!utilitarioSistema.sistemaIniciado);
}

void beginLCD(){
  lcd.init();
  lcd.setBacklight(HIGH);
}

void beginRelay(){
  pinMode(RELAYMODULE_PIN_SIGNAL, OUTPUT);
}

void beginKeypad(){
  keypad.begin(keypadkeys);
}

void connectToWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Tentando conexão com o WIFI - " + String(WIFI_SSID) + ":");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println();

  Serial.println("===== Conexão realizada com sucesso =====");
}

void connectToFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.stream("", [](FirebaseStream stream) {
    if (stream.getEvent() == "put" && stream.getPath() == "/Door/isOpened") {
      handleRelay(stream.getDataBool());
    }else if(stream.getEvent() == "put" && stream.getPath() == "/"){
      StaticJsonDocument<768> doc;
      deserializeJson(doc, stream.getDataString());
      getAllUsers(doc);
    }
  });
  Serial.println("==== Conexão realizada com sucesso com o Firebase ====");
}

char* readData(const char *title, char *data, int maxLength) {
    char character;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(title);
    lcd.setCursor(0, 1);
    lcd.blink();

    Serial.print(title);

    for (int i = 0; i < maxLength-1; i++) {
        character = 0;
        while (character == 0) {
            character = keypad.getKey();
            if (character == '*') {
                data[i] = '\0';
                return data;
            }
            delay(1);
        }

        data[i] = character;

        lcd.setCursor(i, 1);
        lcd.print(character);
        lcd.cursor();
        lcd.blink();
        Serial.print(character);
    }

    data[maxLength-1] = '\0';
    Serial.println();
    return data;
}

void handleRelay(bool isActive){
  digitalWrite(RELAYMODULE_PIN_SIGNAL, isActive == true ? HIGH : LOW);
}

void showMessage(const char* message){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(2000);
}

void showOptionsMenu() {
  const char* opcoesMenu[6] = {"Cadastro", "Busca", "Apagar", "Formatar", "Liberar", "Sair"};
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Menu >");

  if (utilitarioSistema.opcaoAtual >= 1 && utilitarioSistema.opcaoAtual <= 6) {
    lcd.setCursor(0, 1);
    lcd.print("<[" + String(utilitarioSistema.opcaoAtual) + "/6] " + opcoesMenu[utilitarioSistema.opcaoAtual - 1] + ">");
    utilitarioSistema.menuAlterado = true;
  }
}

void getAllUsers(StaticJsonDocument<768> doc){
  int totalUsers = doc["Users"]["totalUsers"];
  utilitarioSistema.numeroUsuario = totalUsers;
  Serial.print("totalUsers: ");
  Serial.println(totalUsers);

  int username;
  int password;
  int type;

  for(int index = 0; index < totalUsers; index++){
    username = doc["Users"][String(index+1)]["username"];
    password = doc["Users"][String(index+1)]["password"];
    type = doc["Users"][String(index+1)]["type"];

    strncpy(usuarios[index].identificador, String(username).c_str(), sizeof(usuarios[index].identificador) - 1);
    strncpy(usuarios[index].senha, String(password).c_str(), sizeof(usuarios[index].senha) - 1);
    usuarios[index].tipoUsuario = type;

    usuarios[index].identificador[sizeof(usuarios[index].identificador) - 1] = '\0';
    usuarios[index].senha[sizeof(usuarios[index].senha) - 1] = '\0';
  }
}

bool storeUser(const char *username, const char *password, int type){
  int newId = Firebase.getInt("/Users/totalUsers") + 1;

  String basePath = "/Users/" + String(newId);

  StaticJsonDocument<768> newUser;
  newUser["username"] = username;
  newUser["password"] = password;
  newUser["type"] = type;
  Firebase.set(basePath, newUser);

  Firebase.setInt("/Users/totalUsers", newId);
  utilitarioSistema.numeroUsuario = newId;

  strncpy(usuarios[newId-1].identificador, String(username).c_str(), sizeof(usuarios[newId-1].identificador) - 1);
  strncpy(usuarios[newId-1].senha, String(password).c_str(), sizeof(usuarios[newId-1].senha) - 1);
  usuarios[newId-1].tipoUsuario = type;

  usuarios[newId-1].identificador[sizeof(usuarios[newId-1].identificador) - 1] = '\0';
  usuarios[newId-1].senha[sizeof(usuarios[newId-1].senha) - 1] = '\0';
  return true;
}

bool checkUsername(const char* username) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(usuarios[i].identificador, username) == 0) {
            return true;
        }
    }
    return false;
}

bool checkPassword(const char* username, const char* password) {
    for (int i = 0; i < 10; i++) {
        if (strcmp(usuarios[i].identificador, username) == 0) {
            if (strcmp(usuarios[i].senha, password) == 0) {
                usuarioLogado = usuarios[i];
                utilitarioSistema.usuarioLogado = true;
                return true;
            }
            break;
        }
    }
    return false;
}

void removeAllUsers() {
  Firebase.setString("/Users", "");
  Firebase.setInt("/Users/totalUsers", 0);
}

void factoryReset(){
  showMessage("< Resetando >");
  removeAllUsers();
}

bool removeByUsername(){ 
  char username[5];

  readData("ID: ", username, 5);
  if(!checkUsername(username)){
    showMessage("< Inexistente >");
    return false;
  }

  showMessage("< Deletando >");

  removeAllUsers();

  for (int i = 0; i < 10; i++) {
    if(usuarios[i].identificador[0] == '\0'){
      continue;
    }

    
    if (strcmp(usuarios[i].identificador, username) == 0) {
      usuarios[i].identificador[0] = '\0';
      usuarios[i].senha[0] = '\0';
      usuarios[i].tipoUsuario = -1;
      continue;
    }

    storeUser(usuarios[i].identificador, usuarios[i].senha, usuarios[i].tipoUsuario);
  }
  return true;
}

bool loginUser(){
  char username[5];
  char password[5];

  readData("ID: ", username, 5);

  if(!checkUsername(username)){
    return false;
  }

  readData("Senha: ", password, 5);

  return checkPassword(username, password);
}

bool createUser(int type){
  char username[5];
  char password[5];

  readData("ID: ", username, 5);

  while(checkUsername(username)){
    showMessage("< Existente >");
    readData("ID: ", username, 5);
  }

  readData("Senha: ", password, 5);

  showMessage("< Criando... >");

  return storeUser(username, password, type);
}

void showMenu(){
  configurateMenu(true);

  char character;
  while (utilitarioSistema.menuAtivo) {
      showOptionsMenu();
      character = readKeypadEntry();

      updateOptionActual(character);
      executeActionMenu(character);
  }

  showMessage("< Saindo >");
}

char readKeypadEntry() {
    char character = 0;
    while (character == 0) {
        character = keypad.getKey();
    }
    return character;
}

void updateOptionActual(char character) {
    if ((character == '6' && utilitarioSistema.opcaoAtual < 6) ||
        (character == '4' && utilitarioSistema.opcaoAtual > 1)) {
        utilitarioSistema.opcaoAtual += (character == '6') ? 1 : -1;
        utilitarioSistema.menuAlterado = false;
    }
}

void searchUser() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Usuarios");

    int usuarioAtual = 0;

    if (utilitarioSistema.numeroUsuario == 0) {
        lcd.setCursor(0, 1);
        lcd.print("< Nenhum >");
        delay(2000);
        return;
    }

    showUser(usuarioAtual);
    char comando = 0;

    while (true) {
        comando = 0;
        while (comando == 0) {
            comando = keypad.getKey();
            delay(1);
        }

        if (comando == '4' && usuarioAtual > 0) {
            usuarioAtual -= 1;
        } else if (comando == '*') {
            return;
        } else if (comando == '6' && usuarioAtual < utilitarioSistema.numeroUsuario - 1) {
            usuarioAtual += 1;
        } else {
            continue;
        }

        showUser(usuarioAtual);
    }
}

void showUser(int userId) {
    lcd.setCursor(0, 1);
    lcd.print("<");
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(0 + (i + 1), 1);
        lcd.print(usuarios[userId].identificador[i]);
    }

    lcd.setCursor(5, 1);
    lcd.print(">");
}

void executeActionMenu(char character) {
    if (character == '5') {
        switch (utilitarioSistema.opcaoAtual) {
            case 1: createUser(0); break;
            case 2: searchUser(); break;
            case 3: removeByUsername(); break;
            case 4: factoryReset(); break;
            case 5: enableDoor(); break;
            case 6: configurateMenu(false); return;
            default: break;
        }
        if (utilitarioSistema.opcaoAtual != 6) {
            configurateMenu(true);
        }
    }
}

void configurateMenu(bool isActive) {
    utilitarioSistema.menuAtivo = isActive;
    utilitarioSistema.opcaoAtual = 1;
    utilitarioSistema.menuAlterado = false;
}

void enableDoor(){
  showMessage("< Aberto >");
  handleRelay(true);
  delay(5000);
  showMessage("< Fechado >");
  handleRelay(false);
  delay(5000);

  return;
}