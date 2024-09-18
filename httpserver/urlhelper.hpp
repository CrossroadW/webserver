#pragma once
#include <iostream>
#include <string>
#include <httpserver/stlhelper.hpp>
#include <httpserver/debug.hpp>
#include <map>
inline bool RemoveProtocolFromUrl(std::string const &url, std::string &protocol, std::string &rest)
{
    debug(), ("RemoveProtocolFromUrl");
    debug(), (std::string("url='") + url + "'");
    std::string::size_type pos_colon = url.find(":");

    if (pos_colon == std::string::npos)
    {
        rest = url;
        return false;
    }

    if (url.size() < pos_colon + 2)
    {
        rest = url;
        return false;
    }

    if (url[pos_colon + 1] != '/' ||
        url[pos_colon + 2] != '/')
    {
        rest = url;
        return false;
    }

    protocol = url.substr(0, pos_colon);
    rest = url.substr(3 + pos_colon); // Skipping three characters ( '://' )

    return true;
}

inline void SplitGetReq(std::string get_req, std::string &path,
                        std::map<std::string, std::string> &params)
{
    debug(), ("SplitGetReq");

    // Remove trailing newlines
    if (get_req[get_req.size() - 1] == '\r' ||
        get_req[get_req.size() - 1] == '\n')
        get_req = get_req.substr(0, get_req.size() - 1);

    if (get_req[get_req.size() - 1] == '\r' ||
        get_req[get_req.size() - 1] == '\n')
        get_req = get_req.substr(0, get_req.size() - 1);

    // Remove potential Trailing HTTP/1.x
    if (get_req.size() > 7)
    {
        if (get_req.substr(get_req.size() - 8, 7) == "HTTP/1.")
        {
            get_req = get_req.substr(0, get_req.size() - 9);
        }
    }

    std::string::size_type qm = get_req.find("?");
    if (qm != std::string::npos)
    {
        std::string url_params = get_req.substr(qm + 1);

        path = get_req.substr(0, qm);

        // Appending a '&' so that there are as many '&' as name-value pairs.
        // It makes it easier to split the url for name value pairs, he he he
        url_params += "&";

        std::string::size_type next_amp = url_params.find("&");

        while (next_amp != std::string::npos)
        {
            std::string name_value = url_params.substr(0, next_amp);
            url_params = url_params.substr(next_amp + 1);
            next_amp = url_params.find("&");

            std::string::size_type pos_equal = name_value.find("=");

            std::string nam = name_value.substr(0, pos_equal);
            std::string val = name_value.substr(pos_equal + 1);

            std::string::size_type pos_plus;
            while ((pos_plus = val.find("+")) != std::string::npos)
            {
                val.replace(pos_plus, 1, " ");
            }

            // Replacing %xy notation
            std::string::size_type pos_hex = 0;
            while ((pos_hex = val.find("%", pos_hex)) != std::string::npos)
            {
                std::stringstream h;
                h << val.substr(pos_hex + 1, 2);
                h << std::hex;

                int i;
                h >> i;

                std::stringstream f;
                f << static_cast<char>(i);
                std::string s;
                f >> s;

                val.replace(pos_hex, 3, s);
                pos_hex++;
            }

            params.insert(std::map<std::string, std::string>::value_type(nam, val));
        }
    }
    else
    {
        path = get_req;
    }
}

inline void SplitUrl(std::string const &url, std::string &protocol, std::string &server, std::string &path)
{
    debug(), ("SplitUrl");
    RemoveProtocolFromUrl(url, protocol, server);

    if (protocol == "http")
    {
        std::string::size_type pos_slash = server.find("/");

        if (pos_slash != std::string::npos)
        {
            debug(), ("slash found");
            path = server.substr(pos_slash);
            server = server.substr(0, pos_slash);
        }
        else
        {
            debug(), ("slash not found");
            path = "/";
        }
    }
    else if (protocol == "file")
    {
        path = ReplaceInStr(server, "\\", "/");
        server = "";
    }
    else
    {
        std::cerr << "unknown protocol in SplitUrl: '" << protocol << "'" << std::endl;
    }
}
