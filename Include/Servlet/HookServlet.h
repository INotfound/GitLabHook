#pragma once
#include <Magic/Magic>
#include <Magic/Utilty/Config.h>
#include <Magic/NetWork/Http/Http.h>
#include <Magic/NetWork/Http/HttpServlet.h>

using namespace Magic::NetWork;
class HookServlet :public Http::IHttpServlet{
public:
    HookServlet();
    void sendMessageRobot(const std::string& message,const std::string& key);
    void hook(const Safe<Http::HttpSocket>&,const Safe<Http::HttpRequest>&,const Safe<Http::HttpResponse>&);
};