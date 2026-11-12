#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// Configuración WiFi - CAMBIA ESTOS VALORES
const char* ssid = "";           // Nombre de tu red WiFi
const char* password = "";    // Contraseña de tu WiFi

// Configuración de la matriz LED
#define LED_PIN 16
#define WIDTH 7
#define HEIGHT 11
#define NUM_LEDS (WIDTH * HEIGHT)

// Configuración del buzzer
#define BUZZER_PIN 4

// Variables de alarma
int ALARMA_HORA = 0;
int ALARMA_MINUTO = 0;
int ALARMA_DURACION = 30;
int MELODIA_SELECCIONADA = 0;  // 0=Mario Tema, 1=Mario Nivel 2, 2=Estrella, 3=Trompeta
bool ALARMA_ACTIVADA = false;  // Desactivada por defecto hasta que el usuario configure
bool alarmaActiva = false;
unsigned long alarmaInicio = 0;

// Variables para modo diseno
bool modoDiseno = false;
int disenoActual = 0; // 0=Pacman, 1=Corazon, 2=Carita, 3=Personalizado

// Estructura para almacenar color RGB
struct ColorRGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Matriz para diseno personalizado con colores
bool disenoPersonalizado[11][7] = {0};
ColorRGB coloresPersonalizados[11][7];

// Variables para la melodía
int notaActual = 0;
unsigned long tiempoNota = 0;
bool melodiaEnCurso = false;
unsigned long ultimaMelodia = 0;

// Colores configurables
uint32_t colorHoraDecena;
uint32_t colorHoraUnidad;
uint32_t colorMinutoDecena;
uint32_t colorMinutoUnidad;
uint32_t colorPuntos;

// Sistema de efectos de color
int modoColor = 0;  // 0=Estatico, 1=Arcoiris, 2=Parpadeo, 3=Random, 4=Gradiente
unsigned long ultimoCambioColor = 0;
int hueActual = 0;  // Para efecto arcoiris (0-360)
bool estadoParpadeo = true;
int colorRandomR = 255, colorRandomG = 0, colorRandomB = 0;

// Colores estaticos personalizados (por defecto)
int colorEstatico[4][3] = {
  {0, 255, 255},    // Hora decena - Cyan
  {0, 255, 255},    // Hora unidad - Cyan
  {255, 255, 0},    // Minuto decena - Amarillo
  {255, 255, 0}     // Minuto unidad - Amarillo
};

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
RTC_DS3231 rtc;
WebServer server(80);

// Definición de números (5x3 píxeles)
const bool numeros[10][5][3] = {
  {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}},
  {{0,1,0},{1,1,0},{0,1,0},{0,1,0},{1,1,1}},
  {{1,1,1},{0,0,1},{1,1,1},{1,0,0},{1,1,1}},
  {{1,1,1},{0,0,1},{1,1,1},{0,0,1},{1,1,1}},
  {{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1}},
  {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}},
  {{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1}},
  {{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}},
  {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1}},
  {{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1}}
};

// Definición de letras para "WAKE UP!"
const bool letras[7][5][3] = {
  {{1,0,1},{1,0,1},{1,0,1},{1,1,1},{1,0,1}},
  {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1}},
  {{1,0,1},{1,1,0},{1,1,0},{1,0,1},{1,0,1}},
  {{1,1,1},{1,0,0},{1,1,1},{1,0,0},{1,1,1}},
  {{1,0,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}},
  {{1,1,1},{1,0,1},{1,1,1},{1,0,0},{1,0,0}},
  {{0,1,0},{0,1,0},{0,1,0},{0,0,0},{0,1,0}}
};

// Disenos predeterminados (7x11 píxeles)
// PACMAN
const bool pacman[11][7] = {
  {0,0,1,1,1,0,0},
  {0,1,1,1,1,1,0},
  {1,1,1,1,1,1,1},
  {1,1,0,1,1,1,0},
  {1,1,1,1,1,0,0},
  {1,1,1,1,0,0,0},
  {1,1,1,1,1,0,0},
  {1,1,0,1,1,1,0},
  {1,1,1,1,1,1,1},
  {0,1,1,1,1,1,0},
  {0,0,1,1,1,0,0}
};

// CORAZÓN
const bool corazon[11][7] = {
  {0,0,0,0,0,0,0},
  {0,1,1,0,1,1,0},
  {1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1},
  {0,1,1,1,1,1,0},
  {0,1,1,1,1,1,0},
  {0,0,1,1,1,0,0},
  {0,0,1,1,1,0,0},
  {0,0,0,1,0,0,0},
  {0,0,0,0,0,0,0}
};

// CARITA FELIZ
const bool carita[11][7] = {
  {0,0,1,1,1,0,0},
  {0,1,1,1,1,1,0},
  {1,1,0,1,0,1,1},
  {1,1,0,1,0,1,1},
  {1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1},
  {1,0,1,1,1,0,1},
  {1,1,0,0,0,1,1},
  {1,1,1,1,1,1,1},
  {0,1,1,1,1,1,0},
  {0,0,1,1,1,0,0}
};

// ========== MELODIAS DISPONIBLES ==========

// MELODIA 0: Super Mario Bros - Tema Principal
const int melodia0[] = {
  659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 0, 0, 392,
  0, 0, 523, 0, 0, 392, 0, 0, 330, 0, 0, 440, 0, 494,
  0, 466, 0, 440, 0, 392, 659, 784, 880, 0, 698, 784,
  0, 659, 0, 523, 587, 494, 0, 0
};
const int duraciones0[] = {
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 200, 150, 150, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 150
};
const int numeroNotas0 = 47;

// MELODIA 1: Super Mario Bros - Nivel Subterraneo
const int melodia1[] = {
  523, 659, 0, 523, 0, 523, 0, 523, 659, 0, 523, 0,
  392, 440, 494, 523, 587, 659, 698, 784, 0,
  698, 784, 0, 698, 784, 659, 587, 523, 440, 392, 0,
  523, 659, 0, 523, 0, 523, 0, 523, 659, 0
};
const int duraciones1[] = {
  200, 200, 100, 200, 100, 200, 100, 200, 200, 100, 200, 100,
  200, 200, 200, 200, 200, 200, 200, 300, 100,
  200, 300, 100, 200, 200, 200, 200, 200, 200, 300, 100,
  200, 200, 100, 200, 100, 200, 100, 200, 200, 100
};
const int numeroNotas1 = 42;

// MELODIA 2: Super Mario Bros - Estrella de Invencibilidad
const int melodia2[] = {
  587, 622, 659, 698, 784, 831, 880, 932,
  932, 932, 932, 0, 784, 831, 880, 932,
  932, 932, 932, 0, 784, 831, 880, 932,
  880, 831, 784, 698, 659, 622, 587, 523,
  523, 587, 622, 659, 698, 784, 831, 880
};
const int duraciones2[] = {
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 200, 50, 100, 100, 100, 100,
  100, 100, 200, 50, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 200
};
const int numeroNotas2 = 40;

// MELODIA 3: Toque de Trompeta Militar (Reveille)
const int melodia3[] = {
  392, 392, 392, 392, 392, 392, 392, 523, 392, 523,
  392, 523, 659, 659, 523, 392, 523, 392, 294, 392,
  392, 392, 392, 392, 392, 523, 392, 523, 392, 523,
  659, 784, 659, 523, 392, 523, 659, 523, 392
};
const int duraciones3[] = {
  150, 150, 200, 150, 150, 200, 150, 150, 150, 150,
  150, 150, 200, 200, 150, 150, 150, 150, 150, 300,
  150, 150, 200, 150, 150, 200, 150, 150, 150, 150,
  150, 150, 150, 150, 150, 150, 150, 150, 400
};
const int numeroNotas3 = 39;

