#include<iostream>
#include<boost/asio.hpp>
using namespace boost;
//basic
//在创建端点（endpoint）之前，客户端应用程序必须获取原始IP地址和指定将与之通信的服务器的协议端口号
//另一方面，服务器应用程序通常侦听所有IP地址上的传入消息，只需要获取要侦听的端口号
//原因：服务器可以有多个IP地址与之建立连接，侦听需要侦听所有的IP地址

//===1.1===
//创建一对endpoint
//客户端方面：
//1.获取服务器应用程序的IP地址和端口号，应将IP地址指定为点分十进制（IPV4）或十六进制（IPV6）表示法中的字符串
//2.将原始的IP地址表示为asio::ip::address类对象
//3.从步骤2中创建的address对象和端口号实例化asio::ip::tcp::endpoint类对象

//===1.2===
//创建服务器的endpoint
//1.获取端口号服务器将侦听传入的请求
//2.创建asio::ip::address对象的特殊实例，表示运行服务器的主机上可用的所有IP地址
//3.从步骤2中创建的地址对象和端口号实例化asio::ip::tcp::endpoint类的对象

//获取原始IP地址之后，客户端应用程序必须根据Boost.Asio类型表示它
//Boost.Asio提供三个用于表示IP地址的类
//asio::ip::address_v4:代表一个IPV4地址
//asio::ip::address_v6:代表一个IPV6地址
//asio::ip::address:不可知类，可以表示V4和V6

//在示例中，使用asio::ip::address类使得客户端应用程序与IP协议版本（v4 or v6）无关
//意味着可以透明地与IPV4和IPV6服务器一起使用
//使用asio::ip::address::from_string()，接受表示为字符串的原始IP地址，解析并验证字符串（因此需要一个errorcode）,实例化asio::ip::address类对象，并将其返回
//此方法有四个重载，使用的是这个
//static asio::ip::address from_string(
//const std::string & str,
//boost::system::error_code & ec);
//此方法非常有用，因为它检查作为参数传递给它的字符串是否包含有效的IPv4或IPv6地址
//如果是，则实例化相应的对象，如果无效则通过第二个参数指定错误
//这意味着此功能可用于验证原始用户输入

//ep对象可用于在Boost.Asio通信相关功能中指定服务器应用程序
//第二个仅提供应侦听传入消息的端口号，未提供IP，因为服务器应用程序通常希望侦听主机上可用的所有IP地址上的传入消息
//为了表示主机上可用的所有IP地址的概念，类asio::ip::address_4和asio::ip::address_v6提供静态方法any()
//它实例化表示概念的相应类的特殊对象
//注意::与IP协议版本无关的类asio::ip::address不提供any()方法
//使用v4 v6则明确指明了是否在IPv4还是IPv6地址上接收请求

//之前的例子中使用到了asio::ip::tcp类范围内声明的端点类
/*class tcp
{
public:
 /// The type of a TCP endpoint.
 typedef basic_endpoint<tcp> endpoint;
 //...
}*/
//这意味着此端点类是basic_endpoint<>模板类的特化,旨在用于通过TCP进行通信的客户端和服务器

//Berkeley套接字API是最流行的TCP UDP协议的API
//它是围绕socket的概念设计的,一个表示通信会话上下文的抽象对象
//在我们可以执行任何网络I/O操作之前,我们必须首先一个套接字对象,然后将每个I/O操作与它相关联

//Boost.Asio借用了Berkeley SocketsAPI的许多概念,与它类似
//它包含一个表示套接字概念的类,它类似于Berkeley Socket API中的接口方法

//基本上有两种类型的套接字,旨在用于向远程应用程序发送数据或从远程应用程序接收数据或用其
//启动连接建立过程的套接字称为活动套接字,而被动套接字用于被动等待来自远程应用程序的传入连接请求
//无源套接字不参与用户数据传输

