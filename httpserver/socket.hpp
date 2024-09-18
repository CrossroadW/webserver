#pragma once
#include <WinSock2.h>
#include <string_view>
#include "debug.hpp"
#include <thread>
#include <exception>
#include <corecrt_io.h>
#include <map>

enum TypeSocket
{
	BlockingSocket,
	NonBlockingSocket
};

class Socket
{
protected:
	struct WinSockCtl
	{
		WinSockCtl()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData))
			{
				throw std::runtime_error("WSAStartup falied!");
			}
			if (LOBYTE(wsaData.wVersion) != 2 ||
				HIBYTE(wsaData.wVersion) != 2)
			{
				throw std::runtime_error("WSAStartup version invalid!");
			}
		}
		~WinSockCtl()
		{
			debug(), "~WinSockCtl()";
			WSACleanup();
		}
	};

public:
	std::string receiveBytes()
	{
		std::string ret;
		char buf[1024]{};

		while (1)
		{
			u_long arg = 0;
			if (ioctlsocket(sock_, FIONREAD, &arg) == SOCKET_ERROR)
				break;

			if (arg != NO_ERROR)
				break;

			if (arg > 1024)
				arg = 1024;

			// int rv = _read(sock_, buf, arg);
			int rv = recv(sock_, buf, arg, 0);
			if (rv <= 0)
				break;

			std::string t;

			t.assign(buf, rv);
			ret += t;
		}

		return ret;
	}

	std::string receiveLine()
	{
		std::string ret;
		while (true)
		{
			char r{};
			// switch (_read(sock_, &r, 1)){
			switch (recv(sock_, &r, 1, 0))
			{
			case 0:
				return ret;
			case -1:
				return "";
			}

			ret += r;
			if (r == '\n')
				return ret;
			;
		}
	}

	void sendLine(std::string s) const
	{
		s += '\n';
		send(sock_, s.c_str(), s.length(), 0);
	}

	void sendBytes(std::string_view s) const
	{
		send(sock_, s.data(), s.length(), 0);
	}

	explicit Socket(const Socket &oth) : Socket(oth.sock_) {}

	Socket(Socket &&) = delete;

	Socket &operator=(const Socket &oth)
	{
		Socket(oth).swap(*this);
	}

	void swap(Socket &oth)
	{
		std::swap(oth.sock_, sock_);
	}

	virtual ~Socket()
	{
		// closesocket(sock_);
		debug(), "Socket ~";
	}

protected:
	friend class SocketSelect;
	friend class SocketServer;
	explicit Socket(SOCKET sock) : sock_(sock) {}

	explicit Socket() : sock_(socket(AF_INET, SOCK_STREAM, 0))
	{
		if (sock_ == INVALID_SOCKET)
		{
			throw std::runtime_error("invalid socket");
		}
	}

	inline static std::shared_ptr<WinSockCtl> WSASockContext_ = std::make_shared<WinSockCtl>();

	SOCKET sock_;
};

class SocketClient : public Socket
{
public:
	SocketClient(std::string_view host, int port)
	{
		std::string error;

		hostent *he;
		if ((he = gethostbyname(host.data())) == 0)
		{
			error = strerror(errno);
			throw std::runtime_error(error);
		}

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr = *((in_addr *)he->h_addr);
		memset(&(addr.sin_zero), 0, 8);

		if (::connect(sock_, (sockaddr *)&addr, sizeof(sockaddr)))
		{
			error = strerror(WSAGetLastError());
			throw std::runtime_error(error);
		}
	}
};

class SocketServer : public Socket
{
public:
	SocketServer(int port, int connections, TypeSocket type = BlockingSocket)
	{
		sockaddr_in sa;

		memset(&sa, 0, sizeof(sa));

		sa.sin_family = PF_INET;
		sa.sin_port = htons(port);
		if (sock_ == INVALID_SOCKET)
		{
			throw std::runtime_error("INVALID_SOCKET");
		}

		if (type == NonBlockingSocket)
		{
			u_long arg = 1;
			ioctlsocket(sock_, FIONBIO, &arg);
		}

		/* bind the socket to the internet address */
		if (bind(sock_, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			closesocket(sock_);
			throw std::runtime_error("INVALID_SOCKET");
		}

		listen(sock_, connections);
	}

	Socket *Accept()
	{
		SOCKET new_sock = accept(sock_, 0, 0);
		if (new_sock == INVALID_SOCKET)
		{
			int rc = WSAGetLastError();
			if (rc == WSAEWOULDBLOCK)
			{
				return nullptr; // non-blocking call, no request pending
			}
			else
			{
				throw "Invalid Socket";
			}
		}

		return new Socket(new_sock);
	}
};

class SocketSelect
{
public:
	SocketSelect(Socket const *const s1, Socket const *const s2 = NULL, TypeSocket type = BlockingSocket)
	{
		FD_ZERO(&fds_);
		FD_SET(const_cast<Socket *>(s1)->sock_, &fds_);
		if (s2)
		{
			FD_SET(const_cast<Socket *>(s2)->sock_, &fds_);
		}

		TIMEVAL tval;
		tval.tv_sec = 0;
		tval.tv_usec = 1;

		TIMEVAL *ptval;
		if (type == NonBlockingSocket)
		{
			ptval = &tval;
		}
		else
		{
			ptval = 0;
		}

		if (select(0, &fds_, (fd_set *)0, (fd_set *)0, ptval) == SOCKET_ERROR)
			throw "Error in select";
	}

	bool Readable(Socket const *const s)
	{
		if (FD_ISSET(s->sock_, &fds_))
			return true;
		return false;
	}

private:
	fd_set fds_;
};

namespace
{

	void testHost1()
	{
		try
		{
			SocketClient s("www.renenyffenegger.ch", 80);

			s.sendLine("GET / HTTP/1.0");
			s.sendLine("Host: www.renenyffenegger.ch");
			s.sendLine("");

			while (1)
			{
				std::string l = s.receiveLine();
				if (l.empty())
					break;
				std::cout << l;
				std::cout.flush();
			}
		}
		catch (std::runtime_error const &e)
		{
			debug(), e.what();
		}
	}

	void testHost2()
	{
		try
		{
			SocketClient s("www.baidu.com", 80);
			s.sendLine("GET / HTTP/1.0");
			s.sendLine("Host: www.baidu.com");
			s.sendLine("");
			while (1)
			{
				std::string l = s.receiveLine();
				if (l.empty())
					break;
				std::cout << l;
			}
		}
		catch (std::runtime_error const &e)
		{
			debug(), e.what();
		}
	}

	void testSocketServer()
	{
		SocketServer in(2000, 5);

		auto s = std::shared_ptr<Socket>(in.Accept());

		while (1)
		{
			std::string r = s->receiveLine();
			if (r.empty())
				break;
			s->sendLine(r);
		}
	}
}