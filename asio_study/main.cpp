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
//p68
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
	std::system("pause");
}