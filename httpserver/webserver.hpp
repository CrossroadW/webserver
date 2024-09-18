#pragma once
#include <httpserver/base64.hpp>
#include <httpserver/socket.hpp>
#include <httpserver/stlhelper.hpp>
#include <httpserver/urlhelper.hpp>
#include <functional>
class webserver
{
public:
	struct http_request
	{

		Socket *s_ = nullptr;
		std::string method_;
		std::string path_;
		std::map<std::string, std::string> params_;

		std::string accept_;
		std::string accept_language_;
		std::string accept_encoding_;
		std::string user_agent_;

		/* status_: used to transmit server's error status, such as
		   o  202 OK
		   o  404 Not Found
		   and so on */
		std::string status_;

		/* auth_realm_: allows to set the basic realm for an authentication,
		   no need to additionally set status_ if set */
		std::string auth_realm_;

		std::string answer_;

		/*   authentication_given_ is true when the user has entered a username and password.
			 These can then be read from username_ and password_ */
		bool authentication_given_ = false;
		std::string username_;
		std::string password_;
	};

	using RequestFunc = void(http_request *);
	webserver(unsigned int port_to_listen, RequestFunc request_func)
	{
		SocketServer in(port_to_listen, 5);

		webserver::request_func_ = request_func;

		while (1)
		{
			Socket *ptr_s = in.Accept();

			std::thread(Request, ptr_s).detach();
		}
	}

private:
	static unsigned __stdcall Request(Socket *ptr_s)
	{
		Socket s = Socket(*ptr_s);

		std::string line = s.receiveLine();
		if (line.empty())
		{
			return 1;
		}

		http_request req;

		if (line.find("GET") == 0)
		{
			req.method_ = "GET";
		}
		else if (line.find("POST") == 0)
		{
			req.method_ = "POST";
		}

		std::string path;
		std::map<std::string, std::string> params;

		size_t posStartPath = line.find_first_not_of(" ", req.method_.length());
		debug(), line.substr(posStartPath);
		SplitGetReq(line.substr(posStartPath), path, params);

		req.status_ = "202 OK";
		req.s_ = &s;
		req.path_ = path;
		req.params_ = params;

		static const std::string authorization = "Authorization: Basic ";
		static const std::string accept = "Accept: ";
		static const std::string accept_language = "Accept-Language: ";
		static const std::string accept_encoding = "Accept-Encoding: ";
		static const std::string user_agent = "User-Agent: ";

		while (1)
		{
			line = s.receiveLine();

			if (line.empty())
				break;

			unsigned int pos_cr_lf = line.find_first_of("\x0a\x0d");
			if (pos_cr_lf == 0)
				break;

			line = line.substr(0, pos_cr_lf);

			if (line.substr(0, authorization.size()) == authorization)
			{
				req.authentication_given_ = true;
				std::string encoded = line.substr(authorization.size());
				std::string decoded = base64_decode(encoded);

				unsigned int pos_colon = decoded.find(":");

				req.username_ = decoded.substr(0, pos_colon);
				req.password_ = decoded.substr(pos_colon + 1);
			}
			else if (line.substr(0, accept.size()) == accept)
			{
				req.accept_ = line.substr(accept.size());
			}
			else if (line.substr(0, accept_language.size()) == accept_language)
			{
				req.accept_language_ = line.substr(accept_language.size());
			}
			else if (line.substr(0, accept_encoding.size()) == accept_encoding)
			{
				req.accept_encoding_ = line.substr(accept_encoding.size());
			}
			else if (line.substr(0, user_agent.size()) == user_agent)
			{
				req.user_agent_ = line.substr(user_agent.size());
			}
		}

		request_func_(&req);

		std::stringstream str_str;
		str_str << req.answer_.size();

		time_t ltime;
		time(&ltime);
		tm *gmt = gmtime(&ltime);

		static std::string const serverName = "RenesWebserver (Windows)";

		char *asctime_remove_nl = asctime(gmt);
		asctime_remove_nl[24] = '\0';

		s.sendBytes("HTTP/1.1 ");

		if (!req.auth_realm_.empty())
		{
			s.sendLine("401 Unauthorized");
			s.sendBytes("WWW-Authenticate: Basic Realm=\"");
			s.sendBytes(req.auth_realm_);
			s.sendLine("\"");
		}
		else
		{
			s.sendLine(req.status_);
		}
		s.sendLine(std::string("Date: ") + asctime_remove_nl + " GMT");
		s.sendLine(std::string("Server: ") + serverName);
		s.sendLine("Connection: close");
		s.sendLine("Content-Type: text/html; charset=ISO-8859-1");
		s.sendLine("Content-Length: " + str_str.str());
		s.sendLine("");
		s.sendLine(req.answer_);
		return 0;
	}
	static inline std::function<RequestFunc> request_func_ = nullptr;
};