// MELODIA 4: Zelda - Tema Principal
const int melodia4[] = {
  494, 587, 659, 494, 587, 659, 494, 587, 659, 587, 494,
  440, 523, 0, 523, 587, 659, 784, 0,
  494, 587, 659, 494, 587, 659, 494, 587, 659, 587, 494,
  440, 523, 0, 523, 494, 440, 392, 0
};
const int duraciones4[] = {
  150, 150, 300, 150, 150, 300, 150, 150, 150, 150, 300,
  150, 450, 150, 150, 150, 150, 450, 150,
  150, 150, 300, 150, 150, 300, 150, 150, 150, 150, 300,
  150, 450, 150, 150, 150, 150, 450, 150
};
const int numeroNotas4 = 38;

// MELODIA 5: Star Wars - Imperial March
const int melodia5[] = {
  392, 392, 392, 311, 466, 392, 311, 466, 392, 0,
  587, 587, 587, 622, 466, 370, 311, 466, 392, 0,
  784, 392, 392, 784, 740, 698, 659, 622, 659, 0,
  415, 554, 523, 494, 466, 440, 466
};
const int duraciones5[] = {
  350, 350, 350, 250, 100, 350, 250, 100, 700, 150,
  350, 350, 350, 250, 100, 350, 250, 100, 700, 150,
  350, 250, 100, 350, 250, 100, 100, 100, 350, 150,
  250, 350, 250, 100, 100, 100, 700
};
const int numeroNotas5 = 37;

// MELODIA 6: Nokia Ringtone Clasico
const int melodia6[] = {
  659, 587, 370, 415,
  554, 494, 294, 330,
  494, 440, 277, 294,
  440, 0
};
const int duraciones6[] = {
  150, 150, 150, 150,
  150, 150, 150, 150,
  150, 150, 150, 150,
  300, 150
};
const int numeroNotas6 = 14;

// Punteros a las melodias
const int* melodias[] = {melodia0, melodia1, melodia2, melodia3, melodia4, melodia5, melodia6};
const int* duracionesMelodias[] = {duraciones0, duraciones1, duraciones2, duraciones3, duraciones4, duraciones5, duraciones6};
const int numeroNotasMelodias[] = {numeroNotas0, numeroNotas1, numeroNotas2, numeroNotas3, numeroNotas4, numeroNotas5, numeroNotas6};

// Nombres de las melodias
const char* nombresMelodias[] = {
  "Super Mario - Tema Principal",
  "Super Mario - Nivel Subterraneo", 
  "Super Mario - Estrella",
  "Trompeta Militar",
  "Zelda - Tema Principal",
  "Star Wars - Imperial March",
  "Nokia Ringtone"
};

int getPixelIndex(int x, int y) {
  int realY = HEIGHT - 1 - y;
  if (x % 2 == 0) {
    return x * HEIGHT + realY;
  } else {
    return x * HEIGHT + (HEIGHT - 1 - realY);
  }
}

void setPixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
    strip.setPixelColor(getPixelIndex(x, y), color);
  }
}

void clearMatrix() {
  strip.clear();
}

void drawNumber(int num, int startX, int startY, uint32_t color) {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (numeros[num][y][x]) {
        setPixel(startX + x, startY + y, color);
      }
    }
  }
}

void drawLetter(int letra, int startX, int startY, uint32_t color) {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (letras[letra][y][x]) {
        setPixel(startX + x, startY + y, color);
      }
    }
  }
}

void drawColon(int startX, int startY, uint32_t color, bool blink) {
  if (blink) {
    setPixel(startX, startY + 1, color);
    setPixel(startX, startY + 3, color);
  }
}

// ========== FUNCIONES DE EFECTOS DE COLOR ==========

// Convertir HSV a RGB (para efecto arcoiris)
uint32_t HSVtoRGB(int hue, int sat, int val) {
  int r, g, b;
  int h = hue % 360;
  int s = sat;
  int v = val;
  
  int c = (v * s) / 255;
  int x = (c * (60 - abs((h % 120) - 60))) / 60;
  int m = v - c;
  
  if (h < 60) { r = c; g = x; b = 0; }
  else if (h < 120) { r = x; g = c; b = 0; }
  else if (h < 180) { r = 0; g = c; b = x; }
  else if (h < 240) { r = 0; g = x; b = c; }
  else if (h < 300) { r = x; g = 0; b = c; }
  else { r = c; g = 0; b = x; }
  
  return strip.Color(r + m, g + m, b + m);
}

// Actualizar colores según el modo seleccionado
void actualizarColoresReloj() {
  switch(modoColor) {
    case 0: // Estatico - Usar colores personalizados
      colorHoraDecena = strip.Color(colorEstatico[0][0], colorEstatico[0][1], colorEstatico[0][2]);
      colorHoraUnidad = strip.Color(colorEstatico[1][0], colorEstatico[1][1], colorEstatico[1][2]);
      colorMinutoDecena = strip.Color(colorEstatico[2][0], colorEstatico[2][1], colorEstatico[2][2]);
      colorMinutoUnidad = strip.Color(colorEstatico[3][0], colorEstatico[3][1], colorEstatico[3][2]);
      colorPuntos = strip.Color(255, 255, 255);
      break;
      
    case 1: // Arcoiris - Ciclo continuo
      if (millis() - ultimoCambioColor > 50) {
        hueActual = (hueActual + 2) % 360;
        ultimoCambioColor = millis();
      }
      colorHoraDecena = HSVtoRGB(hueActual, 255, 255);
      colorHoraUnidad = HSVtoRGB((hueActual + 90) % 360, 255, 255);
      colorMinutoDecena = HSVtoRGB((hueActual + 180) % 360, 255, 255);
      colorMinutoUnidad = HSVtoRGB((hueActual + 270) % 360, 255, 255);
      colorPuntos = HSVtoRGB((hueActual + 45) % 360, 255, 255);
      break;
      
    case 2: // Parpadeo - Alterna entre color y apagado
      if (millis() - ultimoCambioColor > 500) {
        estadoParpadeo = !estadoParpadeo;
        ultimoCambioColor = millis();
      }
      if (estadoParpadeo) {
        colorHoraDecena = strip.Color(colorEstatico[0][0], colorEstatico[0][1], colorEstatico[0][2]);
        colorHoraUnidad = strip.Color(colorEstatico[1][0], colorEstatico[1][1], colorEstatico[1][2]);
        colorMinutoDecena = strip.Color(colorEstatico[2][0], colorEstatico[2][1], colorEstatico[2][2]);
        colorMinutoUnidad = strip.Color(colorEstatico[3][0], colorEstatico[3][1], colorEstatico[3][2]);
        colorPuntos = strip.Color(255, 255, 255);
      } else {
        colorHoraDecena = colorHoraUnidad = colorMinutoDecena = colorMinutoUnidad = colorPuntos = 0;
      }
      break;
      
    case 3: // Random - Cambia a colores aleatorios cada segundo
      if (millis() - ultimoCambioColor > 1000) {
        colorRandomR = random(256);
        colorRandomG = random(256);
        colorRandomB = random(256);
        ultimoCambioColor = millis();
      }
      colorHoraDecena = strip.Color(colorRandomR, colorRandomG, colorRandomB);
      colorHoraUnidad = strip.Color(colorRandomG, colorRandomB, colorRandomR);
      colorMinutoDecena = strip.Color(colorRandomB, colorRandomR, colorRandomG);
      colorMinutoUnidad = strip.Color(colorRandomR, colorRandomB, colorRandomG);
      colorPuntos = strip.Color(255, 255, 255);
      break;
      
    case 4: // Gradiente - Transicion suave entre colores
      if (millis() - ultimoCambioColor > 30) {
        hueActual = (hueActual + 1) % 360;
        ultimoCambioColor = millis();
      }
      uint32_t color1 = HSVtoRGB(hueActual, 255, 255);
      uint32_t color2 = HSVtoRGB((hueActual + 180) % 360, 255, 255);
      colorHoraDecena = color1;
      colorHoraUnidad = color1;
      colorMinutoDecena = color2;
      colorMinutoUnidad = color2;
      colorPuntos = HSVtoRGB((hueActual + 90) % 360, 255, 200);
      break;
  }
}

