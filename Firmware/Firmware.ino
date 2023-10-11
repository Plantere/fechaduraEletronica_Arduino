#include <Arduino.h>

#include "Keypad.h"

#include <LiquidCrystal_I2C.h>

#include "Relay.h"

#include <EEPROM.h>

#include "Utils.h"

Keypad keypad(KEYPAD_PIN_COL1, KEYPAD_PIN_COL2, KEYPAD_PIN_COL3, KEYPAD_PIN_ROW1, KEYPAD_PIN_ROW2, KEYPAD_PIN_ROW3, KEYPAD_PIN_ROW4);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Relay relayModule(RELAYMODULE_PIN_SIGNAL);
Utilitario utilitarioSistema = {
    0,
    0,
    30,
    50,
    54,
    0,
    0,
    0,
    0
};
Usuario usuarios[4];

void buscaUsuario();
void cadastrarUsuario(int tipoUsuario);
void apagarUsuario();
void liberarChave();
void formatarEeprom();
void inicializacaoSistema();
void aguardarInicializacao();
void armazenarEEPROM(Usuario usuario);
void obterUsuarios();
void buscaUsuario();
void apagarUsuario();
void formatarEeprom();
void mostrarOpcoesMenu();
void logarUsuario();
bool checarConta(Usuario usuario);
void acionarChave();
void desligarChave();
void mostrarUsuario(int usuarioID);
void limparUsuariosEEPROM();
void adicionarUsuariosEEPROM();

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

void removerElemento(Usuario * array, int index, int array_length) {
    int i;
    for (i = index; i < array_length - 1; i++) {
        array[i] = array[i + 1];
    }
}

void setup() {
    Serial.begin(9600);

    keypad.begin(keypadkeys);

    lcd.init();
    lcd.setBacklight(HIGH);
    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }
    inicializacaoSistema();

}

void loop() {
    utilitarioSistema.usuarioLogado = 0;

    if (!utilitarioSistema.sistemaIniciado) {
        lcd.setCursor(0, 0);
        lcd.print("Inicie o sistema");
        aguardarInicializacao();
    }

    if (utilitarioSistema.numeroUsuario <= 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Area de Registro");
        delay(500);
        cadastrarUsuario(1);

        if (utilitarioSistema.numeroUsuario > 0) {
            mostrarMenu();
        }
    } else {
        logarUsuario();
    }

    if (utilitarioSistema.usuarioLogado == 1 && utilitarioSistema.informacaoUsuarioLogado.tipoUsuario == 0) {
        liberarChave();
    } else if (utilitarioSistema.usuarioLogado == 1 && utilitarioSistema.informacaoUsuarioLogado.tipoUsuario == 1) {
        mostrarMenu();
    }

    delay(500);
}

void liberarChave() {
    acionarChave();
    delay(2000);
    desligarChave();
    delay(2000);
}

void acionarChave() {
    lcd.clear();
    lcd.setCursor(0, 0);
    relayModule.on();
    lcd.print("< Liberado >");
}

void desligarChave() {
    lcd.clear();
    lcd.setCursor(0, 0);
    relayModule.off();
    lcd.print("< Desligando >");
}

void inicializacaoSistema() {
    EEPROM.get(utilitarioSistema.enderecoNumeroUsuario, utilitarioSistema.numeroUsuario);

    if (isnan(utilitarioSistema.numeroUsuario)) {
        utilitarioSistema.numeroUsuario = 0;
    } else if (utilitarioSistema.numeroUsuario != 0) {
        obterUsuarios();
    }
}

