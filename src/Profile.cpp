#include <Profile.h>

esp_err_t Profile::FromJson(char* json, size_t len, Profile *prof){
    StaticJsonDocument<192> doc;
    auto err = deserializeJson(doc, json, len);

    if(err != DeserializationError::Ok)
        return ESP_ERR_INVALID_ARG;

    prof->Name = doc["name"].as<String>();
    prof->Preheat = (Point){
            Time: doc["preheat"]["time"],
            Temp: doc["preheat"]["temp"]
        };
    prof->Soak = (Point){
            Time: doc["soak"]["time"],
            Temp: doc["soak"]["temp"]
        };
    prof->Reflow = (Point){
            Time: doc["reflow"]["time"],
            Temp: doc["reflow"]["temp"]
        };

    return ESP_OK;
}

String Profile::ToJson(){
    StaticJsonDocument<192> doc;
    String json;

    doc["name"] = this->Name;
    JsonObject preheat = doc.createNestedObject("preheat");
    preheat["time"] = this->Preheat.Time;
    preheat["temp"] = this->Preheat.Temp;
    JsonObject soak = doc.createNestedObject("soak");
    soak["time"] = this->Soak.Time;
    soak["temp"] = this->Soak.Temp;
    JsonObject reflow = doc.createNestedObject("reflow");
    reflow["time"] = this->Reflow.Time;
    reflow["temp"] = this->Reflow.Temp;

    serializeJson(doc, json);
    return json;
}