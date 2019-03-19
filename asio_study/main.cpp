#include<iostream>
#define BOOST_ASIO_ENABLE_CANCELIO
#include<boost/asio.hpp>
#include<thread>
#include<mutex>
#include<memory>
using namespace boost;
//basic
//在创建端点（endpoint）之前，客户端应用程序必须获取原始IP地址和指定将与之通信的服务器的协议端口号
//另一方面，服务器应用程序通常侦听所有IP地址上的传入消息，只需要获取要侦听的端口号
//原因：服务器可以有多个IP地址与之建立连接，侦听需要侦听所有的IP地址
//注意:端点是(服务器的IP,服务器中服务程序的侦听端口)
//而在接受器(acceptor)中,绑定的endpoint是(主机可用的所有IP地址,服务器本机需要侦听的端口号)


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
		//创建并打开套接字
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
int Resolving_DNS_TCP() {
	//step 1
	//首先获取DNS名称和协议端口号,并将它们表示为字符串
	std::string host = "ipv6.google.com";
	std::string port_num = "80";

	//step 2
	//创建io_service类的实例,解析器在DNS名称解析过程中使用该实例来访问底层OS的服务
	asio::io_service ios;

	//step 3
	//创建一个asio::ip::tcp::resolver::query类的对象
	//此对象表示对DNS名称的查询,包含要解析的DNS名称,在DNS名称解析后将用于构造端点对象(endpoint)的端口号以及控制解析过程的某些特定方面的一组标志,表示为位图
	//因为服务被指定为协议端口号(80),而还是服务名称(HTTP,FTP),因此传递了asio::ip::tcp::resolver::query::nummeric_service,显示通知查询对象以便正确解析端口号值
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);

	//step 4
	//创建一个asio::ip::tcp::resolver类的实例,此类提供DNS名称解析功能,它需要底层操作系统的服务,因此通过asio::io_services类的对象作为参数传递给它的构造函数来访问它们
	asio::ip::tcp::resolver resolver(ios);

	//用于存储解析过程中的错误
	boost::system::error_code ec;

	//step 5
	//DNS名称解析在解析器对象的resolve()方法,如果方法失败,后一个对象包含错误信息,否则返回asio::ip::tcp""resolver::iterator类的对象
	//该类是一个迭代器,指向表示解析结果的集合的第一个元素
	//该集合中的对象数与解析产生的IP地址总数一样多,每个集合元素都包含asio::ip::tcp::endpoint类的对象,该对象是从解析过程产生的一个IP地址实例化的
	//以及随相应查询对象提供的端口号(Step 1中的)可以通过asio::ip::basic_resolver_entry<tcp>::endpoint()getter方法访问端点对象
	asio::ip::tcp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);
	//注意:最好不要使用以下直接赋值法,因为可能是一个有误的DNS没有任何结果
	//asio::ip::tcp::endpoint ep = *it;
	//asio::ip::tcp::endpoint ep = it->endpoint();
	//asio::ip::tcp::resolver::iterator类的默认构造对象表示结束迭代器
	asio::ip::tcp::resolver::iterator it_end;
	for (; it != it_end; ++it)
		std::cout << it->endpoint().address() << std::endl;
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
	//通常,当运行服务器应用程序的主机的DNS名称被解析为多个IP地址时并且相应被解析为多个端点时
	//客户端程序可以尝试逐个与每个端点通信,直到收到所需的响应
	//请注意:当DNS名称映射到多个IP地址并且其中一些是IPv4而其他是IPv6地址时,DNS名称可以解析为IPv4地址可IPv6地址
	//也可以解析为两者,因此掭的集合可能包含表示IPv4和IPv6的端点

}
int Resolving_DNS_UDP() {

	asio::io_service ios;

	std::string host = "sirokuma.cc";
	std::string port_num = "80";

	asio::ip::udp::resolver::query resolver_query(host, port_num, asio::ip::udp::resolver::query::numeric_service);

	asio::ip::udp::resolver resolver(ios);

	boost::system::error_code ec;

	asio::ip::udp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);

	asio::ip::udp::resolver::iterator it_end;

	//处理错误
	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}


	for (; it != it_end; ++it)
		std::cout << it->endpoint().address() << std::endl;

	return 0;

}
//socket与endpoint的绑定
//将套接字与特定端点相关联的过程叫绑定(binding)
//当套接字绑定到端点时,所有从该网络端口作为目标地址进入主机的网络数据包将被操作系统重写向到该特定套接字
//同样,从绑定到特定端点的套接字发出的所有数据将通过与该端点中指定的相应IP地址相关联的网络接口从主机输出到网络
//某些操作隐式绑定未绑定的活动套接字,例如,将未绑定的活动套接字连接到远程应用程序的操作会将其隐匿绑定到底层操作系统选择的IP地址和协议端口号
//通常,客户端应用程序不需要将活动套接字显式绑定到特定端点(endpoint),因为它不需要该特定端点服务器通信
//它只需要任何端点用于此目的,因此它通常委派选择套接字应绑定到操作系统的IP地址和端口号的权限
//但是在某些特殊情况下,客户端应用程序可能需要使用特定的IP地址和协议端口号与远程应用程序进行通信,因此将其套接字显式绑定到该特定端点(endpoint)
//即使主机上存在单个网络接口和单个IP地址,每次执行隐式绑定时,套接字可能绑定到不同的协议端口号.
//与通常不关心其活动套接字将与远程应用程序通信的IP地址和协议端口号的客户端程序不同,服务器应用程序通常需要明确地将其接受器套接字绑定到特定端点

//以下算法描述了创建接受器套接字并将其绑定到端点所需的步骤,该端点指定主机上可用的所有IP地址以及IPv4 TCP服务器应用程序中的特定协议端口号:
//1.获取服务器上的应该监听传入的连接请求协议端口号
//2.创建一个端点,该端点表示主机上可用的所有IP地址以及步骤1中获取的协议端口号
//3.创建并打开接收器套接字
//4.调用acceptor套接字的bind()方法,将端点对象作为参数传递给它
int Create_Server_Acceptor_TCP() {
	//Step 1
	//首先获取协议端口号
	unsigned short port_num = 3333;

	//Step 2
	//注意:endpoint构造的第二个参数是本机开放的端点(希望对方连接的本机的端口号)
	//创建一个端点表示主机上可用的所有IP地址和指定的端口号
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	//Step 3
	//创建并打开这个接受器(acceptor)
	//实例化并打开接受器套接字,在步骤2中创建的端点包含有关传输协议和基础IP协议(IPv4)版本的信息
	//因此,我们不需要创建另一个表示协议的对象来将其传递给acceptor套接字的构造函数
	//相反,我们使用端点的protocol()方法,该方法返回表示相应的asio::ip::tcp类的对象
	try
	{
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());

		//用于存储绑定时可能发生的错误的信息
		boost::system::error_code ec;

		//Step 4
		//调用acceptor套接字的bind()方法,传递一个对象,该对象表示应该将acceptor套接字绑定到的端点作为方法的参数
		//如果调用成功,则acceptor套接字绑定到相应的端点,并准备开始侦听该端点上的传入连接请求
		acceptor.bind(ep, ec);

		//错误处理
		if (ec.value() != 0)
		{
			std::cout << "Failed, Code =" <<
				ec.value() << ", Message: " << ec.message();
			return 1;
		}

		return 0;
	}
	catch (boost::system::system_error &e)
	{
		return 1;
	}


}
//UDP版本的Acceptor
//因为UDP不建立连接,因此使用活动套接字等待传入请求
int Create_Server_Acceptor_UDP() {
	unsigned short port_num = 3333;

	asio::ip::udp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	try
	{
		asio::ip::udp::socket socket(ios, ep.protocol());

		boost::system::error_code ec;

		socket.bind(ep, ec);

		//错误处理
		if (ec.value() != 0)
		{
			std::cout << "Failed, Code =" <<
				ec.value() << ", Message: " << ec.message();
			return 1;
		}

		return 0;
	}
	catch (boost::system::system_error &e)
	{
		return 1;
	}

}

//在TCP套接字可用于与远程应用程序通信之前,它必须与其建立逻辑连接
//根据TCP协议,连接建立过程在于在两个应用程序之间交换服务消息,如果成功,则导致两个应用程序在逻辑上连接并准备好
//彼此通信,粗略地说连接过程如下
//客户端应用程序在想要与服务器应用程序通信时,会创建并打开活动套接字并在其上发出connect()命令,
//并指定具有端点对象的目标服务器应用程序
//这导致通过网络将连接建立请求消息发送到服务器应用程序,服务器应用程序接收请求并在其侧创建活动套接字,将其标记为连接到特定客户端
//并使用确认在服务器端成功设置连接的消息回复客户端
//接下来,已经从服务器接收到确认的客户端将其套接字标记为连接到服务器,并向其发送一条消息,确认在客户端侧成功建立了连接
//当服务器从客户端接收到了确认消息时,认为两个应用程序之间的逻辑连接已建立,在两个连接的套接字之间假设点对点通信模型
//这意味着如果套接字A连接到套接字B,则两者只能相互通信并且不能与任何其他套接字C通信,在套接字,在套接字A可以与套接字C通信之前,它必须
//关闭与套接字B的连接并建立新连接使用套接字C

//以下算法描述了在TCP客户端应用程序中执行以将活动套接字连接到服务器应用程序所需的步骤
//1.获取目标服务器应用程序的IP地址和协议端口号
//2.从步骤1中获取的IP地址和协议端口号创建asio::ip::tcp::endpoint类的对象
//3.创建并打开活动套接字
//4.调用套接字的connect()方法,将步骤2中创建的端点对象指定为参数
//5.如果方法成功,则认为套接字已连接,可用于向服务器发送数据和从服务器接收数据

