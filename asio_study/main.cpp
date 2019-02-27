#include<iostream>
#include<boost/asio.hpp>
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
	unsigned short port_num = 5555;
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
int main() {
	//if (Resolving_DNS_TCP() != 0) {
	//	return 1;
	//}
	//if (Resolving_DNS_UDP() != 0) {
	//	return 1;
	//}
	//Create_Server_Acceptor_TCP();
	//Create_Server_Acceptor_UDP();
	//Create_Client_Connect_TCP();
	Create_Client_Connect_DNS_TCP();
	std::system("pause");
}