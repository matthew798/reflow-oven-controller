#include <Configuration.h>
#include <Profile.h>
#include <map>

#define CFG_DEFAULT_P 80.0
#define CFG_DEFAULT_I 0.01
#define CFG_DEFAULT_D 0.1

Preferences Configuration::prefs;

esp_err_t PidValues::FromJson(char* json, size_t len, PidValues *vals){
    StaticJsonDocument<192> doc;
    auto err = deserializeJson(doc, json, len);

    if(err != DeserializationError::Ok)
        return ESP_ERR_INVALID_ARG;

    vals->P = doc["P"];
    vals->I = doc["I"];
    vals->D = doc["D"];

    return ESP_OK;
}

String PidValues::ToJson(){
    StaticJsonDocument<192> doc;
    String json;

    doc["P"] = this->P;
    doc["I"] = this->I;
    doc["D"] = this->D;

    serializeJson(doc, json);
    return json;
}

void Configuration::Init(){
    prefs.begin("prefs");
}

esp_err_t Configuration::SaveProfile(Profile *prof, int slot){
    if(slot > MAX_PROFILES)
        return ESP_ERR_INVALID_ARG;

    char key[33];
    size_t bytes;

    bytes = prefs.putBytes(itoa(slot, key, 10), prof, sizeof(Profile));

    return bytes == sizeof(Profile) ? ESP_OK : ESP_ERR_INVALID_SIZE;
}

esp_err_t Configuration::LoadProfile(int slot, Profile *prof){
    if(slot > MAX_PROFILES)
        return ESP_ERR_INVALID_ARG;

    char key[33];
    size_t bytes;

    bytes = prefs.getBytes(itoa(slot, key, 10), prof, sizeof(Profile));

    return bytes == sizeof(Profile) ? ESP_OK : ESP_ERR_INVALID_SIZE;
}

esp_err_t Configuration::SetPidValues(PidValues vals){
    size_t bytes;

    bytes = prefs.putBytes("pidVals", &vals, sizeof(PidValues));

    return bytes == sizeof(PidValues) ? ESP_OK : ESP_ERR_INVALID_SIZE;
}

esp_err_t Configuration::GetPidValues(PidValues *vals){
    size_t bytes;

    if(!prefs.isKey("pidVals")){
        vals->P = CFG_DEFAULT_P;
        vals->I = CFG_DEFAULT_I;
        vals->D = CFG_DEFAULT_D;
        return ESP_OK;
    }

    bytes = prefs.getBytes("pidVals", vals, sizeof(PidValues));

    return bytes == sizeof(PidValues) ? ESP_OK : ESP_ERR_INVALID_SIZE;
}