int Create_Client_Connect_TCP() {


	//Step 1
	std::string ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try
	{
		asio::io_service ios;
		//Step 2
		//创建一个endpoint指定要连接的服务器应用程序
		asio::ip::address adress = asio::ip::address::from_string(ip_address);
		asio::ip::tcp::endpoint ep(adress, port_num);

		//review
		//使用DNS名称解析
		std::string dns_name = "www.sirokuma.cc";
		std::string str_port_num = "80";

		asio::ip::tcp::resolver::query resolver_query(dns_name, str_port_num, asio::ip::tcp::resolver::query::numeric_service);

		asio::ip::tcp::resolver resolver(ios);

		boost::system::error_code ec;
		asio::ip::tcp::resolver::iterator it =
			resolver.resolve(resolver_query, ec);
		asio::ip::tcp::resolver::iterator it_end;
		for (; it != it_end; ++it)
			std::cout << it->endpoint().address() << std::endl;
		//review over

		//Step 3 
		//创建并打开socket
		asio::ip::tcp::socket socket(ios, ep.protocol());

		//Step 4
		//此处也可以使用System_error异常类来处理错误
		//[注意]
		//connect()功能将套接字连接到服务器,连接是同步执行的,这意味着该方法会阻塞调用程序线程
		//直到建立连接操作或发生错误,请注意,在连接套接字之前,没有将套接字绑定到任何本地端点.
		//这不意味着套接字保持未绑定状态,在执行连接建立过程之前,套接字的connect()方法将套接字绑定到端点
		//该端点由扣件系统选择的IP地址和协议端口号组成
		//注意 小结
		//也就是说,绑定(bind)是相关本机的信息,一般客户端程序不需要显式地bind一个endpoint
		//而服务器的应用程序(TCP)中Acceptor就需要执行bind这一操作,至少端口信息不能明确
		//而connect方法是由客户端发出的,由socket发出,在执行connect()时会隐式绑定本机的
		//IP信息以及所使用的端口
		//在UDP中,不需要建立连接,因此不存在UDP的Acceptor,因此在服务端使用Socket.bind
		//一般bind一个所有主机可用的ip(asio::ip::address_v4::any())和一个特定的端口号
		socket.connect(ep, ec);
		//
		if (ec.value() != 0) {
			std::cout << "Failed, Code =" <<
				ec.value() << ", Message: " << ec.message();

			return 1;
		}
	}
	catch (boost::system::system_error &e)
	{
		return 1;
	}

	//注意:connect()方法和 asio::ip::address::from_string()静态方法
	//都会抛出system_error异常,两个方法都有接受一个error_code参数的重载并且不抛出异常
	//但这种情况下使用异常处理更好
	return 0;
}
//以下算法描述了将套接字连接到由DNS名称和协议端口号表示的服务器应用程序所需的步骤
//1.获取运行服务器应用程序的主机的DNS名称和服务器的端口号,并将它们表示为字符串
//2.使用asio::ip::tco::resolver类解析DNS名称
//3.创建活动套接字而不打开它
//4.调用asio::connect函数,将套接字对象的步骤2中获得的迭代器对象作为参数传递给它
int Create_Client_Connect_DNS_TCP() {
	//Step 1
	std::string host = "sirokuma.cc";
	std::string port_num = "80";

	try
	{
		//Step 2
		asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);

		asio::io_service ios;

		asio::ip::tcp::resolver resolver(ios);

		//可能抛出异常
		asio::ip::tcp::resolver::iterator it =
			resolver.resolve(resolver_query);

		//Step 3
		//注意,在这里创建的socket不会打开套接字,因为不知道提供的DNS名称将解析的IP地址的版本(IPv4/IPv6)
		//asio::connect()函数在将套接字连接到指定正确协议对象的每个端点之前打开套接字,如果失败则关闭它
		asio::ip::tcp::socket sock(ios);

		//Step 4
		//一个工厂函数
		//这个方法会尝试每一个IP直到成功一个或全部失败,然后抛出异常
		asio::connect(sock, it);

		//此时已经成功连接,可以用于发送数据或接收数据
		//
		//

		//注意 小结
		//到目前为止,使用socket的注意
		//socket如果只是创建,需要提供io_service的实例,所有需要使用到底层网络I/O服务的类都需要提供
		//socket创建之后并没有分配底层操作系统的套接字对象,当调用open(协议类型)之后,socket实例拥有了足够的
		//信息之后,才会被分配底层操作系统的套接字对象
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "Error, Code: " <<
			e.code() << ", Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}
//当客户端程序想要通过TCP协议与服务器通信时,它首先需要与该服务器建立逻辑连接
//为此,客户端分配一个活动套接字并在其上发出连接命令(例如通过调用套接字对象上的connect()方法)
//这会导致连接建立请求消息被发送到服务器,在服务器端,必须在服务器应用程序接受和处理来自客户端的连接请求之前执行某些安排
//在此之前,操作系统将拒绝针对此服务器应用程序的所有连接请求

//首先,服务器应用程序创建并打开一个接受套接字其绑定到特定端点
//此时,到达接收器套接字端点的客户端连接请求仍被操作系统拒绝
//为了使操作系统开始接受针对与特定接受器套接字相关联的特定端点的连接请求,必须将该接受器套接字切换到监听模式
//之后,操作系统为与此接受器套接字关联的挂起连接请求分配队列,并开始接受发送给它的连接请求
//当新的连接请求到达时,它最初由操作系统接收,操作系统将其放入与作为连接请求的目标的接受器套接字相关联的待决连接请求队列
//在队列中时,连接请求可供服务器应用程序处理,服务器应用程序在准备好处理下一个连接请求时,对其进行排队并对其进行处理
//请注意,acceptor套接字仅用于与客户端应用程序建立连接,并且不用于进下一步的通信过程
//处理挂起的连接请求时,接受器套接字分配新的活动套接字,将其连接到已发出该连接请求的相应客户端应用程序
//然后,这个新的活动套接字就可以用于与客户端进行通信,接受器套接字可用于处理下一个挂起的连接请求.

//总之就是Acceptor不进行数据通信,只负责TCP连接的建立(bind之后还需要转换为监听模式),并且可以分配新的活动套接字来进行已连接的数据通信

//以下算法描述了如何设置接受器套接字以便它开始侦听传入连接,然后如何使用它来同步处理挂起的连接请求
//该算法假定在同步模式下只处理一个传入连接
//1.获取服务器将接收传入连接请求的端口号
//2.创建服务器端点
//3.实例化并打开一个接受器套接字
//4.将接受器套接字绑定到在步骤2中创建的服务器端点
//5.调用acceptor套接字的listen()方法,使其开始侦听端点上的传入连接请求
//6.实例化活动套接字对象
//7.准备好处理连接请求时,调用acceptor套接字的accept()方法,将步骤6中创建的活动套接字对象作为参数传递
//8.如果调用成功,则活动套接字连接到客户端应用程序,并准备用于与之通信
int Create_Server_TCP_SYN() {

	//待处理连接队列的大小
	const int BACKLOG_SIZE = 30;

	//Step 1
	unsigned short port_num = 3333;
	//Step 2
	//创建一个服务器端点用于指定运行服务器应用程序的主机上可用的所有IP地址以及特定的协议端口号
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	try
	{
		//Step 3
		//可能会抛出异常
		//实例化并打开一个授受器套接字
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());

		//Step 4
		//绑定到之前的endpoint
		acceptor.bind(ep);

		//Step 5
		//调用acceptor的listen()方法将BACKLOG_SIZE常量值作为参数传递
		//此调用将接受器套接字切换到侦听传入连接请求的状态
		//除非我们在acceptor对象上调用listen()方法,否则所有到达相应端点的连接请求都将被操作系统网络软件拒绝
		//应用程序必须显式通知操作系统它希望通过此调用开始侦听特定端点上的传入连接请求
		//listen接受的参数指定了操作系统维护的队列大小,它将连接请求从客户端(client)发送到该操作系统(本程序)
		//请求保留在队列中,并等待服务器应用程序出列并处理它们,当队列变满时,操作系统将拒绝新的连接请求
		acceptor.listen(BACKLOG_SIZE);

		//Step 6
		//创建一个活动的套接字对象而不打开它,和asio::connect()有部分一样的道理
		asio::ip::tcp::socket sock(ios);

		//Step 7
		//调用acceptor socket的accept()方法,此方法接受活动套接字作为参数并执行多个操作,首先,
		//它检查与包含挂起连接请求的接受器套接字关联的队列,如果队列为空,则该方法阻止执行直到新的连接请求到达接受器套接字
		//绑定到的endpoint,并且操作系统将其放入队列中
		//如果队列中至少有一个连接请求可用,则从中提取队列顶部的连接请求并进行处理
		//作为参数传递给accept()方法的活动套接字连接到发出连接请求的相应客户端应用程序
		//如果连接建立过程成功,则accept()方法返回并打开(open())活动套接字并连接到客户端应用程序,并可用于向其发送数据和从中接收数据
		acceptor.accept(sock);
		//注意:在处理连接请求时,acceptor套接字不会将自身连接到客户端应用程序.
		//相反,它打开并连接另一个活动套接字,然后用于与客户端应用程序进行通信
		//接受器套接字仅侦听和处理(接受)传入的连接请求

		//注意:UDP服务器不使用接受器套接字,因为UDP并不意味着建立连接
		//相反,使用绑定到端点并侦听传入I/O消息的活动套接字,并使用相同的活动套接字进行通信

		//此时,sock已经连接到了客户端程序可以用来收发数据
	}
	catch (boost::system::system_error &e)
	{

		return e.code().value();
	}
	return 0;
}
//========Chapter 2==========//
//I/O Operations

//I/O操作是任何分布式应用程序的网络基础结构中的关键操作
//它们直接参与数据交换过程
//输入操作用于从远程应用程序接收数据,而输出操作允许向它们发送数据
//
//I/Obuffer
//通信意味着在两个或多个进程之间交换数据
//网络I/O涉及使用内存缓冲区,这些内存缓冲区是在用于存储数据的进程的地址空间中分配的连续内存块
//在进行任何类型的输入操作(如:通过网络从文件,管道或远程计算机读取某些文件)时,数据到达进程并
//且必须存储在其地址空间中的某个位置以便它可用进一步处理
//也就是说,当缓冲区派上用场时,在执行输入操作之前,分配缓冲区,然后在操作期间用作数据目标点
//当输入操作完成时,缓冲区包含可由应用程序处理的输入数据,同样在执行输出操作之前,必须准备数据并将其放入输出缓冲区
//然后输出缓冲区用于输出操作,在输出操作中它扮演数据源的角色,显然,缓冲区是执行任何类型I/O的任何应用程序的基本组成部分
//包括网络I/O
//p55

//Boost.Asio支持两种类型的I/O操作:同步和异步
//同步操作会阻止调用它们的执行线程,并且仅在操作完成时才解除阻塞
//因此,这种类型的操作的名称:同步
//第二种是异步操作,启动异步操作时,它与回调函数或仿函数相关联,该函数在操作完成时由Boost.Asio库调用
//这些类型的I/O操作提供了很大的灵活性,但可能会使代码复杂化
//操作的启动很简单,并且不会阻塞执行的线程,这允许我们使用线程来运行其他任务,而异步操作则在后台运行
//Boost.Asio库是作为一个框架实现的,它利用了控制方法的反转.一个或多个异步操作启动后,应用处理在这个线程执行的线程库
//而后者使用的一个运行事件循环和调用应用程序提供的回调,通知有关的以前发起的异步操作的完成
//异步操作的结果作为参数传递给回调函数
//除此之外,我们还将考虑取消异步操作,关闭和关闭套接字等操作
//取消先前启动的异步操作的能力非常重要,它允许应用程序声明先前启动的操作不再相关
//这可能会节省应用程序的资源(CPU和内存),否则会造成浪费,当应用层协议没有为我们提供指明消息边界的
//其他方法时,如果需要分布式应用程序的一部分通知另一部分已发送整个消息,则关闭套接字是有用的

//固定长度I/O缓冲区通常与I/O操作一起使用,并且在已知要发送或接收的消息的大小时起到数据源或目标的作用
//例如,这可以是在空闲存储器中分配的可写缓冲区,当从套接字读取数据时,该可写缓冲区用作数据目标点

//在Boost.Asio中,固定长度缓冲区由两个类之一表示:
//asio::mutable_buffer或asio::const_buffer
//这两个类都表示一个连续的内存块,它由块的第一个字节的地址及其大小
//(以字节为单位)指定.正如这些类的名称所示,asio::mutable_buffer表示可写缓冲区,而asio::const_buffer表示只读缓冲区
//但是,asio::mutable_buffer和asio::const_buffer类都不直接在Boost.Asio I/O函数和方法中使用
//相反,引入了MutableBufferSequence和ConstBufferSequence概念

//MutableBufferSequence概念指定一个对象,该对象表示asio::mutable_buffer对象的集合
//相应地,ConstBufferSequence概念指定一个对象,该对象表示asio::const_buffer对象的集合
//执行I/O操作的Boost.Asio函数和方法接受满足MutableBufferSequence或ConstBufferSequence概念要求的对象
//作为表示缓冲区的参数

//虽然在大多数用例中,单个I/O操作中涉及单个缓冲区,但在某些特定情况下(例如,在内存受限的环境中)
//开发人员可能希望使用包含多个较小简单缓冲区的复合缓冲区分布在进程的地址空间
//Boost.Asio I/O函数和方法旨在使用复合缓冲区,复合缓冲区表示为满足MutableBufferSequence或ConstBufferSequence概念要求的缓冲区集合
//例如,std::vector<asio::mutable_buffer>类的对象满足MutableBufferSequence概念的要求,因此,它可用于表示与I/O相关
//的函数和方法中的复合缓冲区,所以,现在我们知道如果我们有一个表示为asio::mutable_buffer或asio::const_buffer类的对象的缓冲区
//我们仍然不能将它与Boost提供的I/O相关函数或方法一起使用,缓冲区必须表示为一个对象,分别满足MutableBufferSequence或ConstBufferSequence概念的要求
//为此,我们例如可以通过实例化std::vector<asio::mutable_buffer>类的对象并将缓冲区组成的缓冲区对象的集合
//现在缓冲区是集合的一部分,满足MutableBufferSequence要求可以在I/O操作中使用
//但是,虽然这种方法可以很好地创建由两个或多个简单缓冲区组成的复合缓冲区,但是当涉及到表示单个简单缓冲区这样的简单任务时它
//为了可以与Boost.Asio I/O函数或方法一起使用,但看起来过于复杂,同时,Boost.Asio为我们提供了一种通过I/O相关函数和方法简化单个缓冲区使用的方法 
//asio::buffer()自由函数有28个重载,它们接受缓冲区的各种表示,并返回asio::mutable_buffer_1或asio::const_buffer_1类的对象,
//如果传递给asio::buffer()函数的缓冲区参数是只读类型,则该函数返回asio::const_buffer_1类的对象;
//否则返回asio::mutable_buffers_1类的对象.asio::mutable_buffers_1和asio::const_buffers_1类分别是asio::mutable_buffer和asio::const_buffer类的
//适配器,它们提供了满足MutableBUfferSequence和ConstBufferSequence概念要求的接口和行为
//这允许我们将这些适配器作为参数传递给Boost.Asio I/O函数和方法

