#include<iostream>
#include<boost/asio.hpp>
//p7
using namespace boost;
int main() {
	//step 1
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;

	//用于存储转化rawIP时可能发生的错误
	boost::system::error_code ec;

	//step 2
	//使用IP协议版本独立表示
	asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, ec);

	if (ec.value() != 0) {
		//IP不合法
		std::cout
			<< "Error IP, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	
	//step 3 
	asio::ip::tcp::endpoint ep(ip_address, port_num);

	//step 4
	std::system("pause");
	return 0;
}