void cadastrarUsuario(int tipoUsuario) {
    Usuario usuario;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Usuario:");

    delay(150);

    lcd.setCursor(0, 1);
    lcd.cursor();
    lcd.blink();

    char characterIdentificador;

    for (int i = 0; i < 4; i++) {
        characterIdentificador = 0;
        while (characterIdentificador == 0) {
            characterIdentificador = keypad.getKey();
            if (characterIdentificador == '*') {
                return;
            }
            delay(1);
        }

        usuario.identificador[i] = characterIdentificador;

        lcd.setCursor(0 + i, 1);
        lcd.print(characterIdentificador);
        lcd.cursor();
        lcd.blink();
    }

    delay(250);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Senha:");

    delay(150);

    lcd.setCursor(0, 1);
    lcd.cursor();
    lcd.blink();

    char characterSenha;
    for (int i = 0; i < 4; i++) {
        characterSenha = 0;
        while (characterSenha == 0) {
            characterSenha = keypad.getKey();
            if (characterSenha == '*') {
                return;
            }
            delay(1);
        }

        usuario.senha[i] = characterSenha;
        lcd.setCursor(0 + i, 1);
        lcd.print(characterSenha);
        lcd.cursor();
        lcd.blink();
    }

    delay(250);

    usuario.tipoUsuario = tipoUsuario;
    armazenarEEPROM(usuario);
    obterUsuarios();

    return;
}

void armazenarEEPROM(Usuario usuario) {
    int endereco = utilitarioSistema.enderecoUsuarios + utilitarioSistema.numeroUsuario * 9;

    for (int i = 0; i < 4; i++) {
        EEPROM.put(endereco + i, usuario.identificador[i] - '0');
        EEPROM.put(endereco + 5 + i, usuario.senha[i] - '0');
    }

    usuarios[utilitarioSistema.numeroUsuario].tipoUsuario = usuario.tipoUsuario;

    utilitarioSistema.numeroUsuario += 1;
    EEPROM.put(utilitarioSistema.enderecoTipoUsuario + utilitarioSistema.numeroUsuario - 1, usuario.tipoUsuario);
    EEPROM.put(utilitarioSistema.enderecoNumeroUsuario, utilitarioSistema.numeroUsuario);
}

void aguardarInicializacao() {
    do {
        char caracterPressionado = keypad.getKey();
        if (isDigit(caracterPressionado) || caracterPressionado == '*' || caracterPressionado == '#') {
            utilitarioSistema.sistemaIniciado = true;
        }
    } while (!utilitarioSistema.sistemaIniciado);
}

void obterUsuarios() {
    EEPROM.get(utilitarioSistema.enderecoNumeroUsuario, utilitarioSistema.numeroUsuario);

    for (int i = 0; i < utilitarioSistema.numeroUsuario; i++) {
        int endereco = 30 + i * 8;
        for (int j = 0; j < 4; j++) {
            usuarios[i].identificador[j] = EEPROM.read(endereco + j) + '0';
            usuarios[i].senha[j] = EEPROM.read(endereco + 5 + j) + '0';
        }


        EEPROM.get(utilitarioSistema.enderecoTipoUsuario + i, usuarios[i].tipoUsuario);
    }
}