//下面算法描述了如何准备一个缓冲区,该缓冲区可以与Boost.Asio socket的方法一起使用,
//该方法执行输出操作,如asio::ip::tcp::socket::send()或asio::write()自由函数
//1.分配一个缓冲区,注意,此步骤不涉及Boost.Asio中的任何功能或数据类型
//2.使用要用作输出的数据填充缓冲区
//3.将缓冲区表示为满足ConstBufferSeqyebce概念要求的对象
//4.缓冲区已准备好与Boost.Asio输出方法和函数一起使用
int Create_Const_Buffer() {

	std::string buf;
	//Step 1 + Step 2
	buf = "hello";

	//Step 3
	//这里使用const_buffer,
	//[注意]const_buffer_1已过时
	//之后会提到
	asio::const_buffer output_buf = asio::buffer(buf);

	//Boost.Asio输出函数:下面是表示TCP套接字的Boost.Asio类的send()方法的声明
	/*template<typename ConstBufferSequence>
	std::size_t send(const ConstBufferSequence & buffers);	*/
	//这是一个模板方法,它接受一个满足ConstBufferSequece概念要求的对象作为表示缓冲区的参数
	//合适的对象是一个复合对象,它表示asio::const_buffer类的对象集合
	//并提供支持对其元素的进行迭代的典型集合接口
	//例如:std::vector<asio::const_buffer>类的对象适合用作send()方法的参数,但std::string或asio::const_buffer类的对象不适合
	//为了使用我们的std::string对象和表示TCP套接字的类的send()方法,我们可以这样做:
	asio::const_buffer asio_buf(buf.c_str(), buf.length());
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);
	//代码中buffer_sequence的对象满足ConstBufferSequence概念的要求,因此,它可以用作套接字对象的send()方法的参数
	//但是,这种方法非常复杂,相反,我们使用Asio提供的asio::buffer()函数来获取适配器对象
	//我们可以在I/O操作中直接使用它们
	//asio::const_buffers_1 output_buf = asio::buffer(buf);
	//然而以上的方法已过时
	asio::io_service ios;
	asio::ip::tcp::socket sock(ios);
	boost::system::error_code ec;
	sock.open(asio::ip::tcp::v4(), ec);
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 3333);

	sock.connect(ep);
	//由此可见目前版本的asio已经抛弃了send只能接受满足XXXXBufferSequence要求的参数的
	//send()函数的接受一个const_buffer对象,同时也依旧接受一个const_buffer_sequence
	sock.send(output_buf);
	sock.send(buffers_sequence);
	//Step 4
	return 0;
}

//以下算法描述了如何准备可以与Boost.Asio套接字执行输入操作的方法一起使用的缓冲区
//例如asio::ip::tcp::socket::receive()或asio::read()自由函数
//1.分配一个缓冲区,缓冲区的大小必须足够大,以适应要接收的数据块,注意,此步骤不涉及Boost.Asio中的任何功能或数据类型
//2.使用满足MutableBufferSequence概念要求的对象表示缓冲区
//3.缓冲区已准备好与Boost.Asio输入方法和函数一起使用

int Create_Mutable_Buffer() {
	//与上一个事例相似,区别在于缓冲区已分配但没有填充数据,因为其目的不同
	//这次,缓冲区旨在在输入操作期间从远程应用程序数据
	//使用输出缓冲区时,必须正确表示输入缓冲区,以便它可以与Boost.Asio I/O方法和函数一起使用
	//但是,在这种情况下,缓冲区必须表示为满足MutableBufferSequence概念要求的对象
	//与ConstBufferSequence相反,此概念表示可变缓冲区的集合,即可以写入的缓冲区
	//在这里,我们使用buffer()函数,它帮助我们创建缓冲区所需的表示
	const size_t BUF_SIZE_BYTES = 20;

	//Step 1
	//分配缓冲区,缓冲区是在空闲内存中分配的字符数组
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);

	//Step 2
	//[注意]mutable_buffers_1已过时,使用mutable_buffer
	asio::mutable_buffer input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);

	//Step 3
	return 0;
	//
	//[注意]
	//const_buffer和mutable_buffer只提供缓冲区的接口,不控制其生命周期

}
//总之:小结
//当前版本的ASIO已经抛弃了send()write()等函数必须使用一个满足一个序列的参数的要求
//因此const_buffers_1和mutable_buffers_1两个适配器类已经被抛弃使用了
//那些函数已经额外可以接受类似const_buffer和mutable_buffer的参数了


//可扩展缓冲区是在向其写入新数据时动态增加其大小的缓冲区
//它们通常用于在传入消息的大小未知时从套接字读取数据
//某些应用程序层协议未定义消息的确切大小
//相反,消息的边界由消息本身末尾的特定符号序列表示,或者由发送者在完成发送消息后
//发出的传输协议服务消息文件结束(EOF)表示
//例如,根据HTTP协议,请求和响应消息的标头部分没有固定长度,其边界由四个
//ASCII符号序列表示<CR><LF><CR><LF>,这是消息的一部分
//在这种情况下,Boost.Asio库提供动态可扩展缓冲区和可以使用它们的函数非常有用
//
//可扩展的面向流的缓冲区在Boost.Asio中用asio::streambuf类表示,它是typedef asio::basic_streambuf<> streambuf;
//asio::basic_streambuf<>类继承自std::streambuf,这意味着它可以用作STL流类的流缓冲区
//除此之外,Boost.Asio提供的几个I/O函数处理表示为此类对象的缓冲区
//我们可以用asio::streambuf类的对象,就像我们处理从std::streambuf类继承的任何流缓冲类一样
//例如,我们可以将此对象分配给流
//std::istream,std::ostream或std::iostream,具体取决于我们的需要
//然后使用stream的运算符<<()和>>()用于流中写入和读取数据
//p62

//示例程序,实例化asio::strembuf的对象,向其写入一些数据,然后将数据从缓冲区读回到std::string类对象
int Test_Buffer_IO() {
	asio::streambuf buf;

	//输出流对象将数据重定向到buf流缓冲区
	//通常,在典型的客户端或服务器应用程序中数据将通过Boost.Asio输入函数
	//(例如asio::read())写入buf流缓冲区,该函数接受流缓冲区对象作为参数并从中读取数据,套接字
	//到该缓冲区,现在要从流缓冲区读回数据,为此,我们分配一个名为
	//message1的字符串对象,然后使用std::getline函数读取当前存储在buf流缓冲区中的
	//部分字符串,直到分隔符符号为\n
	//因此,string1对象包含Messages1,而buf流缓冲区包含分隔符号后面的其余初始字符串,Message2
	std::ostream output(&buf);

	output << "Messages1\nMessages2";

	std::istream input(&buf);

	std::string message1;

	std::getline(input, message1);

	std::cout << message1 << std::endl;

	std::getline(input, message1);

	std::cout << message1 << std::endl;

	std::getline(input, message1);

	std::cout << message1 << std::endl;
	//输出
	//Messages1
	//Messages2
	//Messages2

	return 0;
}
//写入TCP套接字是一种输出操作,用于将数据发送到连接到此套接字的远程应用程序
//同步写入是使用Boost.Asio提供的套接字发送数据的最简单方法
//执行同步写入套接字的方法和函数会阻塞执行的线程,并且在将数据(至少一些
//数据量)写入套接字或发生错误之前不会返回

//写入Boost.Asio库提供的套接字的最基本方法是使用asio::ip::tcp::socket类的
//write_sonme()方法,以下是方法的重载之一的声明
/*template<
typename ConstBufferSequence>
std::size_t write_some(
const ConstBufferSequence & buffers);*/
//此方法接受一个表示复合缓冲区作为参数的对象,顾名思义,它将一些数据从缓冲区写入套接字
//如果方法成功,则返回值表示写入的字节数,这里要强调的是,该方法可能不会发送通过buffers参数提供给它的所有
//数据,该方法仅保证在未发生错误时至少写入一个字节,这意味着,在一般情况下,为了将所有数据从
//缓冲区写入套接字,我们可能需要多次调用此方法

//以下算法描述了将数据同步写入分布式应用程序中的TCP套接字所需的步骤
//1.在客户端应用程序中,分配,打开和连接活动的TCP套接字,在服务器应用程序中,通过使用接受器
//套接字接受连接请求来获取连接的活动TCP套接字
//2.分配缓冲区并将其填入要写入套接字的数据
//3.在循环中,调用套接字的wriet_some()方法多次,以便发送缓冲区中可用的所有数据
void WriteToSocket(asio::ip::tcp::socket &sock) {
	//Step 2
	std::string buf = "Hello ...";

	std::size_t total_bytes_written = 0;

	unsigned short times = 0;
	//Step 3
	while (total_bytes_written != buf.length()) {
		//注意,write_some()返回的值是调用此函数期间
		//成功写入的字节数
		//在单次调用write_some()方法期间写入套接字的字节数取决于几个因素
		//一般情况下,开发人员不知
		total_bytes_written += sock.write_some(
			asio::buffer(buf.c_str() +
				total_bytes_written,
				buf.length() - total_bytes_written));
		++times;
	}
	//本机测试是只用了一次write_some调用就全部发送了
	std::cout << "Times = " << times << std::endl;
}

int Test_First_Client_TCP() {

	std::cout << "Client:" << std::endl;
	//这个一般放在try里面endpoint声明后面
	asio::io_service ios;

	try {
		//可能抛出异常
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 3333);
		//可能抛出异常
		asio::ip::tcp::socket sock(ios, ep.protocol());
		sock.connect(ep);

		//buffer
		//std::string buf = "Hello, Server!";
		//asio::const_buffer buff = asio::buffer(buf);

		//sock.write_some(buff);

		//调用循环write_some方法发送
		WriteToSocket(sock);

		//Read
		//这里使用unique_ptr可能是因为mutable_buffer并不负责释放在堆上申请的内存空间,直接交给unique_ptr处理
		asio::mutable_buffer input(static_cast<void *>((std::unique_ptr<char[]>(new char[20])).get()), 20);
		//这里read_some返回的是已经读取到的字节数
		//与asio::buffer_size()不同(返回的是总的缓冲区的大小)
		std::size_t bytes = sock.read_some(input);
		//从缓冲区读取内存,先cast为char*,再限制读取的字节数
		std::string data(asio::buffer_cast<char *>(input), bytes);


		std::cout << data << std::endl;

	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message = "
			<< e.what() << std::endl;
		return e.code().value();
	}
	return 0;

}
int Test_First_Server_TCP() {
	std::cout << "Server:" << std::endl;



	unsigned short MAX_SEQ_SIZE = 50;

	try {
		asio::io_service ios;
		//可能异常
		asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 3333);
		//
		asio::ip::tcp::acceptor acceptor(ios);

		//不要忘记acceptor也要首先open才能被真正关联底层套接字
		//否则会报错 提供的文件句柄无效
		acceptor.open(ep.protocol());
		//不要忘记bind,bind一般只用在服务端
		//bind绑定的是本机的信息,endpoint(主机可用的所有IP,本机开放的端口)
		acceptor.bind(ep);
		//只是绑定不行,还要切换到监听状态,同时给予消息队列的最大值
		acceptor.listen(MAX_SEQ_SIZE);

		//请注意此处不能直接打开sock
		asio::ip::tcp::socket sock(ios);

		//因为accept函数会协助打开,给予更多的信息(协议和具体的IP地址)
		acceptor.accept(sock);

		//Read
		asio::mutable_buffer input(static_cast<void *>((std::unique_ptr<char[]>(new char[20])).get()), 20);
		std::size_t bytes = sock.read_some(input);

		std::string data(asio::buffer_cast<char *>(input), bytes);


		std::cout << data << std::endl;
		//[注意]
		//这里碰到一个问题,直接使用asio::streambuf作为sock.read_some()
		//的参数会报错,

		//Write
		//因为是本机,只用了一次就将write_some发送完毕
		std::string buf = "Yeah Client!";
		asio::const_buffer buff = asio::buffer(buf);

		sock.write_some(buff);

		//[注意]
		//服务器accept成功之后一定要先read不能write,
		//这样会造成read_some连接中断

	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message = "
			<< e.what() << std::endl;
		return e.code().value();
	}
	return 0;

}
//测试以上的函数
int Test_First_TCP() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {
		Test_First_Server_TCP();
	}
	else {
		Test_First_Client_TCP();
	}
	return 0;
}

