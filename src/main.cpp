/***************************************************************************
  sTune QuickPID Example (MAX6675, PTC Heater / SSR / Software PWM)
  This sketch does on-the-fly tuning and PID control. Tuning parameters are
  quickly determined and applied during temperature ramp-up to setpoint.
  View results using serial plotter.
  Reference: https://github.com/Dlloydev/sTune/wiki/Examples_MAX6675_PTC_SSR
  ***************************************************************************/
#include <max6675.h>
#include <sTune.h>
#include <QuickPID.h>
#include <WiFi.h>
#include <Configuration.h>

#include "blufi.h"
#include "HttpServer.h"

#define LED_BUILTIN 2

// pins
const uint8_t inputPin = 0;
const uint8_t relayPin = 27;
const uint8_t SO = 12;
const uint8_t CS = 15;
const uint8_t sck = 14;

/*
// user settings
uint32_t settleTimeSec = 10;
uint32_t testTimeSec = 300;  // runPid interval = testTimeSec / samples
const uint16_t samples = 300;
const float inputSpan = 250;
const float outputSpan = 2000;
float outputStart = 0;
float outputStep = 700;
float tempLimit = 250;
uint8_t debounce = 1;

// variables
float Input, Output, Setpoint = 100, Kp, Ki, Kd;

MAX6675 module(sck, CS, SO); //SPI
sTune tuner = sTune(&Input, &Output, tuner.NoOvershoot_PID, tuner.direct5T, tuner.printALL);
QuickPID myPID(&Input, &Output, &Setpoint);


void setup() {
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  delay(3000);
  Output = 0;
  tuner.Configure(inputSpan, outputSpan, outputStart, outputStep, testTimeSec, settleTimeSec, samples);
  tuner.SetEmergencyStop(tempLimit);
}

void loop() {
  float optimumOutput = tuner.softPwm(relayPin, Input, Output, Setpoint, outputSpan, debounce);

  
  //Serial.print("Temp: ");
  //Serial.printf("%f", Input);
  //Serial.println("deg C");

  switch (tuner.Run()) {
    case tuner.sample: // active once per sample during test
      Input = module.readCelsius();
      delay(300);
      Input += module.readCelsius();
      delay(300);
      Input += module.readCelsius();
      Input = Input / 3;
      //tuner.plotter(Input, Output, Setpoint, 0.5f, 3); // output scale 0.5, plot every 3rd sample
      break;

    case tuner.tunings: // active just once when sTune is done
      tuner.GetAutoTunings(&Kp, &Ki, &Kd); // sketch variables updated by sTune
      myPID.SetOutputLimits(0, outputSpan * 0.1);
      myPID.SetSampleTimeUs((outputSpan - 1) * 1000);
      debounce = 0; // ssr mode
      Output = outputStep;
      myPID.SetMode(myPID.Control::automatic); // the PID is turned on
      myPID.SetProportionalMode(myPID.pMode::pOnMeas);
      myPID.SetAntiWindupMode(myPID.iAwMode::iAwClamp);
      myPID.SetTunings(Kp, Ki, Kd); // update PID with the new tunings
      break;

    case tuner.runPid: // active once per sample after tunings
      Input = module.readCelsius();
      Serial.printf("PV: %f deg C \r", Input);
      myPID.Compute();
      //tuner.plotter(Input, optimumOutput, Setpoint, 0.5f, 3);
      break;
  }
}*/



// user settings
const unsigned long windowSize = 1000;
const byte debounce = 50;
//K 80 single oscillation
//k 500 oscillation
float Input, Output, Setpoint = 80;

// status
unsigned long windowStartTime, nextSwitchTime;
boolean relayStatus = false;

Preferences prefs;
MAX6675 module(sck, CS, SO); //SPI
QuickPID myPID(&Input, &Output, &Setpoint, 0, 0, 0,
               myPID.pMode::pOnError,
               myPID.dMode::dOnMeas,
               myPID.iAwMode::iAwClamp,
               myPID.Action::direct);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    http_register_handlers();
    http_start();
}

float MultisampleTemp(int delayMs, short samples){
  float val = 0;
  for(short i = 0; i < samples; i++){
    val += module.readCelsius();
    delay(delayMs);
  }
  return val / samples;
}

void setup() {
  PidValues pidVals;
  
  Serial.begin(115200);

  Configuration::Init();
  Configuration::GetPidValues(&pidVals);
  Serial.printf("PID values: P = %f, I = %f, D = %f\n", pidVals.P, pidVals.I, pidVals.D);

  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.setHostname("Reflow-Oven");
  WiFi.begin("GNET", "gingerbeer798");

  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  myPID.SetTunings(pidVals.P, pidVals.I, pidVals.D);
  myPID.SetOutputLimits(0, windowSize);
  myPID.SetSampleTimeUs(windowSize * 1000);
  myPID.SetMode(myPID.Control::automatic);
  Input = module.readCelsius();

}

void loop() {
  unsigned long msNow = millis();
  
  if (myPID.Compute()){
    windowStartTime = msNow;
    Input = MultisampleTemp(250, 3);//module.readCelsius();
    http_ws_printf("{\"time\": %d, \"temp\": %f, \"output\": %f}", (int) (msNow / 1000), Input, Output / windowSize * 100);
  } 

  if (!relayStatus && Output > (msNow - windowStartTime)) {
    if (msNow > nextSwitchTime) {
      nextSwitchTime = msNow + debounce;
      relayStatus = true;
      digitalWrite(relayPin, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  } else if (relayStatus && Output < (msNow - windowStartTime)) {
    if (msNow > nextSwitchTime) {
      nextSwitchTime = msNow + debounce;
      relayStatus = false;
      digitalWrite(relayPin, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  delay(100);
}