void mostrarMenu() {
    configurarMenu(true);

    char characterIdentificador;
    while (utilitarioSistema.menuAtivo) {
        mostrarOpcoesMenu();

        characterIdentificador = 0;
        while (characterIdentificador == 0) {
            characterIdentificador = keypad.getKey();
            delay(1);
        }

        if (characterIdentificador == '6' && utilitarioSistema.opcaoAtual != 6) {
            utilitarioSistema.opcaoAtual += 1;
            utilitarioSistema.menuAlterado = 0;
        } else if (characterIdentificador == '4' && utilitarioSistema.opcaoAtual != 1) {
            utilitarioSistema.opcaoAtual -= 1;
            utilitarioSistema.menuAlterado = 0;
        }

        if (utilitarioSistema.opcaoAtual == 1 && characterIdentificador == '5') {
            cadastrarUsuario(0);
            configurarMenu(true);
        } else if (utilitarioSistema.opcaoAtual == 2 && characterIdentificador == '5') {
            buscaUsuario();
            configurarMenu(true);
        } else if (utilitarioSistema.opcaoAtual == 3 && characterIdentificador == '5') {
            apagarUsuario();
            configurarMenu(true);
        } else if (utilitarioSistema.opcaoAtual == 4 && characterIdentificador == '5') {
            formatarEeprom();
            configurarMenu(true);
            return;
        } else if (utilitarioSistema.opcaoAtual == 5 && characterIdentificador == '5') {
            liberarChave();
            configurarMenu(true);
        } else if (utilitarioSistema.opcaoAtual == 6 && characterIdentificador == '5') {
            configurarMenu(false);
        }

    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("< Saindo >");

    delay(2000);
}

void mostrarOpcoesMenu() {
    if (!utilitarioSistema.menuAlterado) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("< Menu >");

        if (utilitarioSistema.opcaoAtual == 1) {
            lcd.setCursor(0, 1);
            lcd.print("<[1/6] Cadastro>");
            utilitarioSistema.menuAlterado = true;
        } else if (utilitarioSistema.opcaoAtual == 2) {
            lcd.setCursor(0, 1);
            lcd.print("<[2/6] Busca>");
            utilitarioSistema.menuAlterado = true;
        } else if (utilitarioSistema.opcaoAtual == 3) {
            lcd.setCursor(0, 1);
            lcd.print("<[3/6] Apagar>");
            utilitarioSistema.menuAlterado = true;
        } else if (utilitarioSistema.opcaoAtual == 4) {
            lcd.setCursor(0, 1);
            lcd.print("<[4/6] Formatar>");
            utilitarioSistema.menuAlterado = true;
        } else if (utilitarioSistema.opcaoAtual == 5) {
            lcd.setCursor(0, 1);
            lcd.print("<[5/6] Liberar>");
            utilitarioSistema.menuAlterado = true;
        } else if (utilitarioSistema.opcaoAtual == 6) {
            lcd.setCursor(0, 1);
            lcd.print("<[6/6] Sair>");
            utilitarioSistema.menuAlterado = true;
        }
    }
}

void buscaUsuario() {
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

    mostrarUsuario(usuarioAtual);
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

        mostrarUsuario(usuarioAtual);
    }
}

void mostrarUsuario(int usuarioID) {
    lcd.setCursor(0, 1);
    lcd.print("<");
    for (int i = 0; i < 4; i++) {
        lcd.setCursor(0 + (i + 1), 1);
        lcd.print(usuarios[usuarioID].identificador[i]);
    }

    lcd.setCursor(5, 1);
    lcd.print(">");
}

void apagarUsuario() {
    char usuarioIdentificador[4];

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Usuario");

    delay(150);

    lcd.setCursor(0, 1);
    lcd.cursor();
    lcd.blink();
    char characterIdentificador;
    for (int i = 0; i < 4; i++) {
        characterIdentificador = 0;
        while (characterIdentificador == 0) {
            characterIdentificador = keypad.getKey();
            if (characterIdentificador == '*') {
                return;
            }
            delay(1);
        }

        usuarioIdentificador[i] = characterIdentificador;
        lcd.setCursor(0 + i, 1);
        lcd.print(characterIdentificador);
        lcd.cursor();
        lcd.blink();
    }

    int indexUsuario = 255;
    for (int i = 0; i < utilitarioSistema.numeroUsuario; i++) {
        for (int j = 0; j < 4; j++) {
            if (usuarios[i].identificador[j] != usuarioIdentificador[j] || usuarios[i].senha[j] != usuarioIdentificador[j]) {
                indexUsuario = 255;
                break;
            } else {
                indexUsuario = i;
            }
        }

        if (indexUsuario != 255) {
            break;
        }
    }

    if (indexUsuario == 255 || usuarios[indexUsuario].tipoUsuario == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("< Invalido >");
        delay(2000);
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("> Confirmar ?");

    char isConfirmed = 0;
    while (isConfirmed != '*' && isConfirmed != '#') {
        isConfirmed = keypad.getKey();
        delay(1);
    }

    if (isConfirmed == '*') {
        return;
    }

    removerElemento(usuarios, indexUsuario, utilitarioSistema.numeroUsuario);
    limparUsuariosEEPROM();
    adicionarUsuariosEEPROM();

    utilitarioSistema.numeroUsuario -= 1;
    EEPROM.put(utilitarioSistema.enderecoNumeroUsuario, utilitarioSistema.numeroUsuario);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("< Deletado >");
    delay(2000);

    return;
}

void formatarEeprom() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("> Confirmar ?");

    char digitoConfirmacao = 0;

    while (digitoConfirmacao != '*' && digitoConfirmacao != '#') {
        digitoConfirmacao = keypad.getKey();
        delay(1);
    }

    if (digitoConfirmacao == '*') {
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sist. Formatado");

    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }

    utilitarioSistema.numeroUsuario = 0;

    delay(2000);
}