//asio::ip::tcp::socket类包含另一种将数据同步写入名为send()的套接字
//的方法,这种方法有三种重载
//其中一个等同于write_some()方法,如前所述,它具有完全相同的签名和功能
//与write_some()方法相比,第二个重载接受额外的参数
//
/*template<
typename ConstBufferSequence>
std::size_t send(
 const ConstBufferSequence & buffers,
 socket_base::message_flags flags);*/
//这个附加参数名为flags,它可用于指定位掩码,表示控制操作的标志
//因为很少使用这些标志,所以不会考虑它
//第三个重载相当于第二个重载,但在发生问题时不会抛出异常,相反,通过
//error_code类型的附加方法输出参数返回
//p68d

//使用套接字的write_some()方法写入套接字对于这种简单的操作来说似乎非常
//复杂,即使想要发送一个由几个字节组成的小小处,也必须使用一个循环,一个变量来跟踪已经
//写入了多少字节,并为循环的每次迭代正确构造一个缓冲区
//这种方法容易出错,使代码更难理解.
//幸运的是,Boost.Asio提供了一个免费的功能,简化了对的写入,这个函数叫做asio::write()
/*template<
typename SyncWriteStream,
typename ConstBufferSequence>
std::size_t write(
SyncWriteStream & s,
const ConstBufferSequence & buffers);*/
//此函数接受两个参数,第一个命名为s的是对满足SyncWriteStream概念要求的对象的引用
//有关要求的完整列表,请查文档
//表示TCP套接字的asio::ip::tcp::socket类的对象满足这些要求,因此可以用作函数的第一个参数
//名为buffers的第二个参数表示缓冲区(简单或复合),并包含要写入套接字的数据
//与write_some()方法相反,后者将一些数据从缓冲区写入套接字,asio::write()函数写入缓冲区中可用的所有数据
//这简化了对套接字的写入,使代码更简洁,更清晰,如果我们使用asio::write()函数而还是套接字对象的write_some()方法
//将数据写入套接字,那么上一个例子中writeToSocket()函数就会如下所示
void writeToSocketEnhanced(asio::ip::tcp::socket &sock) {
	std::string buf = "hello ...";

	asio::write(sock, asio::buffer(buf));
}
//asio::write()函数的实现方式类似于原始的writeToSkcket()函数,是通过在循环中多次调用socket对象的write_some()方法实现的

//从TCP套接字读取是一种输入操作,用于接收连接到此套接字的远程应用程序发送的数据
//同步读取是使用Boost.Asio提供的套接字接收数据的最简单方法
//从套接字执行同步读取的方法和函数会阻塞执行的线程,并且在从套接字读取数据
//(至少一些数据量)或发生错误之前不会返回

//从Boost.Asio库提供的套接字读取数据的最基本方法是asio::ip::tcp::socket类的read_some()方法
/*template<
typename MutableBufferSequence>
std::size_t read_some(
 const MutableBufferSequence & buffers);*/
//这个方法接受一个表示可写缓冲区(单个或复合)作为参数的对象,顾名思义,它
//从套接字向缓冲区读取一些数据量,如果方法成功,则返回值表示读取的字节数
//重要的是要注意,无法控制方法读取的字节数,该方法仅保证在未发生错误时至少读取一个字节
//这意味着,在一般情况下,为了从套接字读取一定是的数据,我们可能需要多次调用该方法

//以下算法描述了从分布式应用程序中的TCP套接字同步读取数据所需的步骤
//1.在客户端应用程序中,分配,撕开和连接活动的TCP,在服务器应用程序中,通过使用接受器
//套接字接受请求来获取连接的活动TCP套接字
//2.分配足够大小的缓冲区以适合要读取的预期消息
//3.在循环中,调用套接字的read_some()方法的次数与读取消息所需的次数相同

std::string readFromeSocket(asio::ip::tcp::socket &sock) {
	const unsigned char MESSAGE_SIZE = 7;
	char buf[MESSAGE_SIZE];
	std::size_t total_bytes_read = 0;

	while (total_bytes_read != MESSAGE_SIZE) {
		total_bytes_read += sock.read_some(
			asio::buffer(buf + total_bytes_read,
				MESSAGE_SIZE - total_bytes_read));
	}
	//在单次调用read_some()方法期间从套接字读取的字节数取决于几个因素
	//在一般情况下,开发人员不知道,因此应当使用循环来读取套接字中的所有数据
	return std::string(buf, total_bytes_read);
}

int Read_Some_SYN() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;

	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);

		asio::io_service ios;

		asio::ip::tcp::socket sock(ios, ep.protocol());

		sock.connect(ep);
		//sokc参数传递时必须已经connect(),否则失效
		readFromeSocket(sock);
	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message = "
			<< e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}
//asio::ip::tcp::socket类包含另一种套接字同步读取数据的方法
//称为receive(),这种方法有三种重载,其中一个等同于read_some()方法,如前所述
//它具有完全相同的签名,并提供完全相同的功能,这些方法在某种意义上是同义词
//与read_some()方法相比,第二个重载接受一个额外的参数
/*template<
 typename MutableBufferSequence>
std::size_t receive(
 const MutableBufferSequence & buffers,
 socket_base::message_flags flags);*/
//这个附加参数名为flags,它可用于指定位掩码,表示控制操作的标志,
//由于很少使用这些标志,因此我们不会在本书中考虑
//第三个重载相当于第二个,但在发生错误时不会抛出异常而是通过boost::system::error_code类型的附加输出参数返回错误信息
//[注意]
//写入套接字的write_some与send方法与这里的
//读取套接字的read_some与receive方法类似

//使用套接字的read_some（）方法从套接字读取对于这种简单的操作来说似乎非常复杂。
//这种方法要求我们使用循环，变量来跟踪已经读取了多少字节，并为循环的每次迭代正确构造缓冲区。
//这种方法容易出错，使代码更难以理解和维护。
//幸运的是，Boost.Asio提供了一系列免费函数，可以简化从不同上下文中的套接字同步读取数据的过程
//有三个这样的函数,每个函数都有几个重载,提供了丰富的功能,便于从套接字读取数据
//
//[1]
//asio::read()函数
//它是三个函数中最简单的一个
/*template<
typename SyncReadStream,
typename MutableBufferSequence>
std::size_t read(
SyncReadStream & s,
const MutableBufferSequence & buffers);*/
//这个函数接受两个参数,第一个名为s的是对满足SyncReadStream概念要求的对象的引用
//表示 TCP套接字的asio::ip::tcp::socket类的对象满足这些要求,因此可以用作
//函数的第一个参数,名为buffers的第二个参数表示将从套接字读取数据的缓冲区(简单或复合)
//与套接字的read_some()函数相反,asio::read()函数在单个调用期间从套接字读取数据,直到
//一个缓冲区传递给它但发生了错误,这简化了从套接字读取并使代码更清洁
//如果使用asio::read()方法,前面的readFromSocket()函数将是这样的
std::string readFromSocketEnhanced(asio::ip::tcp::socket& sock) {
	const unsigned char MESSAGE_SIZE = 7;
	char buf[MESSAGE_SIZE];
	//这个函数调用将阻塞执行线程,直到正好7个字节或发生错误
	asio::read(sock, asio::buffer(buf, MESSAGE_SIZE));

	return std::string(buf, MESSAGE_SIZE);
}

//
//[2]
//asio::read_until()函数
//它提供了一种从套接字读取数据直到在数据中遇到指定模式的方法
//此函数有八个重载
/*template<
typename SyncReadStream,
typename Allocator>
std::size_t read_until(
SyncReadStream & s,
boost::asio::basic_streambuf< Allocator > & b,
char delim);*/
//第一个参数名为S的量对满足SyncReadStream概念要求的对象的引用
//表示TCP套接字的asio::ip::tcp::socket类的对象满足这些要求,因此可以用作函数的第一个参数
//名为b的第二个参数表示面向流的可扩展缓冲区,其中将读取数据名为delim的最后一个参数指定了分隔符
//asio::read_until()函数将从s套接字读取数据到缓冲区b直到它遇到数据读取部分中delim参数指定的字符
//遇到指定的字符时,函数返回

//重要的是要注意asio::read_until()函数的实现是以可变大小的块读取的(就是使用套接字的read_some()方法读取)
//当函数返回时,缓冲区b可能在分隔符符号后面包含一些符号,远程应用程序在分隔符符号之后发送更多数据
//(例如,它可能连续发送两个消息,每个消息最后都有一个分隔符),则可能会发生这种情况,换名话说,当asio::read_until()
//函数成功返回时,保证缓冲区b包含至少一个分隔符符号,或可能包含更多
//[注意]这个函数会返回包含多出的数据
//开发人员有责任解析缓冲区中的数据,并在分隔符符号后包含数据时处理该情况
//如果我们想要从套接字读取所有数据直到遇到特定符号,我们将如下实现readFromSocket()函数
//假设消息分隔符是一个新的行\n
std::string readFromSocketDelim(asio::ip::tcp::socket &sock) {
	asio::streambuf buf;

	//asio::read(sock, buf);//OK
	asio::read_until(sock, buf, '\n');
	//以下正确
	asio::mutable_buffer bufs = buf.prepare(512);
	//此时buf类型为mutable_buffers_1
	//or mutable_buffer
	sock.receive(bufs);
	//以下是错误的,read_some不能使用asio::streambuf作为参数,即使语法正确
	//sock.read_some(buf);//error
	std::string message;

	std::istream input_stream(&buf);

	//注意:message可能包含'\n'之后的数据
	//但是由于分隔符是'\n'因此调用了getline之后自动分隔了'\n'之后的数据
	std::getline(input_stream, message);

	return message;
}

//
//[3]
//asio::read_at()函数
//此函数提供了一种从套接字特定的偏移量开始读取数据的方法
//但很少使用,函数的实现方式类似于原始的readFromSocket()函数,
//通过多次调用socket中的read_some()来实现的直到满足终止条件呀发生错误

//[异步]
//异步写入是一种将数据发送到远程应用程序的灵活而有效的方法
//用于异步将数据写入Boost提供的套接字的最基本工具
//是asio::ip::tcp::socket类的async_write_some()方法
/*template<
typename ConstBufferSequence,
typename WriteHandler>
void async_write_some(
const ConstBufferSequence & buffers,
WriteHandler handler);*/
//此方法启动写入操作并立即返回,它接受一个对象,该对象表示一个缓冲区
//该缓冲区包含要写入套接字的数据作为其第一个参数
//第二个参数是一个回调,当一个启动(初始)的操作完成时,它将由Boost.Asio调用
//此参数可以是函数指针,仿函数或满足WriteHandler概念要求的任何其他对象
//回调函数应当有以下函数签名
/*void write_handler(
 const boost::system::error_code& ec,
 std::size_t bytes_transferred);*/
