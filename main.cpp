#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <Ps3Controller.h>

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define UP_LEFT 5
#define UP_RIGHT 6
#define DOWN_LEFT 7
#define DOWN_RIGHT 8
#define TURN_LEFT 9
#define TURN_RIGHT 10
#define STOP 0

#define FRONT_RIGHT_MOTOR 0
#define BACK_RIGHT_MOTOR 1
#define FRONT_LEFT_MOTOR 2
#define BACK_LEFT_MOTOR 3

#define FORWARD 1
#define BACKWARD -1

int player = 0;

struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;
};

std::vector<MOTOR_PINS> motorPins =
    {
        {16, 17}, // FRONT_RIGHT_MOTOR
        {18, 19}, // BACK_RIGHT_MOTOR
        {27, 26}, // FRONT_LEFT_MOTOR
        {25, 33}, // BACK_LEFT_MOTOR
};

const char *ssid = "MyWiFiCar";
const char *password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char *htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
< !DOCTYPE html>
    <html>

    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
        <style>
        .arrows {
            font - size: 70px;
            color:
                red;
        }

        .circularArrows {
            font - size: 80px;
            color:
                blue;
        }

        td {
            background - color: black;
            border - radius: 25 %;
            box - shadow: 5px 5px #888888;
        }

        td : active {
            transform:
                translate(5px, 5px);
            box - shadow: none;
        }

        .noselect {
            -webkit - touch - callout: none;
            /* iOS Safari */
            -webkit - user - select: none;
            /* Safari */
            -khtml - user - select: none;
            /* Konqueror HTML */
            -moz - user - select: none;
            /* Firefox */
            -ms - user - select: none;
            /* Internet Explorer/Edge */
            user - select: none;
            /* Non-prefixed version, currently
                                       supported by Chrome and Opera */
        }
        </style>
    </head>

    <body class="noselect" align="center" style="background-color:white">
        <h1 style="color: teal;text-align:center;">Hash Include Electronics</h1>
        <h2 style="color: teal;text-align:center;">Wi - Fi &#128663;Control</h2>
        <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
            <tr>
                <td ontouchstart='onTouchStartAndEnd("5")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#11017;
                    </span>
                </td>
                <td ontouchstart='onTouchStartAndEnd("1")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#8679;
                    </span>
                </td>
                <td ontouchstart='onTouchStartAndEnd("6")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#11016;
                    </span>
                </td>
            </tr>
            <tr>
                <td ontouchstart='onTouchStartAndEnd("3")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#8678;
                    </span>
                </td>
                <td>
                </td>
                <td ontouchstart='onTouchStartAndEnd("4")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#8680;
                    </span>
                </td>
            </tr>
            <tr>
                <td ontouchstart='onTouchStartAndEnd("7")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#11019;
                    </span>
                </td>
                <td ontouchstart='onTouchStartAndEnd("2")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#8681;
                    </span>
                </td>
                <td ontouchstart='onTouchStartAndEnd("8")' ontouchend='onTouchStartAndEnd("0")'><span class="arrows">&#11018;
                    </span>
                </td>
            </tr>
            <tr>
                <td ontouchstart='onTouchStartAndEnd("9")' ontouchend='onTouchStartAndEnd("0")'><span class="circularArrows">&#8634;
                    </span>
                </td>
                <td style="background-color:white;box-shadow:none">
                </td>
                <td ontouchstart='onTouchStartAndEnd("10")' ontouchend='onTouchStartAndEnd("0")'><span class="circularArrows">&#8635;
                    </span>
                </td>
            </tr>
        </table>
        <script>
        var webSocketUrl = "ws:\/\/" + window.location.hostname + "/ws";
        var websocket;

        function initWebSocket() {
            websocket = new WebSocket(webSocketUrl);

            websocket.onopen = function(event) {}

            ;

            websocket.onclose = function(event) {
                setTimeout(initWebSocket, 2000);
            }

            ;

            websocket.onmessage = function(event) {}

            ;
        }

        function onTouchStartAndEnd(value) {
            websocket.send(value);
        }

        window.onload = initWebSocket;

        document.getElementById("mainTable").addEventListener("touchend", function(event) {
                event.preventDefault()
            }

        );
        </script>
    </body>

    </html>                                  

)HTMLHOMEPAGE";

void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
}

