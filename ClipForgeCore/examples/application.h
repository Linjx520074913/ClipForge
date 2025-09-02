#pragma once

#include <Windows.h>
#include "renderer.h"
#include <map>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using WSMessageCallback = std::function<void(json& data)>;

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
    static LRESULT CALLBACK wndproc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);

public:
    HWND hwnd_;

private:
    IRenderer* renderer_;
    std::map<std::string, WSMessageCallback> callback_;
};