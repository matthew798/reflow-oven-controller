#include <HttpServer.h>
#include <Arduino.h>
#ifdef ESP32
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#include <Configuration.h>

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

AsyncWebServer srv(80);
AsyncWebSocket ws("/ws");

void handle_index(AsyncWebServerRequest *req)
{
    req->send(200, "text/html", String((char *)index_html_start));
}

void handle_SaveProfile(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    Profile profile;
    if(Profile::FromJson((char*)data, len, &profile)){
        request->send(422);
        return;
    }
    
    if(Configuration::SaveProfile(&profile, 1)){
        request->send(500);
        return;
    }

    request->send(200);
}

void handle_LoadProfile(AsyncWebServerRequest *request){
    Profile profile;

    if(!request->hasParam("slot")){
        request->send(422);
        return;
    }
        
    auto param = request->getParam("slot");
    int slot = atoi(param->value().c_str());

    if(Configuration::LoadProfile(slot, &profile)){
        request->send(400);
        return;
    }

    request->send(200, "text/json", profile.ToJson());
}

void handle_SavePidValues(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    PidValues values;
    if(PidValues::FromJson((char*)data, len, &values)){
        request->send(422);
        return;
    }
    
    if(Configuration::SetPidValues(values)){
        request->send(500);
        return;
    }

    request->send(200);
}

void handle_LoadPidValues(AsyncWebServerRequest *request){
    PidValues values;

    if(Configuration::GetPidValues(&values)){
        request->send(400);
        return;
    }

    request->send(200, "text/json", values.ToJson());
}

void not_found(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void on_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
        break;
    case WS_EVT_ERROR:
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    default:
        break;
    }
}

void http_register_handlers()
{
    ws.onEvent(on_event);
    srv.addHandler(&ws);

    srv.on("/", HTTP_GET, handle_index);
    srv.on("/api/SaveProfile", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, handle_SaveProfile);
    srv.on("/api/LoadProfile", HTTP_GET, handle_LoadProfile);
    srv.on("/api/SavePidValues", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, handle_SavePidValues);
    srv.on("/api/LoadPidValues", HTTP_GET, handle_LoadPidValues);
    srv.onNotFound(not_found);
}

void http_start()
{
    srv.begin();
}

void http_ws_text_all(const char *msg){
    ws.textAll(msg);
}

void http_ws_printf(const char *format, ...){
    va_list argList;
    va_start(argList, format);
    char buf[64];

    vsnprintf(buf, 64, format, argList);

    ws.textAll(buf);
}