void displayTime(int hora, int minuto) {
  clearMatrix();
  
  // Actualizar colores según modo seleccionado
  actualizarColoresReloj();
  
  int h1 = hora / 10;
  int h2 = hora % 10;
  int m1 = minuto / 10;
  int m2 = minuto % 10;
  
  drawNumber(h1, 0, 0, colorHoraDecena);
  drawNumber(h2, 4, 0, colorHoraUnidad);
  drawNumber(m1, 0, 6, colorMinutoDecena);
  drawNumber(m2, 4, 6, colorMinutoUnidad);
  drawColon(3, 3, colorPuntos, (millis() / 500) % 2 == 0);
  
  strip.show();
}

void displayWakeUp() {
  static int scrollPos = WIDTH;
  static unsigned long lastScroll = 0;
  
  if (millis() - lastScroll > 150) {
    lastScroll = millis();
    scrollPos--;
    if (scrollPos < -28) {
      scrollPos = WIDTH;
    }
  }
  
  clearMatrix();
  
  uint32_t colores[] = {
    strip.Color(255, 0, 0), strip.Color(255, 165, 0), strip.Color(255, 255, 0),
    strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(128, 0, 255),
    strip.Color(255, 0, 255)
  };
  
  for (int i = 0; i < 7; i++) {
    int offset = i * 4;
    if (i >= 4) offset += 2;
    drawLetter(i, scrollPos + offset, 3, colores[i]);
  }
  
  strip.show();
}

void displayDiseno(int tipo) {
  clearMatrix();
  
  const bool (*diseno)[7] = NULL;
  uint32_t color;
  bool usarColoresPersonalizados = false;
  
  switch(tipo) {
    case 0: // Pacman
      diseno = pacman;
      color = strip.Color(255, 255, 0); // Amarillo
      break;
    case 1: // Corazon
      diseno = corazon;
      color = strip.Color(255, 0, 100); // Rosa/Rojo
      break;
    case 2: // Carita
      diseno = carita;
      color = strip.Color(255, 200, 0); // Amarillo suave
      break;
    case 3: // Personalizado con colores
      diseno = disenoPersonalizado;
      usarColoresPersonalizados = true;
      break;
  }
  
  if (diseno != NULL) {
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        if (diseno[y][x]) {
          if (usarColoresPersonalizados) {
            // Usar color específico de cada píxel
            ColorRGB colorPixel = coloresPersonalizados[y][x];
            color = strip.Color(colorPixel.r, colorPixel.g, colorPixel.b);
          }
          setPixel(x, y, color);
        }
      }
    }
  }
  
  strip.show();
}

void iniciarMelodia() {
  notaActual = 0;
  melodiaEnCurso = true;
  tiempoNota = millis();
  ultimaMelodia = millis();
}

void actualizarMelodia() {
  if (!melodiaEnCurso) return;
  
  const int* melodiaActual = melodias[MELODIA_SELECCIONADA];
  const int* duracionesActual = duracionesMelodias[MELODIA_SELECCIONADA];
  int numNotas = numeroNotasMelodias[MELODIA_SELECCIONADA];
  
  if (millis() - tiempoNota >= duracionesActual[notaActual] + 30) {
    notaActual++;
    
    if (notaActual >= numNotas) {
      noTone(BUZZER_PIN);
      melodiaEnCurso = false;
      notaActual = 0;
      return;
    }
    
    if (melodiaActual[notaActual] != 0) {
      tone(BUZZER_PIN, melodiaActual[notaActual]);
    } else {
      noTone(BUZZER_PIN);
    }
    
    tiempoNota = millis();
  }
}

void verificarAlarma(int hora, int minuto, int segundo) {
  if (!ALARMA_ACTIVADA) return;
  
  // Debug cada minuto
  static int ultimoMinutoDebug = -1;
  if (minuto != ultimoMinutoDebug) {
    Serial.printf("[DEBUG] Hora actual: %02d:%02d - Alarma configurada: %02d:%02d - Estado alarma: %s\n", 
                  hora, minuto, ALARMA_HORA, ALARMA_MINUTO, alarmaActiva ? "SONANDO" : "ESPERANDO");
    ultimoMinutoDebug = minuto;
  }
  
  // Si la alarma ya sonó, resetearla cuando pase el minuto
  if (alarmaActiva && (hora != ALARMA_HORA || minuto != ALARMA_MINUTO)) {
    // Ya pasó el minuto de la alarma, permitir que vuelva a sonar
  }
  
  if (hora == ALARMA_HORA && minuto == ALARMA_MINUTO && segundo == 0 && !alarmaActiva) {
    alarmaActiva = true;
    alarmaInicio = millis();
    iniciarMelodia();
    Serial.println("¡¡¡ ALARMA ACTIVADA !!!");
  }
  
  if (alarmaActiva) {
    if ((millis() - alarmaInicio) > (ALARMA_DURACION * 1000)) {
      alarmaActiva = false;
      melodiaEnCurso = false;
      noTone(BUZZER_PIN);
      Serial.println("Alarma finalizada");
      return;
    }
    
    actualizarMelodia();
    
    if (!melodiaEnCurso && (millis() - ultimaMelodia) > 5000) {
      iniciarMelodia();
    }
  }
}

void inicializarColores() {
  colorHoraDecena = strip.Color(0, 100, 255);
  colorHoraUnidad = strip.Color(0, 255, 100);
  colorMinutoDecena = strip.Color(255, 100, 0);
  colorMinutoUnidad = strip.Color(255, 0, 100);
  colorPuntos = strip.Color(255, 0, 0);
}

void mostrarAlarmaConfigurada() {
  Serial.println("======================");
  Serial.print("⏰ Alarma configurada para las ");
  int horaAlarma12 = ALARMA_HORA;
  String periodo = " AM";
  
  if (horaAlarma12 == 0) {
    horaAlarma12 = 12;
  } else if (horaAlarma12 >= 12) {
    periodo = " PM";
    if (horaAlarma12 > 12) {
      horaAlarma12 = horaAlarma12 - 12;
    }
  }
  
  if (horaAlarma12 < 10) Serial.print("0");
  Serial.print(horaAlarma12);
  Serial.print(":");
  if (ALARMA_MINUTO < 10) Serial.print("0");
  Serial.print(ALARMA_MINUTO);
  Serial.print(periodo);
  Serial.print(" (");
  if (ALARMA_HORA < 10) Serial.print("0");
  Serial.print(ALARMA_HORA);
  Serial.print(":");
  if (ALARMA_MINUTO < 10) Serial.print("0");
  Serial.print(ALARMA_MINUTO);
  Serial.println(" formato 24h)");
  Serial.println("======================");
}

