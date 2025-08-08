# Manual del Programador - Sistema de Monitoreo de Variables Eléctricas

Este repositorio contiene el **manual técnico para programadores** del sistema de monitoreo trifásico basado en **ESP32**, desarrollado como parte del proyecto de tesis en la Universidad Nacional de Loja.

## 📋 Contenido

- Manual del programador (PDF)
- Código fuente en Arduino IDE (versión sin credenciales sensibles)
- Diagramas de flujo y esquemas del sistema

## 🔍 Descripción del proyecto

El sistema permite medir variables eléctricas (voltaje, corriente, potencia aparente, potencia real y factor de potencia) utilizando sensores **SCT013** y **ZMPT101B** conectados a un microcontrolador **ESP32**. Los datos se envían a una **Raspberry Pi** con InfluxDB para su visualización en **Grafana**.

## ⚡ Características principales

- Monitoreo trifásico en tiempo real
- Comunicación WiFi con reconexión automática
- Display OLED para visualización local
- Envío de datos a InfluxDB
- Interfaz web con Grafana
- Manejo de errores y reconexión automática

## 🛠️ Configuración inicial

### 1. Librerías requeridas

Instala las siguientes librerías en Arduino IDE:

```
- EmonLib
- InfluxDbClient
- WiFiMulti (ESP32) / ESP8266WiFiMulti (ESP8266)
- Adafruit_GFX
- Adafruit_SH110X
- Wire
- SPI
```

### 2. Configuración de credenciales

**⚠️ IMPORTANTE**: Antes de compilar el código, debes configurar tus credenciales:

#### WiFi:
```cpp
#define WIFI_SSID "TU_NOMBRE_DE_RED_WIFI"
#define WIFI_PASSWORD "TU_CONTRASEÑA_WIFI"
```

#### InfluxDB:
```cpp
#define INFLUXDB_URL "http://IP_DE_TU_SERVIDOR:8086/"
#define INFLUXDB_TOKEN "TU_TOKEN_DE_INFLUXDB_AQUI"
#define INFLUXDB_ORG "TU_ORGANIZACION"
#define INFLUXDB_BUCKET "TU_BUCKET"
```

### 3. Hardware requerido

| Componente | Cantidad | Descripción |
|------------|----------|-------------|
| ESP32 | 1 | Microcontrolador principal |
| SCT013 | 3 | Sensores de corriente (30A/1V) |
| ZMPT101B | 3 | Sensores de voltaje AC |
| Display OLED SH1106 | 1 | Pantalla 128x64px I2C |
| Resistencias | Según esquema | Para acondicionamiento de señal |
| Capacitores | Según esquema | Filtrado de ruido |

### 4. Conexiones

#### Sensores de corriente (SCT013):
- Sensor 1: Pin 34
- Sensor 2: Pin 35  
- Sensor 3: Pin 33

#### Sensores de voltaje (ZMPT101B):
- Sensor 1: Pin 32
- Sensor 2: Pin 36
- Sensor 3: Pin 39

#### Display OLED (I2C):
- SDA: Pin 21
- SCL: Pin 22
- VCC: 3.3V
- GND: GND

## 🚀 Instalación paso a paso

1. **Clona el repositorio:**
   ```bash
   git clone [URL_DE_TU_REPOSITORIO]
   cd [NOMBRE_DEL_REPOSITORIO]
   ```

2. **Abre el código en Arduino IDE**

3. **Instala las librerías mencionadas**

4. **Configura tus credenciales** (WiFi e InfluxDB)

5. **Selecciona la placa ESP32** en Arduino IDE

6. **Compila y sube el código**

## 📊 Configuración de InfluxDB

### Generar token de InfluxDB:

1. Accede a tu interfaz web de InfluxDB: `http://IP_SERVIDOR:8086`
2. Ve a **Data** → **Tokens** → **Generate Token**
3. Selecciona **All Access Token** o configura permisos específicos
4. Copia el token generado y úsalo en `INFLUXDB_TOKEN`

### Crear bucket:

1. Ve a **Data** → **Buckets**
2. Crea un bucket con el nombre que usarás en `INFLUXDB_BUCKET`

## 📈 Configuración de Grafana

1. Accede a Grafana: `http://IP_SERVIDOR:3000`
2. Agrega InfluxDB como fuente de datos
3. Configura los dashboards usando las mediciones:
   - `corriente1`, `corriente2`, `corriente3`
   - Campos: `Corriente1-3`, `Voltaje1-3`, `FactorPotencia1-3`, `PotenciaReal1-3`, `PotenciaAparente1-3`

## 🔧 Calibración

### Valores de calibración por defecto:
```cpp
const float calibracion_volt = 74.5;  // Ajustar según tu transformador de voltaje
const float calibracion_corr = 0.89;   // Ajustar según tus sensores SCT013
```

### Para calibrar:
1. Conecta una carga conocida
2. Mide con un multímetro de referencia
3. Ajusta los valores de calibración hasta obtener lecturas precisas

## 🛡️ Seguridad

- **Nunca subas credenciales reales** a repositorios públicos
- Usa variables de entorno o archivos de configuración locales
- El código en este repositorio tiene placeholders seguros

## 🐛 Solución de problemas

### WiFi no conecta:
- Verifica SSID y contraseña
- Asegúrate de estar en rango de la red
- Revisa que la red sea 2.4GHz (ESP32 no soporta 5GHz)

### InfluxDB no recibe datos:
- Verifica la URL del servidor
- Confirma que el token sea válido
- Revisa que el bucket exista
- Verifica conectividad de red

### Lecturas erróneas:
- Revisa las conexiones físicas
- Ajusta valores de calibración
- Verifica que los sensores estén correctamente orientados

## 📱 Funcionalidades del display

El sistema muestra información rotativa cada 5 segundos:
1. **Pantalla 1**: Sensor 1 (Potencia Real/Aparente, Factor de Potencia, Voltaje, Corriente)
2. **Pantalla 2**: Sensor 2 (mismos parámetros)
3. **Pantalla 3**: Sensor 3 (mismos parámetros)  
4. **Pantalla 4**: Estado del sistema (WiFi, InfluxDB, IP)

## 📝 Estructura del código

```
├── setup()          # Configuración inicial, WiFi, sensores, InfluxDB
├── loop()           # Bucle principal de medición
├── mostrar_estado_wifi()    # Gestión de display OLED
├── verificar_wifi()         # Monitoreo de conexión WiFi
└── Filtros de datos         # Validación y filtrado de lecturas
```

## 📊 Datos enviados a InfluxDB

Para cada sensor (1, 2, 3):
- `Corriente[N]`: Corriente RMS en Amperios
- `Voltaje[N]`: Voltaje RMS en Voltios  
- `FactorPotencia[N]`: Factor de potencia (0-1)
- `PotenciaReal[N]`: Potencia real en Watts
- `PotenciaAparente[N]`: Potencia aparente en VA

## 🤝 Contribuciones

Si encuentras errores o tienes mejoras:
1. Haz un fork del repositorio
2. Crea una rama para tu feature
3. Realiza tus cambios
4. Envía un pull request

## 👨‍💻 Autor

**Tito Zhanay**  
Universidad Nacional de Loja - 2025  

---

## ⭐ Si este proyecto te fue útil, considera darle una estrella en GitHub

Para más detalles técnicos, consulta el manual del programador (PDF) incluido en este repositorio.
