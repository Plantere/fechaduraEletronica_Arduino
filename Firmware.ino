#include "Arduino.h"
#include "Keypad.h"
#include <LiquidCrystal_I2C.h>
#include "Relay.h"

#define KEYPAD_PIN_ROW3	7
#define KEYPAD_PIN_ROW1	5
#define KEYPAD_PIN_COL2	3
#define KEYPAD_PIN_COL3	4
#define KEYPAD_PIN_ROW2	6
#define KEYPAD_PIN_COL1	2
#define KEYPAD_PIN_ROW4	8
#define RELAYMODULE_PIN_SIGNAL	9

char keypadkeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

int opcaoAtual = 1;
bool menuAtivo = true;

Keypad keypad(KEYPAD_PIN_COL1,KEYPAD_PIN_COL2,KEYPAD_PIN_COL3,KEYPAD_PIN_ROW1,KEYPAD_PIN_ROW2,KEYPAD_PIN_ROW3,KEYPAD_PIN_ROW4);
LiquidCrystal_I2C lcd(0x27,16,2);
Relay relayModule(RELAYMODULE_PIN_SIGNAL);

long time0;

void setup() 
{
  Serial.begin(9600);
  keypad.begin(keypadkeys);
  lcd.init();
  lcd.setBacklight(HIGH);
}

void loop() 
{
  while(menuAtivo){
    mostrarMenu();
  }

  // char keypadKey = keypad.getKey();

  // if (isDigit(keypadKey) ||  keypadKey == '*' ||  keypadKey == '#')
  // {
  //     if(keypadKey == '1'){
  //         lcd.setBacklight(HIGH);
  //         lcd.setCursor(0,0);
  //         lcd.print("> Ligado <");
  //         relayModule.on();
  //         delay(1000);
  //     }else if(keypadKey == '0'){
  //         lcd.setBacklight(HIGH);
  //         lcd.setCursor(0,0);
  //         lcd.print("> Desligado <");
  //         relayModule.off();
  //         delay(1000);
  //     }
  // }
  // delay(50);
}

void mostrarMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Menu >");

  switch(opcaoAtual) {
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("<[1/6] Cadastro>");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("<[2/6] Busca>");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print("<[3/6] Apagar>");
      break;
    case 4:
      lcd.setCursor(0, 1);
      lcd.print("<[4/6] Liberar>");
      break;
    case 5:
      lcd.setCursor(0, 1);
      lcd.print("<[5/6] Desligar>");
      break;
    case 6:
      lcd.setCursor(0, 1);
      lcd.print("<[6/6] Sair>");
      break;
  }
  
  char key = keypad.getKey();
  if(key){
    if (key == '6' && opcaoAtual != 6) {
      opcaoAtual++;
    } else if (key == '4' && opcaoAtual != 1) {
      opcaoAtual--;
    } else if(key == '5') {
      switch(opcaoAtual){
        case 1:
          cadastrarUsuario();
          break;
        case 2:
          buscaUsuario();
          break;
        case 3:
          apagarUsuario();
          break;
        case 4:
          ligarRelay();
          break;
        case 5:
          desligarRelay();
          break;
      }
    }
  }else if(key == '*'){
    menuAtivo = false;
  }

  delay(100);
}


void cadastrarUsuario(){
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("> Função nao desenvolvida - CadastrarUsuario <");
  delay(10000);
  lcd.clear();
}
void buscaUsuario(){
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("> Função nao desenvolvida - buscaUsuario <");
  delay(10000);
  lcd.clear();
}
void apagarUsuario(){
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("> Função nao desenvolvida - apagarUsuario <");
  delay(10000);
  lcd.clear();
}

void ligarRelay(){
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("> Ligado <");
  relayModule.off();
  delay(10000);
  lcd.clear();
}
void desligarRelay(){
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("> Desligado <");
  relayModule.on();
  delay(10000);
  lcd.clear();
}