// Función para servir la página principal
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-sRIl4kxILFvY47J16cr9ZwB07vP4J8+LH7qKQnuqkuIAvNWLzeN8tE5YBujZqJLB" crossorigin="anonymous">
    <title>TikTime</title>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:ital,wght@0,200..800;1,200..800&display=swap');
      body {
        font-family: "Plus Jakarta Sans", sans-serif;
        background-color: #f4f4f9;
        color: #333;
      }
      .title {
        color: #007bff;
        margin-bottom: 30px;
      }
      .div-container{
        background-color: rgba(1,1,1,0.1);
        width: 480px;
        height: fit-content;
        padding: 1rem 2rem;
      }
      label {
        font-weight: bold;
        text-align: left;
        margin-bottom: 5px;
        display: block;
        color: #555;
      }
      input[type="number"] {
        width: 100%;
        padding: 10px;
        box-sizing: border-box;
        border: none;
        border-radius: 8px;
        font-size: 16px;
      }
      @media(max-width: 450px){
        .div-container{
          width: 320px;
        }
      }
    </style>
  </head>
  <body class="d-flex align-items-center justify-content-center min-vh-100">
    <div class="div-container d-flex flex-column rounded-3 shadow-lg" id="container">
      <h1 class="title bold fw-bold text-center">Alarma Tik-Time</h1>
      <form action="" id="form" class="d-flex flex-column gap-4">
        <div class="input-group d-flex flex-column gap-3">
          <div class="input-field">
            <label for="h">Hora:</label>
            <input type="number" name="h" id="horas" value=")rawliteral" + String(ALARMA_HORA) + R"rawliteral(" placeholder="Del 0 al 23" required />
          </div>
          <div class="input-field">
            <label for="m">Minutos:</label>
            <input type="number" name="m" id="minutos" value=")rawliteral" + String(ALARMA_MINUTO) + R"rawliteral(" placeholder="Del 0 al 59" required />
          </div>
          <div class="input-field">
            <label for="melodia">Tono de Alarma:</label>
            <select name="melodia" id="melodia" class="form-select" style="padding: 10px; border-radius: 8px; font-size: 16px;">
              <option value="0">🎵 Super Mario - Tema Principal</option>
              <option value="1">🎵 Super Mario - Nivel Subterraneo</option>
              <option value="2">⭐ Super Mario - Estrella</option>
              <option value="3">🎺 Trompeta Militar</option>
              <option value="4">🗡️ Zelda - Tema Principal</option>
              <option value="5">⚔️ Star Wars - Imperial March</option>
              <option value="6">📱 Nokia Ringtone</option>
            </select>
          </div>
        </div>
        <button type="submit" id="submit" class="border-0 btn btn-primary fw-bold">Guardar Alarma</button>
        <a href="/efectos" class="border-0 btn btn-warning fw-bold text-white">Efectos de Color</a>
        <a href="/disenos" class="border-0 btn btn-success fw-bold text-white">Ver Disenos LED</a>
        <a href="/editor" class="border-0 btn btn-info fw-bold text-white">Editor de Disenos</a>
      </form>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js" integrity="sha384-FKyoEForCGlyvwx9Hj09JcYn3nv7wiPVlz7YYwJrWVcXK/BmnVDxM+D2scQbITxI" crossorigin="anonymous"></script>
    <script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
    <script>
      const form = document.getElementById("form");
      const inputHoras = document.getElementById("horas");
      const inputMinutos = document.getElementById("minutos");
      const selectMelodia = document.getElementById("melodia");
      const alertExito = {
        title: "Alarma establecida",
        icon: "success",
        draggable: true,
      };
      const alertError = {
        title: "Por favor, introduzca una hora valida",
        icon: "error",
        draggable: true,
      };
      const regexInputHoras = /^([0-9]|1[0-9]|2[0-3])$/;
      const regexInputMinutos = /^([0-9]|[0-5][0-9])$/;
      const container = document.getElementById("container");
      const alarm = document.createElement("p");
      alarm.classList.add("mt-3");

      form.addEventListener("submit", (e) => {
        e.preventDefault();
        let horas = inputHoras.value;
        let minutos = inputMinutos.value;
        let melodia = selectMelodia.value;

        console.log("Hora ingresada:", horas);
        console.log("Minutos ingresados:", minutos);
        console.log("Melodia seleccionada:", melodia);
        console.log("Hora valida:", regexInputHoras.test(horas));
        console.log("Minutos validos:", regexInputMinutos.test(minutos));

        if (!regexInputHoras.test(horas) || !regexInputMinutos.test(minutos)) {
          e.preventDefault();
          console.log("Validacion fallo");
          return Swal.fire(alertError);
        } else {
          console.log("Enviando peticion a /set?hora=" + horas + "&minuto=" + minutos + "&melodia=" + melodia);
          fetch('/set?hora=' + horas + '&minuto=' + minutos + '&melodia=' + melodia)
            .then(response => {
              console.log("Respuesta recibida:", response.status);
              return response.text();
            })
            .then(data => {
              console.log("Datos:", data);
              Swal.fire(alertExito);
              if (horas < 12) {
                alarm.innerHTML = `Sonara a las: <strong>${horas}:${minutos} am</strong>`;
                container.appendChild(alarm);
              } else {
                alarm.innerHTML = `Sonara a las: <strong>${horas}:${minutos} pm</strong>`;
                container.appendChild(alarm);
              }
            })
            .catch(error => {
              console.error("Error:", error);
              Swal.fire({
                title: "Error al guardar",
                text: error.message,
                icon: "error",
                draggable: true,
              });
            });
        }
      });
    </script>
  </body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para servir el editor de disenos