//这里,ec是一个参数,表示出现错误的code,而bytes_transferred参数表示在相应的异步操作
//期间已向套接字写入了多少字节,正如函数的名字所示,它启动了一个旨在将d一些数据从缓冲区写入套接字
//的操作,如果没有发生错误,此方法可确保在相应的异步操作期间至少写入一个字节
//这意味着,在一般情况下,为了将缓冲区中可用的所有数据写入套接字,我们可能需要多次执行此异步
//操作.

//以下算法描述了执行和实现应用程序所需的步骤,该应用程序异步地将数据写入TCP套接字,
//请注意,此算法提供了实现此类应用程序的可能方法
//Boost.Asio非常灵活,允许我们通过以多种不同的方式异步地将数据写入套接字来组织和构建应用程序
//1.定义一个数据结构,其中包含一个指向套接字对象的指针,一个缓冲区和一个用作写入字节计数器的变量
//2.定义在异步写入操作完成时将调用的回调函数
//3.在客户端应用程序中,分配并打开活动TCP套接字并将其连接到远程应用程序.在服务器应用程序中
//通过接受连接请求获取连接的活动TCP套接字
//4.分配一个缓冲区并用要写入套接字的数据填充它
//5.通过调用套接字的async_write_some()方法启动异步写入操作
//6.在asio::io_service类的对象上调用run()方法;
//7.在回调中,增加写入的字节计数器,如果写入的字节数小于要写入的总字节数,
//则启动新的异步写入操作以写入下一部分数据
//p78

//Step 1
//定义一个数据结构,其中包含一个指向套接字对象的指针,一个
//包含要写入的数据的缓冲区,以及一个包含已写入的字节数的计数器变量
struct Session0 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::string buf;
	std::size_t total_bytes_written;
};
//Step 2
//定义一个回调函数,它将在异步操作完成时调用
void callback(const boost::system::error_code &ec,
	std::size_t bytes_transferred,
	std::shared_ptr<Session0> s)
{
	if (ec.value() != 0) {
		std::cout << "Error, code = "
			<< ec.value() << ", Message = "
			<< ec.message() << std::endl;

		return;
	}

	s->total_bytes_written += bytes_transferred;

	if (s->total_bytes_written == s->buf.length()) {
		std::cout << "===Send OK===" << std::endl;
		return;
	}

	s->sock->async_write_some(
		asio::buffer(s->buf.c_str() + s->total_bytes_written,
			s->buf.length() - s->total_bytes_written),
		std::bind(callback, std::placeholders::_1, std::placeholders::_2, s));

}
//先跳过步骤3,并在单独的函数中实现步骤4和5
void writeToSocket2(std::shared_ptr<asio::ip::tcp::socket> sock) {
	
	//首先在空闲内存中分配Session数据结构的实例
	std::shared_ptr<Session0> s(new Session0);

	//Step 4
	//填充这个实例,并将参数传递给这个对象的sock指针
	s->buf = std::string("Hello123456789");
	s->total_bytes_written = 0;
	s->sock = sock;

	//Step 5
	//因为套接字的async_write_some()方法可能无法一次性将所有数据写入
	//套接字,所以我们可能需要在回调函数中启动另一个异步写操作
	//这就是我们需要Session对象的原因,我们将它分配给空闲内存(堆)而不是栈;
	//因为它必须存在,直到调用回调函数,最后我们启动异步操作,调用socket对象的async_write_some()方法

	//第一个参数是缓冲区,其中包含要写入套接字的数据
	//因为操作是异步的,所以Boost.Asio可能在操作启动和回调函数被调用时访问buffer
	//,这意味着缓冲区必须保持不变,并且必须可用才能调用回调
	//我们通过将缓冲区存储在Session对象中来保证这一点,而Session对象又存储在空闲内存(堆)中
	//第二个参数是在异步操作完成时要调用的回调
	//Boost.Asio将回调定义为一个概念,它可以是函数或函数对象,接受两个参数
	//回调的第一个参数指定在执行操作时发生的错误(如果有)
	//第二个参数指定操作已经写入的字节数

	//因为我们想要向我们的回调函数传递一个额外的参数,一个指向相应Session对象的指针
	//它作为操作的上下文,我们使用std::bind()函数来构造一个我们附加一个
	//指向Session对象的指针作为第三个参的函数对象
	//然后将函数对象作为回调参数传递给套接字对象的async_write_some()方法,
	//因为它是异步的,所以async_write_some方法不会阻止执行的线程
	//它启动写入操作并返回,实际的写入操作由Boost.Asio库和底层操作系统在幕后执行,当
	//操作完成或发生错误时,将调用回调,调用时,我们的示例应用程序中名为callback的回调函数检查操作
	//是否成功或是否发生错误开始,在后一种情况下,错误信息输出到标准输出流并返回函数
	//否则,总写入字节的计数器增加了作为操作结果写入的字节数
	//然后,我们检查写入套接字的总数是否等于缓冲区的大小,如果这些值相等,这意味着所有数据都已写入套接字,
	//并且没有其他工作要做,回调函数返回,但是如果要写入的缓冲区中仍有数据,则启动新的异步写入操作

	//注意缓冲区的开头如何移动已经写入的字节数,以及缓冲区的大小如何相应地减小相同的值
	//作为回调,我们使用std::bind()函数指定相同的callback()函数并附加一个额外的参数
	//Session对象,就像我们启动第一个异步操作时一样
	//启动异步写入操作和后续回调调用的循环重复,直到缓冲区中的所有数据都写入套接字或发生
	//错误,当回调函数返回而不启动新的异步操作时,在main()函数中调用的asio::io_service::run()方法
	//将解除执行线程的阻塞并返回,main函数也会返回,这是应用程序退出的时间
	s->sock -> async_write_some(
		asio::buffer(s->buf),
		std::bind(callback,
			std::placeholders::_1,
			std::placeholders::_2,
			s));

}
//现在回到Step 3
int First_ASYN_TCP_Write(){
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;

	//[注意]
	//Boost.Asio可能会为某些内部操作创建其他线程,但它保证在这些线程
	//的上下文中不执行任何应用程序代码

	//分配,打开并同步将套接字连接到远程应用程序中,然后通过将指针传递给
	//套接字对象来调用writeToSocket()函数
	//此函数启动异步写入操作并返回
	//此函数继续调用asio::io_service类的对象上的run()方法
	//其中Boost.Asio捕获执行线程并在完成时使用它来调用与异步操作相关的回调函数
	//只要至少有一个挂起的异步操作,asio::ioservice::run()方法就会
	//阻塞,当最后一个挂起的异步操作最后一个回调完成时,此方法返回
	//
	try {
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), port_num);

		asio::io_service ios;
		std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios, ep.protocol()));

		sock->connect(ep);

		std::cout << "---Connect OK---" << std::endl;

		writeToSocket2(sock);

		//Step 6
		ios.run();
	}
	catch (boost::system::system_error& e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}

//虽然上一个示例中描述的async_write_some()方法允许异步地将数据写入套接字,
//但基于它的方案有点复杂且容易出错,幸运的是,Boost.Asio提供了一种使用free函数
//asio::async_write()以异步方式将数据写入套接字的方法
//
/*template<
typename AsyncWriteStream,
typename ConstBufferSequence,
typename WriteHandler>
void async_write(
AsyncWriteStream & s,
const ConstBufferSequence & buffers,
WriteHandler handler);*/
//
//此函数与套接字的async_write_some()方法非常相似
//它的第一个参数是满足AsyncWriteStream概念要求的对象
//asio::ip::tco::socket类的对象满足这些要求,因此可以与此函数一起使用
//asio::async_write()函数的第二个和第三个参数类似于上一个示例中描述的TCP
//套接字的async_write_some()方法的第一个和第二个参数
//这些参数是包含要写入的数据的缓冲区以及表示回调的函数或对象,将在操作完成时调用
//与套接字的async_write_some()方法(启动将一些数据从缓冲区写入套接字的操作)相反
//asio::async_write()函数启动操作,该操作写入缓冲区中可用的所有数据
//在这种情况下,只有当缓冲区中的所有可用数据都写入套接字或发生错误时才会调用回调
//这简化了对套接字的写入,使代码更简洁,更清晰.
//
//使用asio::async_write()函数来重写
//首先,我们不需要跟踪写入套接字的字节数,因此,Session结构变得更小
struct Session2 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::string buf;
};
//之后,我们知道当调用回调函数时,这意味着来自缓冲区的所有数据都已写入套接字或发生了错误
//这使得回调函数更加简单
void callback2(const boost::system::error_code &ec,
	std::size_t bytes_transferred,
	std::shared_ptr<Session2> s)
{
	if (ec.value() != 0) {
		std::cout << "Error, code = "
			<< ec.value() << ", Message = "
			<< ec.message() << std::endl;
		return;
	}

	//这里,我们知道所有的数据已经传出
}
//asio::async_write()函数是通过对套接字对象的async_write_some()方法进行零次
//或多次调用来实现的,这类似于我们初始样本中的writeToSocket()函数的实现方式
//注意,asio::async_write还有三个重载,提供了额外的功能
//

//异步读取是一种从远程应用程序接收数据的灵活而有效的方法

//用于从Boost.Asio库提供的TCP套接字异步读取数据的最基本工具是
//asio::ip::tcp::socket类的async_read_some()方法
//这是方法的重载之一
/*template<
typename MutableBufferSequence,
typename ReadHandler>
void async_read_some(
const MutableBufferSequence & buffers,
ReadHandler handler);*/
//此方法启动异步读取操作并立即返回,它接受一个表示可变缓冲区的对象
//作为从套接字读取数据的第一个参数,第二个参数是一个回调,当操作完成时由Boost.Asio
//调用,此参数可以是函数指针,仿函数或满足ReadHandle概念要求的其他对象
//回调应具有以下签名:
/*void read_handler(
const boost::system::error_code& ec,
std::size_t bytes_transferred);*/
//这里,ec是一个参数,如果出现错误代码则通知错误代码
//bytes_transferred参数指示在相应的异步操作期间从套接字读取了多少字节
//正如async_read_some()方法的名称所暗示的那样,它启动一个操作,旨在从套接字读取一些数据到缓冲区
//如果没有发生错误,此方法可确保在相应的异步操作期间至少读取一个字节
//这意味着,在一般情况下,为了从套接字读取所有数据,我们可能需要多次执行此异步操作
//现在,我们知道了key方法的工作原理

//以下算法描述了实现应用程序所需步骤,该应用程序异步读取套接字中的数据
//请注意,此算法提供了实现此类应用程序可能方法
//1.定义一个数据结构,其中包含一个指向套接字对象的指针,一个缓冲区,
//一个定义缓冲区大小的变量,心腹一个用作读取字节计数器的变量
//2.定义在异步读取操作完成时将调用的回调函数
//3.在客户端应用程序中,分配并打开活动的TCP套接字,然后将其连接到远程应用程序
//在服务器应用程序中,通过接受连接请求获取连接的活动TCP套接字
//4.分配足够大的缓冲区心使预期消息适合
//5.调用套接字的async_read_some()方法启动异步读取操作,将步骤2中定义的函数
//指定为回调
//6.在asio::io_service类的对象上调用run()方法
//7.在回调中,增加读取的字节数,如果读取的字节数小于要读取的总字节数(预期消息的大小)
//则启动新的异步读取操作以读取下一部分数据

//根据Step 1 定义一个数据结构,其中包含一个指向名为sock的套接字对象的指针,
//一个指向名为buf的缓冲区的指针,一个名为buf_size的变量,其中包含缓冲区的大小,以及一个包含该变量
//的total_bytes_read变量,指示已读取的字节数

