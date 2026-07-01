#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ==========================================
// WIFI
// ==========================================
//const char* ssid = "DUOC_IoT";
//const char* password = "1ns.D%0c1ot23";

const char* ssid = "Hale";
const char* password = "";
const char* apSsid = "Auravision-AP";
const char* apPassword = "auravision";

// ==========================================
// CONFIGURACIÓN DE PINES (XIAO ESP32-S3)
// ==========================================
#define S1_RX_PIN D0
#define S1_TX_PIN D1
#define S2_RX_PIN D2
#define S2_TX_PIN D3

#define MOTOR_IZQ_PIN D4
#define MOTOR_DER_PIN D5

#define LED_PIN LED_BUILTIN

// ==========================================
// CONFIGURACIÓN GENERAL
// ==========================================
const int MUESTRAS = 1;
HardwareSerial SerialSensor2(2);

// Servidor web + WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// PWM por hardware (LEDC)
const int PWM_FREQ = 5000;
const int PWM_RES = 8;
const int PWM_CH_IZQ = 0;
const int PWM_CH_DER = 1;

// Variables de suavizado
float pwmSuaveIzq = 0.0;
float pwmSuaveDer = 0.0;
const float FACTOR_SUAVIZADO = 0.75;

// Variables LED de estado
unsigned long ultimoParpadeo = 0;
bool estadoLed = false;

// Control de envío WebSocket
unsigned long ultimoEnvioWeb = 0;
const unsigned long INTERVALO_WEB_MS = 100;
unsigned long ultimoLogWifi = 0;
const unsigned long INTERVALO_LOG_WIFI_MS = 5000;

// Página web embebida
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Monitor US-100</title>
  <style>
    :root { color-scheme: dark; }
    * { box-sizing: border-box; }

    body {
      margin: 0;
      font-family: Arial, Helvetica, sans-serif;
      background: #0f1115;
      color: #f2f2f2;
      padding: 20px;
    }

    .contenedor {
      max-width: 1000px;
      margin: 0 auto;
    }

    .titulo {
      margin-bottom: 16px;
    }

    .estado {
      display: inline-block;
      padding: 10px 14px;
      border-radius: 10px;
      background: #1a1f29;
      margin-bottom: 20px;
      font-size: 14px;
    }

    .acciones {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      margin-bottom: 20px;
    }

    button {
      background: #2563eb;
      color: white;
      border: none;
      padding: 10px 14px;
      border-radius: 10px;
      cursor: pointer;
      font-size: 14px;
    }

    button.sec {
      background: #374151;
    }

    button.danger {
      background: #b91c1c;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 16px;
    }

    .card {
      background: #171b22;
      border: 1px solid #263041;
      border-radius: 16px;
      padding: 18px;
      box-shadow: 0 8px 20px rgba(0,0,0,0.25);
    }

    .card h2 {
      margin-top: 0;
      margin-bottom: 16px;
      font-size: 20px;
      color: #7dd3fc;
    }

    .dato {
      margin: 10px 0;
      font-size: 18px;
    }

    .valor {
      color: #86efac;
      font-weight: bold;
    }

    .mono {
      font-family: Consolas, Monaco, monospace;
    }

    .footer {
      margin-top: 20px;
      font-size: 14px;
      color: #aab4c3;
    }

    .tabla-wrap {
      margin-top: 20px;
      overflow-x: auto;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      background: #171b22;
      border-radius: 16px;
      overflow: hidden;
    }

    th, td {
      padding: 10px;
      border-bottom: 1px solid #263041;
      text-align: left;
      font-size: 14px;
    }

    th {
      color: #7dd3fc;
    }
  </style>
