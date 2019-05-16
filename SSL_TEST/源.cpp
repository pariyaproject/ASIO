//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

enum { max_length = 1024 };

class client
{
public:
	client(boost::asio::io_context& io_context,
		boost::asio::ssl::context& context,
		const tcp::resolver::results_type& endpoints)
		: socket_(io_context, context)
	{
		socket_.set_verify_mode(boost::asio::ssl::verify_peer);
		socket_.set_verify_callback(
			std::bind(&client::verify_certificate, this, _1, _2));
			//boost::asio::ssl::rfc2818_verification("bangumi.moe"));
		//以下很关键 关系到SNI的
		// Set SNI Hostname (many hosts need this to handshake successfully)
		if (!SSL_set_tlsext_host_name(socket_.native_handle(), "bangumi.moe"))
		{
			boost::system::error_code ec((int)ERR_get_error(), boost::asio::error::get_ssl_category());
			throw boost::system::system_error(ec);
		}
		connect(endpoints);
	}

private:
	bool verify_certificate(bool preverified,
		boost::asio::ssl::verify_context& ctx)
	{
		// The verify callback can be used to check whether the certificate that is
		// being presented is valid for the peer. For example, RFC 2818 describes
		// the steps involved in doing this for HTTPS. Consult the OpenSSL
		// documentation for more details. Note that the callback is called once
		// for each certificate in the certificate chain, starting from the root
		// certificate authority.

		// In this example we will simply print the certificate's subject name.
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
		std::cout << "Verifying " << subject_name << "\n";

		return true;
	}

	void connect(const tcp::resolver::results_type& endpoints)
	{
		boost::asio::async_connect(socket_.lowest_layer(), endpoints,
			[this](const boost::system::error_code& error,
				const tcp::endpoint& /*endpoint*/)
		{
			if (!error)
			{
				handshake();
			}
			else
			{
				std::cout<< error.value() << "Connect failed: " << error.message() << "\n";
			}
		});
	}

	void handshake()
	{
		socket_.async_handshake(boost::asio::ssl::stream_base::client,
			[this](const boost::system::error_code& error)
		{
			if (!error)
			{
				send_request();
			}
			else
			{
				std::cout << error.value() << "Handshake failed: " << error.message() << "\n";
			}
		});
	}

	void send_request()
	{
		std::cout << "Enter message: ";
		std::cin.getline(request_, max_length);
		size_t request_length = std::strlen(request_);
		std::string request = "GET /rss/latest HTTP/1.1\r\n";
		request += "Host:bangumi.moe\r\n\r\n";

		boost::asio::async_write(socket_,
			boost::asio::buffer(request),
			[this](const boost::system::error_code& error, std::size_t length)
		{
			if (!error)
			{
				receive_response(length);
			}
			else
			{
				std::cout << error.value() << "Write failed: " << error.message() << "\n";
			}
		});
	}

	void receive_response(std::size_t length)
	{
		boost::asio::async_read(socket_,
			boost::asio::buffer(reply_, length),
			[this](const boost::system::error_code& error, std::size_t length)
		{
			if (!error)
			{
				std::cout << "Reply: ";
				std::cout.write(reply_, length);
				std::cout << "\n";
			}
			else
			{
				std::cout << error.value() << "Read failed: " << error.message() << "\n";
			}
		});
	}

	boost::asio::ssl::stream<tcp::socket> socket_;
	char request_[max_length];
	char reply_[max_length];
};

int main(int argc, char* argv[])
{
	try
	{
		//if (argc != 3)
		//{
		//	std::cerr << "Usage: client <host> <port>\n";
		//	return 1;
		//}
		std::string raw_ip_address = "bangumi.moe";
		unsigned short port_num = 80;

		boost::asio::io_context io_context;

		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve(raw_ip_address, "https");

		boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
		ctx.set_default_verify_paths();
		//ctx.load_verify_file("moe.pem");
		// This holds the root certificate used for verification
		//boost::asio::ip::tcp::load_root_certificates(ctx);
		//;
		//ctx.add_verify_path("Z:\\Boost\\OpenSSL-Win32\\certs");

		client c(io_context, ctx, endpoints);

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	try
	{
		//using boost::asio::ssl;
		// Create a context that uses the default paths for
		// finding CA certificates.
		//using boost::asio::ip::tcp;
		//namespace ssl = boost::asio::ssl;
		//ssl::context ctx(ssl::context::sslv23);
		//ctx.set_default_verify_paths();
		//typedef ssl::stream<tcp::socket> ssl_socket;

		//// Open a socket and connect it to the remote host.
		//boost::asio::io_context io_context;
		//ssl_socket sock(io_context, ctx);
		//tcp::resolver resolver(io_context);
		//tcp::resolver::query query("share.dmhy.org", "https");
		//boost::asio::connect(sock.lowest_layer(), resolver.resolve(query));
		//sock.lowest_layer().set_option(tcp::no_delay(true));

		//// Perform SSL handshake and verify the remote host's
		//// certificate.
		//sock.set_verify_mode(ssl::verify_peer);
		//sock.set_verify_callback(ssl::rfc2818_verification("share.dmhy.org"));
		//sock.handshake(ssl_socket::client);
	
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	system("pause");
	return 0;
}