void processCarMovement(String inputValue)
{
  Serial.printf("Got value as %s %ld\n", inputValue.c_str(), inputValue.toInt());
  switch (inputValue.toInt())
  {

  case UP:
    rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
    rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
    rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
    rotateMotor(BACK_LEFT_MOTOR, FORWARD);
    break;

  case DOWN:
    rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
    rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
    rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
    rotateMotor(BACK_LEFT_MOTOR, BACKWARD);
    break;

  case LEFT:
    rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
    rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
    rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
    rotateMotor(BACK_LEFT_MOTOR, FORWARD);
    break;

  case RIGHT:
    rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
    rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
    rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
    rotateMotor(BACK_LEFT_MOTOR, BACKWARD);
    break;

  case UP_LEFT:
    rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
    rotateMotor(BACK_RIGHT_MOTOR, STOP);
    rotateMotor(FRONT_LEFT_MOTOR, STOP);
    rotateMotor(BACK_LEFT_MOTOR, FORWARD);
    break;

  case UP_RIGHT:
    rotateMotor(FRONT_RIGHT_MOTOR, STOP);
    rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
    rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
    rotateMotor(BACK_LEFT_MOTOR, STOP);
    break;

  case DOWN_LEFT:
    rotateMotor(FRONT_RIGHT_MOTOR, STOP);
    rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
    rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
    rotateMotor(BACK_LEFT_MOTOR, STOP);
    break;

  case DOWN_RIGHT:
    rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
    rotateMotor(BACK_RIGHT_MOTOR, STOP);
    rotateMotor(FRONT_LEFT_MOTOR, STOP);
    rotateMotor(BACK_LEFT_MOTOR, BACKWARD);
    break;

  case TURN_LEFT:
    rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
    rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
    rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
    rotateMotor(BACK_LEFT_MOTOR, BACKWARD);
    break;

  case TURN_RIGHT:
    rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
    rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
    rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
    rotateMotor(BACK_LEFT_MOTOR, FORWARD);
    break;

  case STOP:
    rotateMotor(FRONT_RIGHT_MOTOR, STOP);
    rotateMotor(BACK_RIGHT_MOTOR, STOP);
    rotateMotor(FRONT_LEFT_MOTOR, STOP);
    rotateMotor(BACK_LEFT_MOTOR, STOP);
    break;

  default:
    rotateMotor(FRONT_RIGHT_MOTOR, STOP);
    rotateMotor(BACK_RIGHT_MOTOR, STOP);
    rotateMotor(FRONT_LEFT_MOTOR, STOP);
    rotateMotor(BACK_LEFT_MOTOR, STOP);
    break;
  }
}

void handleRoot(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "File Not Found");
}

void onWebSocketEvent(AsyncWebSocket *server,
                      AsyncWebSocketClient *client,
                      AwsEventType type,
                      void *arg,
                      uint8_t *data,
                      size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    // client->text(getRelayPinsStatusJson(ALL_RELAY_PINS_INDEX));
    processCarMovement("0");
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    processCarMovement("0");
    break;
  case WS_EVT_DATA:
    AwsFrameInfo *info;
    info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
      std::string myData = "";
      myData.assign((char *)data, len);
      processCarMovement(myData.c_str());
    }
    break;
  case WS_EVT_PONG:
    processCarMovement("0");
  case WS_EVT_ERROR:
    processCarMovement("0");
    break;
  default:
    processCarMovement("0");
    break;
  }
}

void setUpPinModes()
{
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);
    rotateMotor(i, STOP);
  }
}