struct Session3 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::unique_ptr<char[]> buf;
	std::size_t total_bytes_read;
	unsigned int buf_size;
};
//Step 2 定义一个回调函数
void callback3(const boost::system::error_code &ec,
	std::size_t bytes_transferred,
	std::shared_ptr<Session3> s) {

	if (ec.value() != 0) {
		std::cout << "Error" << std::endl;
		return;
	}

	s->total_bytes_read += bytes_transferred;

	if (s->total_bytes_read == s->buf_size) {
		std::cout << "===Read OK!===" << std::endl
			<< std::string(s->buf.get(), s->buf_size) << std::endl;
		return;
	}

	s->sock->async_read_some(
		asio::buffer(
			s->buf.get() +
			s->total_bytes_read,
			s->buf_size - s->total_bytes_read),
		std::bind(callback3, std::placeholders::_1,
			std::placeholders::_2, s));

}
void readFromSocket(std::shared_ptr<asio::ip::tcp::socket> sock) {
	std::shared_ptr<Session3> s(new Session3);

	//Step 4
	const unsigned int MESSAGE_SIZE = 7;

	s->buf.reset(new char[MESSAGE_SIZE]);
	s->total_bytes_read = 0;
	s->sock = sock;
	s->buf_size = MESSAGE_SIZE;

	//Step 5
	s->sock->async_read_some(
		asio::buffer(
			s->buf.get(), s->buf_size),
		std::bind(callback3, std::placeholders::_1,
			std::placeholders::_2, s));


}
int First_ASYN_TCP_Read() {
	unsigned short port_num = 3333;

	try {
		asio::ip::tcp::endpoint ep(
			asio::ip::address_v4::any(),
			port_num
		);
		asio::io_service ios;
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		acceptor.bind(ep);

		acceptor.listen();

		std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios));

		acceptor.accept(*sock);

		std::cout << "---Accept OK---" << std::endl;

		readFromSocket(sock);

		//只要至少有一个挂起的异步操作,io_service::run()方法就会阻塞
		//当最后一个挂起操作的最后一个回调完成时,此方法返回
		ios.run();
	}
	catch (boost::system::system_error &e) {

		return e.code().value();
	}
	return 0;
}
//测试异步消息传输
int Test_First_TCP_ASYN() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {
		First_ASYN_TCP_Read();
	}
	else {
		First_ASYN_TCP_Write();
	}
	return 0;
}
//尽管async_read_some方法允许从套接字异步读取数据,但基于它的解决方法
//有点复杂且容量出错
//使用asio::async_read()
/*template<
typename AsyncReadStream,
typename MutableBufferSequence,
typename ReadHandler>
void async_read(
AsyncReadStream & s,
const MutableBufferSequence & buffers,
ReadHandler handler);*/
//这个函数会启动从套接字读取数据的操作,直到缓冲区填满
struct Session4 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::unique_ptr<char[]>buf;
	std::size_t buf_size;
};
void callback4(const boost::system::error_code& ec,
	std::size_t bytes_transferred,
	std::shared_ptr<Session4> s) {
	if (ec.value() != 0) {
		return;
	}

	//此时可变缓冲区已经填满了
}
//asio::async_read内部实现也是多次调用了socket的async_read_some方法

//有时,在启动异步操作并且尚未完成之后,应用程序中的条件可能会发生
//变化,以致启动的操作变得无关紧要或过时,并且没有人对完成操作感兴趣
//除此之外,如果启动的异步操作是对用户命令的反应,则用户可以在执行操作时改变主意
//用户可能想要丢弃先前发出的命令,并且可能想要发出不同的命令或决定退出应用程序
//考虑用户在典型的Web浏览器的地址栏中写入网站地址并按Enter的情况
//浏览器立即启动DNS名称解析操作,解析DNS名称并获取相应IP地址后,将启动连接操作
//以向服务器发送请求,最后当发送请求时,浏览器开始等待响应消息,取决于服务器应用程序的
//响应性,通过网络传输的数据量,网络状态和其他因素,所有这些操作可能花费大量时间
//并且用户在等待加载所有请求的网页时可能会改变主意,并且在页面加载之前,用户可以在地址栏中
//写入另一个网站并Enter,另一个极端情况是客户端应用程序向服务器应用程序发送请求
//并开始等待响应消息,但服务器应用程序在处理客户端请求时,出错陷入死锁
//这种情况下,用户必须永远等待

//这两种情况下,客户端应用程序的用户都将能够在完成之前取消他们的启动的操作
//通常,为用户提供取消可能花费大量时间的操作的能力是一种好的做法,由于网络通信操作属于可能持续不可
//预测的长时间的一类操作,因此支持取消通过网络通信的分布式应用程序中的操作非常重要
//Boost.Asio库提供的异步操作的一个好处是它们可以在启动后的任何时刻取消
//
//以下算法提供了使用Boost.Asio启动和取消异步操作所需的步骤
//1.如果应用程序要在XP或2003上运行,请定义这些版本的WIN上启用异步操作取消的标志
//2.分配并打开TCP或UDP套接字,它可以是客户端或服务器应用程序中的主动或被动(acceptor)套接字
//3.为异步操作定义回调函数或函数对象,如果需要,在此回调中,实现一个代码分支,用于处理取消操作时的情况
//4.启动一个或多个异步操作,并将步骤4中定义的函数指定为回调
//5.生成一个额外的线程并使用它来运行Boost.Asio事件循环
//6.在套接字对象上调用cancel()方法以取消与此套接字关联的所有未完成的异步操作
//
//默认情况下为Windows编译时,Boost.Asio使用I/O完成商品框架异步运行操作
//在win xp和 2003上,此框架在取消操作方面存在一些问题和限制
//因此,Boost.Asio要求开发人员针对相关windows版本中的应用程序定位时
//明确通知他们希望启用异步操作取消功能,尽管存在已知问题,为此,必须在包含Boost.Asio头之前
//定义BOOST_ASIO_ENABLE_CANCELIO宏
//否则,如果未定义此宏,则当应用程序包含对异步操作,取消方法和函数的调用时,编译将始终
//失败,也可以通过定义另一个名为BOOST_ASIO_DISABLE_IOCO宏和阻止Boost.Asio使用此框架
//因此,与异步操作取消相关的问题消失了,但是I/O端口框架的的可伸缩性和效率的好处也消失了
//注意,在Windows Vista和2008及更高版本上不存在与异步操作取消相关的上述问题和限制
//因此,在定位这些版本的Win时,取消工作上学,除非有其他原因,否则无需禁用I/O端口框架使用
//要在编译时确定目标操作系统,我们使用Boost.Predef库
//该库为我们提供了宏定义,允许我们识别将代码编译为目标操作系统及其版本,处理器体系结构,编译器等
//的环境参数

//此时需要用到<thread>库
int ASYNC_CANCEL() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;

	try {
		asio::ip::tcp::endpoint ep(
			asio::ip::address::from_string(raw_ip_address),
			port_num
		);

		asio::io_service ios;

		std::shared_ptr<asio::ip::tcp::socket> sock(new asio::ip::tcp::socket(ios, ep.protocol()));

		//在套接字上启动异步连接操作,提供给该方法的回调实现为lambda函数
		//取消异步操作时,将调用回调,并且其指定错误代码的参数包含
		//Boost.Asio中定义的OS相关错误asio::error::operation_aborted
		//然后,我们生成一个名为worker_thread的线程,该线程用于运行Boost.Asio事件循环
		//在此线程的上下文中,库将调用回调函数
		sock->async_connect(ep,
			[sock](const boost::system::system_error& ec) {
			if (ec.code().value() != 0) {
				if (ec.code() == asio::error::operation_aborted) {
					std::cout << "Operation cancelled!";

				}
				else {
					std::cout << "Error, code ="
						<< ec.code() << ", Message ="
						<< ec.what() << std::endl;
					
				}
				return;
			}
			//此处socket已经连接并可以使用
		});

		//开启一个线程,用来当异步操作完成时调用回调函数
		//线程的入口点函数非常简单,它包含一个try-catch块和一个对asio::io_service对象的run()方法
		//的调用
		std::thread worker_thread([&ios]() {
			try {
				ios.run();
			}
			catch (system::system_error &e) {
				std::cout << "Error, code = " << e.code()
					<< ". Message = " << e.what();
			}
		});

		//模拟延迟
		//主线程将进入休眠状态2s
		//这是为了允许连接操作稍微前进并模拟用户在实际应用程序中发出的两个命令之间的延迟
		//由于过快,因此不再使用模拟延迟
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		//Cancelling the initiated operation
		//最后调用套接字对象的cancel()方法来取消启动的连接操作
		//此时,如果操作尚未完成,它将被取消,并且将使用一个参数调用相应回调
		//该参数指定包含asio::error::operation_aborted值的错误代码,
		//以通知操作已被取消,但是如果操作已经完成,则调用cancel()方法无效
		//经测试,即使在新的操作系统上,也仍需要定义#define BOOST_ASIO_ENABLE_CANCELIO
		//如果已经连接成功,则操作无效
		sock->cancel();

		//等待线程完成
		worker_thread.join();

	}
	catch (boost::system::system_error &e) {
		return e.code().value();
	}
	return 0;
}
//
//在上一个示例中,我们考虑了取消与活动TCP套接字关联的异步连接操作
//但是,可以以类似的方式取消与TCP和UDP套接字相关的任何操作
//在启动操作后,就在相应的套接字对象上调用cancel()方法
//除此之外,可以通过调用解析器对象的取消来取消用于异步解析DNS名称的asio::ip::tcp::resolver或
//asio::ip::udp::resolver类的async_resolve()方法
//由Boost提供的相应自由函数启动的所有异步操作,通过对作为第一个参数传递给free函数
//的对象调用cancel()方法,也可以取消Asio,此对象可以表示套接字(主动或被动)或解析程序

//在通过TCP协议进行通信的一些分布式应用程序中,需要传输没有固定大小和
//特定字节序列的消息,标记其边界,这意味着接收方在从套接字读取消息时,无法通过分析消息本身的大小或内容
//来确定消息的结束位置,解决此问题的一种方法是以这样的方式构造每个消息,即它由逻辑头部分和逻辑主体部分组成
//标题部分具有固定的大小,这允许接收方首先读取并解析标头,找出消息体的大小,然后正确
//记取消息的其余部分

//这种方法非常简单并且被广泛使用,但是,它带来一些冗余和额外的计算开销
//这在某些情况下可能是不可接受的.当应用程序为发送给其对等方的每条消息使用单独
//的套接字时,可以应用另一种方法,这是一种非常流行的做法(shut down)
//这种方法的想法是在将消息写入套接字后由消息发送方关闭(shut down)套接字的发送部分
//这会导致向接收方发送特殊服务消息,通知接收方消息已结束,发送方不会使用当前连接发送任何其他消息
//第二种方法提供了比第一种方法更多的好处,并且因为它是TCP协议软件的一部分,所以开发人员可以随时使用它
//套接字上的另一个操作,即可能看起来类似于关闭(closing),但它实际上与它非常不同
//关闭(closing)套接字假定将套接字和与之关联的所有其他资源返回给操作系统,就像内存,进程或线程,文件句柄或互斥锁一样,
//套接字是操作系统的资源,与任何其他资源一样,套接字应在分配,使用后,再返回操作系统,并且应用程序不再
//需要该套接字,否则可能发生资源泄漏,这可能最终资源耗尽以及应用程序的错误或整个操作系统的不稳定
//当套接字未关闭时可能发生的严重问题使得关闭非常重要,关闭(shut down)和关闭(closing)TCP套接字的主要区别在于,建立连接
//关闭(closing)会中断连接,并最终释放并将其返回给操作系统,而关闭(shut down)仅禁用写入,读取两个操作或
//在套接字上的所有操作并向对等应用程序发送服务消息,通知此事实,关闭(shut down)套接字永远不会导致套接字解除分配
// shut down and close a TCP socket

//客户端应用程序
//其目的是分配套接字并将其连接到服务器应用程序,建立连接后,应用程序应准备并发送
//请求消息,通过在向其写入消息后关闭套接字来通知其边界,发送请求后,客户端应用程序应读取响应
//未知的消息边界,因此,应该执行读取,直到服务器关闭(shut down)其套接字以通知响应边界