void handleEditor() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet">
    <title>Editor de Disenos - TikTime</title>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:ital,wght@0,200..800;1,200..800&display=swap');
      body {
        font-family: "Plus Jakarta Sans", sans-serif;
        background-color: #f4f4f9;
        color: #333;
      }
      .title {
        color: #007bff;
        margin-bottom: 20px;
      }
      .div-container {
        background-color: rgba(1,1,1,0.1);
        max-width: 600px;
        height: fit-content;
        padding: 2rem;
      }
      .matrix-container {
        display: inline-block;
        background: #1a1a1a;
        padding: 15px;
        border-radius: 15px;
        box-shadow: 0 8px 16px rgba(0,0,0,0.3);
      }
      .matrix-grid {
        display: grid;
        grid-template-columns: repeat(7, 1fr);
        gap: 5px;
        margin-bottom: 20px;
      }
      .led-cell {
        width: 35px;
        height: 35px;
        background-color: #2a2a2a;
        border: 2px solid #444;
        border-radius: 5px;
        cursor: pointer;
        transition: all 0.2s;
      }
      .led-cell:hover {
        transform: scale(1.1);
        border-color: #00ff00;
      }
      .led-cell.active {
        border-width: 3px;
        box-shadow: 0 0 10px var(--cell-color);
      }
      .color-palette {
        display: flex;
        gap: 10px;
        justify-content: center;
        flex-wrap: wrap;
        margin: 20px 0;
        padding: 15px;
        background: white;
        border-radius: 10px;
      }
      .color-option {
        width: 40px;
        height: 40px;
        border-radius: 50%;
        cursor: pointer;
        border: 3px solid transparent;
        transition: all 0.2s;
        position: relative;
      }
      .color-option:hover {
        transform: scale(1.1);
      }
      .color-option.selected {
        border-color: #000;
        box-shadow: 0 0 10px rgba(0,0,0,0.5);
        transform: scale(1.15);
      }
      .color-option.selected::after {
        content: '✓';
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        color: white;
        font-weight: bold;
        font-size: 20px;
        text-shadow: 0 0 3px black;
      }
      .mode-selector {
        display: flex;
        gap: 10px;
        justify-content: center;
        margin-bottom: 15px;
      }
      .mode-btn {
        padding: 8px 15px;
        border: 2px solid #ddd;
        border-radius: 8px;
        background: white;
        cursor: pointer;
        transition: all 0.2s;
      }
      .mode-btn.active {
        background: #007bff;
        color: white;
        border-color: #007bff;
      }
      .controls {
        display: flex;
        gap: 10px;
        justify-content: center;
        flex-wrap: wrap;
        margin-top: 20px;
      }
      @media(max-width: 650px){
        .led-cell {
          width: 28px;
          height: 28px;
        }
        .div-container {
          padding: 1rem;
        }
        .color-option {
          width: 35px;
          height: 35px;
        }
      }
      @media(max-width: 450px){
        .led-cell {
          width: 22px;
          height: 22px;
        }
        .color-option {
          width: 30px;
          height: 30px;
        }
      }
    </style>
  </head>
  <body class="d-flex align-items-center justify-content-center min-vh-100">
    <div class="div-container rounded-3 shadow-lg">
      <h1 class="title fw-bold text-center">Editor de Disenos LED</h1>
      <p class="text-center text-muted mb-3">Haz clic en las celdas para dibujar</p>
      
      <div class="mode-selector">
        <button class="mode-btn active" onclick="setMode('draw')" id="drawMode">
          ✏ Dibujar
        </button>
        <button class="mode-btn" onclick="setMode('erase')" id="eraseMode">
          🧹 Borrar
        </button>
      </div>

      <div class="color-palette">
        <div class="color-option selected" style="background-color: #FF0000;" onclick="selectColor('#FF0000', 255, 0, 0)" title="Rojo"></div>
        <div class="color-option" style="background-color: #00FF00;" onclick="selectColor('#00FF00', 0, 255, 0)" title="Verde"></div>
        <div class="color-option" style="background-color: #0000FF;" onclick="selectColor('#0000FF', 0, 0, 255)" title="Azul"></div>
        <div class="color-option" style="background-color: #FFFF00;" onclick="selectColor('#FFFF00', 255, 255, 0)" title="Amarillo"></div>
        <div class="color-option" style="background-color: #FF00FF;" onclick="selectColor('#FF00FF', 255, 0, 255)" title="Magenta"></div>
        <div class="color-option" style="background-color: #00FFFF;" onclick="selectColor('#00FFFF', 0, 255, 255)" title="Cyan"></div>
        <div class="color-option" style="background-color: #FF8800;" onclick="selectColor('#FF8800', 255, 136, 0)" title="Naranja"></div>
        <div class="color-option" style="background-color: #8800FF;" onclick="selectColor('#8800FF', 136, 0, 255)" title="Violeta"></div>
        <div class="color-option" style="background-color: #FF0088;" onclick="selectColor('#FF0088', 255, 0, 136)" title="Rosa"></div>
        <div class="color-option" style="background-color: #FFFFFF;" onclick="selectColor('#FFFFFF', 255, 255, 255)" title="Blanco" style="border: 2px solid #ccc;"></div>
      </div>
      
      <div class="text-center">
        <div class="matrix-container">
          <div class="matrix-grid" id="matrixGrid"></div>
        </div>
      </div>

      <div class="controls">
        <button onclick="limpiarMatriz()" class="btn btn-danger fw-bold">
          🗑 Limpiar
        </button>
        <button onclick="invertirMatriz()" class="btn btn-secondary fw-bold">
          🔄 Invertir
        </button>
        <button onclick="guardarDiseno()" class="btn btn-success fw-bold">
          💾 Guardar
        </button>
      </div>

      <div class="controls mt-3">
        <a href="/disenos" class="btn btn-info fw-bold">
          🎨 Galería
        </a>
        <a href="/" class="btn btn-warning fw-bold">
          🏠 Inicio
        </a>
      </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
    <script>
      const WIDTH = 7;
      const HEIGHT = 11;
      let matriz = [];
      let colorMatriz = [];
      let currentColor = { hex: '#FF0000', r: 255, g: 0, b: 0 };
      let drawMode = 'draw'; // 'draw' o 'erase'

      // Inicializar matriz
      function inicializarMatriz() {
        const grid = document.getElementById('matrixGrid');
        for (let y = 0; y < HEIGHT; y++) {
          matriz[y] = [];
          colorMatriz[y] = [];
          for (let x = 0; x < WIDTH; x++) {
            matriz[y][x] = false;
            colorMatriz[y][x] = { r: 255, g: 0, b: 0 }; // Rojo por defecto
            const cell = document.createElement('div');
            cell.className = 'led-cell';
            cell.dataset.x = x;
            cell.dataset.y = y;
            cell.onclick = () => toggleCell(x, y);
            grid.appendChild(cell);
          }
        }
      }

      function setMode(mode) {
        drawMode = mode;
        document.getElementById('drawMode').classList.remove('active');
        document.getElementById('eraseMode').classList.remove('active');
        if (mode === 'draw') {
          document.getElementById('drawMode').classList.add('active');
        } else {
          document.getElementById('eraseMode').classList.add('active');
        }
      }

      function selectColor(hex, r, g, b) {
        currentColor = { hex: hex, r: r, g: g, b: b };
        
        // Actualizar selección visual
        document.querySelectorAll('.color-option').forEach(opt => {
          opt.classList.remove('selected');
        });
        event.target.classList.add('selected');
      }

      function toggleCell(x, y) {
        if (drawMode === 'draw') {
          matriz[y][x] = true;
          colorMatriz[y][x] = { r: currentColor.r, g: currentColor.g, b: currentColor.b };
        } else {
          matriz[y][x] = false;
        }
        actualizarVista();
      }

      function actualizarVista() {
        const cells = document.querySelectorAll('.led-cell');
        cells.forEach(cell => {
          const x = parseInt(cell.dataset.x);
          const y = parseInt(cell.dataset.y);
          if (matriz[y][x]) {
            const color = colorMatriz[y][x];
            const colorHex = rgbToHex(color.r, color.g, color.b);
            cell.classList.add('active');
            cell.style.backgroundColor = colorHex;
            cell.style.borderColor = colorHex;
            cell.style.setProperty('--cell-color', colorHex);
          } else {
            cell.classList.remove('active');
            cell.style.backgroundColor = '#2a2a2a';
            cell.style.borderColor = '#444';
          }
        });
      }

      function rgbToHex(r, g, b) {
        return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase();
      }

      function limpiarMatriz() {
        for (let y = 0; y < HEIGHT; y++) {
          for (let x = 0; x < WIDTH; x++) {
            matriz[y][x] = false;
          }
        }
        actualizarVista();
      }

      function invertirMatriz() {
        for (let y = 0; y < HEIGHT; y++) {
          for (let x = 0; x < WIDTH; x++) {
            matriz[y][x] = !matriz[y][x];
            if (matriz[y][x] && (colorMatriz[y][x].r === 0 && colorMatriz[y][x].g === 0 && colorMatriz[y][x].b === 0)) {
              colorMatriz[y][x] = { r: currentColor.r, g: currentColor.g, b: currentColor.b };
            }
          }
        }
        actualizarVista();
      }

      function guardarDiseno() {
        // Convertir matriz y colores a string
        let dataStr = '';
        for (let y = 0; y < HEIGHT; y++) {
          for (let x = 0; x < WIDTH; x++) {
            if (matriz[y][x]) {
              dataStr += '1';
              dataStr += String(colorMatriz[y][x].r).padStart(3, '0');
              dataStr += String(colorMatriz[y][x].g).padStart(3, '0');
              dataStr += String(colorMatriz[y][x].b).padStart(3, '0');
            } else {
              dataStr += '0000000000';
            }
          }
        }

        fetch('/guardarDiseno?data=' + dataStr)
          .then(response => response.text())
          .then(data => {
            Swal.fire({
              title: '✅ ¡Diseno guardado!',
              text: 'Tu diseno multicolor se está mostrando en la matriz LED',
              icon: 'success',
              draggable: true,
              confirmButtonText: 'Ver en la matriz'
            }).then(() => {
              window.location.href = '/disenos';
            });
          })
          .catch(error => {
            Swal.fire({
              title: "Error",
              text: "No se pudo guardar el diseno",
              icon: "error",
              draggable: true,
            });
          });
      }

      // Inicializar al cargar
      inicializarMatriz();
    </script>
  </body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para servir la página de disenos
