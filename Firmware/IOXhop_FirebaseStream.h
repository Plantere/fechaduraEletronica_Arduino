#ifndef _IOXhop_FirebaseStream_H_INCLUDED
#define _IOXhop_FirebaseStream_H_INCLUDED

#include <Arduino.h>
#include <ArduinoJson.h>

#define STREAM_JSON_BUFFER_SIZE 1024
#define STREAM_JSON_DATA_BUFFER_SIZE 1024

class FirebaseStream {
public:
    FirebaseStream(String event, String data) {
        _event = event;
        DynamicJsonDocument jsonBuffer(STREAM_JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(jsonBuffer, data);
        if (!error) {
            JsonObject root = jsonBuffer.as<JsonObject>();
            if (root.containsKey("path") && root.containsKey("data")) {
                _path = root["path"].as<String>();
                _data = root["data"].as<String>();
            }
        }
    }
    
    String getEvent() {
        return _event;
    }
    
    String getPath() {
        return _path;
    }
    
    int getDataInt() {
        return _data.toInt();
    }
    
    float getDataFloat() {
        return _data.toFloat();
    }
    
    String getDataString() {
        return _data;
    }
    
    bool getDataBool() {
        return _data.indexOf("true") >= 0;
    }
    
    void getData(int &value) {
        value = getDataInt();
    }
    
    void getData(float &value) {
        value = getDataFloat();
    }
    
    void getData(String &value) {
        value = getDataString();
    }
    
    void getData(bool &value) {
        value = getDataBool();
    }
    
    JsonVariant getData() {
        DynamicJsonDocument document(STREAM_JSON_DATA_BUFFER_SIZE);
        deserializeJson(document, _data);
        return document.as<JsonVariant>();
    }
    
private:
    String _event, _path, _data = "";
    bool _dataError = false, _begin = false;
};

#endif
