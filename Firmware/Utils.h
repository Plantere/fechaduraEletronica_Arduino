#define KEYPAD_PIN_ROW3	7
#define KEYPAD_PIN_ROW1	5
#define KEYPAD_PIN_COL2	3
#define KEYPAD_PIN_COL3	4
#define KEYPAD_PIN_ROW2	6
#define KEYPAD_PIN_COL1	2
#define KEYPAD_PIN_ROW4	8
#define RELAYMODULE_PIN_SIGNAL	9

typedef struct {
   char identificador[4];
   char senha[4];
   int tipoUsuario;
} Usuario;

typedef struct {
   bool sistemaIniciado;
   bool usuarioLogado;
   int enderecoUsuarios;
   int enderecoTipoUsuario;
   int enderecoNumeroUsuario;
   int numeroUsuario;
   bool menuAtivo;
   int opcaoAtual;
   bool menuAlterado;
   Usuario informacaoUsuarioLogado;
   bool buzzerAtivo;
   int usuarioAutorizado;
} Utilitario;