void handleDisenos() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet">
    <title>Disenos LED - TikTime</title>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:ital,wght@0,200..800;1,200..800&display=swap');
      body {
        font-family: "Plus Jakarta Sans", sans-serif;
        background-color: #f4f4f9;
        color: #333;
      }
      .title {
        color: #007bff;
        margin-bottom: 30px;
      }
      .div-container {
        background-color: rgba(1,1,1,0.1);
        width: 480px;
        height: fit-content;
        padding: 2rem;
      }
      .design-card {
        background: white;
        border-radius: 15px;
        padding: 20px;
        margin: 15px 0;
        box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        transition: transform 0.2s;
        cursor: pointer;
      }
      .design-card:hover {
        transform: translateY(-5px);
        box-shadow: 0 6px 12px rgba(0,0,0,0.15);
      }
      .design-icon {
        font-size: 3rem;
        margin-bottom: 10px;
      }
      .btn-volver {
        margin-top: 20px;
      }
      @media(max-width: 450px){
        .div-container{
          width: 320px;
        }
      }
    </style>
  </head>
  <body class="d-flex align-items-center justify-content-center min-vh-100">
    <div class="div-container rounded-3 shadow-lg">
      <h1 class="title fw-bold text-center">🎨 Disenos LED</h1>
      <p class="text-center text-muted mb-4">Selecciona un diseno para mostrar en la matriz LED</p>
      
      <div class="design-card text-center" onclick="seleccionarDiseno(0)">
        <div class="design-icon">🟡</div>
        <h4 class="fw-bold">Pacman</h4>
        <p class="text-muted mb-0">El clasico comecocos amarillo</p>
      </div>

      <div class="design-card text-center" onclick="seleccionarDiseno(1)">
        <div class="design-icon">❤</div>
        <h4 class="fw-bold">Corazon</h4>
        <p class="text-muted mb-0">Un corazon lleno de amor</p>
      </div>

      <div class="design-card text-center" onclick="seleccionarDiseno(2)">
        <div class="design-icon">😊</div>
        <h4 class="fw-bold">Carita Feliz</h4>
        <p class="text-muted mb-0">Una sonrisa brillante</p>
      </div>

      <div class="design-card text-center" onclick="seleccionarDiseno(3)">
        <div class="design-icon">✏</div>
        <h4 class="fw-bold">Diseno Personalizado</h4>
        <p class="text-muted mb-0">Tu propio diseno creado</p>
      </div>

      <a href="/editor" class="btn btn-info fw-bold w-100 mb-2">
        ✏ Crear Nuevo Diseno
      </a>

      <button onclick="volverReloj()" class="btn btn-warning fw-bold w-100 btn-volver">
        ⏰ Volver al Reloj
      </button>
      <a href="/" class="btn btn-secondary fw-bold w-100 mt-2">
        🏠 Volver a Inicio
      </a>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
    <script>
      function seleccionarDiseno(tipo) {
        fetch('/setDiseno?tipo=' + tipo)
          .then(response => response.text())
          .then(data => {
            let nombre = '';
            let emoji = '';
            switch(tipo) {
              case 0:
                nombre = 'Pacman';
                emoji = '🟡';
                break;
              case 1:
                nombre = 'Corazon';
                emoji = '❤';
                break;
              case 2:
                nombre = 'Carita Feliz';
                emoji = '😊';
                break;
              case 3:
                nombre = 'Diseno Personalizado';
                emoji = '✏';
                break;
            }
            Swal.fire({
              title: emoji + ' ' + nombre,
              text: '¡Diseno activado en la matriz LED!',
              icon: 'success',
              draggable: true,
            });
          })
          .catch(error => {
            Swal.fire({
              title: "Error",
              text: "No se pudo activar el diseno",
              icon: "error",
              draggable: true,
            });
          });
      }

      function volverReloj() {
        fetch('/volverReloj')
          .then(response => response.text())
          .then(data => {
            Swal.fire({
              title: '⏰ Reloj activado',
              text: 'Volviendo al modo reloj',
              icon: 'info',
              timer: 1500,
              showConfirmButton: false
            });
          });
      }
    </script>
  </body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para guardar la alarma
void handleSet() {
  if (server.hasArg("hora") && server.hasArg("minuto")) {
    int nuevaHora = server.arg("hora").toInt();
    int nuevoMinuto = server.arg("minuto").toInt();
    int nuevaMelodia = 0;
    
    // Obtener melodía seleccionada (opcional, por defecto 0)
    if (server.hasArg("melodia")) {
      nuevaMelodia = server.arg("melodia").toInt();
      // Validar que esté en rango válido (0-6)
      if (nuevaMelodia < 0 || nuevaMelodia > 6) {
        nuevaMelodia = 0;
      }
    }
    
    Serial.println();
    Serial.println("╔════════════════════════════════════╗");
    Serial.println("║   CONFIGURANDO NUEVA ALARMA       ║");
    Serial.println("╚════════════════════════════════════╝");
    Serial.printf("📥 Hora recibida: %d\n", nuevaHora);
    Serial.printf("📥 Minuto recibido: %d\n", nuevoMinuto);
    Serial.printf("🎵 Melodia seleccionada: %s\n", nombresMelodias[nuevaMelodia]);
    
    ALARMA_HORA = nuevaHora;
    ALARMA_MINUTO = nuevoMinuto;
    MELODIA_SELECCIONADA = nuevaMelodia;
    ALARMA_ACTIVADA = true;
    
    // Resetear estado de alarma activa
    alarmaActiva = false;
    melodiaEnCurso = false;
    noTone(BUZZER_PIN);
    
    Serial.println("─────────────────────────────────────");
    Serial.printf("✅ Nueva alarma: %02d:%02d (formato 24h)\n", ALARMA_HORA, ALARMA_MINUTO);
    
    // Convertir a formato 12 horas para mostrar
    int horaAlarma12 = ALARMA_HORA;
    String periodo = "AM";
    if (horaAlarma12 == 0) {
      horaAlarma12 = 12;
    } else if (horaAlarma12 >= 12) {
      periodo = "PM";
      if (horaAlarma12 > 12) {
        horaAlarma12 = horaAlarma12 - 12;
      }
    }
    Serial.printf("✅ En formato 12h: %02d:%02d %s\n", horaAlarma12, ALARMA_MINUTO, periodo.c_str());
    
    // Mostrar hora actual del RTC
    DateTime now = rtc.now();
    Serial.println("─────────────────────────────────────");
    Serial.printf("🕐 Hora actual: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
    
    // Calcular tiempo restante
    int minutosRestantes = (ALARMA_HORA * 60 + ALARMA_MINUTO) - (now.hour() * 60 + now.minute());
    if (minutosRestantes < 0) minutosRestantes += 24 * 60;
    
    int horasRestantes = minutosRestantes / 60;
    minutosRestantes = minutosRestantes % 60;
    
    Serial.printf("⏰ Sonara en: %d horas y %d minutos\n", horasRestantes, minutosRestantes);
    Serial.println("╚════════════════════════════════════╝");
    Serial.println();
    
    server.send(200, "text/plain", "OK");
  } else {
    Serial.println("❌ ERROR: Parámetros faltantes en /set");
    server.send(400, "text/plain", "Parámetros faltantes");
  }
}

// Función para establecer diseno
void handleSetDiseno() {
  if (server.hasArg("tipo")) {
    disenoActual = server.arg("tipo").toInt();
    modoDiseno = true;
    Serial.printf("Diseno activado: %d\n", disenoActual);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Parámetro faltante");
  }
}

// Función para volver al reloj
void handleVolverReloj() {
  modoDiseno = false;
  Serial.println("Modo reloj activado");
  server.send(200, "text/plain", "OK");
}

// Función para guardar diseno personalizado
void handleGuardarDiseno() {
  if (server.hasArg("data")) {
    String data = server.arg("data");
    
    // Nuevo formato: cada píxel = 10 caracteres
    // 1 char (encendido/apagado) + 3 chars (R) + 3 chars (G) + 3 chars (B)
    // Total esperado: 7 x 11 x 10 = 770 caracteres
    
    if (data.length() != 770) {
      Serial.printf("ERROR: Longitud incorrecta. Esperado: 770, Recibido: %d\n", data.length());
      server.send(400, "text/plain", "Datos inválidos");
      return;
    }
    
    // Convertir string a matriz con colores
    int index = 0;
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        // Leer estado (1 char)
        char estado = data.charAt(index);
        disenoPersonalizado[y][x] = (estado == '1');
        index++;
        
        // Leer RGB (9 chars)
        String rStr = data.substring(index, index + 3);
        String gStr = data.substring(index + 3, index + 6);
        String bStr = data.substring(index + 6, index + 9);
        
        coloresPersonalizados[y][x].r = rStr.toInt();
        coloresPersonalizados[y][x].g = gStr.toInt();
        coloresPersonalizados[y][x].b = bStr.toInt();
        
        index += 9;
      }
    }
    
    // Activar diseno personalizado
    disenoActual = 3;
    modoDiseno = true;
    
    Serial.println("=============================");
    Serial.println("🎨 Diseno personalizado multicolor guardado");
    Serial.println("=============================");
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Parámetro faltante");
  }
}