</head>
<body>
  <div class="contenedor">
    <h1 class="titulo">Monitor en tiempo real</h1>
    <div class="estado">Estado WebSocket: <span id="estadoWs" class="valor">Conectando...</span></div>

    <div class="acciones">
      <button onclick="descargarCSV()">Descargar CSV</button>
      <button class="sec" onclick="descargarJSON()">Descargar JSON</button>
      <button class="danger" onclick="limpiarDatos()">Limpiar historial</button>
    </div>

    <div class="grid">
      <div class="card">
        <h2>Sensor Izquierdo</h2>
        <div class="dato">Distancia: <span id="distIzq" class="valor">--</span> cm</div>
        <div class="dato">PWM: <span id="pwmIzq" class="valor">--</span></div>
      </div>

      <div class="card">
        <h2>Sensor Derecho</h2>
        <div class="dato">Distancia: <span id="distDer" class="valor">--</span> cm</div>
        <div class="dato">PWM: <span id="pwmDer" class="valor">--</span></div>
      </div>

      <div class="card">
        <h2>Sistema</h2>
        <div class="dato">IP ESP32: <span id="ip" class="valor mono">--</span></div>
        <div class="dato">Uptime: <span id="uptime" class="valor">--</span> ms</div>
        <div class="dato">Registros guardados: <span id="totalRegistros" class="valor">0</span></div>
      </div>
    </div>

    <div class="tabla-wrap">
      <table>
        <thead>
          <tr>
            <th>Fecha</th>
            <th>Dist Izq</th>
            <th>PWM Izq</th>
            <th>Dist Der</th>
            <th>PWM Der</th>
          </tr>
        </thead>
        <tbody id="tablaDatos"></tbody>
      </table>
    </div>

    <div class="footer">
      Los datos quedan guardados en el navegador y puedes exportarlos a CSV o JSON.
    </div>
  </div>

  <script>
    let ws;
    const STORAGE_KEY = "us100_historial";
    let historial = [];

    function cargarHistorial() {
      try {
        const guardado = localStorage.getItem(STORAGE_KEY);
        historial = guardado ? JSON.parse(guardado) : [];
      } catch (e) {
        historial = [];
      }
      actualizarContador();
      renderTabla();
    }

    function guardarHistorial() {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(historial));
      actualizarContador();
    }

    function actualizarContador() {
      document.getElementById("totalRegistros").textContent = historial.length;
    }

    function agregarRegistro(data) {
      const registro = {
        fecha: new Date().toISOString(),
        distIzq: data.distIzq,
        pwmIzq: data.pwmIzq,
        distDer: data.distDer,
        pwmDer: data.pwmDer,
        ip: data.ip,
        uptime: data.uptime
      };

      historial.push(registro);

      if (historial.length > 5000) {
        historial = historial.slice(-5000);
      }

      guardarHistorial();
      renderTabla();
    }

    function renderTabla() {
      const tbody = document.getElementById("tablaDatos");
      const ultimos = historial.slice(-20).reverse();

      tbody.innerHTML = ultimos.map(r => `
        <tr>
          <td>${r.fecha}</td>
          <td>${r.distIzq}</td>
          <td>${r.pwmIzq}</td>
          <td>${r.distDer}</td>
          <td>${r.pwmDer}</td>
        </tr>
      `).join("");
    }

    function conectarWS() {
      const url = `ws://${window.location.hostname}/ws`;
      ws = new WebSocket(url);

      ws.onopen = () => {
        document.getElementById("estadoWs").textContent = "Conectado";
      };

      ws.onclose = () => {
        document.getElementById("estadoWs").textContent = "Desconectado - reconectando...";
        setTimeout(conectarWS, 2000);
      };

      ws.onerror = () => {
        document.getElementById("estadoWs").textContent = "Error";
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);

          document.getElementById("distIzq").textContent = data.distIzq;
          document.getElementById("pwmIzq").textContent = data.pwmIzq;
          document.getElementById("distDer").textContent = data.distDer;
          document.getElementById("pwmDer").textContent = data.pwmDer;
          document.getElementById("ip").textContent = data.ip;
          document.getElementById("uptime").textContent = data.uptime;

          agregarRegistro(data);
        } catch (e) {
          console.error("JSON inválido:", e);
        }
      };
    }

    function descargarCSV() {
      if (!historial.length) {
        alert("No hay datos para exportar.");
        return;
      }

      const encabezado = ["fecha","distIzq","pwmIzq","distDer","pwmDer","ip","uptime"];
      const filas = historial.map(r => [
        r.fecha, r.distIzq, r.pwmIzq, r.distDer, r.pwmDer, r.ip, r.uptime
      ]);

      const csv = [
        encabezado.join(","),
        ...filas.map(f => f.map(v => `"${String(v).replace(/"/g, '""')}"`).join(","))
      ].join("\n");

      const blob = new Blob([csv], { type: "text/csv;charset=utf-8;" });
      const url = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = "historial_us100.csv";
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }

    function descargarJSON() {
      if (!historial.length) {
        alert("No hay datos para exportar.");
        return;
      }

      const json = JSON.stringify(historial, null, 2);
      const blob = new Blob([json], { type: "application/json;charset=utf-8;" });
      const url = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = "historial_us100.json";
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }

    function limpiarDatos() {
      if (!confirm("¿Seguro que quieres borrar el historial guardado?")) return;
      historial = [];
      localStorage.removeItem(STORAGE_KEY);
      actualizarContador();
      renderTabla();
    }

    window.addEventListener("load", () => {
      cargarHistorial();
      conectarWS();
    });
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// FUNCIONES AUXILIARES
// ==========================================
void ordenarArray(float arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        float temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

float medirDistanciaSensor(HardwareSerial &puertoSerial) {
  while (puertoSerial.available()) {
    puertoSerial.read();
  }

  puertoSerial.write(0x55);

  delay(80);

  if (puertoSerial.available() >= 2) {
    uint8_t alta = puertoSerial.read();
    uint8_t baja = puertoSerial.read();

    int distancia_mm = (alta << 8) | baja;
    float distancia_cm = distancia_mm / 10.0;

    if (distancia_cm >= 2.0 && distancia_cm <= 450.0) {
      return distancia_cm;
    }
  }

  return -1.0;
}

// ==========================================
// INTENSIDAD SEGÚN DISTANCIA
// ==========================================
int calcularIntensidadRecomendada(float distancia) {
  if (distancia == -1.0) {
    return 0;
  }

  if (distancia < 20.0 || distancia > 180.0) {
    return 0;
  }

  if (distancia < 80.0) {
    return 225;
  }

  if (distancia < 140.0) {
    return 175;
  }

  return 130;
}

// ==========================================
// WEBSOCKET
// ==========================================
void imprimirEstadoWiFi() {
  Serial.print("[WiFi] modo=");
  switch (WiFi.getMode()) {
    case WIFI_AP:
      Serial.print("AP");
      break;
    case WIFI_STA:
      Serial.print("STA");
      break;
    case WIFI_AP_STA:
      Serial.print("AP+STA");
      break;
    default:
      Serial.print("OFF");
      break;
  }

  Serial.print(" estado=");
  Serial.print(WiFi.status());
  Serial.print(" IP=");
  if (WiFi.getMode() == WIFI_AP) {
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println(WiFi.localIP());
  }
}

void notificarClientes(float distIzq, float distDer, int pwmIzq, int pwmDer) {
  String ipTexto = (WiFi.getMode() == WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
  String json = "{";
  json += "\"distIzq\":" + String(distIzq, 1) + ",";
  json += "\"distDer\":" + String(distDer, 1) + ",";
  json += "\"pwmIzq\":" + String(pwmIzq) + ",";
  json += "\"pwmDer\":" + String(pwmDer) + ",";
  json += "\"ip\":\"" + ipTexto + "\",";
  json += "\"uptime\":" + String(millis());
  json += "}";

  ws.textAll(json);
}

void onWebSocketEvent(AsyncWebSocket *server,
                      AsyncWebSocketClient *client,
                      AwsEventType type,
                      void *arg,
                      uint8_t *data,
                      size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("Cliente WS conectado: #%u desde %s\n",
                  client->id(),
                  client->remoteIP().toString().c_str());
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("Cliente WS desconectado: #%u\n", client->id());
  } else if (type == WS_EVT_DATA) {
    String msg = "";
    for (size_t i = 0; i < len; i++) {
      msg += (char)data[i];
    }
    Serial.print("Mensaje WS recibido: ");
    Serial.println(msg);
  }
}

// ==========================================
// WIFI
// ==========================================
void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");
  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado");
    imprimirEstadoWiFi();
    return;
  }

  Serial.println("No se pudo conectar al WiFi. Activando modo AP...");
  WiFi.mode(WIFI_AP);
  bool apOk = WiFi.softAP(apSsid, apPassword);

  if (apOk) {
    Serial.println("Modo AP activado");
    imprimirEstadoWiFi();
  } else {
    Serial.println("No se pudo activar el modo AP");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial1.begin(9600, SERIAL_8N1, S1_RX_PIN, S1_TX_PIN);
  SerialSensor2.begin(9600, SERIAL_8N1, S2_RX_PIN, S2_TX_PIN);

  pinMode(MOTOR_IZQ_PIN, OUTPUT);
  pinMode(MOTOR_DER_PIN, OUTPUT);

  ledcSetup(PWM_CH_IZQ, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_DER, PWM_FREQ, PWM_RES);

  ledcAttachPin(MOTOR_IZQ_PIN, PWM_CH_IZQ);
  ledcAttachPin(MOTOR_DER_PIN, PWM_CH_DER);

  ledcWrite(PWM_CH_IZQ, 0);
  ledcWrite(PWM_CH_DER, 0);

  conectarWiFi();

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.begin();

  Serial.println("=== SISTEMA INICIADO ===");
  Serial.println("Abre en navegador: ");
  imprimirEstadoWiFi();
}

