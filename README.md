# ⏰ TikTime - Reloj & Alarma Inteligente LED con ESP32

[![ESP32](https://img.shields.io/badge/Board-ESP32-blue.svg)](https://www.espressif.com/)
[![Arduino IDE](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**TikTime** es un sistema de reloj digital, alarma interactiva y matriz de arte píxel RGB construido sobre **ESP32**. Incorpora un servidor web dinámico integrado que permite configurar alarmas, cambiar temas musicales de juegos clásicos, modificar efectos de iluminación en tiempo real y diseñar Pixel Art personalizado mediante una matriz de $7 \times 11$ Neopíxeles.

---

## 📸 Vista General del Sistema

* **Reloj en Tiempo Real:** Precisión garantizada con el módulo RTC DS3231.
* **Matriz LED $7 \times 11$:** Control dinámico de $77$ LEDs WS2812B / Neopíxel en disposición *Zig-Zag*.
* **Buzzer Pasivo:** Reproducción de temas retro sin bloqueo del sistema.
* **Servidor Web Incorporado:** Interfaz responsiva desarrollada con Bootstrap 5 y SweetAlert2 servida desde el propio ESP32.

---

## ✨ Características Principales

### 1. ⏰ Alarma Inteligente & Melodías Retro
* Selección de horario en formato de 12h/24h.
* **7 Melodías polifónicas preconfiguradas:**
  1. 🍄 *Super Mario Bros - Tema Principal*
  2. 🧱 *Super Mario Bros - Nivel Subterráneo*
  3. ⭐ *Super Mario Bros - Estrella de Invencibilidad*
  4. 🎺 *Toque de Trompeta Militar (Reveille)*
  5. 🗡️ *The Legend of Zelda - Tema Principal*
  6. ⚔️ *Star Wars - Imperial March*
  7. 📱 *Nokia Ringtone Clásico*
* Texto animado (*Scroll*) con el mensaje **"WAKE UP!"** en colores arcoíris al activarse la alarma.

### 2. 🎨 Editor de Diseños Pixel Art (Matriz $7 \times 11$)
* **Dibujo interactivo:** Dibuja píxel a píxel desde la interfaz web de tu móvil o PC.
* **Paleta de 10 colores RGB predefinidos.**
* Funciones de dibujo, borrado, inversión de matriz y borrado completo.
* Muestra instantánea del diseño creado en la matriz física de LEDs.

### 3. 🌈 Modos de Color y Efectos Visuales
* **Estático:** Color personalizado independiente para decenas y unidades de horas y minutos.
* **Arcoíris:** Ciclo fluido de tonos HSV a través de la matriz.
* **Parpadeo:** Destellos rítmicos cada 0.5 segundos.
* **Aleatorio:** Colores cambiantes cada segundo.
* **Gradiente:** Transición suave de dos tonos armónicos.

---

## 🛠️ Requisitos de Hardware y Conexión (Pinout)

### Componentes Necesarios
* 1x Placa de Desarrollo **ESP32** (NodeMCU-32S / ESP32-WROOM-32)
* 1x Módulo RTC **DS3231** (I2C)
* 1x Matriz / Tira LED Neopíxel WS2812B ($77$ LEDs en formato $7 \times 11$)
* 1x Buzzer Pasivo / Altavoz pequeño
* Cables de conexión y fuente de alimentación de 5V

### Esquema de Pines

| Componente | Pin del Componente | Pin del ESP32 | Descripción |
| :--- | :--- | :--- | :--- |
| **Matriz NeoPixel** | DIN | `GPIO 16` | Entrada de datos LED |
| **Buzzer** | VCC / I/O | `GPIO 4` | Salida de tono |
| **RTC DS3231** | SDA | `GPIO 21` (SDA) | Bus I2C de datos |
| **RTC DS3231** | SCL | `GPIO 22` (SCL) | Bus I2C de reloj |
| **RTC / NeoPixel** | VCC | `5V / VIN` | Alimentación de 5V |
| **Todos** | GND | `GND` | Tierra Común |

> ⚠️ **Nota:** Asegúrate de conectar todos los terminales GND al GND común del ESP32.

---

## 📚 Bibliotecas Necesarias (Arduino IDE)

Instala las siguientes librerías desde el **Gestor de Librerías** del Arduino IDE (`Ctrl + Shift + I`):

1. **Adafruit NeoPixel** (por Adafruit) — Para el control de la matriz de LEDs.
2. **RTClib** (por Adafruit) — Para la lectura y control del módulo RTC DS3231.
3. **Wire** — (Incluida en el core de ESP32 para el protocolo I2C).
4. **WiFi** y **WebServer** — (Incluidas en el paquete de tarjetas ESP32).

---

## 🚀 Guía de Instalación y Configuración

1. **Clonar el repositorio:**
   ```bash
   git clone https://github.com/tu-usuario/TikTime-ESP32.git
   ```

2. **Abrir el proyecto:**
   Abre el archivo `.ino` dentro de tu IDE de Arduino.

3. **Configurar Credenciales WiFi:**
   En las primeras líneas del código, ingresa el nombre y la contraseña de tu red WiFi (recuerda que el ESP32 requiere redes **2.4 GHz**):
   ```cpp
   const char* ssid = "TU_NOMBRE_DE_RED";
   const char* password = "TU_CONTRASEÑA";
   ```

4. **Cargar el código:**
   Conecta tu ESP32 por USB, selecciona la placa adecuada (*ESP32 Dev Module*) y el puerto COM correspondiente. Haz clic en **Subir**.

5. **Acceso a la Interfaz Web:**
   * Abre el **Monitor Serie** a `115200 baudios`.
   * Una vez conectado a la red WiFi, el ESP32 mostrará su dirección IP local, por ejemplo: `http://192.168.1.50`.
   * Ingresa esa dirección IP en el navegador de tu smartphone o computador conectado a la misma red WiFi.

---

## 🌐 Rutas de la Interfaz Web

| Ruta | Descripción |
| :--- | :--- |
| `/` | Página principal: Configuración de la hora de alarma y selección del tono. |
| `/efectos` | Selector de efectos de luz (Arcoíris, Parpadeo, Gradiente, etc.) y asignación de colores. |
| `/disenos` | Galería con diseños predeterminados (Pacman, Corazón, Carita Feliz, Personalizado). |
| `/editor` | Editor gráfico de Pixel Art interactivamente mapeado a la matriz $7 \times 11$. |

---

## 🤝 Contribuciones

Las contribuciones son bien recibidas. Si deseas añadir una nueva melodía, optimizar el rendimiento del servidor o agregar nuevos efectos visuales:

1. Haz un Fork del proyecto.
2. Crea una rama para tu característica (`git checkout -b feature/NuevaCaracteristica`).
3. Realiza un Commit de tus cambios (`git commit -m 'Añade nueva característica'`).
4. Haz Push a la rama (`git push origin feature/NuevaCaracteristica`).
5. Abre un **Pull Request**.

---

## 📄 Licencia

Este proyecto está bajo la Licencia **MIT**. Consulta el archivo `LICENSE` para más información.