// Handler para página de efectos de color
void handleEfectos() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/css/bootstrap.min.css" rel="stylesheet">
    <title>Efectos de Color - TikTime</title>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:ital,wght@0,200..800;1,200..800&display=swap');
      body {
        font-family: "Plus Jakarta Sans", sans-serif;
        background-color: #f4f4f9;
        color: #333;
      }
      .title {
        color: #007bff;
        margin-bottom: 30px;
      }
      .div-container {
        background-color: rgba(1,1,1,0.1);
        max-width: 600px;
        height: fit-content;
        padding: 2rem;
      }
      .effect-card {
        background: white;
        border-radius: 15px;
        padding: 20px;
        margin: 15px 0;
        box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        transition: transform 0.2s;
        cursor: pointer;
      }
      .effect-card:hover {
        transform: translateY(-5px);
        box-shadow: 0 6px 12px rgba(0,0,0,0.15);
      }
      .color-input {
        width: 60px;
        height: 40px;
        border: none;
        border-radius: 8px;
        cursor: pointer;
      }
      .color-group {
        display: flex;
        align-items: center;
        gap: 10px;
        margin: 10px 0;
      }
    </style>
  </head>
  <body class="d-flex align-items-center justify-content-center min-vh-100">
    <div class="div-container rounded-3 shadow-lg">
      <h1 class="title fw-bold text-center">Efectos de Color</h1>
      <p class="text-center text-muted mb-4">Personaliza el reloj LED</p>
      
      <div class="effect-card text-center" onclick="seleccionarEfecto(0)">
        <div style="font-size: 2rem;">🎨</div>
        <h4 class="fw-bold">Colores Personalizados</h4>
        <p class="text-muted mb-0">Elige tus propios colores</p>
      </div>

      <div class="effect-card text-center" onclick="seleccionarEfecto(1)">
        <div style="font-size: 2rem;">🌈</div>
        <h4 class="fw-bold">Arcoiris</h4>
        <p class="text-muted mb-0">Ciclo continuo de colores</p>
      </div>

      <div class="effect-card text-center" onclick="seleccionarEfecto(2)">
        <div style="font-size: 2rem;">💫</div>
        <h4 class="fw-bold">Parpadeo</h4>
        <p class="text-muted mb-0">Intermitente cada 0.5s</p>
      </div>

      <div class="effect-card text-center" onclick="seleccionarEfecto(3)">
        <div style="font-size: 2rem;">🎲</div>
        <h4 class="fw-bold">Aleatorio</h4>
        <p class="text-muted mb-0">Colores random cada segundo</p>
      </div>

      <div class="effect-card text-center" onclick="seleccionarEfecto(4)">
        <div style="font-size: 2rem;">🌊</div>
        <h4 class="fw-bold">Gradiente</h4>
        <p class="text-muted mb-0">Transicion suave de colores</p>
      </div>

      <hr class="my-4">

      <h5 class="fw-bold text-center mb-3">Personalizar Colores</h5>
      <div class="color-group">
        <label style="min-width: 120px;">Hora (Decena):</label>
        <input type="color" id="colorHD" value="#00FFFF" class="color-input">
      </div>
      <div class="color-group">
        <label style="min-width: 120px;">Hora (Unidad):</label>
        <input type="color" id="colorHU" value="#00FFFF" class="color-input">
      </div>
      <div class="color-group">
        <label style="min-width: 120px;">Minuto (Decena):</label>
        <input type="color" id="colorMD" value="#FFFF00" class="color-input">
      </div>
      <div class="color-group">
        <label style="min-width: 120px;">Minuto (Unidad):</label>
        <input type="color" id="colorMU" value="#FFFF00" class="color-input">
      </div>

      <button onclick="guardarColores()" class="btn btn-success fw-bold w-100 mt-3">
        Guardar Colores Personalizados
      </button>

      <a href="/" class="btn btn-secondary fw-bold w-100 mt-2">Volver a Inicio</a>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.8/dist/js/bootstrap.bundle.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/sweetalert2@11"></script>
    <script>
      function seleccionarEfecto(modo) {
        fetch('/setEfecto?modo=' + modo)
          .then(response => response.text())
          .then(data => {
            let nombreEfecto = ['Colores Personalizados', 'Arcoiris', 'Parpadeo', 'Aleatorio', 'Gradiente'][modo];
            Swal.fire({
              title: 'Efecto Activado',
              text: nombreEfecto,
              icon: 'success',
              draggable: true,
            });
          })
          .catch(error => {
            Swal.fire({
              title: 'Error',
              text: 'No se pudo cambiar el efecto',
              icon: 'error',
              draggable: true,
            });
          });
      }

      function hexToRgb(hex) {
        let r = parseInt(hex.slice(1, 3), 16);
        let g = parseInt(hex.slice(3, 5), 16);
        let b = parseInt(hex.slice(5, 7), 16);
        return {r, g, b};
      }

      function guardarColores() {
        let hd = hexToRgb(document.getElementById('colorHD').value);
        let hu = hexToRgb(document.getElementById('colorHU').value);
        let md = hexToRgb(document.getElementById('colorMD').value);
        let mu = hexToRgb(document.getElementById('colorMU').value);

        let url = `/setColoresPersonalizados?hd=${hd.r},${hd.g},${hd.b}&hu=${hu.r},${hu.g},${hu.b}&md=${md.r},${md.g},${md.b}&mu=${mu.r},${mu.g},${mu.b}`;

        fetch(url)
          .then(response => response.text())
          .then(data => {
            Swal.fire({
              title: 'Colores Guardados',
              text: 'Tus colores personalizados se han aplicado',
              icon: 'success',
              draggable: true,
            });
          })
          .catch(error => {
            Swal.fire({
              title: 'Error',
              text: 'No se pudieron guardar los colores',
              icon: 'error',
              draggable: true,
            });
          });
      }
    </script>
  </body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Handler para cambiar modo de efecto