// ==========================================
// LOOP
// ==========================================
void loop() {
  unsigned long inicioLoop = millis();

  if (millis() - ultimoParpadeo >= 500) {
    ultimoParpadeo = millis();
    estadoLed = !estadoLed;
    digitalWrite(LED_PIN, estadoLed);
  }

  float distIzq = medirDistanciaSensor(Serial1);
  float distDer = medirDistanciaSensor(SerialSensor2);

  int pwmObjetivoIzq = calcularIntensidadRecomendada(distIzq);
  int pwmObjetivoDer = calcularIntensidadRecomendada(distDer);

  if (pwmObjetivoIzq == 0) {
    pwmSuaveIzq = 0;
  } else {
    pwmSuaveIzq += FACTOR_SUAVIZADO * (pwmObjetivoIzq - pwmSuaveIzq);
  }

  if (pwmObjetivoDer == 0) {
    pwmSuaveDer = 0;
  } else {
    pwmSuaveDer += FACTOR_SUAVIZADO * (pwmObjetivoDer - pwmSuaveDer);
  }

  ledcWrite(PWM_CH_IZQ, (int)pwmSuaveIzq);
  ledcWrite(PWM_CH_DER, (int)pwmSuaveDer);

  Serial.print("IZQ: ");
  Serial.print(distIzq);
  Serial.print(" cm | PWM: ");
  Serial.print((int)pwmSuaveIzq);

  Serial.print(" || DER: ");
  Serial.print(distDer);
  Serial.print(" cm | PWM: ");
  Serial.println((int)pwmSuaveDer);

  if (millis() - ultimoLogWifi >= INTERVALO_LOG_WIFI_MS) {
    ultimoLogWifi = millis();
    imprimirEstadoWiFi();
  }

  if (millis() - ultimoEnvioWeb >= INTERVALO_WEB_MS) {
    ultimoEnvioWeb = millis();
    notificarClientes(distIzq, distDer, (int)pwmSuaveIzq, (int)pwmSuaveDer);
  }

  ws.cleanupClients();

  unsigned long tiempoLoop = millis() - inicioLoop;
  if (tiempoLoop < 83) {
    delay(83 - tiempoLoop);
  }
  
}