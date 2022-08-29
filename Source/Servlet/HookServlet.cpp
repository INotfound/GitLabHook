#include <map>
#include <tuple>
#include "Servlet/HookServlet.h"
#include "Magic/Utilty/Logger.h"

#include <RapidJson/writer.h>
#include <RapidJson/document.h>
#include <Magic/NetWork/Http/HttpClient.h>

static std::map<std::string,std::string> g_StateMap{
        {"open","创建"},
        {"close","关闭"},
        {"merge","合并"},
        {"update","更新"},
        {"approved","批准"},
        {"reopen","重新打开"},
        {"unapproved","未批准"}
};

static std::string g_FailedJson = R"({"return_code":0,"return_msg":"Failed!","data":{}})";

HookServlet::HookServlet(){
}

void HookServlet::sendMessageRobot(const std::string& message,const std::string& key) {
    MAGIC_DEBUG() << "https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=" + key;
    Safe<Http::HttpRequest> httpRequest = std::make_shared<Http::HttpRequest>();
    httpRequest->setMethod(Http::HttpMethod::POST)->setBody(message);
    Safe<Http::HttpClient> httpClient = std::make_shared<Http::HttpClient>("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key="+key,10000);
    httpClient->onError([](const asio::error_code& errorCode){
        MAGIC_WARN() << errorCode.message();
    })->onTimeOut([](){
        MAGIC_WARN() << "Http Get Time Out";
    })->onResponse([](const Safe<Http::HttpResponse>& response){
        MAGIC_DEBUG() << response->getBody();
    })->execute(httpRequest);
}

void HookServlet::hook(const Safe<Http::HttpSocket>& socket,const Safe<Http::HttpRequest>& request,const Safe<Http::HttpResponse>& response){
    MAGIC_DEBUG() << request->getBody();
    rapidjson::Document document;
    response->setStatus(Http::HttpStatus::OK);
    if(document.Parse(request->getBody().c_str()).HasParseError() && !document.IsObject()){
        response->setStatus(Http::HttpStatus::METHOD_NOT_ALLOWED);
        response->setBody(g_FailedJson);
        socket->sendResponse(response);
        return;
    }
    if(!document.HasMember("object_kind") || !document["object_kind"].IsString()){
        MAGIC_WARN() << "Not Found Key: object_kind";
        response->setBody(g_FailedJson);
        socket->sendResponse(response);
        return;
    }
    std::string kind = document["object_kind"].GetString();
    if(kind == "push"){
        if(!document.HasMember("commits") || !document["commits"].IsArray()){
            MAGIC_WARN() << "Not Found Key: commits";
            response->setBody(g_FailedJson);
            socket->sendResponse(response);
            return;
        }
        std::string userName;
        std::string repositoryName;
        if(document.HasMember("user_name") && document["user_name"].IsString()){
            userName = document["user_name"].GetString();
        }
        if(document.HasMember("repository") && document["repository"].IsObject()){
            auto repository = document["repository"].GetObject();
            if(repository.HasMember("name") && repository["name"].IsString()){
                repositoryName = repository["name"].GetString();
            }
        }


        auto array = document["commits"].GetArray();

        std::string id;
        std::string name;
        std::string message;
        std::vector<std::tuple<std::string,std::string,std::string>> commits;
        for(const auto& v :array){
            id.clear();
            name.clear();
            message.clear();
            if(v.HasMember("id") && v["id"].IsString()){
                id = v["id"].GetString();
            }

            if(v.HasMember("author") && v["author"].IsObject()){
                auto author = v["author"].GetObject();
                if(author.HasMember("name") && author["name"].IsString()){
                    name = author["name"].GetString();
                }
            }

            if(v.HasMember("message") && v["message"].IsString()){
                message = Magic::Trim(v["message"].GetString(),"\n");
            }
            commits.emplace_back(id,name,message);
        }

        if(!commits.empty()){
            std::string content = "## Git Push\\n" +userName+ "将<font color=\\\"info\\\">" + Magic::AsString(commits.size()) + "</font>个提交上传到"+repositoryName+"仓库\\n\n";
            for(const auto& v : commits){
                content += ">开发者:<font color=\\\"comment\\\">" + std::get<1>(v) + "</font>\n";
                content += ">提交信息:<font color=\\\"comment\\\">\\\"" + Magic::Replace(std::get<2>(v),"\n","\\\"</font>\n<font color=\\\"comment\\\">\\\"") + "\\\"</font>\n";
                content += ">Commit Id:<font color=\\\"comment\\\">" + std::get<0>(v) + "</font>\n\n";
            }
            std::string robot = R"({"msgtype":"markdown","markdown":{"content": ")"+ content +R"("}})";
            MAGIC_DEBUG() << robot;
            this->sendMessageRobot(robot,request->getParam("key"));
        }
    }else if(kind == "merge_request"){
        if(!document.HasMember("object_attributes") || !document["object_attributes"].IsObject()){
            MAGIC_WARN() << "Not Found Key: object_attributes";
            response->setBody(g_FailedJson);
            socket->sendResponse(response);
            return;
        }

        std::string userName;
        if(document.HasMember("user") && document["user"].IsObject()){
            auto user = document["user"].GetObject();
            if(user.HasMember("name") && user["name"].IsString()){
                userName = user["name"].GetString();
            }
        }
        std::string repositoryName;
        if(document.HasMember("repository") && document["repository"].IsObject()){
            auto repository = document["repository"].GetObject();
            if(repository.HasMember("name") && repository["name"].IsString()){
                repositoryName = repository["name"].GetString();
            }
        }

        auto attributes = document["object_attributes"].GetObject();
        std::string state;
        std::string title;
        std::string description;
        std::string targetBranch;
        std::string sourceBranch;

        if(attributes.HasMember("title") && attributes["title"].IsString()
            && attributes.HasMember("action") && attributes["action"].IsString()
            && attributes.HasMember("description") && attributes["description"].IsString()
            && attributes.HasMember("target_branch") && attributes["target_branch"].IsString()
            && attributes.HasMember("source_branch") && attributes["source_branch"].IsString()){
            title = attributes["title"].GetString();
            state = attributes["action"].GetString();
            description = attributes["description"].GetString();
            targetBranch = attributes["target_branch"].GetString();
            sourceBranch = attributes["source_branch"].GetString();
        }
        std::string content = "## Git Merge Request\\n";
        content += userName+ "<font color=\\\"info\\\">" + g_StateMap[state] + "</font>一个合并请求\\n\n";
        content += "> 请求主题: " + title + "\n";
        content += "> 目标分支: <font color=\\\"warning\\\">" + targetBranch + "</font>\n";
        content += "> 来源分支: <font color=\\\"warning\\\">" + sourceBranch + "</font>\n";
        if(!description.empty())
            content += "> 请求描述： " + description;

        std::string robot = R"({"msgtype":"markdown","markdown":{"content": ")"+ content +R"("}})";
        this->sendMessageRobot(robot,request->getParam("key"));
    }
    response->setBody(R"({"return_code":1,"return_msg":"ok!","data":{}})");
    socket->sendResponse(response);
}