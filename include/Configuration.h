#pragma once
#include <Preferences.h>
#include <Profile.h>
#include <map>
#include <ArduinoJson.h>

#define MAX_PROFILES 6

struct PidValues{
    float P;
    float I;
    float D;

    static esp_err_t FromJson(char* json, size_t len, PidValues *vals);
    String ToJson();
};

class Configuration{
    private:
        static Preferences prefs;

    public:
        static void Init();
        static esp_err_t SaveProfile(Profile *prof, int slot);
        static esp_err_t LoadProfile(int slot, Profile *prof);
        static esp_err_t SetPidValues(PidValues vals);
        static esp_err_t GetPidValues(PidValues *vals);
};