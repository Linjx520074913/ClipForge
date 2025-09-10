#pragma once

#include <Windows.h>
#include "renderer.h"
#include <map>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using WSMessageCallback = std::function<void(json& data)>;

// #define DEBUG_BUILD

class Application
{
public:
    Application();
    ~Application(){}

    int run(IRenderer* renderer);
    void subscribe(std::string event, WSMessageCallback callback);

protected:
    int init_websocket();
    int init_window();
    int init_config();
    static LRESULT CALLBACK wndproc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

public:
    HWND hwnd_;
    int w_;
    int h_;
    int x_;
    int y_;

private:
    IRenderer* renderer_;
    std::map<std::string, WSMessageCallback> callback_;
};