//以下算法描述了在客户端应用程序中执行以创建和打开活动套接字所需的步骤
//1.创建asio::io_service类的实例或使用之前创建的实例
//2.创建表示传输层协议(TCP/UDP)的类的对象以及套接字要与之通信的基础IP协议(IPv4/IPv6)的版本
//3.创建表示与所需协议类型对应的套接字的对象,将asio::io_service类的对象传递给套接字的构造函数
//4.调用套接字的open()方法,将表示在步骤2中创建的协议的对象作为参数传递
int Create_TCP_Socket_client_1() {
	//step 1 
	//实例化io_service,它是Boost.Asio的核心组件,提供对底层操作系统的网络I/O
	//服务的访问,Boost.Asio套接字可以通过此类的对象访问这些服务
	asio::io_service ios;

	//step 2
	//此类表示TCP协议,它不提供任何功能,而是像一个包含一组描述协议的值的数据结构
	//asio::ip::tcp类没有公共构造函数,相反它提供两个静态方法
	//v4(),v6(),它们返回asio::ip::tcp类的对象,表示TCP协议与底层IPv4或IPv6协议相对应
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	//此外,这个类还包含一些旨在与TCP协议一起使用的基本类型的声明,
	//其中包括asio::ip::tcp::endpoint asio::ip::tcp::socket asio::ip::tcp::acceptor等
	//在boost/asio/ip/tcp.hpp中
	//
	/*namespace boost {
	namespace asio {
	namespace ip {
	// ...*/
	/* class tcp
	{
	public:
	/// The type of a TCP endpoint.
	typedef basic_endpoint<tcp> endpoint;

	// ...

	/// The TCP socket type.
	typedef basic_stream_socket<tcp> socket;
	/// The TCP acceptor type.
	typedef basic_socket_acceptor<tcp> acceptor;

	// ...*/

	//step 3
	//创建一个asio::ip::tcp::socket类的对象将io_service类的对象作为参数传递给它的构造函数
	//注意:此构造函数不分配底层操作系统的套接字对象
	//当调用open方法并将指定协议的对象作为参数传递给它时,在step4中分配实际操作系统的套接字
	//在boost.asio中,打开(open)套接字意味着将它与描述套接字要通信的特定协议的完整参数集相关联
	//当Boost.Asio套接字对象提供这些参数时,它有足够的信息来分配底层操作系统的真实套接字对象
	//asio::ip::tcp::socket类提供了另一个接受协议对象作为参数的构造函数,此构造函数构造一个套接字并将其打开(open)
	//注意:如果失败,此构造函数将抛出boost::system::system_error类型的异常
	asio::ip::tcp::socket sock(ios);


	//用于存储open socket的错误
	boost::system::error_code ec;

	//step 4
	sock.open(protocol, ec);

	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	return 0;
	//可以用以下方法替换3-4
	try {
		asio::ip::tcp::socket sock2(ios, protocol);

	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message: " << e.what();
	}
}
//以下是创建一个UDP的例子
int Create_UDP_Socket_Client_1() {
	asio::io_service ios;

	asio::ip::udp protocol = asio::ip::udp::v6();

	boost::system::error_code ec;
	asio::ip::udp::socket sock(ios);

	sock.open(protocol, ec);

	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	return 0;
}
//创建被动套接字passive socket/acceptor
//它是一种套接字,用于等待来自通过TCP协议进行通信的远程应用程序的连接建立请求
//此定义有两个重要含义
//1.被动套接字仅用于服务器应用程序或可能同时扮演客户端和服务器角色混合应用程序
//2.仅为TCP协议定义被动套接字,由于UDP协议并不意味着建立连接,因此通过UDP执行通信时不需要被动套接字

//在Boost.Asio中,被动套接字由asio::ip::tcp::acceptor类表示
//类的名称建议类的对象的关键功能监听和接受或处理传入的连接请求
//1.创建asio::io_service类的实例或使用之前创建的实例
//2.创建asio::ip::tcp类的对象,该对象表示TCP协议和基础IP协议(IPv4/IPv6)的所需版本
//3.创建表示接受器套接字的asio::ip::tcp::acceptor类的对象,将asio::io_service类的对象传递给其构造函数
//4.调用acceptor socket的open()方法,将表示在2中创建的协议的对象作为参数传递

int Create_TCP_Acceptor_Server() {
	//step 1
	//所有需要访问底层操作系统服务的Boost.Asio组件都需要此类
	asio::io_service ios;

	//step 2
	//创建一个表示TCP协议的对象
	asio::ip::tcp protocol = asio::ip::tcp::v4();

	//step 3
	//就像在活动套接字中一样,此构造函数实例化了Boost.Asio的对象asio::ip::tcp::acceptor类,介不分配底层操作系统的实际套接字对象
	asio::ip::tcp::acceptor ac(ios);

	boost::system::error_code ec;

	//step 4
	//扣件系统套接字对象在步骤4中分配,调用open()方法将协议对象作为参数传递给它
	ac.open(protocol, ec);

	//
	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	return 0;
}

//解析一个DNS名字
//原始IP地址不便于人们感知和记忆
//为了使用友好的名称票房网络中的设备,引入了域名系统(DNS)
//DNS是一种分布式命名系统允许将人性化的名称与计算机网络中的设备相关联
//确切地说,DNS名称是一个或多个IP地址的别名,但不是设备的别名
//它没有命名特定的物理设备,而是可以分配给设备的IP地址
//DNS充当分布式数据库,存储DNS名称到相应IP地址的映射到的IP地址,将DNS名称转换为相应IP地址的过程称为
//DNS域名解析,现代网络操作系统包含可以查询DNS以解析DNS名称的功能,并提供可由应用程序用于执行DNS名称解析的接口

//解析DNS名称以获得运行客户端应用程序要与之通信的服务器应用程序的主机(0个或多个)的IP地址(0 or 0+)所需的步骤
//1.获取指定服务器应用程序的DNS名称和协议端口号,并将它们表示为字符串
//2.创建asio::io_service类的实例或使用之前创建的实例
//3.创建表示DNS名称腹板查询的解析程序::查询类对象
//4.创建和合必要协议的DNS名称解析程序类的实例
//5.调用解析器的resolve()方法,将步骤3中创建的查询对象作为参数传递给它
//Resolve DNS
int Resolving_DNS() {
	//step 1
	std::string host = "sirokuma.cc";
	std::string port_num = "80";

	//step 2
	asio::io_service ios;

	//step 3
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);

	//step 4
	asio::ip::tcp::resolver resolver(ios);

	//用于存储解析过程中的错误
	boost::system::error_code ec;

	//step 5
	asio::ip::tcp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);
	asio::ip::tcp::endpoint ep = *it;
	std::cout << ep.address() << std::endl;
	//处理错误
	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	return 0;

}
int main() {
	if (Resolving_DNS() != 0) {
		return 1;
	}
	std::system("pause");
}