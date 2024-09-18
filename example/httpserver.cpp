#include <httpserver/socket.hpp>
#include <httpserver/debug.hpp>
#include <httpserver/webserver.hpp>

inline void Request_Handler(webserver::http_request *request)
{
    Socket s = Socket(*(request->s_));

    std::string title;
    std::string body;
    std::string bgcolor = "#ffffff";
    std::string links =
        "<p><a href='/red'>red</a> "
        "<br><a href='/blue'>blue</a> "
        "<br><a href='/form'>form</a> "
        "<br><a href='/auth'>authentication example</a> [use <b>rene</b> as username and <b>secretGarden</b> as password"
        "<br><a href='/header'>show some HTTP header details</a> ";

    if (request->path_ == "/")
    {
        title = "Web Server Example";
        body = "<h1>Welcome to Rene's Web Server</h1>"
               "I wonder what you're going to click" +
               links;
    }
    else if (request->path_ == "/red")
    {
        bgcolor = "#ff4444";
        title = "You chose red";
        body = "<h1>Red</h1>" + links;
    }
    else if (request->path_ == "/blue")
    {
        bgcolor = "#4444ff";
        title = "You chose blue";
        body = "<h1>Blue</h1>" + links;
    }
    else if (request->path_ == "/form")
    {
        title = "Fill a form";

        body = "<h1>Fill a form</h1>";
        body += "<form action='/form'>"
                "<table>"
                "<tr><td>Field 1</td><td><input name=field_1></td></tr>"
                "<tr><td>Field 2</td><td><input name=field_2></td></tr>"
                "<tr><td>Field 3</td><td><input name=field_3></td></tr>"
                "</table>"
                "<input type=submit></form>";

        for (std::map<std::string, std::string>::const_iterator i = request->params_.begin();
             i != request->params_.end();
             i++)
        {

            body += "<br>" + i->first + " = " + i->second;
        }

        body += "<hr>" + links;
    }
    else if (request->path_ == "/auth")
    {
        if (request->authentication_given_)
        {
            if (request->username_ == "rene" && request->password_ == "secretGarden")
            {
                body = "<h1>Successfully authenticated</h1>" + links;
            }
            else
            {
                body = "<h1>Wrong username or password</h1>" + links;
                request->auth_realm_ = "Private Stuff";
            }
        }
        else
        {
            request->auth_realm_ = "Private Stuff";
        }
    }
    else if (request->path_ == "/header")
    {
        title = "some HTTP header details";
        body = std::string("<table>") +
               "<tr><td>Accept:</td><td>" + request->accept_ + "</td></tr>" +
               "<tr><td>Accept-Encoding:</td><td>" + request->accept_encoding_ + "</td></tr>" +
               "<tr><td>Accept-Language:</td><td>" + request->accept_language_ + "</td></tr>" +
               "<tr><td>User-Agent:</td><td>" + request->user_agent_ + "</td></tr>" +
               "</table>" +
               links;
    }
    else
    {
        request->status_ = "404 Not Found";
        title = "Wrong URL";
        body = "<h1>Wrong URL</h1>";
        body += "Path is : &gt;" + request->path_ + "&lt;";
    }

    request->answer_ = "<html><head><title>";
    request->answer_ += title;
    request->answer_ += "</title></head><body bgcolor='" + bgcolor + "'>";
    request->answer_ += body;
    request->answer_ += "</body></html>";
}

int main()
{
#if _WIN32
    setlocale(LC_ALL, ".utf-8");
    SetConsoleOutputCP(CP_UTF8);
#endif

    webserver(8080, Request_Handler);
}
