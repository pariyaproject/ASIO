#include <iostream>
#include <boost/asio.hpp>
//p8
using namespace boost;
int main() {
	//step 1
	unsigned short port_num = 3333;

	//step 2
	asio::ip::address ip_address = asio::ip::address_v4::any();

	//step 3
	asio::ip::tcp::endpoint ep(ip_address, port_num);

	//step 4
	return 0;
}