//定义一个函数,该函数接受对连接到服务器的套接字对象的引用,并使用此套接字与服务器进行通信
void communicate(asio::ip::tcp::socket &sock) {
	const char request_buf[] = { 0x48,0x65,0x0,0x6c,0x6c,0x6f };

	asio::write(sock, asio::buffer(request_buf));

	sock.shutdown(asio::socket_base::shutdown_send);

	asio::streambuf response_buf;

	system::error_code ec;
	asio::read(sock, response_buf, ec);

	if (ec == asio::error::eof) {
		//整个消息已经收到
		std::istream input(&response_buf);
		std::string x;
		std::getline(input, x);
		std::cout << "+++Received+++\n"
			<< x << std::endl;
	}
	else {
		throw system::system_error(ec);
	}
}

int Test_communicate() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try {
		asio::ip::tcp::endpoint ep(
			asio::ip::address::from_string(raw_ip_address),
			port_num
		);
		asio::io_service ios;

		asio::ip::tcp::socket sock(ios, ep.protocol());

		sock.connect(ep);

		communicate(sock);
	}
	catch (boost::system::system_error& ec) {
		std::cout << "Error, code = " << ec.code()
			<< ". Message = " << ec.what();
		return ec.code().value();
	}
	return 0;
}

//服务端
//旨在分配接受器套接字并被支地等待连接请求,当连接请求到达时,它应该接受它并从
//连接到客户端的套接字读取数据,直到客户端应用程序关闭其侧面的套接字,收到请求消息后
//服务器应用程序应通过关闭套接字发送响应消息通知其边界,我们通过指定include和using指令来
//开始客户端应用程序

//定义一个函数,该函数接受对连接到客户端应用程序的套接字对象的引用,并使用此套接字与客户端进行通信 
void processRequest(asio::ip::tcp::socket &sock) {
	asio::streambuf request_buf;

	system::error_code ec;

	asio::read(sock, request_buf, ec);

	std::istream input(&request_buf);

	std::string x;
	std::getline(input, x);
	std::cout << x <<std::endl;

	if (ec != asio::error::eof)
		throw system::system_error(ec);

	const char response_buf[] = { 0x48,0x69,0x21 };

	asio::write(sock, asio::buffer(response_buf));

	sock.shutdown(asio::socket_base::shutdown_send);
}

int Test_processRequest() {
	//首先启动服务器应用程序
	unsigned short port_num = 3333;
	try {
		//分配,打开绑定到端口3333,并开始等待来自客户端的传入连接请求
		//然后启动客户端应用程序,打开并连接到服务器,建立连接后,调用communic()函数
		//客户端应用程序将请求写入套接字,然后调用套接字的shutdown()方法,并将asio::socket_base::shutdown_send常量作为参数传递
		//此调用将关闭套接字的发送部分,此时,禁用写入套接字,
		//并且无法恢复套接字状态以使其再次可写:
		//sock.shutdown(asio::socket_base::shutdown_send);
		//关闭客户端应用程序中的套接字在服务器应用程序中可以看作是到达服务器的
		//协议服务消息,通知对等应用程序已关闭套接字的事实
		//Boost.Asio通过asio::read()函数返回的错误代码将此消息传递给应用程序代码
		//Boost.Asio库将此code定义为asio::error::eof
		//服务器应用程序使用此错误代码来查明客户端何时完成发送请求消息
		//当服务器收到完整请求消息时,服务器和客户端交换其角色
		//当服务器完成将响应消息写入套接字时,它会关闭其套接字的发送部分
		//以暗示整个消息已发送给其对等体,同时客户端应用程序在asio::read()函数
		//中被阻塞,并读取服务器发送的响应,直到函数返回错误代码==asio::error::eof,
		//这意味着服务器已完成发送响应消息,当asio::read()函数返回此错误代码时
		//客户端知道它已读取整个响应消息,然后它可以开始处理它

		//[注意]在客户端关闭其套接字的发送部分后,它仍然可以从套接字读取数据
		//因为套接字的接收部分独立于发送部分保持打开状态
		asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),
			port_num);

		asio::io_service ios;
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		//之前报错listen没有正确的参数,这里是忘记bind了
		acceptor.bind(ep);
		acceptor.listen();
		asio::ip::tcp::socket sock(ios);

		acceptor.accept(sock);

		processRequest(sock);
	}
	catch (system::system_error &e) {
		std::cout << "Error, code = " << e.code()
			<< ". Message = " << e.what();
		return e.code().value();
	}

	return 0;
}
int Test_Shutdown_TCP() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {
		Test_processRequest();
	}
	else {
		Test_communicate();
	}
	return 0;
}
//p106
//为了关闭已分配的套接字,应该在asio::ip::tcp::socket类的相应对象上调用close()方法
//但是,通常不需要显式执行它,因为如果没有显式关闭套接字对象的析构函数,则套接字对象的析构函数会关闭套接字


//==== Chapter 3 ====
//一个典型的tcp同步客户端按照以下算法工作
//1.获取IP地址和服务器应用程序的协议端口号
//2.分配一个活动套接字
//3.与服务器应用程序建立连接
//4.与服务器交换信息
//5.关闭连接
//6.取消分配套接字

//
class SyncTCPClient {
public:
	SyncTCPClient(const std::string & raw_ip_address,
		unsigned short port_num)
		:m_ep(asio::ip::address::from_string(raw_ip_address),port_num),
		m_sock(m_ios){
		m_sock.open(m_ep.protocol());
	}

	void connect() {
		m_sock.connect(m_ep);
	}

	void close() {
		m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		m_sock.close();
	}

	std::string emulateLongComputationOp(
		unsigned int durarion_sec
	) {
		std::string request = "EMULATE_LONG_COMP_OP "
			+ std::to_string(durarion_sec)
			+ "\n";

		sendRequest(request);
		return receiveResponse();
	}


private:
	void sendRequest(const std::string &request) {
		asio::write(m_sock, asio::buffer(request));

	}

	std::string receiveResponse() {
		asio::streambuf buf;
		//这里需要注意:
		//read_until传递给buf的可能包含\n之后的字符,
		//因此,使用\n作为标识便可以用getline直接忽略换行之后的无关字符
		asio::read_until(m_sock, buf, '\n');

		std::istream input(&buf);
		std::string response;
		std::getline(input, response);

		return response;
	}

private:
	asio::io_service m_ios;
	asio::ip::tcp::endpoint m_ep;
	asio::ip::tcp::socket m_sock;
};
int C3_SYN_TCP_CLIENT() {
	const std::string raw_ip_address = "127.0.0.1";
	const unsigned short port_num = 3333;

	try {
		SyncTCPClient client(raw_ip_address, port_num);

		client.connect();

		std::cout << "Sending request to the server..."
			<< std::endl;

		std::string response = client.emulateLongComputationOp(10);

		std::cout << "Response received: " << response << std::endl;

		client.close();
	}
	catch (boost::system::system_error &e) {
		std::cout << "Error!, code = "
			<< e.code() << " Message = " << e.what();
		return e.code().value();
	}
	return 0;
}
//
//典型的UDP同步客户端根据以下算法工作
//1.获取IP地址和客户端应用程序要与之通信的每个服务器的协议端口号
//2.分配UDP套接字
//3.与服务器交换消息
//4.取消分配套接字

class SyncUDPClient {
public:
	SyncUDPClient() :
		m_sock(m_ios) {

		m_sock.open(asio::ip::udp::v4());
	}
	
	std::string emulateLongComputeationOp(
		unsigned int durarion_sec,
		const std::string &raw_ip_address,
		unsigned short port_num
	) {
		std::string request = "EMULATE_LONG_COMP_OP "
			+ std::to_string(durarion_sec)
			+ "\n";

		asio::ip::udp::endpoint ep(
			asio::ip::address::from_string(raw_ip_address),
				port_num
			);
		sendRequest(ep, request);
		return receiveResponse(ep);
	
	}
private:
	void sendRequest(const asio::ip::udp::endpoint &ep,
		const std::string& request) {

		m_sock.send_to(asio::buffer(request), ep);
	}

	std::string receiveResponse(asio::ip::udp::endpoint& ep) {
		char response[6];
		//注意
		//receive_from是一个同步方法,阻止线程,直到数据报从指定的服务器到达
		//如果数据报永远不会到达,该方法永远不会解除阻塞,整个应用程序将挂起
		//如果从服务器到达的数据报大小大于提供的缓冲区的大小,则该方法将失败
		std::size_t bytes_received =
			m_sock.receive_from(asio::buffer(response), ep);

		m_sock.shutdown(asio::ip::udp::socket::shutdown_both);
		return std::string(response, bytes_received);
	}
	

private:
	asio::io_service m_ios;
	asio::ip::udp::socket m_sock;
};
int C3_SYN_UDP_CLIENT() {
	const std::string server1_raw_ip_address = "127.0.0.1";
	const unsigned short server1_port_num = 3333;

	const std::string server2_raw_ip_address = "192.168.1.10";
	const unsigned short server2_port_num = 3334;

	try {
		SyncUDPClient client;

		std::cout << "Sending request to the server #1 ..."
			<< std::endl;
		std::string response = client.emulateLongComputeationOp(10,
			server1_raw_ip_address, server1_port_num);

		std::cout << "Response from the server #1 received: "
			<< response << std::endl;

		std::cout << "Sending request to the server #2 ..." << std::endl;

		response = client.emulateLongComputeationOp(10,
			server2_raw_ip_address, server2_port_num);

		std::cout << "Response from the server #2 received: " << response
			<< std::endl;
	}
	catch (boost::system::system_error &e) {
		std::cout << "Error!, code = "
			<< e.code() << " Message = " << e.what();
		return e.code().value();
	}
	return 0;
}

//异步TCP客户端
//定义回调函数类型
typedef void(*Callback)(unsigned int resquest_id,
	const std::string &response,
	const system::error_code &ec);
//定义一个数据结构,目的是在执行时保持与请求相关的数据,命名为Session
struct Session {
	Session(asio::io_service& ios,
		const std::string &raw_ip_address,
		unsigned short port_num,
		const std::string& request,
		unsigned int id,
		Callback callback) :

		m_sock(ios),
		m_ep(asio::ip::address::from_string(raw_ip_address), port_num),
		m_request(request),
		m_id(id),
		m_callback(callback),
		m_was_cancelled(false)
	{

	}

	asio::ip::tcp::socket m_sock;
	asio::ip::tcp::endpoint m_ep;
	std::string m_request;

	asio::streambuf m_response_buf;
	std::string m_response;

	system::error_code m_ec;

	unsigned int m_id;

	Callback m_callback;
	//
	//这个属性被设置的原因:
	//当一个异步操作已经完成并且尚未启动下一个异步操作时,有可能会调用cancelRequest()
	//方法,例如,假设I/O线程现在正在运行与特定套接字关联的async_connect操作的回调
	//此时,没有与此套接字关联的异步操作正在进行中,因为尚未启动下一个异步操作async_write()
	//因此,在此套接字上调用cancel()将不起作用,故设置标记
	bool m_was_cancelled;
	//通过Mutex保护可以确保
	//请求可以在m_was_cancelled被测试之前或在下一次异步操作被初始化之后
	std::mutex m_cancel_guard;
};
//接下来定义一个提供异步通信功能的类
class AsyncTCPClient :public boost::noncopyable {
public:
	AsyncTCPClient() {
		m_work.reset(new boost::asio::io_service::work(m_ios));

		m_thread.reset(new std::thread([this]() {
			m_ios.run();
		}));
	}