void notify()
{
  //--- Digital cross/square/triangle/circle button events ---
  if (Ps3.event.button_down.cross)
    Serial.println("Started pressing the cross button");
  if (Ps3.event.button_up.cross)
    Serial.println("Released the cross button");

  if (Ps3.event.button_down.square)
    Serial.println("Started pressing the square button");
  if (Ps3.event.button_up.square)
    Serial.println("Released the square button");

  if (Ps3.event.button_down.triangle)
    Serial.println("Started pressing the triangle button");
  if (Ps3.event.button_up.triangle)
    Serial.println("Released the triangle button");

  if (Ps3.event.button_down.circle)
    Serial.println("Started pressing the circle button");
  if (Ps3.event.button_up.circle)
    Serial.println("Released the circle button");

  //--------------- Digital D-pad button events --------------
  if (Ps3.event.button_down.up)
  {
    processCarMovement("1");
    Serial.println("Started pressing the up button");
  }
  if (Ps3.event.button_up.up)
  {
    processCarMovement("0");
    Serial.println("Released the up button");
  }

  if (Ps3.event.button_down.right)
  {
    processCarMovement("4");
    Serial.println("Started pressing the right button");
  }
  if (Ps3.event.button_up.right)
  {
    processCarMovement("0");
    Serial.println("Released the right button");
  }

  if (Ps3.event.button_down.down)
  {
    processCarMovement("2");
    Serial.println("Started pressing the down button");
  }
  if (Ps3.event.button_up.down)
  {
    processCarMovement("0");
    Serial.println("Released the down button");
  }

  if (Ps3.event.button_down.left)
  {
    processCarMovement("3");
    Serial.println("Started pressing the left button");
  }
  if (Ps3.event.button_up.left)
  {
    processCarMovement("0");
    Serial.println("Released the left button");
  }

  //------------- Digital shoulder button events -------------
  if (Ps3.event.button_down.l1)
    Serial.println("Started pressing the left shoulder button");
  if (Ps3.event.button_up.l1)
    Serial.println("Released the left shoulder button");

  if (Ps3.event.button_down.r1)
    Serial.println("Started pressing the right shoulder button");
  if (Ps3.event.button_up.r1)
    Serial.println("Released the right shoulder button");

  //-------------- Digital trigger button events -------------
  if (Ps3.event.button_down.l2)
  {
    processCarMovement("9");
    Serial.println("Started pressing the left trigger button");
  }
  if (Ps3.event.button_up.l2)
  {
    processCarMovement("0");
    Serial.println("Released the left trigger button");
  }

  if (Ps3.event.button_down.r2)
  {
    processCarMovement("10");
    Serial.println("Started pressing the right trigger button");
  }
  if (Ps3.event.button_up.r2)
  {
    processCarMovement("0");
    Serial.println("Released the right trigger button");
  }

  //--------------- Digital stick button events --------------
  if (Ps3.event.button_down.l3)
    Serial.println("Started pressing the left stick button");
  if (Ps3.event.button_up.l3)
    Serial.println("Released the left stick button");

  if (Ps3.event.button_down.r3)
    Serial.println("Started pressing the right stick button");
  if (Ps3.event.button_up.r3)
    Serial.println("Released the right stick button");

  //---------- Digital select/start/ps button events ---------
  if (Ps3.event.button_down.select)
    Serial.println("Started pressing the select button");
  if (Ps3.event.button_up.select)
    Serial.println("Released the select button");

  if (Ps3.event.button_down.start)
    Serial.println("Started pressing the start button");
  if (Ps3.event.button_up.start)
    Serial.println("Released the start button");

  if (Ps3.event.button_down.ps)
    Serial.println("Started pressing the Playstation button");
  if (Ps3.event.button_up.ps)
    Serial.println("Released the Playstation button");

  //---------------- Analog stick value events ---------------
  if (abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2)
  {
    Serial.print("Moved the left stick:");
    Serial.print(" x=");
    Serial.print(Ps3.data.analog.stick.lx, DEC);
    Serial.print(" y=");
    Serial.print(Ps3.data.analog.stick.ly, DEC);
    Serial.println();
  }

  if (abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2)
  {
    Serial.print("Moved the right stick:");
    Serial.print(" x=");
    Serial.print(Ps3.data.analog.stick.rx, DEC);
    Serial.print(" y=");
    Serial.print(Ps3.data.analog.stick.ry, DEC);
    Serial.println();
  }

  //--------------- Analog D-pad button events ----------------
  if (abs(Ps3.event.analog_changed.button.up))
  {
    Serial.print("Pressing the up button: ");
    Serial.println(Ps3.data.analog.button.up, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.right))
  {
    Serial.print("Pressing the right button: ");
    Serial.println(Ps3.data.analog.button.right, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.down))
  {
    Serial.print("Pressing the down button: ");
    Serial.println(Ps3.data.analog.button.down, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.left))
  {
    Serial.print("Pressing the left button: ");
    Serial.println(Ps3.data.analog.button.left, DEC);
  }

  //---------- Analog shoulder/trigger button events ----------
  if (abs(Ps3.event.analog_changed.button.l1))
  {
    Serial.print("Pressing the left shoulder button: ");
    Serial.println(Ps3.data.analog.button.l1, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.r1))
  {
    Serial.print("Pressing the right shoulder button: ");
    Serial.println(Ps3.data.analog.button.r1, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.l2))
  {
    Serial.print("Pressing the left trigger button: ");
    Serial.println(Ps3.data.analog.button.l2, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.r2))
  {
    Serial.print("Pressing the right trigger button: ");
    Serial.println(Ps3.data.analog.button.r2, DEC);
  }

  //---- Analog cross/square/triangle/circle button events ----
  if (abs(Ps3.event.analog_changed.button.triangle))
  {
    Serial.print("Pressing the triangle button: ");
    Serial.println(Ps3.data.analog.button.triangle, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.circle))
  {
    Serial.print("Pressing the circle button: ");
    Serial.println(Ps3.data.analog.button.circle, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.cross))
  {
    Serial.print("Pressing the cross button: ");
    Serial.println(Ps3.data.analog.button.cross, DEC);
  }

  if (abs(Ps3.event.analog_changed.button.square))
  {
    Serial.print("Pressing the square button: ");
    Serial.println(Ps3.data.analog.button.square, DEC);
  }
}

void onConnect()
{
  Serial.println("Connected.");
}

void setup(void)
{
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.begin();
  Serial.println("HTTP server started");

  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  // this is your pscontroller mac address
  Ps3.begin("c0:14:3d:86:88:d8");
}

void loop()
{

  ws.cleanupClients();

  if (ws.count() <= 0 && Ps3.isConnected() == false)
  {
    Ps3.setPlayer(0);
    processCarMovement("0");
    delay(1000);
  }

  if (Ps3.isConnected())
  {
    Ps3.setPlayer(1);
    delay(1500);
  }
  else
  {
    Ps3.setPlayer(0);
    processCarMovement("0");
    delay(1500);
  }
}
