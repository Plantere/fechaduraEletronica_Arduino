#define KEYPAD_PIN_COL2	13
#define KEYPAD_PIN_ROW1	12
#define KEYPAD_PIN_COL1	14
#define KEYPAD_PIN_ROW4	27
#define KEYPAD_PIN_COL3	26
#define KEYPAD_PIN_ROW3	25
#define KEYPAD_PIN_ROW2	33

#define RELAYMODULE_PIN_SIGNAL 32

#define WIFI_SSID "Plantere"
#define WIFI_PASSWORD "benficatorres"
#define FIREBASE_HOST "https://door-security-43ae7-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Fq4ggji0Q5oTVPcJSQQLwMuPMG7d7CALKutu9chN"

typedef struct {
   char identificador[5];
   char senha[5];
   int tipoUsuario;
} Usuario;

typedef struct {
   bool sistemaIniciado;
   bool usuarioLogado;
   bool menuAtivo;
   bool menuAlterado;
   int opcaoAtual;
   int numeroUsuario;
} Utilitario;