void handleSetEfecto() {
  if (server.hasArg("modo")) {
    int nuevoModo = server.arg("modo").toInt();
    if (nuevoModo >= 0 && nuevoModo <= 4) {
      modoColor = nuevoModo;
      modoDiseno = false; // Volver a modo reloj
      
      String nombres[] = {"Estatico", "Arcoiris", "Parpadeo", "Aleatorio", "Gradiente"};
      Serial.printf("🎨 Efecto de color cambiado a: %s\n", nombres[modoColor].c_str());
      
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Modo invalido");
    }
  } else {
    server.send(400, "text/plain", "Parametro faltante");
  }
}

// Handler para cambiar colores personalizados
void handleSetColoresPersonalizados() {
  if (server.hasArg("hd") && server.hasArg("hu") && server.hasArg("md") && server.hasArg("mu")) {
    // Parsear colores en formato "R,G,B"
    sscanf(server.arg("hd").c_str(), "%d,%d,%d", &colorEstatico[0][0], &colorEstatico[0][1], &colorEstatico[0][2]);
    sscanf(server.arg("hu").c_str(), "%d,%d,%d", &colorEstatico[1][0], &colorEstatico[1][1], &colorEstatico[1][2]);
    sscanf(server.arg("md").c_str(), "%d,%d,%d", &colorEstatico[2][0], &colorEstatico[2][1], &colorEstatico[2][2]);
    sscanf(server.arg("mu").c_str(), "%d,%d,%d", &colorEstatico[3][0], &colorEstatico[3][1], &colorEstatico[3][2]);
    
    modoColor = 0; // Activar modo estatico
    modoDiseno = false; // Volver a modo reloj
    
    Serial.println("🎨 Colores personalizados actualizados:");
    Serial.printf("  Hora Decena: RGB(%d,%d,%d)\n", colorEstatico[0][0], colorEstatico[0][1], colorEstatico[0][2]);
    Serial.printf("  Hora Unidad: RGB(%d,%d,%d)\n", colorEstatico[1][0], colorEstatico[1][1], colorEstatico[1][2]);
    Serial.printf("  Minuto Decena: RGB(%d,%d,%d)\n", colorEstatico[2][0], colorEstatico[2][1], colorEstatico[2][2]);
    Serial.printf("  Minuto Unidad: RGB(%d,%d,%d)\n", colorEstatico[3][0], colorEstatico[3][1], colorEstatico[3][2]);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Parametros faltantes");
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  strip.begin();
  strip.setBrightness(50);
  strip.show();
  
  inicializarColores();
  
  if (!rtc.begin()) {
    Serial.println("No se encontró el RTC DS3231");
    while (1);
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC perdió energía, configurando fecha/hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // Conectar a WiFi
  Serial.println("======================");
  Serial.println("INICIANDO CONEXIÓN WiFi");
  Serial.println("======================");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Longitud password: ");
  Serial.println(strlen(password));
  Serial.println("Asegúrate de estar en una red 2.4GHz");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.println("Conectando");
  
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    intentos++;
    
    if (intentos % 10 == 0) {
      Serial.println();
      Serial.printf("Intento %d/30...\n", intentos);
      Serial.print("Estado WiFi: ");
      Serial.println(WiFi.status());
    }
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("======================");
    Serial.println("✅ ¡CONECTADO A WiFi!");
    Serial.println("======================");
    Serial.print("IP Local: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("RSSI (Señal): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.println("======================");
    Serial.println("IMPORTANTE: Copia esta URL exacta:");
    Serial.print("http://");
    Serial.println(WiFi.localIP());
    Serial.println("======================");
    Serial.println("PASOS PARA ACCEDER:");
    Serial.println("1. Conecta tu celular/PC a la MISMA red WiFi");
    Serial.print("   Red: ");
    Serial.println(ssid);
    Serial.println("2. Abre el navegador");
    Serial.print("3. Escribe: http://");
    Serial.println(WiFi.localIP());
    Serial.println("4. NO uses https:// , solo http://");
    Serial.println("======================");
  } else {
    Serial.println("======================");
    Serial.println("❌ ERROR: No se pudo conectar a WiFi");
    Serial.println("======================");
    Serial.println("VERIFICA:");
    Serial.print("1. SSID es exactamente: ");
    Serial.println(ssid);
    Serial.println("2. Password es correcta (distingue mayúsculas)");
    Serial.println("3. La red es 2.4GHz (ESP32 NO soporta 5GHz)");
    Serial.println("4. Router está encendido y funcionando");
    Serial.print("Estado WiFi: ");
    Serial.println(WiFi.status());
    Serial.println("Estados: 0=Idle, 1=NoSSID, 3=Conectado, 4=FalloConexión, 6=Desconectado");
    Serial.println("======================");
    Serial.println("El reloj continuará funcionando sin WiFi...");
    Serial.println("No podrás usar la interfaz web hasta solucionar esto.");
  }
  
  // Configurar rutas del servidor
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/editor", handleEditor);
  server.on("/disenos", handleDisenos);
  server.on("/setDiseno", handleSetDiseno);
  server.on("/volverReloj", handleVolverReloj);
  server.on("/guardarDiseno", handleGuardarDiseno);
  server.on("/efectos", handleEfectos);
  server.on("/setEfecto", handleSetEfecto);
  server.on("/setColoresPersonalizados", handleSetColoresPersonalizados);
  
  server.begin();
  Serial.println("Servidor web iniciado");
  
  // Mostrar configuración de alarma
  DateTime now = rtc.now();
  Serial.println("=== HORA DEL RTC ===");
  Serial.print("Fecha: ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.println(now.year());
  Serial.print("Hora: ");
  if (now.hour() < 10) Serial.print("0");
  Serial.print(now.hour());
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute());
  Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());
  Serial.println("===================");
  
  if (ALARMA_ACTIVADA) {
    Serial.println("======================");
    Serial.print("⏰ Alarma configurada para las ");
    int horaAlarma12 = ALARMA_HORA;
    String periodo = " AM";
    
    if (horaAlarma12 == 0) {
      horaAlarma12 = 12;
    } else if (horaAlarma12 >= 12) {
      periodo = " PM";
      if (horaAlarma12 > 12) {
        horaAlarma12 = horaAlarma12 - 12;
      }
    }
    
    if (horaAlarma12 < 10) Serial.print("0");
    Serial.print(horaAlarma12);
    Serial.print(":");
    if (ALARMA_MINUTO < 10) Serial.print("0");
    Serial.print(ALARMA_MINUTO);
    Serial.print(periodo);
    Serial.print(" (");
    if (ALARMA_HORA < 10) Serial.print("0");
    Serial.print(ALARMA_HORA);
    Serial.print(":");
    if (ALARMA_MINUTO < 10) Serial.print("0");
    Serial.print(ALARMA_MINUTO);
    Serial.println(" formato 24h)");
    Serial.println("======================");
  } else {
    Serial.println("======================");
    Serial.println("⏰ Alarma NO configurada");
    Serial.println("Configúrala desde la interfaz web");
    Serial.println("======================");
  }
}

void loop() {
  // Manejar peticiones del servidor web
  server.handleClient();
  
  DateTime now = rtc.now();
  
  int hora = now.hour();
  int minuto = now.minute();
  int segundo = now.second();
  
  int hora12 = hora;
  if (hora12 == 0) {
    hora12 = 12;
  } else if (hora12 > 12) {
    hora12 = hora12 - 12;
  }
  
  verificarAlarma(hora, minuto, segundo);
  
  if (alarmaActiva) {
    displayWakeUp();
  } else if (modoDiseno) {
    displayDiseno(disenoActual);
  } else {
    displayTime(hora12, minuto);
  }
  
  delay(100);
}