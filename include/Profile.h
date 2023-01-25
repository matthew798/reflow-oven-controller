#pragma once
#include <ArduinoJson.h>

struct Point{
    int Time;
    int Temp;
};

struct Profile{
    String Name;
    Point Preheat;
    Point Soak;
    Point Reflow;

    static esp_err_t FromJson(char* json, size_t len, Profile *prof);
    String ToJson();
};