	void emulateLongComputationOp(
		unsigned int durarion_sec,
		const std::string &raw_ip_address,
		unsigned short port_num,
		Callback callback,
		unsigned int request_id
	) {
		std::string request = "EMULATE_LONG_CALC_OP " +
			std::to_string(durarion_sec)
			+ "\n";

		std::shared_ptr<Session> session = std::shared_ptr<Session>(new Session(m_ios, raw_ip_address,
			port_num, request, request_id, callback));

		session->m_sock.open(session->m_ep.protocol());

		std::unique_lock<std::mutex>
			lock(m_active_sessions_guard);

		m_active_sessions[request_id] = session;
		lock.unlock();

		session->m_sock.async_connect(session->m_ep,
			[this, session](const system::error_code &ec)
		{
			if (ec.value() != 0) {
				session->m_ec = ec;
				onRequestComplete(session);
				return;
			}

			std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);

			if (session->m_was_cancelled) {
				onRequestComplete(session);
				return;
			}

			asio::async_write(session->m_sock,
				asio::buffer(session->m_request),
				[this, session](const boost::system::error_code &ec,
					std::size_t bytes_transferred) {
				if (ec.value() != 0) {
					session->m_ec = ec;
					onRequestComplete(session);
					return;
				}
			

			std::unique_lock<std::mutex>
				cancel_lock(session->m_cancel_guard);

			if (session->m_was_cancelled) {
				onRequestComplete(session);
				return;
			}


			asio::async_read_until(session->m_sock,
				session->m_response_buf,'\n',
				[this, session](const boost::system::error_code& ec,
					std::size_t bytes_transferred) {
				if (ec.value() != 0) {
					session->m_ec = ec;
				}
				else {
					std::istream strm(&session->m_response_buf);
					std::getline(strm, session->m_response);
				}

				onRequestComplete(session);
			}); });


		});


	}

	void cancelRequest(unsigned int request_id) {
		std::unique_lock<std::mutex>
			lock(m_active_sessions_guard);

		auto it = m_active_sessions.find(request_id);
		if (it != m_active_sessions.end()) {
			std::unique_lock<std::mutex>
				cancel_lock(it->second->m_cancel_guard);
			it->second->m_was_cancelled = true;
			it->second->m_sock.cancel();
		}
	}

	void close(){
		m_work.reset(NULL);

		m_thread->join();
	}
private:
	void onRequestComplete(std::shared_ptr<Session> session) {
		boost::system::error_code ignord_ec;
		session->m_sock.shutdown(
			asio::ip::tcp::socket::shutdown_both,
			ignord_ec
		);

		std::unique_lock<std::mutex>
			lock(m_active_sessions_guard);

		auto it = m_active_sessions.find(session->m_id);
		if (it != m_active_sessions.end())
			m_active_sessions.erase(it);

		lock.unlock();

		boost::system::error_code ec;

		if (session->m_ec.value() == 0 && session->m_was_cancelled)
			ec = asio::error::operation_aborted;
		else
			ec = session->m_ec;

		session->m_callback(session->m_id,
			session->m_response,ec);
	}
private:
	asio::io_service m_ios;
	std::map<int, std::shared_ptr<Session>> m_active_sessions;
	std::mutex m_active_sessions_guard;
	//asio::io_service::work类的对象使运行事件循环的线程不再退出此循环
	std::unique_ptr<boost::asio::io_service::work> m_work;
	std::unique_ptr<std::thread> m_thread;
};
//现在,定义一个将作为回调函数,将其传递给AsyncTCPClient::enmulateLongComputationOp()方法
void handler(unsigned int request_id,
	const std::string &response,
	const system::error_code& ec) {
	if (ec.value() == 0) {
		std::cout << "Request #" << request_id
			<< " has completed. Response: "
			<< response << std::endl;
	}
	else if (ec == asio::error::operation_aborted) {
		std::cout << "Request #" << request_id
			<< " has been cancelled by the user."
			<< std::endl;
	}
	else {
		std::cout << "Request #" << request_id
			<< " failed! Error code = " << ec.value()
			<< ". Error message = " << ec.message()
			<< std::endl;
	}
	return;
}
int C3_ASYN_TCP_CLIENT() {
	try {
		AsyncTCPClient client;

		client.emulateLongComputationOp(10, "127.0.0.1", 3333,
			handler,1);

		std::this_thread::sleep_for(std::chrono::seconds(5));

		client.emulateLongComputationOp(11, "127.0.0.1", 3334, handler, 2);

		client.cancelRequest(1);

		std::this_thread::sleep_for(std::chrono::seconds(6));

		client.emulateLongComputationOp(12, "127.0.0.1", 3335, handler, 3);

		std::this_thread::sleep_for(std::chrono::seconds(15));

		client.close();
	}
	catch (system::system_error &e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}
//p129

//运行比计算机中安装的核心或处理器数量更多的线程的应用程序
//可能由于线程切换开销的影响而使应用程序变慢。

//要实现多线程,需要替换表示单个I/O线程的AsyncTCPClient类的m_thread成员,以及
//指向std::thread对象的指针列表,它将表示I/O线程的集合
//std::list<std::unique_ptr<std::thread>> m_threads;
//接下来,更改类的构造函数,以便它接受一个表示要创建的线程数
//此外,构造函数应该生成指定数量的I/O线程,并将它们全部添加到运行
//事件循环的线程池中p140

//就像单线程版本一样,每个线程都调用m_ios对象的run()方法,因此,所有线程都
//将添加到线程池中,由m_ios对象控制,池中所有线程将用于调用相应异步操作完成回调
//这意味着在多核或多处理器计算机上,可以在不同的线程中同时运行多个回调
//每个回调都在单独的处理器上,最后是close()函数遍历线程池每个都执行join


//====CH.3====
//实现一个服务器端应用程序
//
//根据特定服务器提供的服务,请求处理可能意味着不同的事情
//例如:HTTP服务器通常会读取请求消息中指定的文件的内容,并将其发送回客户端
//代理服务器只是将客户端的请求重定向到不同的服务器以进行实际处理(或者可能
//是另一轮重定向)其他更具体的服务器可以提供对客户端在请求中提供的数据执行复杂
//计算的服务,并将这些计算的结果返回给客户端,并非所有服务器都会扮演被动角色
//某些服务器应用程序可能会向客户端发送消息,而无需等待客户端首先发送请求
//通常,此类服务器充当通知程序,并通知客户端一些有趣的事件,在这种情况下,客户端
//可能根本不需要向服务器发送任何数据,相反,他们被动等待来自服务器通知并且已经收到通知,
//他们会做出相应的反应,这种通信模型称为推正通信,该模型在现代web应用程序中越来越流行
//提供了额外的灵活性,因此,对服务器应用程序进行分类的第一种方法是通过它们执行的功能
//或它们为客户提供的服务,另一个明显的分类维度是传输层协议
//TCP当今非常流行,许多通用服务器应用程序使用它进行通信,其他更具体的
//服务器可能使用UDP协议,同时通过TCP和UDP协议提供服务的混合服务器应用程序
//属于多协议服务器
//服务器的另一个特征是它为客户提供服务的方式。
//迭代服务器以一对一的方式为客户端提供服务，这意味着它在完成为当前服务的客户端提供服务之前不会开始为下一个客户端提供服务
//并行服务器可以并行服务多个客户端
//在单处理器计算机上，并行服务器与在单个处理器上运行它们的多个客户端交错不同的通信阶段
//例如，连接到一个客户端并在等待来自它的请求消息时，服务器可以切换到连接第二个客户端，或者从第三个客户端读取请求
//在此之后，它可以切换回第一个客户端继续提供它。
//这种并行性称为伪并行性，因为处理器仅在几个客户端之间切换，但不能真正同时为它们服务，这对于单个处理器是不可能的。
//在多处理器计算机上，当服务器为每个客户端使用不同的硬件线程同时为多个客户端提供服务时，真正的并行性是可能的。

//迭代服务器实现起来相对简单,并且可以在请求速率足够低时使用,以便服务器
//有时间在下一个请求到达之前完成处理一个请求,很明显,迭代服务器不可扩展
//向运行此类服务器的计算机添加更多处理器不会增加服务器的吞吐量
//另一方面,并行服务器可以处理更高的请求率,如果实施得当,它们是可扩展的
//在多处理器计算机上运行的真正并行服务器可以处理比在单处理器计算机上运行的同一服务器
//更高的请求率,从实现的角度来看,对服务器应用程序进行分类的另一种方法是根据服务器是同步还是
//异步,同步服务器使用同步套接字API调用来阻止执行线程,直到请求的操作完成,否则会发生错误
//因此典型的同步TCP服务器将使用asio::ip::tcp::acceptor::accept()从客户端接收请求消息,然后asio::
//ip::tcp::socket::read_some()从客户端接收请求消息,然后asio::ip::tcp::socket::write_some()
//将响应消息发送回客户端,
//这三种方法都是封锁的,它们阻止执行的线程,直到请求的操作完成,或发生错误,这使得服务器使用这些操作同步

//与同步服务器应用程序相反,异步服务器应用程序使用异步套接字API调用,例如
//异步TCP服务器可以使用asio::ip::tcp::acceptor::accept()方法异步接受客户端连接请求
//asio::ip::tcp::socket::async_read_some()方法或asio::async_read()自由函数从客户端异步接收请求消息
//然后asio::ip::tcp::socket::async_write_some()方法或asio::async_write()自由函数
//异步发送响应消息回客户端

//由于同步服务器应用程序的结构与异步服务器应用程序的结构明显不同,因此应当在服务器应用程序设计阶段尽早
//决定应用哪种方法,此决应基于对应用程序要求的仔细分析,此外,应考虑并考虑可能出现的应用
//演进路径和新要求,通常每种方法都有优缺点,当同步方法在一种情况下产生更好的结果时,在另一种情况下它可能是
//绝对不可接受的,在这种情况下,异步方法可能是正确的选择

//两者对比:
//与异步方法相比,同步方法的主要优点是其简单性,与功能相同的异步服务器相比
//同步服务器更容易实现,调试和支持,异步服务器更复杂,因为它们使用异步操作在代码中的其他位置完成而还是它们被
//启动,通常,这需要在空闲内存中分配额外的数据结构以保持请求的上下文,实现回调函数函数,线程同步以及可能使应用程序
//结构非常复杂且容易出错的其他附加内容,

//同步服务器中不需要大多数这些额外功能,此外异步方法会带来额外的计算和内存开销
//这可能使其在某些情况下比同步方法效率低,然而,同步方法具有一些功能限制,这通常使其不可接受,这些限制包括无法在启动后
//取消同步操作,或为其分配超时,以便在运行时间过长时中断
//与同步操作相反,可以在启动操作后的任何时间取消异步操作,同步操作无法取消的事实明显限制了同步服务器应用的范围
//使用同步操作的公共服务器很容易受到攻击

//使用异步操作的服务器比同步服务器更高效和可扩展,尤其是当它们在具有本机支持异步网络I/O的操作系统的多处理
//器计算机上运行时

//同步迭代TCP服务器根据以下算法:
//1.分配一个接受器套接字并将其绑定到一个特定的TCP端口上
//2.运行循环直到服务器停止
////1.等待来自客户端的连接请求
////2.到达时接受客户端的连接请求
////3.等待来自客户端的请求消息
////4.阅读请求消息
////5.处理请求
////6.将响应消息发送给客户端
////7.关闭与客户端的连接并取消分配套接字

//p147
int main() {

	//====CH.1====
	//if (Resolving_DNS_TCP() != 0) {
	//	return 1;
	//}
	//if (Resolving_DNS_UDP() != 0) {
	//	return 1;
	//}
	//Create_Server_Acceptor_TCP();
	//Create_Server_Acceptor_UDP();
	//Create_Client_Connect_TCP();
	//Create_Client_Connect_DNS_TCP();
	//Create_Server_TCP_SYN();

	//====CH.2====
	//Create_Const_Buffer();
	//Create_Mutable_Buffer();
	//Test_Buffer_IO();
	//Test_First_TCP();
	//Test_First_TCP_ASYN();
	//ASYNC_CANCEL();
	//Test_Shutdown_TCP();
	

	//====CH.3====
	//C3_SYN_TCP_CLIENT();
	//C3_SYN_UDP_CLIENT();
	//C3_ASYN_TCP_CLIENT();


	//====CH.4====

	std::system("pause");
}