#pragma once
#include <Magic/Magic>
#include <Magic/Utilty/Config.h>
#include <Magic/NetWork/Http/Http.h>
#include <Magic/NetWork/Http/HttpServlet.h>

using namespace Magic::NetWork;
class HookServlet :public Http::IHttpServlet{
public:
    HookServlet(const Safe<Magic::Config>& configuration);
    void sendMessageRobot(const std::string& message);
    void hook(const Safe<Http::HttpSocket>&,const Safe<Http::HttpRequest>&,const Safe<Http::HttpResponse>&);

private:
    std::string  m_Token;
};