void configurarMenu(bool menuAtivo) {
    utilitarioSistema.menuAtivo = menuAtivo;
    utilitarioSistema.opcaoAtual = 1;
    utilitarioSistema.menuAlterado = false;
}

void logarUsuario() {
    Usuario usuario;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Usuario");

    delay(150);

    lcd.setCursor(0, 1);
    lcd.cursor();
    lcd.blink();

    char characterIdentificador;
    for (int i = 0; i < 4; i++) {
        characterIdentificador = 0;
        while (characterIdentificador == 0) {
            characterIdentificador = keypad.getKey();
            if (characterIdentificador == '*') {
                return 0;
            }
            delay(1);
        }

        usuario.identificador[i] = characterIdentificador;
        lcd.setCursor(0 + i, 1);
        lcd.print(characterIdentificador);
        lcd.cursor();
        lcd.blink();
    }

    delay(250);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Senha");

    delay(150);

    lcd.setCursor(0, 1);
    lcd.cursor();
    lcd.blink();

    char characterSenha;
    for (int i = 0; i < 4; i++) {
        characterSenha = 0;

        while (characterSenha == 0) {
            characterSenha = keypad.getKey();
            if (characterSenha == '*') {
                return 0;
            }
            delay(1);
        }

        usuario.senha[i] = characterSenha;
        lcd.setCursor(0 + i, 1);
        lcd.print(characterSenha);
        lcd.cursor();
        lcd.blink();
    }

    delay(250);

    usuario.tipoUsuario = 0;

    bool usuarioLogado = checarConta(usuario);

    if (!usuarioLogado) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("< Invalido >");
        delay(2000);
        return 0;
    }

    utilitarioSistema.usuarioLogado = usuarioLogado;

    return;
}

void limparUsuariosEEPROM() {
    int endereco;

    for (int i = 0; i < utilitarioSistema.numeroUsuario; i++) {
        endereco = 0x30 + i * 8;

        for (int j = 0; j < 4; j++) {
            EEPROM.put(endereco + j, 0xFF);
            EEPROM.put(endereco + 5 + j, 0xFF);
        }

        EEPROM.put(utilitarioSistema.enderecoTipoUsuario + i, 0);
    }
}
void adicionarUsuariosEEPROM() {
    int endereco;

    for (int i = 0; i < utilitarioSistema.numeroUsuario - 1; i++) {
        endereco = 0x30 + i * 8;

        for (int j = 0; j < 4; j++) {
            EEPROM.put(endereco + j, usuarios[i].identificador[j]);
            EEPROM.put(endereco + 5 + j, usuarios[i].senha[j]);
        }

        EEPROM.put(utilitarioSistema.enderecoTipoUsuario + i, usuarios[i].tipoUsuario);
    }
}

bool checarConta(Usuario usuario) {
    bool usuarioLogado = false;
    int usuarioAtual = 0;

    while (usuarioLogado == 0 && usuarioAtual < utilitarioSistema.numeroUsuario) {
        for (int x = 0; x < 4; x++) {
            if (usuarios[usuarioAtual].identificador[x] != usuario.identificador[x] || usuarios[usuarioAtual].senha[x] != usuario.senha[x]) {
                usuarioLogado = false;
                break;
            } else {
                utilitarioSistema.informacaoUsuarioLogado = usuarios[usuarioAtual];
                usuarioLogado = true;
            }
        }
        usuarioAtual += 1;
    }

    return usuarioLogado;
}