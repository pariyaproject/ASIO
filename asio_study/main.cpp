#include<iostream>
#define BOOST_ASIO_ENABLE_CANCELIO
#include<boost/asio.hpp>
#include<thread>
#include<mutex>
#include<memory>
#include <boost/filesystem.hpp>
//#include <boost/asio/ssl.hpp>
using namespace boost;
//basic
//�ڴ����˵㣨endpoint��֮ǰ���ͻ���Ӧ�ó�������ȡԭʼIP��ַ��ָ������֮ͨ�ŵķ�������Э��˿ں�
//��һ���棬������Ӧ�ó���ͨ����������IP��ַ�ϵĴ�����Ϣ��ֻ��Ҫ��ȡҪ�����Ķ˿ں�
//ԭ�򣺷����������ж��IP��ַ��֮�������ӣ�������Ҫ�������е�IP��ַ
//ע��:�˵���(��������IP,�������з������������˿�)
//���ڽ�����(acceptor)��,�󶨵�endpoint��(�������õ�����IP��ַ,������������Ҫ�����Ķ˿ں�)


//===1.1===
//����һ��endpoint
//�ͻ��˷��棺
//1.��ȡ������Ӧ�ó����IP��ַ�Ͷ˿ںţ�Ӧ��IP��ַָ��Ϊ���ʮ���ƣ�IPV4����ʮ�����ƣ�IPV6����ʾ���е��ַ���
//2.��ԭʼ��IP��ַ��ʾΪasio::ip::address�����
//3.�Ӳ���2�д�����address����Ͷ˿ں�ʵ����asio::ip::tcp::endpoint�����

//===1.2===
//������������endpoint
//1.��ȡ�˿ںŷ��������������������
//2.����asio::ip::address���������ʵ������ʾ���з������������Ͽ��õ�����IP��ַ
//3.�Ӳ���2�д����ĵ�ַ����Ͷ˿ں�ʵ����asio::ip::tcp::endpoint��Ķ���

//��ȡԭʼIP��ַ֮�󣬿ͻ���Ӧ�ó���������Boost.Asio���ͱ�ʾ��
//Boost.Asio�ṩ�������ڱ�ʾIP��ַ����
//asio::ip::address_v4:����һ��IPV4��ַ
//asio::ip::address_v6:����һ��IPV6��ַ
//asio::ip::address:����֪�࣬���Ա�ʾV4��V6

//��ʾ���У�ʹ��asio::ip::address��ʹ�ÿͻ���Ӧ�ó�����IPЭ��汾��v4 or v6���޹�
//��ζ�ſ���͸������IPV4��IPV6������һ��ʹ��
//ʹ��asio::ip::address::from_string()�����ܱ�ʾΪ�ַ�����ԭʼIP��ַ����������֤�ַ����������Ҫһ��errorcode��,ʵ����asio::ip::address����󣬲����䷵��
//�˷������ĸ����أ�ʹ�õ������
//static asio::ip::address from_string(
//const std::string & str,
//boost::system::error_code & ec);
//�˷����ǳ����ã���Ϊ�������Ϊ�������ݸ������ַ����Ƿ������Ч��IPv4��IPv6��ַ
//����ǣ���ʵ������Ӧ�Ķ��������Ч��ͨ���ڶ�������ָ������
//����ζ�Ŵ˹��ܿ�������֤ԭʼ�û�����

//ep�����������Boost.Asioͨ����ع�����ָ��������Ӧ�ó���
//�ڶ������ṩӦ����������Ϣ�Ķ˿ںţ�δ�ṩIP����Ϊ������Ӧ�ó���ͨ��ϣ�����������Ͽ��õ�����IP��ַ�ϵĴ�����Ϣ
//Ϊ�˱�ʾ�����Ͽ��õ�����IP��ַ�ĸ����asio::ip::address_4��asio::ip::address_v6�ṩ��̬����any()
//��ʵ������ʾ�������Ӧ����������
//ע��::��IPЭ��汾�޹ص���asio::ip::address���ṩany()����
//ʹ��v4 v6����ȷָ�����Ƿ���IPv4����IPv6��ַ�Ͻ�������

//֮ǰ��������ʹ�õ���asio::ip::tcp�෶Χ�������Ķ˵���
/*class tcp
{
public:
 /// The type of a TCP endpoint.
 typedef basic_endpoint<tcp> endpoint;
 //...
}*/
//����ζ�Ŵ˶˵�����basic_endpoint<>ģ������ػ�,ּ������ͨ��TCP����ͨ�ŵĿͻ��˺ͷ�����

//Berkeley�׽���API�������е�TCP UDPЭ���API
//����Χ��socket�ĸ�����Ƶ�,һ����ʾͨ�ŻỰ�����ĵĳ������
//�����ǿ���ִ���κ�����I/O����֮ǰ,���Ǳ�������һ���׽��ֶ���,Ȼ��ÿ��I/O�������������

//Boost.Asio������Berkeley SocketsAPI��������,��������
//������һ����ʾ�׽��ָ������,��������Berkeley Socket API�еĽӿڷ���

//���������������͵��׽���,ּ��������Զ��Ӧ�ó��������ݻ��Զ��Ӧ�ó���������ݻ�����
//�������ӽ������̵��׽��ֳ�Ϊ��׽���,�������׽������ڱ����ȴ�����Զ��Ӧ�ó���Ĵ�����������
//��Դ�׽��ֲ������û����ݴ���

//�����㷨�������ڿͻ���Ӧ�ó�����ִ���Դ����ʹ򿪻�׽�������Ĳ���
//1.����asio::io_service���ʵ����ʹ��֮ǰ������ʵ��
//2.������ʾ�����Э��(TCP/UDP)����Ķ����Լ��׽���Ҫ��֮ͨ�ŵĻ���IPЭ��(IPv4/IPv6)�İ汾
//3.������ʾ������Э�����Ͷ�Ӧ���׽��ֵĶ���,��asio::io_service��Ķ��󴫵ݸ��׽��ֵĹ��캯��
//4.�����׽��ֵ�open()����,����ʾ�ڲ���2�д�����Э��Ķ�����Ϊ��������
int Create_TCP_Socket_client_1() {
	//step 1 
	//ʵ����io_service,����Boost.Asio�ĺ������,�ṩ�Եײ����ϵͳ������I/O
	//����ķ���,Boost.Asio�׽��ֿ���ͨ������Ķ��������Щ����
	asio::io_service ios;

	//step 2
	//�����ʾTCPЭ��,�����ṩ�κι���,������һ������һ������Э���ֵ�����ݽṹ
	//asio::ip::tcp��û�й������캯��,�෴���ṩ������̬����
	//v4(),v6(),���Ƿ���asio::ip::tcp��Ķ���,��ʾTCPЭ����ײ�IPv4��IPv6Э�����Ӧ
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	//����,����໹����һЩּ����TCPЭ��һ��ʹ�õĻ������͵�����,
	//���а���asio::ip::tcp::endpoint asio::ip::tcp::socket asio::ip::tcp::acceptor��
	//��boost/asio/ip/tcp.hpp��
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
	//����һ��asio::ip::tcp::socket��Ķ���io_service��Ķ�����Ϊ�������ݸ����Ĺ��캯��
	//ע��:�˹��캯��������ײ����ϵͳ���׽��ֶ���
	//������open��������ָ��Э��Ķ�����Ϊ�������ݸ���ʱ,��step4�з���ʵ�ʲ���ϵͳ���׽���
	//��boost.asio��,��(open)�׽�����ζ�Ž����������׽���Ҫͨ�ŵ��ض�Э������������������
	//��Boost.Asio�׽��ֶ����ṩ��Щ����ʱ,�����㹻����Ϣ������ײ����ϵͳ����ʵ�׽��ֶ���
	//asio::ip::tcp::socket���ṩ����һ������Э�������Ϊ�����Ĺ��캯��,�˹��캯������һ���׽��ֲ������(open)
	//ע��:���ʧ��,�˹��캯�����׳�boost::system::system_error���͵��쳣
	asio::ip::tcp::socket sock(ios);


	//���ڴ洢open socket�Ĵ���
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
	//���������·����滻3-4
	try {
		//���������׽���
		asio::ip::tcp::socket sock2(ios, protocol);

	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message: " << e.what();
	}
}
//�����Ǵ���һ��UDP������
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
//���������׽���passive socket/acceptor
//����һ���׽���,���ڵȴ�����ͨ��TCPЭ�����ͨ�ŵ�Զ��Ӧ�ó�������ӽ�������
//�˶�����������Ҫ����
//1.�����׽��ֽ����ڷ�����Ӧ�ó�������ͬʱ���ݿͻ��˺ͷ�������ɫ���Ӧ�ó���
//2.��ΪTCPЭ�鶨�屻���׽���,����UDPЭ�鲢����ζ�Ž�������,���ͨ��UDPִ��ͨ��ʱ����Ҫ�����׽���

//��Boost.Asio��,�����׽�����asio::ip::tcp::acceptor���ʾ
//������ƽ�����Ķ���Ĺؼ����ܼ����ͽ��ܻ��������������
//1.����asio::io_service���ʵ����ʹ��֮ǰ������ʵ��
//2.����asio::ip::tcp��Ķ���,�ö����ʾTCPЭ��ͻ���IPЭ��(IPv4/IPv6)������汾
//3.������ʾ�������׽��ֵ�asio::ip::tcp::acceptor��Ķ���,��asio::io_service��Ķ��󴫵ݸ��乹�캯��
//4.����acceptor socket��open()����,����ʾ��2�д�����Э��Ķ�����Ϊ��������

int Create_TCP_Acceptor_Server() {
	//step 1
	//������Ҫ���ʵײ����ϵͳ�����Boost.Asio�������Ҫ����
	asio::io_service ios;

	//step 2
	//����һ����ʾTCPЭ��Ķ���
	asio::ip::tcp protocol = asio::ip::tcp::v4();

	//step 3
	//�����ڻ�׽�����һ��,�˹��캯��ʵ������Boost.Asio�Ķ���asio::ip::tcp::acceptor��,�鲻����ײ����ϵͳ��ʵ���׽��ֶ���
	asio::ip::tcp::acceptor ac(ios);

	boost::system::error_code ec;

	//step 4
	//�ۼ�ϵͳ�׽��ֶ����ڲ���4�з���,����open()������Э�������Ϊ�������ݸ���
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

//����һ��DNS����
//ԭʼIP��ַ���������Ǹ�֪�ͼ���
//Ϊ��ʹ���Ѻõ�����Ʊ�������е��豸,����������ϵͳ(DNS)
//DNS��һ�ֲַ�ʽ����ϵͳ�������Ի������������������е��豸�����
//ȷ�е�˵,DNS������һ������IP��ַ�ı���,�������豸�ı���
//��û�������ض��������豸,���ǿ��Է�����豸��IP��ַ
//DNS�䵱�ֲ�ʽ���ݿ�,�洢DNS���Ƶ���ӦIP��ַ��ӳ�䵽��IP��ַ,��DNS����ת��Ϊ��ӦIP��ַ�Ĺ��̳�Ϊ
//DNS��������,�ִ��������ϵͳ�������Բ�ѯDNS�Խ���DNS���ƵĹ���,���ṩ����Ӧ�ó�������ִ��DNS���ƽ����Ľӿ�

//����DNS�����Ի�����пͻ���Ӧ�ó���Ҫ��֮ͨ�ŵķ�����Ӧ�ó��������(0������)��IP��ַ(0 or 0+)����Ĳ���
//1.��ȡָ��������Ӧ�ó����DNS���ƺ�Э��˿ں�,�������Ǳ�ʾΪ�ַ���
//2.����asio::io_service���ʵ����ʹ��֮ǰ������ʵ��
//3.������ʾDNS���Ƹ����ѯ�Ľ�������::��ѯ�����
//4.�����ͺϱ�ҪЭ���DNS���ƽ����������ʵ��
//5.���ý�������resolve()����,������3�д����Ĳ�ѯ������Ϊ�������ݸ���
//Resolve DNS
int Resolving_DNS_TCP() {
	//step 1
	//���Ȼ�ȡDNS���ƺ�Э��˿ں�,�������Ǳ�ʾΪ�ַ���
	std::string host = "ipv6.google.com";
	std::string port_num = "80";

	//step 2
	//����io_service���ʵ��,��������DNS���ƽ���������ʹ�ø�ʵ�������ʵײ�OS�ķ���
	asio::io_service ios;

	//step 3
	//����һ��asio::ip::tcp::resolver::query��Ķ���
	//�˶����ʾ��DNS���ƵĲ�ѯ,����Ҫ������DNS����,��DNS���ƽ��������ڹ���˵����(endpoint)�Ķ˿ں��Լ����ƽ������̵�ĳЩ�ض������һ���־,��ʾΪλͼ
	//��Ϊ����ָ��ΪЭ��˿ں�(80),�����Ƿ�������(HTTP,FTP),��˴�����asio::ip::tcp::resolver::query::nummeric_service,��ʾ֪ͨ��ѯ�����Ա���ȷ�����˿ں�ֵ
	asio::ip::tcp::resolver::query resolver_query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);

	//step 4
	//����һ��asio::ip::tcp::resolver���ʵ��,�����ṩDNS���ƽ�������,����Ҫ�ײ����ϵͳ�ķ���,���ͨ��asio::io_services��Ķ�����Ϊ�������ݸ����Ĺ��캯������������
	asio::ip::tcp::resolver resolver(ios);

	//���ڴ洢���������еĴ���
	boost::system::error_code ec;

	//step 5
	//DNS���ƽ����ڽ����������resolve()����,�������ʧ��,��һ���������������Ϣ,���򷵻�asio::ip::tcp""resolver::iterator��Ķ���
	//������һ��������,ָ���ʾ��������ļ��ϵĵ�һ��Ԫ��
	//�ü����еĶ����������������IP��ַ����һ����,ÿ������Ԫ�ض�����asio::ip::tcp::endpoint��Ķ���,�ö����Ǵӽ������̲�����һ��IP��ַʵ������
	//�Լ�����Ӧ��ѯ�����ṩ�Ķ˿ں�(Step 1�е�)����ͨ��asio::ip::basic_resolver_entry<tcp>::endpoint()getter�������ʶ˵����
	asio::ip::tcp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);
	//ע��:��ò�Ҫʹ������ֱ�Ӹ�ֵ��,��Ϊ������һ�������DNSû���κν��
	//asio::ip::tcp::endpoint ep = *it;
	//asio::ip::tcp::endpoint ep = it->endpoint();
	//asio::ip::tcp::resolver::iterator���Ĭ�Ϲ�������ʾ����������
	asio::ip::tcp::resolver::iterator it_end;
	for (; it != it_end; ++it)
		std::cout << it->endpoint().address() << std::endl;
	//�������
	if (ec.value() != 0)
	{
		//Failed
		std::cout
			<< "Failed, Error code = "
			<< ec.value() << ", Message: " << ec.message();
		return ec.value();
	}
	return 0;
	//ͨ��,�����з�����Ӧ�ó����������DNS���Ʊ�����Ϊ���IP��ַʱ������Ӧ������Ϊ����˵�ʱ
	//�ͻ��˳�����Գ��������ÿ���˵�ͨ��,ֱ���յ��������Ӧ
	//��ע��:��DNS����ӳ�䵽���IP��ַ��������һЩ��IPv4��������IPv6��ַʱ,DNS���ƿ��Խ���ΪIPv4��ַ��IPv6��ַ
	//Ҳ���Խ���Ϊ����,����ݵļ��Ͽ��ܰ�����ʾIPv4��IPv6�Ķ˵�

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

	//�������
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
//socket��endpoint�İ�
//���׽������ض��˵�������Ĺ��̽а�(binding)
//���׽��ְ󶨵��˵�ʱ,���дӸ�����˿���ΪĿ���ַ�����������������ݰ���������ϵͳ��д�򵽸��ض��׽���
//ͬ��,�Ӱ󶨵��ض��˵���׽��ַ������������ݽ�ͨ����ö˵���ָ������ӦIP��ַ�����������ӿڴ��������������
//ĳЩ������ʽ��δ�󶨵Ļ�׽���,����,��δ�󶨵Ļ�׽������ӵ�Զ��Ӧ�ó���Ĳ����Ὣ������󶨵��ײ����ϵͳѡ���IP��ַ��Э��˿ں�
//ͨ��,�ͻ���Ӧ�ó�����Ҫ����׽�����ʽ�󶨵��ض��˵�(endpoint),��Ϊ������Ҫ���ض��˵������ͨ��
//��ֻ��Ҫ�κζ˵����ڴ�Ŀ��,�����ͨ��ί��ѡ���׽���Ӧ�󶨵�����ϵͳ��IP��ַ�Ͷ˿ںŵ�Ȩ��
//������ĳЩ���������,�ͻ���Ӧ�ó��������Ҫʹ���ض���IP��ַ��Э��˿ں���Զ��Ӧ�ó������ͨ��,��˽����׽�����ʽ�󶨵����ض��˵�(endpoint)
//��ʹ�����ϴ��ڵ�������ӿں͵���IP��ַ,ÿ��ִ����ʽ��ʱ,�׽��ֿ��ܰ󶨵���ͬ��Э��˿ں�.
//��ͨ�����������׽��ֽ���Զ��Ӧ�ó���ͨ�ŵ�IP��ַ��Э��˿ںŵĿͻ��˳���ͬ,������Ӧ�ó���ͨ����Ҫ��ȷ�ؽ���������׽��ְ󶨵��ض��˵�

//�����㷨�����˴����������׽��ֲ�����󶨵��˵�����Ĳ���,�ö˵�ָ�������Ͽ��õ�����IP��ַ�Լ�IPv4 TCP������Ӧ�ó����е��ض�Э��˿ں�:
//1.��ȡ�������ϵ�Ӧ�ü����������������Э��˿ں�
//2.����һ���˵�,�ö˵��ʾ�����Ͽ��õ�����IP��ַ�Լ�����1�л�ȡ��Э��˿ں�
//3.�������򿪽������׽���
//4.����acceptor�׽��ֵ�bind()����,���˵������Ϊ�������ݸ���
int Create_Server_Acceptor_TCP() {
	//Step 1
	//���Ȼ�ȡЭ��˿ں�
	unsigned short port_num = 3333;

	//Step 2
	//ע��:endpoint����ĵڶ��������Ǳ������ŵĶ˵�(ϣ���Է����ӵı����Ķ˿ں�)
	//����һ���˵��ʾ�����Ͽ��õ�����IP��ַ��ָ���Ķ˿ں�
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	//Step 3
	//�����������������(acceptor)
	//ʵ�������򿪽������׽���,�ڲ���2�д����Ķ˵�����йش���Э��ͻ���IPЭ��(IPv4)�汾����Ϣ
	//���,���ǲ���Ҫ������һ����ʾЭ��Ķ��������䴫�ݸ�acceptor�׽��ֵĹ��캯��
	//�෴,����ʹ�ö˵��protocol()����,�÷������ر�ʾ��Ӧ��asio::ip::tcp��Ķ���
	try
	{
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());

		//���ڴ洢��ʱ���ܷ����Ĵ������Ϣ
		boost::system::error_code ec;

		//Step 4
		//����acceptor�׽��ֵ�bind()����,����һ������,�ö����ʾӦ�ý�acceptor�׽��ְ󶨵��Ķ˵���Ϊ�����Ĳ���
		//������óɹ�,��acceptor�׽��ְ󶨵���Ӧ�Ķ˵�,��׼����ʼ�����ö˵��ϵĴ�����������
		acceptor.bind(ep, ec);

		//������
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
//UDP�汾��Acceptor
//��ΪUDP����������,���ʹ�û�׽��ֵȴ���������
int Create_Server_Acceptor_UDP() {
	unsigned short port_num = 3333;

	asio::ip::udp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	try
	{
		asio::ip::udp::socket socket(ios, ep.protocol());

		boost::system::error_code ec;

		socket.bind(ep, ec);

		//������
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

//��TCP�׽��ֿ�������Զ��Ӧ�ó���ͨ��֮ǰ,���������佨���߼�����
//����TCPЭ��,���ӽ�����������������Ӧ�ó���֮�佻��������Ϣ,����ɹ�,��������Ӧ�ó������߼������Ӳ�׼����
//�˴�ͨ��,���Ե�˵���ӹ�������
//�ͻ���Ӧ�ó�������Ҫ�������Ӧ�ó���ͨ��ʱ,�ᴴ�����򿪻�׽��ֲ������Ϸ���connect()����,
//��ָ�����ж˵�����Ŀ�������Ӧ�ó���
//�⵼��ͨ�����罫���ӽ���������Ϣ���͵�������Ӧ�ó���,������Ӧ�ó��������������ഴ����׽���,������Ϊ���ӵ��ض��ͻ���
//��ʹ��ȷ���ڷ������˳ɹ��������ӵ���Ϣ�ظ��ͻ���
//������,�Ѿ��ӷ��������յ�ȷ�ϵĿͻ��˽����׽��ֱ��Ϊ���ӵ�������,�����䷢��һ����Ϣ,ȷ���ڿͻ��˲�ɹ�����������
//���������ӿͻ��˽��յ���ȷ����Ϣʱ,��Ϊ����Ӧ�ó���֮����߼������ѽ���,���������ӵ��׽���֮������Ե�ͨ��ģ��
//����ζ������׽���A���ӵ��׽���B,������ֻ���໥ͨ�Ų��Ҳ������κ������׽���Cͨ��,���׽���,���׽���A�������׽���Cͨ��֮ǰ,������
//�ر����׽���B�����Ӳ�����������ʹ���׽���C

//�����㷨��������TCP�ͻ���Ӧ�ó�����ִ���Խ���׽������ӵ�������Ӧ�ó�������Ĳ���
//1.��ȡĿ�������Ӧ�ó����IP��ַ��Э��˿ں�
//2.�Ӳ���1�л�ȡ��IP��ַ��Э��˿ںŴ���asio::ip::tcp::endpoint��Ķ���
//3.�������򿪻�׽���
//4.�����׽��ֵ�connect()����,������2�д����Ķ˵����ָ��Ϊ����
//5.��������ɹ�,����Ϊ�׽���������,��������������������ݺʹӷ�������������

int Create_Client_Connect_TCP() {


	//Step 1
	std::string ip_address = "127.0.0.1";
	unsigned short port_num = 3333;
	try
	{
		asio::io_service ios;
		//Step 2
		//����һ��endpointָ��Ҫ���ӵķ�����Ӧ�ó���
		asio::ip::address adress = asio::ip::address::from_string(ip_address);
		asio::ip::tcp::endpoint ep(adress, port_num);

		//review
		//ʹ��DNS���ƽ���
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
		//��������socket
		asio::ip::tcp::socket socket(ios, ep.protocol());

		//Step 4
		//�˴�Ҳ����ʹ��System_error�쳣�����������
		//[ע��]
		//connect()���ܽ��׽������ӵ�������,������ͬ��ִ�е�,����ζ�Ÿ÷������������ó����߳�
		//ֱ���������Ӳ�����������,��ע��,�������׽���֮ǰ,û�н��׽��ְ󶨵��κα��ض˵�.
		//�ⲻ��ζ���׽��ֱ���δ��״̬,��ִ�����ӽ�������֮ǰ,�׽��ֵ�connect()�������׽��ְ󶨵��˵�
		//�ö˵��ɿۼ�ϵͳѡ���IP��ַ��Э��˿ں����
		//ע�� С��
		//Ҳ����˵,��(bind)����ر�������Ϣ,һ��ͻ��˳�����Ҫ��ʽ��bindһ��endpoint
		//����������Ӧ�ó���(TCP)��Acceptor����Ҫִ��bind��һ����,���ٶ˿���Ϣ������ȷ
		//��connect�������ɿͻ��˷�����,��socket����,��ִ��connect()ʱ����ʽ�󶨱�����
		//IP��Ϣ�Լ���ʹ�õĶ˿�
		//��UDP��,����Ҫ��������,��˲�����UDP��Acceptor,����ڷ����ʹ��Socket.bind
		//һ��bindһ�������������õ�ip(asio::ip::address_v4::any())��һ���ض��Ķ˿ں�
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

	//ע��:connect()������ asio::ip::address::from_string()��̬����
	//�����׳�system_error�쳣,�����������н���һ��error_code���������ز��Ҳ��׳��쳣
	//�����������ʹ���쳣�������
	return 0;
}
//�����㷨�����˽��׽������ӵ���DNS���ƺ�Э��˿ںű�ʾ�ķ�����Ӧ�ó�������Ĳ���
//1.��ȡ���з�����Ӧ�ó����������DNS���ƺͷ������Ķ˿ں�,�������Ǳ�ʾΪ�ַ���
//2.ʹ��asio::ip::tco::resolver�����DNS����
//3.������׽��ֶ�������
//4.����asio::connect����,���׽��ֶ���Ĳ���2�л�õĵ�����������Ϊ�������ݸ���
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

		//�����׳��쳣
		asio::ip::tcp::resolver::iterator it =
			resolver.resolve(resolver_query);

		//Step 3
		//ע��,�����ﴴ����socket������׽���,��Ϊ��֪���ṩ��DNS���ƽ�������IP��ַ�İ汾(IPv4/IPv6)
		//asio::connect()�����ڽ��׽������ӵ�ָ����ȷЭ������ÿ���˵�֮ǰ���׽���,���ʧ����ر���
		asio::ip::tcp::socket sock(ios);

		//Step 4
		//һ����������
		//��������᳢��ÿһ��IPֱ���ɹ�һ����ȫ��ʧ��,Ȼ���׳��쳣
		asio::connect(sock, it);

		//��ʱ�Ѿ��ɹ�����,�������ڷ������ݻ��������
		//
		//

		//ע�� С��
		//��ĿǰΪֹ,ʹ��socket��ע��
		//socket���ֻ�Ǵ���,��Ҫ�ṩio_service��ʵ��,������Ҫʹ�õ��ײ�����I/O������඼��Ҫ�ṩ
		//socket����֮��û�з���ײ����ϵͳ���׽��ֶ���,������open(Э������)֮��,socketʵ��ӵ�����㹻��
		//��Ϣ֮��,�Żᱻ����ײ����ϵͳ���׽��ֶ���
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "Error, Code: " <<
			e.code() << ", Message: " << e.what() << std::endl;
		return e.code().value();
	}
	return 0;
}
//���ͻ��˳�����Ҫͨ��TCPЭ���������ͨ��ʱ,��������Ҫ��÷����������߼�����
//Ϊ��,�ͻ��˷���һ����׽��ֲ������Ϸ�����������(����ͨ�������׽��ֶ����ϵ�connect()����)
//��ᵼ�����ӽ���������Ϣ�����͵�������,�ڷ�������,�����ڷ�����Ӧ�ó�����ܺʹ������Կͻ��˵���������֮ǰִ��ĳЩ����
//�ڴ�֮ǰ,����ϵͳ���ܾ���Դ˷�����Ӧ�ó����������������

//����,������Ӧ�ó��򴴽�����һ�������׽�����󶨵��ض��˵�
//��ʱ,����������׽��ֶ˵�Ŀͻ������������Ա�����ϵͳ�ܾ�
//Ϊ��ʹ����ϵͳ��ʼ����������ض��������׽�����������ض��˵����������,���뽫�ý������׽����л�������ģʽ
//֮��,����ϵͳΪ��˽������׽��ֹ����Ĺ�����������������,����ʼ���ܷ��͸�������������
//���µ��������󵽴�ʱ,������ɲ���ϵͳ����,����ϵͳ�����������Ϊ���������Ŀ��Ľ������׽���������Ĵ��������������
//�ڶ�����ʱ,��������ɹ�������Ӧ�ó�����,������Ӧ�ó�����׼���ô�����һ����������ʱ,��������ŶӲ�������д���
//��ע��,acceptor�׽��ֽ�������ͻ���Ӧ�ó���������,���Ҳ����ڽ���һ����ͨ�Ź���
//����������������ʱ,�������׽��ַ����µĻ�׽���,�������ӵ��ѷ����������������Ӧ�ͻ���Ӧ�ó���
//Ȼ��,����µĻ�׽��־Ϳ���������ͻ��˽���ͨ��,�������׽��ֿ����ڴ�����һ���������������.

//��֮����Acceptor����������ͨ��,ֻ����TCP���ӵĽ���(bind֮����Ҫת��Ϊ����ģʽ),���ҿ��Է����µĻ�׽��������������ӵ�����ͨ��

//�����㷨������������ý������׽����Ա�����ʼ������������,Ȼ�����ʹ������ͬ������������������
//���㷨�ٶ���ͬ��ģʽ��ֻ����һ����������
//1.��ȡ�����������մ�����������Ķ˿ں�
//2.�����������˵�
//3.ʵ��������һ���������׽���
//4.���������׽��ְ󶨵��ڲ���2�д����ķ������˵�
//5.����acceptor�׽��ֵ�listen()����,ʹ�俪ʼ�����˵��ϵĴ�����������
//6.ʵ������׽��ֶ���
//7.׼���ô�����������ʱ,����acceptor�׽��ֵ�accept()����,������6�д����Ļ�׽��ֶ�����Ϊ��������
//8.������óɹ�,���׽������ӵ��ͻ���Ӧ�ó���,��׼��������֮ͨ��
int Create_Server_TCP_SYN() {

	//���������Ӷ��еĴ�С
	const int BACKLOG_SIZE = 30;

	//Step 1
	unsigned short port_num = 3333;
	//Step 2
	//����һ���������˵�����ָ�����з�����Ӧ�ó���������Ͽ��õ�����IP��ַ�Լ��ض���Э��˿ں�
	asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

	asio::io_service ios;

	try
	{
		//Step 3
		//���ܻ��׳��쳣
		//ʵ��������һ���������׽���
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());

		//Step 4
		//�󶨵�֮ǰ��endpoint
		acceptor.bind(ep);

		//Step 5
		//����acceptor��listen()������BACKLOG_SIZE����ֵ��Ϊ��������
		//�˵��ý��������׽����л��������������������״̬
		//����������acceptor�����ϵ���listen()����,�������е�����Ӧ�˵���������󶼽�������ϵͳ��������ܾ�
		//Ӧ�ó��������ʽ֪ͨ����ϵͳ��ϣ��ͨ���˵��ÿ�ʼ�����ض��˵��ϵĴ�����������
		//listen���ܵĲ���ָ���˲���ϵͳά���Ķ��д�С,������������ӿͻ���(client)���͵��ò���ϵͳ(������)
		//�������ڶ�����,���ȴ�������Ӧ�ó�����в���������,�����б���ʱ,����ϵͳ���ܾ��µ���������
		acceptor.listen(BACKLOG_SIZE);

		//Step 6
		//����һ������׽��ֶ����������,��asio::connect()�в���һ���ĵ���
		asio::ip::tcp::socket sock(ios);

		//Step 7
		//����acceptor socket��accept()����,�˷������ܻ�׽�����Ϊ������ִ�ж������,����,
		//����������������������Ľ������׽��ֹ����Ķ���,�������Ϊ��,��÷�����ִֹ��ֱ���µ��������󵽴�������׽���
		//�󶨵���endpoint,���Ҳ���ϵͳ������������
		//���������������һ�������������,�������ȡ���ж������������󲢽��д���
		//��Ϊ�������ݸ�accept()�����Ļ�׽������ӵ����������������Ӧ�ͻ���Ӧ�ó���
		//������ӽ������̳ɹ�,��accept()�������ز���(open())��׽��ֲ����ӵ��ͻ���Ӧ�ó���,�����������䷢�����ݺʹ��н�������
		acceptor.accept(sock);
		//ע��:�ڴ�����������ʱ,acceptor�׽��ֲ��Ὣ�������ӵ��ͻ���Ӧ�ó���.
		//�෴,���򿪲�������һ����׽���,Ȼ��������ͻ���Ӧ�ó������ͨ��
		//�������׽��ֽ������ʹ���(����)�������������

		//ע��:UDP��������ʹ�ý������׽���,��ΪUDP������ζ�Ž�������
		//�෴,ʹ�ð󶨵��˵㲢��������I/O��Ϣ�Ļ�׽���,��ʹ����ͬ�Ļ�׽��ֽ���ͨ��

		//��ʱ,sock�Ѿ����ӵ��˿ͻ��˳�����������շ�����
	}
	catch (boost::system::system_error &e)
	{

		return e.code().value();
	}
	return 0;
}
//========Chapter 2==========//
//I/O Operations

//I/O�������κηֲ�ʽӦ�ó������������ṹ�еĹؼ�����
//����ֱ�Ӳ������ݽ�������
//����������ڴ�Զ��Ӧ�ó����������,������������������Ƿ�������
//
//I/Obuffer
//ͨ����ζ����������������֮�佻������
//����I/O�漰ʹ���ڴ滺����,��Щ�ڴ滺�����������ڴ洢���ݵĽ��̵ĵ�ַ�ռ��з���������ڴ��
//�ڽ����κ����͵��������(��:ͨ��������ļ�,�ܵ���Զ�̼������ȡĳЩ�ļ�)ʱ,���ݵ�����̲�
//�ұ���洢�����ַ�ռ��е�ĳ��λ���Ա������ý�һ������
//Ҳ����˵,�������������ó�ʱ,��ִ���������֮ǰ,���仺����,Ȼ���ڲ����ڼ���������Ŀ���
//������������ʱ,��������������Ӧ�ó��������������,ͬ����ִ���������֮ǰ,����׼�����ݲ�����������������
//Ȼ����������������������,���������������������Դ�Ľ�ɫ,��Ȼ,��������ִ���κ�����I/O���κ�Ӧ�ó���Ļ�����ɲ���
//��������I/O
//p55

//Boost.Asio֧���������͵�I/O����:ͬ�����첽
//ͬ����������ֹ�������ǵ�ִ���߳�,���ҽ��ڲ������ʱ�Ž������
//���,�������͵Ĳ���������:ͬ��
//�ڶ������첽����,�����첽����ʱ,����ص�������º��������,�ú����ڲ������ʱ��Boost.Asio�����
//��Щ���͵�I/O�����ṩ�˺ܴ�������,�����ܻ�ʹ���븴�ӻ�
//�����������ܼ�,���Ҳ�������ִ�е��߳�,����������ʹ���߳���������������,���첽�������ں�̨����
//Boost.Asio������Ϊһ�����ʵ�ֵ�,�������˿��Ʒ����ķ�ת.һ�������첽����������,Ӧ�ô���������߳�ִ�е��߳̿�
//������ʹ�õ�һ�������¼�ѭ���͵���Ӧ�ó����ṩ�Ļص�,֪ͨ�йص���ǰ������첽���������
//�첽�����Ľ����Ϊ�������ݸ��ص�����
//����֮��,���ǻ�������ȡ���첽����,�رպ͹ر��׽��ֵȲ���
//ȡ����ǰ�������첽�����������ǳ���Ҫ,������Ӧ�ó���������ǰ�����Ĳ����������
//����ܻ��ʡӦ�ó������Դ(CPU���ڴ�),���������˷�,��Ӧ�ò�Э��û��Ϊ�����ṩָ����Ϣ�߽��
//��������ʱ,�����Ҫ�ֲ�ʽӦ�ó����һ����֪ͨ��һ�����ѷ���������Ϣ,��ر��׽��������õ�

//�̶�����I/O������ͨ����I/O����һ��ʹ��,��������֪Ҫ���ͻ���յ���Ϣ�Ĵ�Сʱ������Դ��Ŀ�������
//����,��������ڿ��д洢���з���Ŀ�д������,�����׽��ֶ�ȡ����ʱ,�ÿ�д��������������Ŀ���

//��Boost.Asio��,�̶����Ȼ�������������֮һ��ʾ:
//asio::mutable_buffer��asio::const_buffer
//�������඼��ʾһ���������ڴ��,���ɿ�ĵ�һ���ֽڵĵ�ַ�����С
//(���ֽ�Ϊ��λ)ָ��.������Щ���������ʾ,asio::mutable_buffer��ʾ��д������,��asio::const_buffer��ʾֻ��������
//����,asio::mutable_buffer��asio::const_buffer�඼��ֱ����Boost.Asio I/O�����ͷ�����ʹ��
//�෴,������MutableBufferSequence��ConstBufferSequence����

//MutableBufferSequence����ָ��һ������,�ö����ʾasio::mutable_buffer����ļ���
//��Ӧ��,ConstBufferSequence����ָ��һ������,�ö����ʾasio::const_buffer����ļ���
//ִ��I/O������Boost.Asio�����ͷ�����������MutableBufferSequence��ConstBufferSequence����Ҫ��Ķ���
//��Ϊ��ʾ�������Ĳ���

//��Ȼ�ڴ����������,����I/O�������漰����������,����ĳЩ�ض������(����,���ڴ����޵Ļ�����)
//������Ա����ϣ��ʹ�ð��������С�򵥻������ĸ��ϻ������ֲ��ڽ��̵ĵ�ַ�ռ�
//Boost.Asio I/O�����ͷ���ּ��ʹ�ø��ϻ�����,���ϻ�������ʾΪ����MutableBufferSequence��ConstBufferSequence����Ҫ��Ļ���������
//����,std::vector<asio::mutable_buffer>��Ķ�������MutableBufferSequence�����Ҫ��,���,�������ڱ�ʾ��I/O���
//�ĺ����ͷ����еĸ��ϻ�����,����,��������֪�����������һ����ʾΪasio::mutable_buffer��asio::const_buffer��Ķ���Ļ�����
//������Ȼ���ܽ�����Boost�ṩ��I/O��غ����򷽷�һ��ʹ��,�����������ʾΪһ������,�ֱ�����MutableBufferSequence��ConstBufferSequence�����Ҫ��
//Ϊ��,�����������ͨ��ʵ����std::vector<asio::mutable_buffer>��Ķ��󲢽���������ɵĻ���������ļ���
//���ڻ������Ǽ��ϵ�һ����,����MutableBufferSequenceҪ�������I/O������ʹ��
//����,��Ȼ���ַ������Ժܺõش��������������򵥻�������ɵĸ��ϻ�����,���ǵ��漰����ʾ�����򵥻����������ļ�����ʱ��
//Ϊ�˿�����Boost.Asio I/O�����򷽷�һ��ʹ��,�����������ڸ���,ͬʱ,Boost.AsioΪ�����ṩ��һ��ͨ��I/O��غ����ͷ����򻯵���������ʹ�õķ��� 
//asio::buffer()���ɺ�����28������,���ǽ��ܻ������ĸ��ֱ�ʾ,������asio::mutable_buffer_1��asio::const_buffer_1��Ķ���,
//������ݸ�asio::buffer()�����Ļ�����������ֻ������,��ú�������asio::const_buffer_1��Ķ���;
//���򷵻�asio::mutable_buffers_1��Ķ���.asio::mutable_buffers_1��asio::const_buffers_1��ֱ���asio::mutable_buffer��asio::const_buffer���
//������,�����ṩ������MutableBUfferSequence��ConstBufferSequence����Ҫ��Ľӿں���Ϊ
//���������ǽ���Щ��������Ϊ�������ݸ�Boost.Asio I/O�����ͷ���

//�����㷨���������׼��һ��������,�û�����������Boost.Asio socket�ķ���һ��ʹ��,
//�÷���ִ���������,��asio::ip::tcp::socket::send()��asio::write()���ɺ���
//1.����һ��������,ע��,�˲��費�漰Boost.Asio�е��κι��ܻ���������
//2.ʹ��Ҫ���������������仺����
//3.����������ʾΪ����ConstBufferSeqyebce����Ҫ��Ķ���
//4.��������׼������Boost.Asio��������ͺ���һ��ʹ��
int Create_Const_Buffer() {

	std::string buf;
	//Step 1 + Step 2
	buf = "hello";

	//Step 3
	//����ʹ��const_buffer,
	//[ע��]const_buffer_1�ѹ�ʱ
	//֮����ᵽ
	asio::const_buffer output_buf = asio::buffer(buf);

	//Boost.Asio�������:�����Ǳ�ʾTCP�׽��ֵ�Boost.Asio���send()����������
	/*template<typename ConstBufferSequence>
	std::size_t send(const ConstBufferSequence & buffers);	*/
	//����һ��ģ�巽��,������һ������ConstBufferSequece����Ҫ��Ķ�����Ϊ��ʾ�������Ĳ���
	//���ʵĶ�����һ�����϶���,����ʾasio::const_buffer��Ķ��󼯺�
	//���ṩ֧�ֶ���Ԫ�صĽ��е����ĵ��ͼ��Ͻӿ�
	//����:std::vector<asio::const_buffer>��Ķ����ʺ�����send()�����Ĳ���,��std::string��asio::const_buffer��Ķ����ʺ�
	//Ϊ��ʹ�����ǵ�std::string����ͱ�ʾTCP�׽��ֵ����send()����,���ǿ���������:
	asio::const_buffer asio_buf(buf.c_str(), buf.length());
	std::vector<asio::const_buffer> buffers_sequence;
	buffers_sequence.push_back(asio_buf);
	//������buffer_sequence�Ķ�������ConstBufferSequence�����Ҫ��,���,�����������׽��ֶ����send()�����Ĳ���
	//����,���ַ����ǳ�����,�෴,����ʹ��Asio�ṩ��asio::buffer()��������ȡ����������
	//���ǿ�����I/O������ֱ��ʹ������
	//asio::const_buffers_1 output_buf = asio::buffer(buf);
	//Ȼ�����ϵķ����ѹ�ʱ
	asio::io_service ios;
	asio::ip::tcp::socket sock(ios);
	boost::system::error_code ec;
	sock.open(asio::ip::tcp::v4(), ec);
	asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 3333);

	sock.connect(ep);
	//�ɴ˿ɼ�Ŀǰ�汾��asio�Ѿ�������sendֻ�ܽ�������XXXXBufferSequenceҪ��Ĳ�����
	//send()�����Ľ���һ��const_buffer����,ͬʱҲ���ɽ���һ��const_buffer_sequence
	sock.send(output_buf);
	sock.send(buffers_sequence);
	//Step 4
	return 0;
}

//�����㷨���������׼��������Boost.Asio�׽���ִ����������ķ���һ��ʹ�õĻ�����
//����asio::ip::tcp::socket::receive()��asio::read()���ɺ���
//1.����һ��������,�������Ĵ�С�����㹻��,����ӦҪ���յ����ݿ�,ע��,�˲��費�漰Boost.Asio�е��κι��ܻ���������
//2.ʹ������MutableBufferSequence����Ҫ��Ķ����ʾ������
//3.��������׼������Boost.Asio���뷽���ͺ���һ��ʹ��

int Create_Mutable_Buffer() {
	//����һ����������,�������ڻ������ѷ��䵫û���������,��Ϊ��Ŀ�Ĳ�ͬ
	//���,������ּ������������ڼ��Զ��Ӧ�ó�������
	//ʹ�����������ʱ,������ȷ��ʾ���뻺����,�Ա���������Boost.Asio I/O�����ͺ���һ��ʹ��
	//����,�����������,�����������ʾΪ����MutableBufferSequence����Ҫ��Ķ���
	//��ConstBufferSequence�෴,�˸����ʾ�ɱ仺�����ļ���,������д��Ļ�����
	//������,����ʹ��buffer()����,���������Ǵ�������������ı�ʾ
	const size_t BUF_SIZE_BYTES = 20;

	//Step 1
	//���仺����,���������ڿ����ڴ��з�����ַ�����
	std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);

	//Step 2
	//[ע��]mutable_buffers_1�ѹ�ʱ,ʹ��mutable_buffer
	asio::mutable_buffer input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);

	//Step 3
	return 0;
	//
	//[ע��]
	//const_buffer��mutable_bufferֻ�ṩ�������Ľӿ�,����������������

}
//��֮:С��
//��ǰ�汾��ASIO�Ѿ�������send()write()�Ⱥ�������ʹ��һ������һ�����еĲ�����Ҫ��
//���const_buffers_1��mutable_buffers_1�������������Ѿ�������ʹ����
//��Щ�����Ѿ�������Խ�������const_buffer��mutable_buffer�Ĳ�����


//����չ��������������д��������ʱ��̬�������С�Ļ�����
//����ͨ�������ڴ�����Ϣ�Ĵ�Сδ֪ʱ���׽��ֶ�ȡ����
//ĳЩӦ�ó����Э��δ������Ϣ��ȷ�д�С
//�෴,��Ϣ�ı߽�����Ϣ����ĩβ���ض��������б�ʾ,�����ɷ���������ɷ�����Ϣ��
//�����Ĵ���Э�������Ϣ�ļ�����(EOF)��ʾ
//����,����HTTPЭ��,�������Ӧ��Ϣ�ı�ͷ����û�й̶�����,��߽����ĸ�
//ASCII�������б�ʾ<CR><LF><CR><LF>,������Ϣ��һ����
//�����������,Boost.Asio���ṩ��̬����չ�������Ϳ���ʹ�����ǵĺ����ǳ�����
//
//����չ���������Ļ�������Boost.Asio����asio::streambuf���ʾ,����typedef asio::basic_streambuf<> streambuf;
//asio::basic_streambuf<>��̳���std::streambuf,����ζ������������STL�������������
//����֮��,Boost.Asio�ṩ�ļ���I/O���������ʾΪ�������Ļ�����
//���ǿ�����asio::streambuf��Ķ���,�������Ǵ����std::streambuf��̳е��κ���������һ��
//����,���ǿ��Խ��˶���������
//std::istream,std::ostream��std::iostream,����ȡ�������ǵ���Ҫ
//Ȼ��ʹ��stream�������<<()��>>()��������д��Ͷ�ȡ����
//p62

//ʾ������,ʵ����asio::strembuf�Ķ���,����д��һЩ����,Ȼ�����ݴӻ��������ص�std::string�����
int Test_Buffer_IO() {
	asio::streambuf buf;

	//��������������ض���buf��������
	//ͨ��,�ڵ��͵Ŀͻ��˻������Ӧ�ó��������ݽ�ͨ��Boost.Asio���뺯��
	//(����asio::read())д��buf��������,�ú���������������������Ϊ���������ж�ȡ����,�׽���
	//���û�����,����Ҫ������������������,Ϊ��,���Ƿ���һ����Ϊ
	//message1���ַ�������,Ȼ��ʹ��std::getline������ȡ��ǰ�洢��buf���������е�
	//�����ַ���,ֱ���ָ�������Ϊ\n
	//���,string1�������Messages1,��buf�������������ָ����ź���������ʼ�ַ���,Message2
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
	//���
	//Messages1
	//Messages2
	//Messages2

	return 0;
}
//д��TCP�׽�����һ���������,���ڽ����ݷ��͵����ӵ����׽��ֵ�Զ��Ӧ�ó���
//ͬ��д����ʹ��Boost.Asio�ṩ���׽��ַ������ݵ���򵥷���
//ִ��ͬ��д���׽��ֵķ����ͺ���������ִ�е��߳�,�����ڽ�����(����һЩ
//������)д���׽��ֻ�������֮ǰ���᷵��

//д��Boost.Asio���ṩ���׽��ֵ������������ʹ��asio::ip::tcp::socket���
//write_sonme()����,�����Ƿ���������֮һ������
/*template<
typename ConstBufferSequence>
std::size_t write_some(
const ConstBufferSequence & buffers);*/
//�˷�������һ����ʾ���ϻ�������Ϊ�����Ķ���,����˼��,����һЩ���ݴӻ�����д���׽���
//��������ɹ�,�򷵻�ֵ��ʾд����ֽ���,����Ҫǿ������,�÷������ܲ��ᷢ��ͨ��buffers�����ṩ����������
//����,�÷�������֤��δ��������ʱ����д��һ���ֽ�,����ζ��,��һ�������,Ϊ�˽��������ݴ�
//������д���׽���,���ǿ�����Ҫ��ε��ô˷���

//�����㷨�����˽�����ͬ��д��ֲ�ʽӦ�ó����е�TCP�׽�������Ĳ���
//1.�ڿͻ���Ӧ�ó�����,����,�򿪺����ӻ��TCP�׽���,�ڷ�����Ӧ�ó�����,ͨ��ʹ�ý�����
//�׽��ֽ���������������ȡ���ӵĻTCP�׽���
//2.���仺��������������Ҫд���׽��ֵ�����
//3.��ѭ����,�����׽��ֵ�wriet_some()�������,�Ա㷢�ͻ������п��õ���������
void WriteToSocket(asio::ip::tcp::socket &sock) {
	//Step 2
	std::string buf = "Hello ...";

	std::size_t total_bytes_written = 0;

	unsigned short times = 0;
	//Step 3
	while (total_bytes_written != buf.length()) {
		//ע��,write_some()���ص�ֵ�ǵ��ô˺����ڼ�
		//�ɹ�д����ֽ���
		//�ڵ��ε���write_some()�����ڼ�д���׽��ֵ��ֽ���ȡ���ڼ�������
		//һ�������,������Ա��֪
		total_bytes_written += sock.write_some(
			asio::buffer(buf.c_str() +
				total_bytes_written,
				buf.length() - total_bytes_written));
		++times;
	}
	//����������ֻ����һ��write_some���þ�ȫ��������
	std::cout << "Times = " << times << std::endl;
}

int Test_First_Client_TCP() {

	std::cout << "Client:" << std::endl;
	//���һ�����try����endpoint��������
	asio::io_service ios;

	try {
		//�����׳��쳣
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 3333);
		//�����׳��쳣
		asio::ip::tcp::socket sock(ios, ep.protocol());
		sock.connect(ep);

		//buffer
		//std::string buf = "Hello, Server!";
		//asio::const_buffer buff = asio::buffer(buf);

		//sock.write_some(buff);

		//����ѭ��write_some��������
		WriteToSocket(sock);

		//Read
		//����ʹ��unique_ptr��������Ϊmutable_buffer���������ͷ��ڶ���������ڴ�ռ�,ֱ�ӽ���unique_ptr����
		asio::mutable_buffer input(static_cast<void *>((std::unique_ptr<char[]>(new char[20])).get()), 20);
		//����read_some���ص����Ѿ���ȡ�����ֽ���
		//��asio::buffer_size()��ͬ(���ص����ܵĻ������Ĵ�С)
		std::size_t bytes = sock.read_some(input);
		//�ӻ�������ȡ�ڴ�,��castΪchar*,�����ƶ�ȡ���ֽ���
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
		//�����쳣
		asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 3333);
		//
		asio::ip::tcp::acceptor acceptor(ios);

		//��Ҫ����acceptorҲҪ����open���ܱ����������ײ��׽���
		//����ᱨ�� �ṩ���ļ������Ч
		acceptor.open(ep.protocol());
		//��Ҫ����bind,bindһ��ֻ���ڷ����
		//bind�󶨵��Ǳ�������Ϣ,endpoint(�������õ�����IP,�������ŵĶ˿�)
		acceptor.bind(ep);
		//ֻ�ǰ󶨲���,��Ҫ�л�������״̬,ͬʱ������Ϣ���е����ֵ
		acceptor.listen(MAX_SEQ_SIZE);

		//��ע��˴�����ֱ�Ӵ�sock
		asio::ip::tcp::socket sock(ios);

		//��Ϊaccept������Э����,����������Ϣ(Э��;����IP��ַ)
		acceptor.accept(sock);

		//Read
		asio::mutable_buffer input(static_cast<void *>((std::unique_ptr<char[]>(new char[20])).get()), 20);
		std::size_t bytes = sock.read_some(input);

		std::string data(asio::buffer_cast<char *>(input), bytes);


		std::cout << data << std::endl;
		//[ע��]
		//��������һ������,ֱ��ʹ��asio::streambuf��Ϊsock.read_some()
		//�Ĳ����ᱨ��,

		//Write
		//��Ϊ�Ǳ���,ֻ����һ�ξͽ�write_some�������
		std::string buf = "Yeah Client!";
		asio::const_buffer buff = asio::buffer(buf);

		sock.write_some(buff);

		//[ע��]
		//������accept�ɹ�֮��һ��Ҫ��read����write,
		//���������read_some�����ж�

	}
	catch (boost::system::system_error &e) {
		std::cout << "Error, Code = "
			<< e.code() << ", Message = "
			<< e.what() << std::endl;
		return e.code().value();
	}
	return 0;

}
//�������ϵĺ���
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

//asio::ip::tcp::socket�������һ�ֽ�����ͬ��д����Ϊsend()���׽���
//�ķ���,���ַ�������������
//����һ����ͬ��write_some()����,��ǰ����,��������ȫ��ͬ��ǩ���͹���
//��write_some()�������,�ڶ������ؽ��ܶ���Ĳ���
//
/*template<
typename ConstBufferSequence>
std::size_t send(
 const ConstBufferSequence & buffers,
 socket_base::message_flags flags);*/
//������Ӳ�����Ϊflags,��������ָ��λ����,��ʾ���Ʋ����ı�־
//��Ϊ����ʹ����Щ��־,���Բ��ῼ����
//�����������൱�ڵڶ�������,���ڷ�������ʱ�����׳��쳣,�෴,ͨ��
//error_code���͵ĸ��ӷ��������������
//p68d

//ʹ���׽��ֵ�write_some()����д���׽��ֶ������ּ򵥵Ĳ�����˵�ƺ��ǳ�
//����,��ʹ��Ҫ����һ���ɼ����ֽ���ɵ�СС��,Ҳ����ʹ��һ��ѭ��,һ�������������Ѿ�
//д���˶����ֽ�,��Ϊѭ����ÿ�ε�����ȷ����һ��������
//���ַ������׳���,ʹ����������.
//���˵���,Boost.Asio�ṩ��һ����ѵĹ���,���˶Ե�д��,�����������asio::write()
/*template<
typename SyncWriteStream,
typename ConstBufferSequence>
std::size_t write(
SyncWriteStream & s,
const ConstBufferSequence & buffers);*/
//�˺���������������,��һ������Ϊs���Ƕ�����SyncWriteStream����Ҫ��Ķ��������
//�й�Ҫ��������б�,����ĵ�
//��ʾTCP�׽��ֵ�asio::ip::tcp::socket��Ķ���������ЩҪ��,��˿������������ĵ�һ������
//��Ϊbuffers�ĵڶ���������ʾ������(�򵥻򸴺�),������Ҫд���׽��ֵ�����
//��write_some()�����෴,���߽�һЩ���ݴӻ�����д���׽���,asio::write()����д�뻺�����п��õ���������
//����˶��׽��ֵ�д��,ʹ��������,������,�������ʹ��asio::write()�����������׽��ֶ����write_some()����
//������д���׽���,��ô��һ��������writeToSocket()�����ͻ�������ʾ
void writeToSocketEnhanced(asio::ip::tcp::socket &sock) {
	std::string buf = "hello ...";

	asio::write(sock, asio::buffer(buf));
}
//asio::write()������ʵ�ַ�ʽ������ԭʼ��writeToSkcket()����,��ͨ����ѭ���ж�ε���socket�����write_some()����ʵ�ֵ�

//��TCP�׽��ֶ�ȡ��һ���������,���ڽ������ӵ����׽��ֵ�Զ��Ӧ�ó����͵�����
//ͬ����ȡ��ʹ��Boost.Asio�ṩ���׽��ֽ������ݵ���򵥷���
//���׽���ִ��ͬ����ȡ�ķ����ͺ���������ִ�е��߳�,�����ڴ��׽��ֶ�ȡ����
//(����һЩ������)��������֮ǰ���᷵��

//��Boost.Asio���ṩ���׽��ֶ�ȡ���ݵ������������asio::ip::tcp::socket���read_some()����
/*template<
typename MutableBufferSequence>
std::size_t read_some(
 const MutableBufferSequence & buffers);*/
//�����������һ����ʾ��д������(�����򸴺�)��Ϊ�����Ķ���,����˼��,��
//���׽����򻺳�����ȡһЩ������,��������ɹ�,�򷵻�ֵ��ʾ��ȡ���ֽ���
//��Ҫ����Ҫע��,�޷����Ʒ�����ȡ���ֽ���,�÷�������֤��δ��������ʱ���ٶ�ȡһ���ֽ�
//����ζ��,��һ�������,Ϊ�˴��׽��ֶ�ȡһ���ǵ�����,���ǿ�����Ҫ��ε��ø÷���

//�����㷨�����˴ӷֲ�ʽӦ�ó����е�TCP�׽���ͬ����ȡ��������Ĳ���
//1.�ڿͻ���Ӧ�ó�����,����,˺�������ӻ��TCP,�ڷ�����Ӧ�ó�����,ͨ��ʹ�ý�����
//�׽��ֽ�����������ȡ���ӵĻTCP�׽���
//2.�����㹻��С�Ļ��������ʺ�Ҫ��ȡ��Ԥ����Ϣ
//3.��ѭ����,�����׽��ֵ�read_some()�����Ĵ������ȡ��Ϣ����Ĵ�����ͬ

std::string readFromeSocket(asio::ip::tcp::socket &sock) {
	const unsigned char MESSAGE_SIZE = 7;
	char buf[MESSAGE_SIZE];
	std::size_t total_bytes_read = 0;

	while (total_bytes_read != MESSAGE_SIZE) {
		total_bytes_read += sock.read_some(
			asio::buffer(buf + total_bytes_read,
				MESSAGE_SIZE - total_bytes_read));
	}
	//�ڵ��ε���read_some()�����ڼ���׽��ֶ�ȡ���ֽ���ȡ���ڼ�������
	//��һ�������,������Ա��֪��,���Ӧ��ʹ��ѭ������ȡ�׽����е���������
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
		//sokc��������ʱ�����Ѿ�connect(),����ʧЧ
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
//asio::ip::tcp::socket�������һ���׽���ͬ����ȡ���ݵķ���
//��Ϊreceive(),���ַ�������������,����һ����ͬ��read_some()����,��ǰ����
//��������ȫ��ͬ��ǩ��,���ṩ��ȫ��ͬ�Ĺ���,��Щ������ĳ����������ͬ���
//��read_some()�������,�ڶ������ؽ���һ������Ĳ���
/*template<
 typename MutableBufferSequence>
std::size_t receive(
 const MutableBufferSequence & buffers,
 socket_base::message_flags flags);*/
//������Ӳ�����Ϊflags,��������ָ��λ����,��ʾ���Ʋ����ı�־,
//���ں���ʹ����Щ��־,������ǲ����ڱ����п���
//�����������൱�ڵڶ���,���ڷ�������ʱ�����׳��쳣����ͨ��boost::system::error_code���͵ĸ�������������ش�����Ϣ
//[ע��]
//д���׽��ֵ�write_some��send�����������
//��ȡ�׽��ֵ�read_some��receive��������

//ʹ���׽��ֵ�read_some�����������׽��ֶ�ȡ�������ּ򵥵Ĳ�����˵�ƺ��ǳ����ӡ�
//���ַ���Ҫ������ʹ��ѭ���������������Ѿ���ȡ�˶����ֽڣ���Ϊѭ����ÿ�ε�����ȷ���컺������
//���ַ������׳���ʹ�������������ά����
//���˵��ǣ�Boost.Asio�ṩ��һϵ����Ѻ��������Լ򻯴Ӳ�ͬ�������е��׽���ͬ����ȡ���ݵĹ���
//�����������ĺ���,ÿ���������м�������,�ṩ�˷ḻ�Ĺ���,���ڴ��׽��ֶ�ȡ����
//
//[1]
//asio::read()����
//����������������򵥵�һ��
/*template<
typename SyncReadStream,
typename MutableBufferSequence>
std::size_t read(
SyncReadStream & s,
const MutableBufferSequence & buffers);*/
//�������������������,��һ����Ϊs���Ƕ�����SyncReadStream����Ҫ��Ķ��������
//��ʾ TCP�׽��ֵ�asio::ip::tcp::socket��Ķ���������ЩҪ��,��˿�������
//�����ĵ�һ������,��Ϊbuffers�ĵڶ���������ʾ�����׽��ֶ�ȡ���ݵĻ�����(�򵥻򸴺�)
//���׽��ֵ�read_some()�����෴,asio::read()�����ڵ��������ڼ���׽��ֶ�ȡ����,ֱ��
//һ�����������ݸ����������˴���,����˴��׽��ֶ�ȡ��ʹ��������
//���ʹ��asio::read()����,ǰ���readFromSocket()��������������
std::string readFromSocketEnhanced(asio::ip::tcp::socket& sock) {
	const unsigned char MESSAGE_SIZE = 7;
	char buf[MESSAGE_SIZE];
	//����������ý�����ִ���߳�,ֱ������7���ֽڻ�������
	asio::read(sock, asio::buffer(buf, MESSAGE_SIZE));

	return std::string(buf, MESSAGE_SIZE);
}

//
//[2]
//asio::read_until()����
//���ṩ��һ�ִ��׽��ֶ�ȡ����ֱ��������������ָ��ģʽ�ķ���
//�˺����а˸�����
/*template<
typename SyncReadStream,
typename Allocator>
std::size_t read_until(
SyncReadStream & s,
boost::asio::basic_streambuf< Allocator > & b,
char delim);*/
//��һ��������ΪS����������SyncReadStream����Ҫ��Ķ��������
//��ʾTCP�׽��ֵ�asio::ip::tcp::socket��Ķ���������ЩҪ��,��˿������������ĵ�һ������
//��Ϊb�ĵڶ���������ʾ�������Ŀ���չ������,���н���ȡ������Ϊdelim�����һ������ָ���˷ָ���
//asio::read_until()��������s�׽��ֶ�ȡ���ݵ�������bֱ�����������ݶ�ȡ������delim����ָ�����ַ�
//����ָ�����ַ�ʱ,��������

//��Ҫ����Ҫע��asio::read_until()������ʵ�����Կɱ��С�Ŀ��ȡ��(����ʹ���׽��ֵ�read_some()������ȡ)
//����������ʱ,������b�����ڷָ������ź������һЩ����,Զ��Ӧ�ó����ڷָ�������֮���͸�������
//(����,��������������������Ϣ,ÿ����Ϣ�����һ���ָ���),����ܻᷢ���������,������˵,��asio::read_until()
//�����ɹ�����ʱ,��֤������b��������һ���ָ�������,����ܰ�������
//[ע��]��������᷵�ذ������������
//������Ա�����ν����������е�����,���ڷָ������ź��������ʱ��������
//���������Ҫ���׽��ֶ�ȡ��������ֱ�������ض�����,���ǽ�����ʵ��readFromSocket()����
//������Ϣ�ָ�����һ���µ���\n
std::string readFromSocketDelim(asio::ip::tcp::socket &sock) {
	asio::streambuf buf;

	//asio::read(sock, buf);//OK
	asio::read_until(sock, buf, '\n');
	//������ȷ
	asio::mutable_buffer bufs = buf.prepare(512);
	//��ʱbuf����Ϊmutable_buffers_1
	//or mutable_buffer
	sock.receive(bufs);
	//�����Ǵ����,read_some����ʹ��asio::streambuf��Ϊ����,��ʹ�﷨��ȷ
	//sock.read_some(buf);//error
	std::string message;

	std::istream input_stream(&buf);

	//ע��:message���ܰ���'\n'֮�������
	//�������ڷָ�����'\n'��˵�����getline֮���Զ��ָ���'\n'֮�������
	std::getline(input_stream, message);

	return message;
}

//
//[3]
//asio::read_at()����
//�˺����ṩ��һ�ִ��׽����ض���ƫ������ʼ��ȡ���ݵķ���
//������ʹ��,������ʵ�ַ�ʽ������ԭʼ��readFromSocket()����,
//ͨ����ε���socket�е�read_some()��ʵ�ֵ�ֱ��������ֹ����ѽ��������

//[�첽]
//�첽д����һ�ֽ����ݷ��͵�Զ��Ӧ�ó����������Ч�ķ���
//�����첽������д��Boost�ṩ���׽��ֵ����������
//��asio::ip::tcp::socket���async_write_some()����
/*template<
typename ConstBufferSequence,
typename WriteHandler>
void async_write_some(
const ConstBufferSequence & buffers,
WriteHandler handler);*/
//�˷�������д���������������,������һ������,�ö����ʾһ��������
//�û���������Ҫд���׽��ֵ�������Ϊ���һ������
//�ڶ���������һ���ص�,��һ������(��ʼ)�Ĳ������ʱ,������Boost.Asio����
//�˲��������Ǻ���ָ��,�º���������WriteHandler����Ҫ����κ���������
//�ص�����Ӧ�������º���ǩ��
/*void write_handler(
 const boost::system::error_code& ec,
 std::size_t bytes_transferred);*/
 //����,ec��һ������,��ʾ���ִ����code,��bytes_transferred������ʾ����Ӧ���첽����
 //�ڼ������׽���д���˶����ֽ�,���纯����������ʾ,��������һ��ּ�ڽ�dһЩ���ݴӻ�����д���׽���
 //�Ĳ���,���û�з�������,�˷�����ȷ������Ӧ���첽�����ڼ�����д��һ���ֽ�
 //����ζ��,��һ�������,Ϊ�˽��������п��õ���������д���׽���,���ǿ�����Ҫ���ִ�д��첽
 //����.

 //�����㷨������ִ�к�ʵ��Ӧ�ó�������Ĳ���,��Ӧ�ó����첽�ؽ�����д��TCP�׽���,
 //��ע��,���㷨�ṩ��ʵ�ִ���Ӧ�ó���Ŀ��ܷ���
 //Boost.Asio�ǳ����,��������ͨ���Զ��ֲ�ͬ�ķ�ʽ�첽�ؽ�����д���׽�������֯�͹���Ӧ�ó���
 //1.����һ�����ݽṹ,���а���һ��ָ���׽��ֶ����ָ��,һ����������һ������д���ֽڼ������ı���
 //2.�������첽д��������ʱ�����õĻص�����
 //3.�ڿͻ���Ӧ�ó�����,���䲢�򿪻TCP�׽��ֲ��������ӵ�Զ��Ӧ�ó���.�ڷ�����Ӧ�ó�����
 //ͨ���������������ȡ���ӵĻTCP�׽���
 //4.����һ������������Ҫд���׽��ֵ����������
 //5.ͨ�������׽��ֵ�async_write_some()���������첽д�����
 //6.��asio::io_service��Ķ����ϵ���run()����;
 //7.�ڻص���,����д����ֽڼ�����,���д����ֽ���С��Ҫд������ֽ���,
 //�������µ��첽д�������д����һ��������
 //p78

 //Step 1
 //����һ�����ݽṹ,���а���һ��ָ���׽��ֶ����ָ��,һ��
 //����Ҫд������ݵĻ�����,�Լ�һ��������д����ֽ����ļ���������
struct Session0 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::string buf;
	std::size_t total_bytes_written;
};
//Step 2
//����һ���ص�����,�������첽�������ʱ����
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
//����������3,���ڵ����ĺ�����ʵ�ֲ���4��5
void writeToSocket2(std::shared_ptr<asio::ip::tcp::socket> sock) {

	//�����ڿ����ڴ��з���Session���ݽṹ��ʵ��
	std::shared_ptr<Session0> s(new Session0);

	//Step 4
	//������ʵ��,�����������ݸ���������sockָ��
	s->buf = std::string("Hello123456789");
	s->total_bytes_written = 0;
	s->sock = sock;

	//Step 5
	//��Ϊ�׽��ֵ�async_write_some()���������޷�һ���Խ���������д��
	//�׽���,�������ǿ�����Ҫ�ڻص�������������һ���첽д����
	//�����������ҪSession�����ԭ��,���ǽ�������������ڴ�(��)������ջ;
	//��Ϊ���������,ֱ�����ûص�����,������������첽����,����socket�����async_write_some()����

	//��һ�������ǻ�����,���а���Ҫд���׽��ֵ�����
	//��Ϊ�������첽��,����Boost.Asio�����ڲ��������ͻص�����������ʱ����buffer
	//,����ζ�Ż��������뱣�ֲ���,���ұ�����ò��ܵ��ûص�
	//����ͨ�����������洢��Session����������֤��һ��,��Session�����ִ洢�ڿ����ڴ�(��)��
	//�ڶ������������첽�������ʱҪ���õĻص�
	//Boost.Asio���ص�����Ϊһ������,�������Ǻ�����������,������������
	//�ص��ĵ�һ������ָ����ִ�в���ʱ�����Ĵ���(�����)
	//�ڶ�������ָ�������Ѿ�д����ֽ���

	//��Ϊ������Ҫ�����ǵĻص���������һ������Ĳ���,һ��ָ����ӦSession�����ָ��
	//����Ϊ������������,����ʹ��std::bind()����������һ�����Ǹ���һ��
	//ָ��Session�����ָ����Ϊ�������εĺ�������
	//Ȼ�󽫺���������Ϊ�ص��������ݸ��׽��ֶ����async_write_some()����,
	//��Ϊ�����첽��,����async_write_some����������ִֹ�е��߳�
	//������д�����������,ʵ�ʵ�д�������Boost.Asio��͵ײ����ϵͳ��Ļ��ִ��,��
	//������ɻ�������ʱ,�����ûص�,����ʱ,���ǵ�ʾ��Ӧ�ó�������Ϊcallback�Ļص�����������
	//�Ƿ�ɹ����Ƿ�������ʼ,�ں�һ�������,������Ϣ�������׼����������غ���
	//����,��д���ֽڵļ�������������Ϊ�������д����ֽ���
	//Ȼ��,���Ǽ��д���׽��ֵ������Ƿ���ڻ������Ĵ�С,�����Щֵ���,����ζ���������ݶ���д���׽���,
	//����û����������Ҫ��,�ص���������,�������Ҫд��Ļ���������������,�������µ��첽д�����

	//ע�⻺�����Ŀ�ͷ����ƶ��Ѿ�д����ֽ���,�Լ��������Ĵ�С�����Ӧ�ؼ�С��ͬ��ֵ
	//��Ϊ�ص�,����ʹ��std::bind()����ָ����ͬ��callback()����������һ������Ĳ���
	//Session����,��������������һ���첽����ʱһ��
	//�����첽д������ͺ����ص����õ�ѭ���ظ�,ֱ���������е��������ݶ�д���׽��ֻ���
	//����,���ص��������ض��������µ��첽����ʱ,��main()�����е��õ�asio::io_service::run()����
	//�����ִ���̵߳�����������,main����Ҳ�᷵��,����Ӧ�ó����˳���ʱ��
	s->sock->async_write_some(
		asio::buffer(s->buf),
		std::bind(callback,
			std::placeholders::_1,
			std::placeholders::_2,
			s));

}
//���ڻص�Step 3
int First_ASYN_TCP_Write() {
	std::string raw_ip_address = "127.0.0.1";
	unsigned short port_num = 3333;

	//[ע��]
	//Boost.Asio���ܻ�ΪĳЩ�ڲ��������������߳�,������֤����Щ�߳�
	//���������в�ִ���κ�Ӧ�ó������

	//����,�򿪲�ͬ�����׽������ӵ�Զ��Ӧ�ó�����,Ȼ��ͨ����ָ�봫�ݸ�
	//�׽��ֶ���������writeToSocket()����
	//�˺��������첽д�����������
	//�˺�����������asio::io_service��Ķ����ϵ�run()����
	//����Boost.Asio����ִ���̲߳������ʱʹ�������������첽������صĻص�����
	//ֻҪ������һ��������첽����,asio::ioservice::run()�����ͻ�
	//����,�����һ��������첽�������һ���ص����ʱ,�˷�������
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

//��Ȼ��һ��ʾ����������async_write_some()���������첽�ؽ�����д���׽���,
//���������ķ����е㸴�������׳���,���˵���,Boost.Asio�ṩ��һ��ʹ��free����
//asio::async_write()���첽��ʽ������д���׽��ֵķ���
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
//�˺������׽��ֵ�async_write_some()�����ǳ�����
//���ĵ�һ������������AsyncWriteStream����Ҫ��Ķ���
//asio::ip::tco::socket��Ķ���������ЩҪ��,��˿�����˺���һ��ʹ��
//asio::async_write()�����ĵڶ����͵�����������������һ��ʾ����������TCP
//�׽��ֵ�async_write_some()�����ĵ�һ���͵ڶ�������
//��Щ�����ǰ���Ҫд������ݵĻ������Լ���ʾ�ص��ĺ��������,���ڲ������ʱ����
//���׽��ֵ�async_write_some()����(������һЩ���ݴӻ�����д���׽��ֵĲ���)�෴
//asio::async_write()������������,�ò���д�뻺�����п��õ���������
//�����������,ֻ�е��������е����п������ݶ�д���׽��ֻ�������ʱ�Ż���ûص�
//����˶��׽��ֵ�д��,ʹ��������,������.
//
//ʹ��asio::async_write()��������д
//����,���ǲ���Ҫ����д���׽��ֵ��ֽ���,���,Session�ṹ��ø�С
struct Session2 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::string buf;
};
//֮��,����֪�������ûص�����ʱ,����ζ�����Ի��������������ݶ���д���׽��ֻ����˴���
//��ʹ�ûص��������Ӽ�
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

	//����,����֪�����е������Ѿ�����
}
//asio::async_write()������ͨ�����׽��ֶ����async_write_some()�����������
//���ε�����ʵ�ֵ�,�����������ǳ�ʼ�����е�writeToSocket()������ʵ�ַ�ʽ
//ע��,asio::async_write������������,�ṩ�˶���Ĺ���
//

//�첽��ȡ��һ�ִ�Զ��Ӧ�ó���������ݵ�������Ч�ķ���

//���ڴ�Boost.Asio���ṩ��TCP�׽����첽��ȡ���ݵ������������
//asio::ip::tcp::socket���async_read_some()����
//���Ƿ���������֮һ
/*template<
typename MutableBufferSequence,
typename ReadHandler>
void async_read_some(
const MutableBufferSequence & buffers,
ReadHandler handler);*/
//�˷��������첽��ȡ��������������,������һ����ʾ�ɱ仺�����Ķ���
//��Ϊ���׽��ֶ�ȡ���ݵĵ�һ������,�ڶ���������һ���ص�,���������ʱ��Boost.Asio
//����,�˲��������Ǻ���ָ��,�º���������ReadHandle����Ҫ�����������
//�ص�Ӧ��������ǩ��:
/*void read_handler(
const boost::system::error_code& ec,
std::size_t bytes_transferred);*/
//����,ec��һ������,������ִ��������֪ͨ�������
//bytes_transferred����ָʾ����Ӧ���첽�����ڼ���׽��ֶ�ȡ�˶����ֽ�
//����async_read_some()��������������ʾ������,������һ������,ּ�ڴ��׽��ֶ�ȡһЩ���ݵ�������
//���û�з�������,�˷�����ȷ������Ӧ���첽�����ڼ����ٶ�ȡһ���ֽ�
//����ζ��,��һ�������,Ϊ�˴��׽��ֶ�ȡ��������,���ǿ�����Ҫ���ִ�д��첽����
//����,����֪����key�����Ĺ���ԭ��

//�����㷨������ʵ��Ӧ�ó������貽��,��Ӧ�ó����첽��ȡ�׽����е�����
//��ע��,���㷨�ṩ��ʵ�ִ���Ӧ�ó�����ܷ���
//1.����һ�����ݽṹ,���а���һ��ָ���׽��ֶ����ָ��,һ��������,
//һ�����建������С�ı���,�ĸ�һ��������ȡ�ֽڼ������ı���
//2.�������첽��ȡ�������ʱ�����õĻص�����
//3.�ڿͻ���Ӧ�ó�����,���䲢�򿪻��TCP�׽���,Ȼ�������ӵ�Զ��Ӧ�ó���
//�ڷ�����Ӧ�ó�����,ͨ���������������ȡ���ӵĻTCP�׽���
//4.�����㹻��Ļ�������ʹԤ����Ϣ�ʺ�
//5.�����׽��ֵ�async_read_some()���������첽��ȡ����,������2�ж���ĺ���
//ָ��Ϊ�ص�
//6.��asio::io_service��Ķ����ϵ���run()����
//7.�ڻص���,���Ӷ�ȡ���ֽ���,�����ȡ���ֽ���С��Ҫ��ȡ�����ֽ���(Ԥ����Ϣ�Ĵ�С)
//�������µ��첽��ȡ�����Զ�ȡ��һ��������

//����Step 1 ����һ�����ݽṹ,���а���һ��ָ����Ϊsock���׽��ֶ����ָ��,
//һ��ָ����Ϊbuf�Ļ�������ָ��,һ����Ϊbuf_size�ı���,���а����������Ĵ�С,�Լ�һ�������ñ���
//��total_bytes_read����,ָʾ�Ѷ�ȡ���ֽ���

struct Session3 {
	std::shared_ptr<asio::ip::tcp::socket> sock;
	std::unique_ptr<char[]> buf;
	std::size_t total_bytes_read;
	unsigned int buf_size;
};
//Step 2 ����һ���ص�����
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

		//ֻҪ������һ��������첽����,io_service::run()�����ͻ�����
		//�����һ��������������һ���ص����ʱ,�˷�������
		ios.run();
	}
	catch (boost::system::system_error &e) {

		return e.code().value();
	}
	return 0;
}
//�����첽��Ϣ����
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
//����async_read_some����������׽����첽��ȡ����,���������Ľ������
//�е㸴������������
//ʹ��asio::async_read()
/*template<
typename AsyncReadStream,
typename MutableBufferSequence,
typename ReadHandler>
void async_read(
AsyncReadStream & s,
const MutableBufferSequence & buffers,
ReadHandler handler);*/
//����������������׽��ֶ�ȡ���ݵĲ���,ֱ������������
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

	//��ʱ�ɱ仺�����Ѿ�������
}
//asio::async_read�ڲ�ʵ��Ҳ�Ƕ�ε�����socket��async_read_some����

//��ʱ,�������첽����������δ���֮��,Ӧ�ó����е��������ܻᷢ��
//�仯,���������Ĳ�������޹ؽ�Ҫ���ʱ,����û���˶���ɲ�������Ȥ
//����֮��,����������첽�����Ƕ��û�����ķ�Ӧ,���û�������ִ�в���ʱ�ı�����
//�û�������Ҫ������ǰ����������,���ҿ�����Ҫ������ͬ�����������˳�Ӧ�ó���
//�����û��ڵ��͵�Web������ĵ�ַ����д����վ��ַ����Enter�����
//�������������DNS���ƽ�������,����DNS���Ʋ���ȡ��ӦIP��ַ��,���������Ӳ���
//�����������������,��󵱷�������ʱ,�������ʼ�ȴ���Ӧ��Ϣ,ȡ���ڷ�����Ӧ�ó����
//��Ӧ��,ͨ�����紫���������,����״̬����������,������Щ�������ܻ��Ѵ���ʱ��
//�����û��ڵȴ����������������ҳʱ���ܻ�ı�����,������ҳ�����֮ǰ,�û������ڵ�ַ����
//д����һ����վ��Enter,��һ����������ǿͻ���Ӧ�ó����������Ӧ�ó���������
//����ʼ�ȴ���Ӧ��Ϣ,��������Ӧ�ó����ڴ���ͻ�������ʱ,������������
//���������,�û�������Զ�ȴ�

//�����������,�ͻ���Ӧ�ó�����û������ܹ������֮ǰȡ�����ǵ������Ĳ���
//ͨ��,Ϊ�û��ṩȡ�����ܻ��Ѵ���ʱ��Ĳ�����������һ�ֺõ�����,��������ͨ�Ų������ڿ��ܳ�������
//Ԥ��ĳ�ʱ���һ�����,���֧��ȡ��ͨ������ͨ�ŵķֲ�ʽӦ�ó����еĲ����ǳ���Ҫ
//Boost.Asio���ṩ���첽������һ���ô������ǿ�������������κ�ʱ��ȡ��
//
//�����㷨�ṩ��ʹ��Boost.Asio������ȡ���첽��������Ĳ���
//1.���Ӧ�ó���Ҫ��XP��2003������,�붨����Щ�汾��WIN�������첽����ȡ���ı�־
//2.���䲢��TCP��UDP�׽���,�������ǿͻ��˻������Ӧ�ó����е������򱻶�(acceptor)�׽���
//3.Ϊ�첽��������ص�������������,�����Ҫ,�ڴ˻ص���,ʵ��һ�������֧,���ڴ���ȡ������ʱ�����
//4.����һ�������첽����,��������4�ж���ĺ���ָ��Ϊ�ص�
//5.����һ��������̲߳�ʹ����������Boost.Asio�¼�ѭ��
//6.���׽��ֶ����ϵ���cancel()������ȡ������׽��ֹ���������δ��ɵ��첽����
//
//Ĭ�������ΪWindows����ʱ,Boost.Asioʹ��I/O�����Ʒ����첽���в���
//��win xp�� 2003��,�˿����ȡ�������������һЩ���������
//���,Boost.AsioҪ�󿪷���Ա������windows�汾�е�Ӧ�ó���λʱ
//��ȷ֪ͨ����ϣ�������첽����ȡ������,���ܴ�����֪����,Ϊ��,�����ڰ���Boost.Asioͷ֮ǰ
//����BOOST_ASIO_ENABLE_CANCELIO��
//����,���δ����˺�,��Ӧ�ó���������첽����,ȡ�������ͺ����ĵ���ʱ,���뽫ʼ��
//ʧ��,Ҳ����ͨ��������һ����ΪBOOST_ASIO_DISABLE_IOCO�����ֹBoost.Asioʹ�ô˿��
//���,���첽����ȡ����ص�������ʧ��,����I/O�˿ڿ�ܵĵĿ������Ժ�Ч�ʵĺô�Ҳ��ʧ��
//ע��,��Windows Vista��2008�����߰汾�ϲ��������첽����ȡ����ص��������������
//���,�ڶ�λ��Щ�汾��Winʱ,ȡ��������ѧ,����������ԭ��,�����������I/O�˿ڿ��ʹ��
//Ҫ�ڱ���ʱȷ��Ŀ�����ϵͳ,����ʹ��Boost.Predef��
//�ÿ�Ϊ�����ṩ�˺궨��,��������ʶ�𽫴������ΪĿ�����ϵͳ����汾,��������ϵ�ṹ,��������
//�Ļ�������

//��ʱ��Ҫ�õ�<thread>��
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

		//���׽����������첽���Ӳ���,�ṩ���÷����Ļص�ʵ��Ϊlambda����
		//ȡ���첽����ʱ,�����ûص�,������ָ���������Ĳ�������
		//Boost.Asio�ж����OS��ش���asio::error::operation_aborted
		//Ȼ��,��������һ����Ϊworker_thread���߳�,���߳���������Boost.Asio�¼�ѭ��
		//�ڴ��̵߳���������,�⽫���ûص�����
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
			//�˴�socket�Ѿ����Ӳ�����ʹ��
		});

		//����һ���߳�,�������첽�������ʱ���ûص�����
		//�̵߳���ڵ㺯���ǳ���,������һ��try-catch���һ����asio::io_service�����run()����
		//�ĵ���
		std::thread worker_thread([&ios]() {
			try {
				ios.run();
			}
			catch (system::system_error &e) {
				std::cout << "Error, code = " << e.code()
					<< ". Message = " << e.what();
			}
		});

		//ģ���ӳ�
		//���߳̽���������״̬2s
		//����Ϊ���������Ӳ�����΢ǰ����ģ���û���ʵ��Ӧ�ó����з�������������֮����ӳ�
		//���ڹ���,��˲���ʹ��ģ���ӳ�
		//std::this_thread::sleep_for(std::chrono::seconds(1));

		//Cancelling the initiated operation
		//�������׽��ֶ����cancel()������ȡ�����������Ӳ���
		//��ʱ,���������δ���,������ȡ��,���ҽ�ʹ��һ������������Ӧ�ص�
		//�ò���ָ������asio::error::operation_abortedֵ�Ĵ������,
		//��֪ͨ�����ѱ�ȡ��,������������Ѿ����,�����cancel()������Ч
		//������,��ʹ���µĲ���ϵͳ��,Ҳ����Ҫ����#define BOOST_ASIO_ENABLE_CANCELIO
		//����Ѿ����ӳɹ�,�������Ч
		sock->cancel();

		//�ȴ��߳����
		worker_thread.join();

	}
	catch (boost::system::system_error &e) {
		return e.code().value();
	}
	return 0;
}
//
//����һ��ʾ����,���ǿ�����ȡ����TCP�׽��ֹ������첽���Ӳ���
//����,���������Ƶķ�ʽȡ����TCP��UDP�׽�����ص��κβ���
//������������,������Ӧ���׽��ֶ����ϵ���cancel()����
//����֮��,����ͨ�����ý����������ȡ����ȡ�������첽����DNS���Ƶ�asio::ip::tcp::resolver��
//asio::ip::udp::resolver���async_resolve()����
//��Boost�ṩ����Ӧ���ɺ��������������첽����,ͨ������Ϊ��һ���������ݸ�free����
//�Ķ������cancel()����,Ҳ����ȡ��Asio,�˶�����Ա�ʾ�׽���(�����򱻶�)���������

//��ͨ��TCPЭ�����ͨ�ŵ�һЩ�ֲ�ʽӦ�ó�����,��Ҫ����û�й̶���С��
//�ض��ֽ����е���Ϣ,�����߽�,����ζ�Ž��շ��ڴ��׽��ֶ�ȡ��Ϣʱ,�޷�ͨ��������Ϣ����Ĵ�С������
//��ȷ����Ϣ�Ľ���λ��,����������һ�ַ������������ķ�ʽ����ÿ����Ϣ,�������߼�ͷ���ֺ��߼����岿�����
//���ⲿ�־��й̶��Ĵ�С,��������շ����ȶ�ȡ��������ͷ,�ҳ���Ϣ��Ĵ�С,Ȼ����ȷ
//��ȡ��Ϣ�����ಿ��

//���ַ����ǳ��򵥲��ұ��㷺ʹ��,����,������һЩ����Ͷ���ļ��㿪��
//����ĳЩ����¿����ǲ��ɽ��ܵ�.��Ӧ�ó���Ϊ���͸���Եȷ���ÿ����Ϣʹ�õ���
//���׽���ʱ,����Ӧ����һ�ַ���,����һ�ַǳ����е�����(shut down)
//���ַ������뷨���ڽ���Ϣд���׽��ֺ�����Ϣ���ͷ��ر�(shut down)�׽��ֵķ��Ͳ���
//��ᵼ������շ��������������Ϣ,֪ͨ���շ���Ϣ�ѽ���,���ͷ�����ʹ�õ�ǰ���ӷ����κ�������Ϣ
//�ڶ��ַ����ṩ�˱ȵ�һ�ַ�������ĺô�,������Ϊ����TCPЭ�������һ����,���Կ�����Ա������ʱʹ����
//�׽����ϵ���һ������,�����ܿ����������ڹر�(closing),����ʵ���������ǳ���ͬ
//�ر�(closing)�׽��ּٶ����׽��ֺ���֮����������������Դ���ظ�����ϵͳ,�����ڴ�,���̻��߳�,�ļ�����򻥳���һ��,
//�׽����ǲ���ϵͳ����Դ,���κ�������Դһ��,�׽���Ӧ�ڷ���,ʹ�ú�,�ٷ��ز���ϵͳ,����Ӧ�ó�����
//��Ҫ���׽���,������ܷ�����Դй©,�����������Դ�ľ��Լ�Ӧ�ó���Ĵ������������ϵͳ�Ĳ��ȶ�
//���׽���δ�ر�ʱ���ܷ�������������ʹ�ùرշǳ���Ҫ,�ر�(shut down)�͹ر�(closing)TCP�׽��ֵ���Ҫ��������,��������
//�ر�(closing)���ж�����,�������ͷŲ����䷵�ظ�����ϵͳ,���ر�(shut down)������д��,��ȡ����������
//���׽����ϵ����в�������Ե�Ӧ�ó����ͷ�����Ϣ,֪ͨ����ʵ,�ر�(shut down)�׽�����Զ���ᵼ���׽��ֽ������
// shut down and close a TCP socket

//�ͻ���Ӧ�ó���
//��Ŀ���Ƿ����׽��ֲ��������ӵ�������Ӧ�ó���,�������Ӻ�,Ӧ�ó���Ӧ׼��������
//������Ϣ,ͨ��������д����Ϣ��ر��׽�����֪ͨ��߽�,���������,�ͻ���Ӧ�ó���Ӧ��ȡ��Ӧ
//δ֪����Ϣ�߽�,���,Ӧ��ִ�ж�ȡ,ֱ���������ر�(shut down)���׽�����֪ͨ��Ӧ�߽�

//����һ������,�ú������ܶ����ӵ����������׽��ֶ��������,��ʹ�ô��׽��������������ͨ��
void communicate(asio::ip::tcp::socket &sock) {
	const char request_buf[] = { 0x48,0x65,0x0,0x6c,0x6c,0x6f };

	asio::write(sock, asio::buffer(request_buf));

	sock.shutdown(asio::socket_base::shutdown_send);

	asio::streambuf response_buf;

	system::error_code ec;
	asio::read(sock, response_buf, ec);

	if (ec == asio::error::eof) {
		//������Ϣ�Ѿ��յ�
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

//�����
//ּ�ڷ���������׽��ֲ���֧�صȴ���������,���������󵽴�ʱ,��Ӧ�ý���������
//���ӵ��ͻ��˵��׽��ֶ�ȡ����,ֱ���ͻ���Ӧ�ó���ر��������׽���,�յ�������Ϣ��
//������Ӧ�ó���Ӧͨ���ر��׽��ַ�����Ӧ��Ϣ֪ͨ��߽�,����ͨ��ָ��include��usingָ����
//��ʼ�ͻ���Ӧ�ó���

//����һ������,�ú������ܶ����ӵ��ͻ���Ӧ�ó�����׽��ֶ��������,��ʹ�ô��׽�����ͻ��˽���ͨ�� 
void processRequest(asio::ip::tcp::socket &sock) {
	asio::streambuf request_buf;

	system::error_code ec;

	asio::read(sock, request_buf, ec);

	std::istream input(&request_buf);

	std::string x;
	std::getline(input, x);
	std::cout << x << std::endl;

	if (ec != asio::error::eof)
		throw system::system_error(ec);

	const char response_buf[] = { 0x48,0x69,0x21 };

	asio::write(sock, asio::buffer(response_buf));

	sock.shutdown(asio::socket_base::shutdown_send);
}

int Test_processRequest() {
	//��������������Ӧ�ó���
	unsigned short port_num = 3333;
	try {
		//����,�򿪰󶨵��˿�3333,����ʼ�ȴ����Կͻ��˵Ĵ�����������
		//Ȼ�������ͻ���Ӧ�ó���,�򿪲����ӵ�������,�������Ӻ�,����communic()����
		//�ͻ���Ӧ�ó�������д���׽���,Ȼ������׽��ֵ�shutdown()����,����asio::socket_base::shutdown_send������Ϊ��������
		//�˵��ý��ر��׽��ֵķ��Ͳ���,��ʱ,����д���׽���,
		//�����޷��ָ��׽���״̬��ʹ���ٴο�д:
		//sock.shutdown(asio::socket_base::shutdown_send);
		//�رտͻ���Ӧ�ó����е��׽����ڷ�����Ӧ�ó����п��Կ����ǵ����������
		//Э�������Ϣ,֪ͨ�Ե�Ӧ�ó����ѹر��׽��ֵ���ʵ
		//Boost.Asioͨ��asio::read()�������صĴ�����뽫����Ϣ���ݸ�Ӧ�ó������
		//Boost.Asio�⽫��code����Ϊasio::error::eof
		//������Ӧ�ó���ʹ�ô˴�������������ͻ��˺�ʱ��ɷ���������Ϣ
		//���������յ�����������Ϣʱ,�������Ϳͻ��˽������ɫ
		//����������ɽ���Ӧ��Ϣд���׽���ʱ,����ر����׽��ֵķ��Ͳ���
		//�԰�ʾ������Ϣ�ѷ��͸���Ե���,ͬʱ�ͻ���Ӧ�ó�����asio::read()����
		//�б�����,����ȡ���������͵���Ӧ,ֱ���������ش������==asio::error::eof,
		//����ζ�ŷ���������ɷ�����Ӧ��Ϣ,��asio::read()�������ش˴������ʱ
		//�ͻ���֪�����Ѷ�ȡ������Ӧ��Ϣ,Ȼ�������Կ�ʼ������

		//[ע��]�ڿͻ��˹ر����׽��ֵķ��Ͳ��ֺ�,����Ȼ���Դ��׽��ֶ�ȡ����
		//��Ϊ�׽��ֵĽ��ղ��ֶ����ڷ��Ͳ��ֱ��ִ�״̬
		asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),
			port_num);

		asio::io_service ios;
		asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
		//֮ǰ����listenû����ȷ�Ĳ���,����������bind��
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
//Ϊ�˹ر��ѷ�����׽���,Ӧ����asio::ip::tcp::socket�����Ӧ�����ϵ���close()����
//����,ͨ������Ҫ��ʽִ����,��Ϊ���û����ʽ�ر��׽��ֶ������������,���׽��ֶ��������������ر��׽���


//==== Chapter 3 ====
//һ�����͵�tcpͬ���ͻ��˰��������㷨����
//1.��ȡIP��ַ�ͷ�����Ӧ�ó����Э��˿ں�
//2.����һ����׽���
//3.�������Ӧ�ó���������
//4.�������������Ϣ
//5.�ر�����
//6.ȡ�������׽���

//
class SyncTCPClient {
public:
	SyncTCPClient(const std::string & raw_ip_address,
		unsigned short port_num)
		:m_ep(asio::ip::address::from_string(raw_ip_address), port_num),
		m_sock(m_ios) {
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
		//������Ҫע��:
		//read_until���ݸ�buf�Ŀ��ܰ���\n֮����ַ�,
		//���,ʹ��\n��Ϊ��ʶ�������getlineֱ�Ӻ��Ի���֮����޹��ַ�
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
	static int num = 1;
	std::cout << "Num = " << num++ << std::endl;
	const std::string raw_ip_address = "127.0.0.1";
	const unsigned short port_num = 3333;

	try {
		SyncTCPClient client(raw_ip_address, port_num);

		client.connect();

		std::cout << " > Sending request to the server..."
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
//���͵�UDPͬ���ͻ��˸��������㷨����
//1.��ȡIP��ַ�Ϳͻ���Ӧ�ó���Ҫ��֮ͨ�ŵ�ÿ����������Э��˿ں�
//2.����UDP�׽���
//3.�������������Ϣ
//4.ȡ�������׽���

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
		//ע��
		//receive_from��һ��ͬ������,��ֹ�߳�,ֱ�����ݱ���ָ���ķ���������
		//������ݱ���Զ���ᵽ��,�÷�����Զ����������,����Ӧ�ó��򽫹���
		//����ӷ�������������ݱ���С�����ṩ�Ļ������Ĵ�С,��÷�����ʧ��
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

//�첽TCP�ͻ���
//����ص���������
typedef void(*Callback)(unsigned int resquest_id,
	const std::string &response,
	const system::error_code &ec);
//����һ�����ݽṹ,Ŀ������ִ��ʱ������������ص�����,����ΪSession
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
	//������Ա����õ�ԭ��:
	//��һ���첽�����Ѿ���ɲ�����δ������һ���첽����ʱ,�п��ܻ����cancelRequest()
	//����,����,����I/O�߳����������������ض��׽��ֹ�����async_connect�����Ļص�
	//��ʱ,û������׽��ֹ������첽�������ڽ�����,��Ϊ��δ������һ���첽����async_write()
	//���,�ڴ��׽����ϵ���cancel()����������,�����ñ��
	bool m_was_cancelled;
	//ͨ��Mutex��������ȷ��
	//���������m_was_cancelled������֮ǰ������һ���첽��������ʼ��֮��
	std::mutex m_cancel_guard;
};
//����������һ���ṩ�첽ͨ�Ź��ܵ���
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
					session->m_response_buf, '\n',
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

	void close() {
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
			session->m_response, ec);
	}
private:
	asio::io_service m_ios;
	std::map<int, std::shared_ptr<Session>> m_active_sessions;
	std::mutex m_active_sessions_guard;
	//asio::io_service::work��Ķ���ʹ�����¼�ѭ�����̲߳����˳���ѭ��
	std::unique_ptr<boost::asio::io_service::work> m_work;
	std::unique_ptr<std::thread> m_thread;
};
//����,����һ������Ϊ�ص�����,���䴫�ݸ�AsyncTCPClient::enmulateLongComputationOp()����
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
	static int num = 1;
	std::cout << "Num = " << num++ << std::endl;
	try {
		AsyncTCPClient client;

		client.emulateLongComputationOp(10, "127.0.0.1", 3333,
			handler, 1);

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

//���бȼ�����а�װ�ĺ��Ļ���������������̵߳�Ӧ�ó���
//���������߳��л�������Ӱ���ʹӦ�ó��������

//Ҫʵ�ֶ��߳�,��Ҫ�滻��ʾ����I/O�̵߳�AsyncTCPClient���m_thread��Ա,�Լ�
//ָ��std::thread�����ָ���б�,������ʾI/O�̵߳ļ���
//std::list<std::unique_ptr<std::thread>> m_threads;
//������,������Ĺ��캯��,�Ա�������һ����ʾҪ�������߳���
//����,���캯��Ӧ������ָ��������I/O�߳�,��������ȫ����ӵ�����
//�¼�ѭ�����̳߳���p140

//�����̰߳汾һ��,ÿ���̶߳�����m_ios�����run()����,���,�����̶߳�
//����ӵ��̳߳���,��m_ios�������,���������߳̽����ڵ�����Ӧ�첽������ɻص�
//����ζ���ڶ�˻�ദ�����������,�����ڲ�ͬ���߳���ͬʱ���ж���ص�
//ÿ���ص����ڵ����Ĵ�������,�����close()���������̳߳�ÿ����ִ��join


//====CH.3====
//ʵ��һ����������Ӧ�ó���
//
//�����ض��������ṩ�ķ���,�����������ζ�Ų�ͬ������
//����:HTTP������ͨ�����ȡ������Ϣ��ָ�����ļ�������,�����䷢�ͻؿͻ���
//���������ֻ�ǽ��ͻ��˵������ض��򵽲�ͬ�ķ������Խ���ʵ�ʴ���(���߿���
//����һ���ض���)����������ķ����������ṩ�Կͻ������������ṩ������ִ�и���
//����ķ���,������Щ����Ľ�����ظ��ͻ���,�������з�����������ݱ�����ɫ
//ĳЩ������Ӧ�ó�����ܻ���ͻ��˷�����Ϣ,������ȴ��ͻ������ȷ�������
//ͨ��,����������䵱֪ͨ����,��֪ͨ�ͻ���һЩ��Ȥ���¼�,�����������,�ͻ���
//���ܸ�������Ҫ������������κ�����,�෴,���Ǳ����ȴ����Է�����֪ͨ�����Ѿ��յ�֪ͨ,
//���ǻ�������Ӧ�ķ�Ӧ,����ͨ��ģ�ͳ�Ϊ����ͨ��,��ģ�����ִ�webӦ�ó�����Խ��Խ����
//�ṩ�˶���������,���,�Է�����Ӧ�ó�����з���ĵ�һ�ַ�����ͨ������ִ�еĹ���
//������Ϊ�ͻ��ṩ�ķ���,��һ�����Եķ���ά���Ǵ����Э��
//TCP����ǳ�����,���ͨ�÷�����Ӧ�ó���ʹ��������ͨ��,�����������
//����������ʹ��UDPЭ��,ͬʱͨ��TCP��UDPЭ���ṩ����Ļ�Ϸ�����Ӧ�ó���
//���ڶ�Э�������
//����������һ����������Ϊ�ͻ��ṩ����ķ�ʽ��
//������������һ��һ�ķ�ʽΪ�ͻ����ṩ��������ζ���������Ϊ��ǰ����Ŀͻ����ṩ����֮ǰ���ῪʼΪ��һ���ͻ����ṩ����
//���з��������Բ��з������ͻ���
//�ڵ�������������ϣ����з��������ڵ������������������ǵĶ���ͻ��˽���ͬ��ͨ�Ž׶�
//���磬���ӵ�һ���ͻ��˲��ڵȴ���������������Ϣʱ�������������л������ӵڶ����ͻ��ˣ����ߴӵ������ͻ��˶�ȡ����
//�ڴ�֮���������л��ص�һ���ͻ��˼����ṩ����
//���ֲ����Գ�Ϊα�����ԣ���Ϊ���������ڼ����ͻ���֮���л�������������ͬʱΪ���Ƿ�������ڵ����������ǲ����ܵġ�
//�ڶദ����������ϣ���������Ϊÿ���ͻ���ʹ�ò�ͬ��Ӳ���߳�ͬʱΪ����ͻ����ṩ����ʱ�������Ĳ������ǿ��ܵġ�

//����������ʵ��������Լ�,���ҿ��������������㹻��ʱʹ��,�Ա������
//��ʱ������һ�����󵽴�֮ǰ��ɴ���һ������,������,����������������չ
//�����д���������ļ������Ӹ��ദ�����������ӷ�������������
//��һ����,���з��������Դ�����ߵ�������,���ʵʩ�õ�,�����ǿ���չ��
//�ڶദ��������������е��������з��������Դ�����ڵ�����������������е�ͬһ������
//���ߵ�������,��ʵ�ֵĽǶ�����,�Է�����Ӧ�ó�����з������һ�ַ����Ǹ��ݷ�������ͬ������
//�첽,ͬ��������ʹ��ͬ���׽���API��������ִֹ���߳�,ֱ������Ĳ������,����ᷢ������
//��˵��͵�ͬ��TCP��������ʹ��asio::ip::tcp::acceptor::accept()�ӿͻ��˽���������Ϣ,Ȼ��asio::
//ip::tcp::socket::read_some()�ӿͻ��˽���������Ϣ,Ȼ��asio::ip::tcp::socket::write_some()
//����Ӧ��Ϣ���ͻؿͻ���,
//�����ַ������Ƿ�����,������ִֹ�е��߳�,ֱ������Ĳ������,��������,��ʹ�÷�����ʹ����Щ����ͬ��

//��ͬ��������Ӧ�ó����෴,�첽������Ӧ�ó���ʹ���첽�׽���API����,����
//�첽TCP����������ʹ��asio::ip::tcp::acceptor::accept()�����첽���ܿͻ�����������
//asio::ip::tcp::socket::async_read_some()������asio::async_read()���ɺ����ӿͻ����첽����������Ϣ
//Ȼ��asio::ip::tcp::socket::async_write_some()������asio::async_write()���ɺ���
//�첽������Ӧ��Ϣ�ؿͻ���

//����ͬ��������Ӧ�ó���Ľṹ���첽������Ӧ�ó���Ľṹ���Բ�ͬ,���Ӧ���ڷ�����Ӧ�ó�����ƽ׶ξ���
//����Ӧ�����ַ���,�˾�Ӧ���ڶ�Ӧ�ó���Ҫ�����ϸ����,����,Ӧ���ǲ����ǿ��ܳ��ֵ�Ӧ��
//�ݽ�·������Ҫ��,ͨ��ÿ�ַ���������ȱ��,��ͬ��������һ������²������õĽ��ʱ,����һ���������������
//���Բ��ɽ��ܵ�,�����������,�첽������������ȷ��ѡ��

//���߶Ա�:
//���첽�������,ͬ����������Ҫ�ŵ��������,�빦����ͬ���첽���������
//ͬ��������������ʵ��,���Ժ�֧��,�첽������������,��Ϊ����ʹ���첽�����ڴ����е�����λ����ɶ��������Ǳ�
//����,ͨ��,����Ҫ�ڿ����ڴ��з����������ݽṹ�Ա��������������,ʵ�ֻص���������,�߳�ͬ���Լ�����ʹӦ�ó���
//�ṹ�ǳ����������׳����������������,

//ͬ���������в���Ҫ�������Щ���⹦��,�����첽�������������ļ�����ڴ濪��
//�����ʹ����ĳЩ����±�ͬ������Ч�ʵ�,Ȼ��,ͬ����������һЩ��������,��ͨ��ʹ�䲻�ɽ���,��Щ���ư����޷���������
//ȡ��ͬ������,��Ϊ����䳬ʱ,�Ա�������ʱ�����ʱ�ж�
//��ͬ�������෴,������������������κ�ʱ��ȡ���첽����,ͬ�������޷�ȡ������ʵ����������ͬ��������Ӧ�õķ�Χ
//ʹ��ͬ�������Ĺ����������������ܵ�����

//ʹ���첽�����ķ�������ͬ������������Ч�Ϳ���չ,�����ǵ������ھ��б���֧���첽����I/O�Ĳ���ϵͳ�Ķദ��
//�������������ʱ

//ͬ������TCP���������������㷨:
//1.����һ���������׽��ֲ�����󶨵�һ���ض���TCP�˿���
//2.����ѭ��ֱ��������ֹͣ
////1.�ȴ����Կͻ��˵���������
////2.����ʱ���ܿͻ��˵���������
////3.�ȴ����Կͻ��˵�������Ϣ
////4.�Ķ�������Ϣ
////5.��������
////6.����Ӧ��Ϣ���͸��ͻ���
////7.�ر���ͻ��˵����Ӳ�ȡ�������׽���

//p147

class Service {
public:
	Service() {}

	//Boost.Asio I/O�����ͷ��������׳��쳣��HandleClient()
	//�����б�����ʹ���,���Ҳ��ᴫ��������������
	//������һ���ͻ��˵Ĵ���ʧ��,����������������
	void HandleClient(asio::ip::tcp::socket &sock) {
		try
		{
			asio::streambuf request;

			asio::read_until(sock, request, '\n');

			std::istream input(&request);
			std::string x;
			getline(input, x);
			std::cout << x << std::endl;

			int i = 0;
			while (i != 100000)
				i++;
			;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			//
			std::string response = "Response\n";
			asio::write(sock, asio::buffer(response));

		}
		catch (boost::system::system_error &e)
		{
			std::cout << "Error occured! "
				<< "Code = " << e.code()
				<< " Message = " << e.what();

		}

	}
};

//������������һ����ʾ�߼��������������

class Acceptor {
public:
	Acceptor(asio::io_service &ios, unsigned short port_num) :
		m_ios(ios), m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num))
	{
		//��ʼ�������Կͻ��˵���������
		m_acceptor.listen();
	}

	void Accept() {
		asio::ip::tcp::socket sock(m_ios);

		//����п��õĹ�����������,������������
		//������׽���sock���ӵ��¿ͻ���
		//����˷���������,ֱ���µ��������󵽴�
		m_acceptor.accept(sock);

		Service svc;
		svc.HandleClient(sock);
	}
private:
	asio::io_service &m_ios;
	asio::ip::tcp::acceptor m_acceptor;
};

//�������������
class Server
{
public:
	Server() :m_stop(false) {}

	//��������
	void Start(unsigned short port_num) {
		m_thread.reset(new std::thread([this, port_num]() {
			Run(port_num);
		}));
	}

	//��ֹ���ó����߳�ֱ��������ֹͣ
	//ȱ��:����Stop������Զ���᷵��
	//����Ҫ����:��������������ֹͣ
	//Stop��������Զ��ֹ�������
	//�������Stop���������ڼ��Run()�����е�ѭ����ֹ֮ǰ
	//��ԭ�ӱ���m_stop��ֵ����Ϊtrue,���������������ֹͣ���Ҳ����������
	//����,�������Stop()����ͬʱ��acc.Accept()����������
	//�ȴ����Կͻ��˵���һ����������,����Service���еȴ���һ��ͬ��i/o������
	//���ӵĿͻ��˵�������Ϣ,��ͻ��˽�����Ӧ��Ϣ,�������Щ��������֮ǰ
	//�������޷�ֹͣ.�������,�����ʱ����Stop()����ʱ,û�й������������
	//�����¿ͻ������Ӳ�����֮ǰ����������ֹͣ,ֱ��һ��client���Ӳ�������
	//���·�������Զ����ֹ
	//
	void Stop() {
		m_stop.store(true);
		m_thread->join();
	}
private:
	std::unique_ptr<std::thread>m_thread;
	std::atomic<bool> m_stop;
	asio::io_service m_ios;

	void Run(unsigned short port_num) {
		Acceptor acc(m_ios, port_num);

		while (!m_stop.load()) {
			acc.Accept();
		}
	}
};
int C4_SYN_TCP_Server() {
	unsigned short port_num = 3333;
	try {
		Server srv;
		srv.Start(port_num);

		std::this_thread::sleep_for(std::chrono::seconds(60));

		srv.Stop();
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "Error occured! "
			<< "Code = " << e.code()
			<< " Message = " << e.what();
		return e.code().value();
	}
	return 0;
}
//��������������
//1.����ڷ������̱߳������ȴ�������������ʱ������Stop()����,������޷�ֹͣ
//2.�����������ױ���������ͻ��˹���,ʹ��������ͻ��˲�����(ֻ���Ӳ�������Ϣ����)
//�����Ҽ򵥵Ľ��������Ϊ����������ʱ,��α�֤����������ȡ�������Լ���Ƿ��ѷ���ֹͣ
//����,���һ�ǿ�ƶ�������������Ŀͻ���
//����,BoostAsioû���ṩȡ��ͬ��������Ϊ����䳬ʱ�ķ���,���,Ӧ�ó���Ѱ������������
//ʹͬ��������������Ӧ�Ժ��ȶ���

//��û�й������������ʱ,ʹ�������׽��ֵ�acceptͬ���������������Ψһ�Ϸ�������
//����������������Ķ˿ڷ��������������,���ǿ���ִ�����¼��������
//��Server���Stop������,�ڽ�m_stopԭ�ӱ�����ֵ����Ϊtrue֮��,���Դ���
//һ�������׽���,ʹ�����ӵ�ͬһ��������,������һЩ��������
//�⽫��֤�������߳̽��뿪acceptor�׽��ֵ�accept()����,�����ռ��m_stopԭ�ӱ�����ֵ
//���ҳ���ֵ����true,�⽫ѭ����ֹ�����

//Acceptor::Accept()����,���������ķ�����,���������ͨ��������������Ϣ��ֹͣ����(ʵ������Ϣ��I/O
//�̷߳��͵������߳�)
//��һ�ַ�����ӵ��һ������Ŀͻ���(������Ӧ�ó���),�������Ӳ���������������������Ϣ(����:stop\n)
//�������������ΪҪֹͣ���ź�,�����������,�����������ⲿ(���Բ�ͬ��Ӧ�ó���)
//���п���,��Server�಻��Ҫ����Stop()����


//ʵ��һ��ͬ�����е�TCP������
//���͵�ͬ������TCP���������������㷨����
//1.����������׽��ֲ�����󶨵��ض���TCP�˿�
//2.����ѭ��ֱ��������ֹͣ
////3.�ȴ����Կͻ��˵Ĵ�����������
////4.���ܿͻ��˵���������
////5.������̵߳��������в���һ�������߳�
//////6.�ȴ����Կͻ��˵�������Ϣ
//////7.�Ķ�������Ϣ
//////8.��������
//////9.��ͻ��˷�����Ӧ��Ϣ
//////10�ر���ͻ��˵����Ӳ��ͷ��׽���


class Service2 {
public:
	Service2() {};

	void StartHandligClient(std::shared_ptr<asio::ip::tcp::socket>sock)
	{
		std::thread th(([this, sock]() {
			HandleClient(sock);
		}));

		th.detach();

	}
private:
	void HandleClient(std::shared_ptr<asio::ip::tcp::socket>sock) {
		try {
			asio::streambuf request;
			asio::read_until(*sock.get(), request, '\n');

			std::istream input(&request);
			std::string x;
			getline(input, x);
			std::cout << x << std::endl;

			int i = 0;
			while (i != 100000)
				i++;

			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			std::string response = "Response\n";
			asio::write(*sock.get(), asio::buffer(response));

		}
		catch (boost::system::system_error &e) {

		}

		//�������
		delete this;
	}
};

//������,������һ����ʾ�߼��������������
//���ฺ��������Կͻ��˵���������ʵ����Service��Ķ���,�⽫Ϊ���ӵĿͻ���
//�ṩ����
class Acceptor2 {
public:
	Acceptor2(asio::io_service &ios,
		unsigned short port_num)
		:m_ios(ios), m_acceptor
		(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num)) {
		m_acceptor.listen();
	}


	//����˷�����û��acceptʱ����������,ֻ����һ���ж������accept
	//һ��ִ��Accept����������,��Ϊʹ���˷����߳�ȥ����
	//
	//�����һ��ͬ�����̴߳�������,���Accept������Ϊ���ô�������������
	//��˲�����ֱ�ӵ���HandleClient��,������һ��������һ���µ��߳�ȥ
	//���ò������������߳�(���������ִ����delete�ͷ�����)
	//��Accept�е�new ��Ӧ
	void Accept() {
		//��Ϊ�ײ��ǿ������µ��̲߳����Ƿ����,���ʹ��������ָ���sock
		std::shared_ptr<asio::ip::tcp::socket>sock(new asio::ip::tcp::socket(m_ios));

		m_acceptor.accept(*sock);

		(new Service2)->StartHandligClient(sock);
	}

private:
	asio::io_service &m_ios;
	asio::ip::tcp::acceptor m_acceptor;
};
//��������Server2

class Server2 {
public:
	Server2() :m_stop(false) {}

	void Start(unsigned short port_num) {
		m_thread.reset(new std::thread([this, port_num]() {
			Run(port_num);
		}));

	}
	/*�������������ܲ�������ֹͣ��
	����Ҫ���ǣ���������������ֹͣ��Stop������������Զ��ֹ�������*/
	//�������ͬ��һ��ͬ��������
	void Stop() {
		m_stop.store(true);
		m_thread->join();
	}
private:
	void Run(unsigned short port_num) {
		Acceptor2 acc(m_ios, port_num);

		while (!m_stop.load()) {
			acc.Accept();
		}
	}

	std::unique_ptr<std::thread>m_thread;
	std::atomic<bool>m_stop;
	asio::io_service m_ios;
};
//
int C4_SYN_TCP_MServer() {
	unsigned short port_num = 3333;

	try {
		Server2 srv;
		srv.Start(port_num);

		std::this_thread::sleep_for(std::chrono::seconds(60));

		srv.Stop();
	}
	catch (boost::system::system_error &e) {
		return e.code().value();
	}
	return 0;
}
int Test_C3_C4_1() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {

		C4_SYN_TCP_Server();
	}
	else {
		std::thread x1(C3_SYN_TCP_CLIENT);
		std::thread x2(C3_SYN_TCP_CLIENT);
		std::thread x3(C3_SYN_TCP_CLIENT);
		x1.join();
		x2.join();
		x3.join();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
	}
	return 0;
}
int Test_C3_C4_2() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {

		C4_SYN_TCP_MServer();
	}
	else {
		std::thread x1(C3_SYN_TCP_CLIENT);
		std::thread x2(C3_SYN_TCP_CLIENT);
		std::thread x3(C3_SYN_TCP_CLIENT);
		x1.join();
		x2.join();
		x3.join();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
	}
	return 0;
}
//ʵ��һ���첽��TCP������

//���͵��첽TCP���������������㷨����
//1.����������׽��ֲ�����󶨵��ض���TCP�˿�
//2.�����첽���ܲ���
//3.����һ�����������߳�,����������ӵ�����Boost.Asio�¼�ѭ�����̳߳���
//4.�첽���ܲ�����ɺ�,����һ���µĲ����Խ�����һ����������
//5.�����첽��ȡ�����Դ����ӵĿͻ��˶�ȡ����
//6.�����첽��ȡ������ɺ�,��������׼����Ӧ��Ϣ
//7.�����첽д���������Ӧ��Ϣ���͵��ͻ���
//8.�첽д�������ɺ�,�ر����Ӳ�ȡ�������׽���
//ע��:ȡ���ھ���Ӧ���еľ����첽��������Զ�ʱ,������������˳��ִ�д�ǰ
//���㷨�еĵ��Ĳ���ʼ�Ĳ���
//p163


class Service3 {
public:
	Service3(std::shared_ptr<asio::ip::tcp::socket> sock)
		:m_sock(sock) {}

	void StartHandling() {
		asio::async_read_until(*m_sock, m_request,
			'\n',
			[this](const boost::system::error_code &e,
				std::size_t bytes_transferred) {
			onRequestReceived(e, bytes_transferred);


		});

	}
private:
	void onRequestReceived(const boost::system::error_code &ec, std::size_t bytes_transferred) {
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
			onFinish();
			return;
		}

		m_response = ProcessRequest(m_request);

		asio::async_write(*m_sock,
			asio::buffer(m_response),
			[this](
				const boost::system::error_code &ec,
				std::size_t bytes_transferred) {
			onResponseSent(ec,
				bytes_transferred);


		});
	}

	void onResponseSent(const boost::system::error_code&ec,
		std::size_t bytes_transferred) {
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();


		}
		onFinish();
	}

	void onFinish() {
		delete this;
	}

	std::string ProcessRequest(asio::streambuf &request) {
		int i = 0;
		while (i != 200000)
			i++;

		std::string response = "Response\n";
		return response;
	}
private:
	std::shared_ptr<asio::ip::tcp::socket> m_sock;
	std::string m_response;
	asio::streambuf m_request;
};
#define DMESSAGE(x)\
{\
std::cout <<"DEBUG: "<< x << std::endl;\
}
//����
namespace HTTP_SERVER {


	class Service {
		static const std::map<unsigned int, std::string>
			http_status_table;

	public:
		Service(std::shared_ptr<boost::asio::ip::tcp::socket> sock) :
			m_sock(sock),
			m_request(4096),
			m_response_status_code(200), // Assume success.
			m_resource_size_bytes(0)
		{};


		void start_handling() {
			DMESSAGE("start_handling");
			asio::async_read_until(*m_sock.get(),
				m_request,
				"\r\n",
				[this](
					const boost::system::error_code& ec,
					std::size_t bytes_transferred)
			{
				DMESSAGE("BEFORE: on_request_line_received");
				on_request_line_received(ec,
					bytes_transferred);
			});
		}

	private:
		void on_request_line_received(
			const boost::system::error_code& ec,
			std::size_t bytes_transferred)
		{
			if (ec.value() != 0) {
				std::cout << "Error occured! Error code = "
					<< ec.value()
					<< ". Message: " << ec.message();
				if (ec == asio::error::not_found) {
					// No delimiter has been found in the
					// request message.
					m_response_status_code = 413;
					send_response();
					return;
				}
				else {
					// In case of any other error �C
					// close the socket and clean up.
					on_finish();
					return;
				}
			}
			// Parse the request line.
			std::string request_line;
			DMESSAGE("request_line = " + request_line);
			std::istream request_stream(&m_request);
			std::getline(request_stream, request_line, '\r');
			// Remove symbol '\n' from the buffer.
			request_stream.get();
			// Parse the request line.
			std::string request_method;
			std::istringstream request_line_stream(request_line);
			request_line_stream >> request_method;
			// We only support GET method.
			if (request_method.compare("GET") != 0) {
				// Unsupported method.
				m_response_status_code = 501;
				send_response();
				return;
			}
			request_line_stream >> m_requested_resource;
			std::string request_http_version;
			request_line_stream >> request_http_version;
			if (request_http_version.compare("HTTP/1.1") != 0) {
				// Unsupported HTTP version or bad request.
				m_response_status_code = 505;
				send_response();
				return;
			}
			// At this point the request line is successfully
			// received and parsed. Now read the request headers.
			asio::async_read_until(*m_sock.get(),
				m_request,
				"\r\n\r\n",
				[this](
					const boost::system::error_code& ec,
					std::size_t bytes_transferred)
			{
				on_headers_received(ec,
					bytes_transferred);
			});
			return;
		}
		void on_headers_received(const boost::system::error_code& ec,
			std::size_t bytes_transferred)
		{
			if (ec.value() != 0) {
				std::cout << "Error occured! Error code = "
					<< ec.value()
					<< ". Message: " << ec.message();
				if (ec == asio::error::not_found) {
					// No delimiter has been fonud in the
					// request message.
					m_response_status_code = 413;
					send_response();
					return;
				}
				else {
					// In case of any other error - close the
					// socket and clean up.
					on_finish();
					return;
				}
			}
			// Parse and store headers.
			std::istream request_stream(&m_request);
			std::string header_name, header_value;
			while (!request_stream.eof()) {
				std::getline(request_stream, header_name, ':');
				if (!request_stream.eof()) {
					std::getline(request_stream,
						header_value,
						'\r');
					// Remove symbol \n from the stream.
					request_stream.get();
					m_request_headers[header_name] =
						header_value;
				}
			}
			// Now we have all we need to process the request.
			process_request();
			send_response();
			return;
		}

		void process_request() {
			// Read file.
			std::string resource_file_path =
				std::string("Z:\\github\\Bangumi_for_qq\\") +
				m_requested_resource;
			if (!boost::filesystem::exists(resource_file_path)) {
				// Resource not found.
				m_response_status_code = 404;
				return;
			}
			std::ifstream resource_fstream(
				resource_file_path,
				std::ifstream::binary);
			if (!resource_fstream.is_open()) {
				// Could not open file.
				// Something bad has happened.
				m_response_status_code = 500;

				return;
			}
			// Find out file size.
			resource_fstream.seekg(0, std::ifstream::end);
			m_resource_size_bytes =
				static_cast<std::size_t>(
					resource_fstream.tellg());
			m_resource_buffer.reset(
				new char[m_resource_size_bytes]);
			resource_fstream.seekg(std::ifstream::beg);
			resource_fstream.read(m_resource_buffer.get(),
				m_resource_size_bytes);
			m_response_headers += std::string("content-length") +
				": " +
				std::to_string(m_resource_size_bytes) +
				"\r\n";
		}

		void send_response() {
			m_sock->shutdown(
				asio::ip::tcp::socket::shutdown_receive);
			auto status_line =
				http_status_table.at(m_response_status_code);
			m_response_status_line = std::string("HTTP/1.1 ") +
				status_line +
				"\r\n";
			m_response_headers += "\r\n";
			std::vector<asio::const_buffer> response_buffers;
			response_buffers.push_back(
				asio::buffer(m_response_status_line));
			if (m_response_headers.length() > 0) {
				response_buffers.push_back(
					asio::buffer(m_response_headers));
			}
			if (m_resource_size_bytes > 0) {
				response_buffers.push_back(
					asio::buffer(m_resource_buffer.get(),
						m_resource_size_bytes));
			}
			// Initiate asynchronous write operation.
			asio::async_write(*m_sock.get(),
				response_buffers,
				[this](
					const boost::system::error_code& ec,
					std::size_t bytes_transferred)
			{
				on_response_sent(ec,
					bytes_transferred);
			});
		}
		void on_response_sent(const boost::system::error_code& ec,
			std::size_t bytes_transferred)
		{
			if (ec.value() != 0) {
				std::cout << "Error occured! Error code = "
					<< ec.value()
					<< ". Message: " << ec.message();
			}
			m_sock->shutdown(asio::ip::tcp::socket::shutdown_both);
			on_finish();
		}

		// Here we perform the cleanup.
		void on_finish() {
			delete this;
		}


	private:
		std::shared_ptr<boost::asio::ip::tcp::socket> m_sock;
		boost::asio::streambuf m_request;
		std::map<std::string, std::string> m_request_headers;
		std::string m_requested_resource;
		std::unique_ptr<char[]> m_resource_buffer;
		unsigned int m_response_status_code;
		std::size_t m_resource_size_bytes;
		std::string m_response_headers;
		std::string m_response_status_line;

	};
	const std::map<unsigned int, std::string>
		Service::http_status_table =
	{
		{ 200, "200 OK" },
		{ 404, "404 Not Found" },
		{ 413, "413 Request Entity Too Large" },
		{ 500, "500 Server Error" },
		{ 501, "501 Not Implemented" },
		{ 505, "505 HTTP Version Not Supported" }
	};

	void Test_Server() {
		asio::io_service ios;
		std::shared_ptr<boost::asio::ip::tcp::socket> sock = std::make_shared<boost::asio::ip::tcp::socket>(ios);

		try {
			asio::ip::tcp::acceptor acc(ios);
			unsigned short port_num = 3333;

			asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

			acc.open(ep.protocol());

			acc.bind(ep);

			acc.listen();

			Service sr(sock);
			acc.accept(*sock);

			sr.start_handling();

			ios.run();
		}
		catch (boost::system::system_error &e) {
			std::cout << "Error Code = " << e.code()
				<< ", Message = " << e.what();
		}


	}



}

//

class Acceptor3 {
public:
	Acceptor3(asio::io_service& ios, unsigned short port_num)
		:m_ios(ios),
		m_acceptor(m_ios, asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port_num))
	{

	}

	void Start() {
		m_acceptor.listen();
		InitAccept();
	}

	void Stop() {
		m_isStopped.store(true);
	}
private:
	void InitAccept() {
		std::shared_ptr<asio::ip::tcp::socket>
			sock(new asio::ip::tcp::socket(m_ios));

		m_acceptor.async_accept(*sock,
			[this, sock](const boost::system::error_code& error) {

			onAccept(error, sock);
		});

	}

	void onAccept(const boost::system::error_code&ec,
		std::shared_ptr<asio::ip::tcp::socket>sock) {
		if (ec.value() == 0) {
			(new HTTP_SERVER::Service(sock))->start_handling();
		}

		else {
			std::cout << "Error code = "
				<< ec.value() << " Message = "
				<< ec.message();
		}

		if (!m_isStopped.load()) {
			InitAccept();
		}
		else {
			m_acceptor.close();
		}
	}
private:
	asio::io_service &m_ios;
	asio::ip::tcp::acceptor m_acceptor;
	std::atomic<bool> m_isStopped;
};

class Server3 {
public:
	Server3() {
		m_work.reset(new asio::io_service::work(m_ios));
	}

	void Start(unsigned short port_num,
		unsigned int thread_pool_size) {
		assert(thread_pool_size > 0);

		acc.reset(new Acceptor3(m_ios, port_num));
		acc->Start();

		for (unsigned int i = 0; i < thread_pool_size; i++) {
			std::unique_ptr<std::thread> th(
				//https://blog.csdn.net/jlusuoya/article/details/75299096
				//�����б�
				new std::thread([this]() {
				m_ios.run();
			})
			);
			m_thread_pool.push_back(std::move(th));
		}
	}

	void Stop() {
		acc->Stop();
		m_ios.stop();

		for (auto&th : m_thread_pool) {
			th->join();
		}
	}
private:
	asio::io_service m_ios;
	std::unique_ptr<asio::io_service::work>m_work;
	std::unique_ptr<Acceptor3>acc;
	std::vector<std::unique_ptr<std::thread>>m_thread_pool;
};
int C4_ASYN_TCP_MServer() {
	const unsigned int DEFAULT_THREAD_POOL_SIZE = 2;

	unsigned short port_num = 3333;

	try {
		Server3 srv;

		//ͨ���ڲ���Ӧ�ó��������ڲ�������߳�������ͨ�ù�ʽ�Ǽ��������2�Ĵ�������
		//ʹ��std::thread::hardware_concurrency��̬��������ȡ������������
		//����,�˷������ܷ���0
		unsigned int thread_pool_size =
			std::thread::hardware_concurrency() * 2;

		if (thread_pool_size == 0)
			thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

		srv.Start(port_num, thread_pool_size);

		std::this_thread::sleep_for(std::chrono::seconds(60));

		srv.Stop();
	}
	catch (system::system_error&e) {
		std::cout << "Error code = "
			<< e.code() << ", Message = "
			<< e.what();

		return e.code().value();
	}

	return 0;
}
int Test_C3_C4_3() {
	std::cout << "Select Your Func:"
		<< "\nTest_First_Client_TCP = 0"
		<< "\nTest_First_Server_TCP = 1"
		<< std::endl;
	unsigned IServer;
	std::cin >> IServer;
	if (IServer != 0) {

		C4_ASYN_TCP_MServer();
	}
	else {
		std::thread x1(C3_ASYN_TCP_CLIENT);
		std::thread x2(C3_ASYN_TCP_CLIENT);
		std::thread x3(C3_ASYN_TCP_CLIENT);
		x1.join();
		x2.join();
		x3.join();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
		//C3_SYN_TCP_CLIENT();
	}
	return 0;
}


//====CH.5====

//HTTPЭ������TCPЭ��֮�����е�Ӧ�ò�Э��
//����ͻ���Ӧ�ó���ӷ����������ض�����Դ,���������������Դ�����
//�ͻ���
//��ΪGET����򵥷����ٶ������¼���
//1.HTTP�ͻ���Ӧ�ó�������������Ϣ,����Ϣ�����й�Ҫ������ض���Դ
//����Ϣ�����䷢�͵�ʹ��TCP��Ϊ����Э���HTTP������Ӧ�ó���
//2.HTTP������Ӧ�ó������յ����Կͻ��˵�����������н���,�Ӵ洢��
//��ȡ���������Դ,��������Ϊһ���ַ��ͻؿͻ���

//Boost.Asio������SSL/TLSЭ��ʵ��,�෴,��������OpenSSL��
//
//��ʵ��HTTP�ͻ���Ӧ�ó���ʱ,��Ҫ�����������
//1.��ִ��Boost.Asio��������ķ���ʱ���ܳ��ֵĴ��������ʾ
//����:����ڱ�ʾ��δ�򿪵��׽��ֵĶ����ϵ���write_some()����,��÷��������������ϵͳ��صĴ������
//(ͨ���׳��쳣��ͨ��out�����ķ�ʽ,����ȡ���ڷ���(��ʹ�ù���)),ָ����δ�򿪵��׽�����ִ��
//����Ч��������ʵ
//2.HTTPЭ�鶨��Ĵ���ͷǴ���״̬,������������ص�״̬����200��Ϊ�Կͻ��˷������ض��������Ӧ
//ָ���ͻ��˵������ѳɹ���ɵ���ʵ,��һ����,״̬����500ָ����ִ��������
//�Ĳ���ʱ,�ڷ������Ϸ�����������δ������Ĵ���
//3.��HTTPЭ�鱾����صĴ���,���������������Ϣ,��Ϊ�����ͻ����������Ӧ
//���Ҵ���Ϣ������ȷ�Ľṹ����HTTP��Ӧ,��ͻ���Ӧ�ó���Ӧ���и��ݴ���
//�����ʾ����ʵ�ķ���

//��һ�����Ĵ��������Boost.Asio���Դ�ж���
//�ڶ����״̬������HTTPЭ�鶨��
//������û�����κεط�����,Ӧ�����Լ��ĳ����ж�����Ӧ�Ĵ������

//����һ���������,��������һ���ǳ�һ��Ĵ���,ָ���ӷ������յ�����Ϣ������ȷ��HTTP
//��Ӧ��Ϣ,��˿ͻ����޷�������,��������Ϊinvalid_response
namespace http_errors {
	enum http_error_codes
	{
		invalid_response = 1
	};
	//Ȼ����һ����ʾ����������
	//���а������涨���invalid_response�������
	//����Ϊhttp_errors_category
	class http_errors_category
		:public boost::system::error_category
	{
		//�������麯��
	public:
		const char* name() const BOOST_SYSTEM_NOEXCEPT
		{
			return "http_errors";
		}

		std::string message(int e)const {
			switch (e)
			{
			case invalid_response:
				return "Server response cannot be parsed";
				break;
			default:
				return "Unknown error";
				break;
			}
		}
	};
	//p178

	const boost::system::error_category &
		get_http_errors_category() {
		static http_errors_category cat;
		return cat;
	}

	//����һ����������
	boost::system::error_code
		make_error_code(http_error_codes e) {
		return boost::system::error_code(
			static_cast<int>(e), get_http_errors_category());

	}
}

//��Ӧ�ó�����ʹ���µĴ������֮ǰ,��Ҫִ�е����һ��������Boost��֪��http_error_code
//ö�ٵĳ�ԱӦ�ñ���Ϊ�������,Ϊ��,������boost::system�����ռ��а������½ṹ����

//���û�������������
//�޷������� 1 �ӡ�http_errors::http_error_codes��ת��Ϊ��const boost::system::error_code &��
namespace boost {
	namespace system {
		//make_error_code�����������ÿɱ��Զ�����
		//����Request::on_finish()�в�����һ��enum��ʽת��Ϊһ��boost::system::error_code
		template<>
		struct is_error_code_enum
			<http_errors::http_error_codes>
		{
			BOOST_STATIC_CONSTANT(bool, value = true);
		};
	}
}
//��Ϊ���ǵ�HTTP�ͻ���Ӧ�ó������첽��,���Կͻ��˵��û��ڷ�������ʱ
//��Ҫ�ṩָ��ص�������ָ��,�ú��������������ʱ����
//������Ҫ����һ����ʾ���ֻص�����ָ�������,����ʱ�Ļص�������Ҫ������ȷָ��
//�����µĲ���
//1.�ĸ������Ѿ����
//2.��Ӧ��ʲô 
//3.�����Ƿ�ɹ����,���û�д������ָ�������Ĵ���

//�����ǻص�����ָ���������������
class HTTPClient;
class HTTPRequest;
class HTTPResponse;

typedef void(*Callback_Client)(const HTTPRequest& request,
	const HTTPResponse& response,
	const system::error_code& ec);

//����������һ������

class HTTPResponse {
	friend class HTTPRequest;
	HTTPResponse() :
		m_response_stream(&m_response_buf) {}
public:
	unsigned int get_status_code()const {
		return m_status_code;
	}

	const std::string& get_status_message()const {
		return m_status_message;

	}

	const std::map<std::string, std::string>& get_headers() {
		return m_headers;
	}

	const std::istream&get_response()const {
		return m_response_stream;
	}

private:
	asio::streambuf&get_response_buf() {
		return m_response_buf;
	}

	void set_status_code(unsigned int status_code) {
		m_status_code = status_code;
	}

	void set_status_message(const std::string& status_message) {
		m_status_message = status_message;
	}

	void add_header(const std::string& name, const std::string& value) {
		m_headers[name] = value;
	}

private:
	unsigned int m_status_code;
	std::string m_status_message;

	std::map<std::string, std::string>m_headers;
	asio::streambuf m_response_buf;
	std::istream m_response_stream;
};

class HTTPRequest {
	friend class HTTPClient;

	static const unsigned int DEFAULT_PORT = 80;

	HTTPRequest(asio::io_service& ios, unsigned int id) :
		m_port(DEFAULT_PORT),
		m_id(id),
		m_callback(nullptr),
		m_sock(ios),
		m_resolver(ios),
		m_was_cancelled(false),
		m_ios(ios)
	{}
public:
	void set_host(const std::string& host) {
		m_host = host;
	}
	void set_port(unsigned int port) {
		m_port = port;
	}
	void set_uri(const std::string& uri) {
		m_uri = uri;
	}
	void set_callback(Callback_Client callback) {
		m_callback = callback;
	}
	std::string get_host() const {
		return m_host;
	}
	unsigned int get_port() const {
		return m_port;
	}
	const std::string& get_uri() const {
		return m_uri;
	}
	unsigned int get_id() const {
		return m_id;
	}
	void execute() {
		// Ensure that precorditions hold.
		assert(m_port > 0);
		assert(m_host.length() > 0);
		assert(m_uri.length() > 0);
		assert(m_callback != nullptr);
		// Prepare the resolving query.
		asio::ip::tcp::resolver::query resolver_query(m_host,
			std::to_string(m_port),
			asio::ip::tcp::resolver::query::numeric_service);
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();
			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}

		// Resolve the host name.
		m_resolver.async_resolve(resolver_query,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
			asio::ip::tcp::resolver::iterator end;
			for (auto i = iterator; i != end; ++i) {
				DMESSAGE(i->endpoint().address());
			}
			on_host_name_resolved(ec, iterator);
		});
	}

	void cancel() {
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);

		m_was_cancelled = true;

		m_resolver.cancel();

		if (m_sock.is_open()) {
			m_sock.cancel();
		}
	}

private:
	void on_host_name_resolved(
		const boost::system::error_code& ec,
		asio::ip::tcp::resolver::iterator iterator)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();
			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}
		// Connect to the host.
		//���ӵ���һ����Ч��IP
		asio::async_connect(m_sock,
			iterator,
			[this](const boost::system::error_code& ec,
				asio::ip::tcp::resolver::iterator iterator)
		{
			DMESSAGE("Connect OK!");
			on_connection_established(ec, iterator);
		});
	}

	void on_connection_established(
		const boost::system::error_code& ec,
		asio::ip::tcp::resolver::iterator iterator)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		// Compose the request message.
		m_request_buf += "GET " + m_uri + " HTTP/1.1\r\n";
		// Add mandatory header.
		m_request_buf += "Host: " + m_host + "\r\n";
		//m_request_buf += "accept: application/json""\r\n";
		m_request_buf += "\r\n";
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();
			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}
		// Send the request message.
		asio::async_write(m_sock,
			asio::buffer(m_request_buf),
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			DMESSAGE("Established OK!");
			DMESSAGE(">\n" + m_request_buf + "\n");
			on_request_sent(ec, bytes_transferred);
		});
	}

	void on_request_sent(const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		//���ﲻ��ֱ��shutdown(send)
		//���ܻ�����ղ���Response
		//ԭ���������Ϊ���첽���������shutdown����ִ�����async_write()����
		//m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();
			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}
		// Read the status line.
		//ע�������Ѿ���ȡ�����е�ͷ��
		//�йر��ĵĽṹ https://www.cnblogs.com/rainydayfmb/p/5319318.html
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n",
			[this](const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//[ע��]
			//���Ϊ�˱�֤���ͷ��׽�����Դ֮ǰ��ȷ�رշ���
			//�ڻص�������shutdown()
			m_sock.shutdown(asio::ip::tcp::socket::shutdown_send);
			DMESSAGE("Request_Send OK!");
			//�ƻ��Լ��,�����������
			//std::istream input(&m_response.get_response_buf());
			//std::vector<std::string> x;
			//while (input) {
			//	std::string temp;
			//	input >> temp;
			//	x.push_back(std::move(temp));

			//}
			//for each (auto c in x)
			//{

			//	DMESSAGE(c);
			//}
			std::istream xxx(&m_response.get_response_buf());
			//DMESSAGE(xxx.rdbuf());
			//DMESSAGE("DDDDDDDDDDDD");

			//ʵ���������ֱ�Ӷ��������е�ͷ��
			//istream��rdbuf()����Ҳ���ƶ�����ǰ��
			on_status_line_received(ec, bytes_transferred);
			//on_headers_received(ec,bytes_transferred);
		});
	}

	void on_status_line_received(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		DMESSAGE("Line");
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		// Parse the status line.
		std::string http_version;
		std::string str_status_code;
		std::string status_message;
		std::istream response_stream(
			&m_response.get_response_buf());
		response_stream >> http_version;

		if (http_version != "HTTP/1.1") {
			// Response is incorrect.
			on_finish(http_errors::invalid_response);
			return;
		}
		response_stream >> str_status_code;
		// Convert status code to integer.
		unsigned int status_code = 200;
		try {
			status_code = std::stoul(str_status_code);
		}
		catch (std::logic_error&) {
			// Response is incorrect.
			on_finish(http_errors::invalid_response);
			return;
		}
		std::getline(response_stream, status_message, '\r');
		// Remove symbol '\n' from the buffer.
		response_stream.get();

		DMESSAGE("http_version: " + http_version);
		DMESSAGE("status_code: " + str_status_code);
		DMESSAGE("status_message: " + status_message);
		DMESSAGE("<<>>");
		std::istream xxx(&m_response.get_response_buf());
		DMESSAGE(xxx.rdbuf());
		m_response.set_status_code(status_code);
		m_response.set_status_message(status_message);
		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();

			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}
		// At this point the status line is successfully
		// received and parsed.
		// Now read the response headers.
		asio::async_read_until(m_sock,
			m_response.get_response_buf(),
			"\r\n\r\n",
			[this, &status_code, &status_message](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			DMESSAGE("Status_line_received OK!");
			//DMESSAGE(">Code = " + status_code);
			//DMESSAGE(">Message = " + status_message);
			on_headers_received(ec,
				bytes_transferred);
		});
	}

	void on_headers_received(const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec.value() != 0) {
			on_finish(ec);
			return;
		}
		// Parse and store headers.
		std::string header, header_name, header_value;
		std::istream response_stream(
			&m_response.get_response_buf());
		while (true) {
			std::getline(response_stream, header, '\r');
			// Remove \n symbol from the stream.
			response_stream.get();

			if (header == "")
				break;
			size_t separator_pos = header.find(':');
			if (separator_pos != std::string::npos) {
				header_name = header.substr(0,
					separator_pos);
				if (separator_pos < header.length() - 1)
					header_value =
					header.substr(separator_pos + 1);
				else
					header_value = "";
				m_response.add_header(header_name,
					header_value);
			}
		}

		std::unique_lock<std::mutex>
			cancel_lock(m_cancel_mux);
		if (m_was_cancelled) {
			cancel_lock.unlock();
			on_finish(boost::system::error_code(
				asio::error::operation_aborted));
			return;
		}
		// Now we want to read the response body.
		asio::async_read(m_sock,
			m_response.get_response_buf(),
			[this](
				const boost::system::error_code& ec,
				std::size_t bytes_transferred)
		{
			//��󣬵���on_response_body_received����������֪ͨ�������յ�������Ӧ��Ϣ��
			//��ΪHTTP��������������������Ӧ��Ϣ����󲿷�֮��
			//�ر����׽��ֵķ��Ͳ���,�����ڿͻ���,���Ķ�ȡ�����������
			//����������asio::error::eof
			on_response_body_received(ec,
				bytes_transferred);
		});
		return;
	}

	void on_response_body_received(
		const boost::system::error_code& ec,
		std::size_t bytes_transferred)
	{
		if (ec == asio::error::eof)
			on_finish(boost::system::error_code());
		else
			on_finish(ec);
	}

	void on_finish(const boost::system::error_code& ec)
	{
		if (ec.value() != 0) {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message();
		}
		m_callback(*this, m_response, ec);
		return;
	}
private:
	std::string m_host;
	unsigned int m_port;
	std::string m_uri;

	unsigned int m_id;
	Callback_Client m_callback;
	std::string m_request_buf;

	asio::ip::tcp::socket m_sock;
	asio::ip::tcp::resolver m_resolver;

	HTTPResponse m_response;

	bool m_was_cancelled;
	std::mutex m_cancel_mux;

	asio::io_service &m_ios;
};
class HTTPClient {
public:
	HTTPClient() {
		//���ȶ���һ��work����ȷ����û�й�����첽����ʱ
		//�¼�ѭ�����̲߳����˳���ѭ��
		//һ��ֻҪ�Ƿ�װ��asio���첽���ܵĶ���Ҫ
		//һ��ʼ��ʼ��ʱ��û�й�����첽����
		m_work.reset(new boost::asio::io_service::work(m_ios));
		m_thread.reset(new std::thread([this]() {
			m_ios.run();
		}));
	}
	std::shared_ptr<HTTPRequest>
		create_request(unsigned int id)
	{
		return std::shared_ptr<HTTPRequest>(
			new HTTPRequest(m_ios, id));
	}
	void close() {
		// Destroy the work object.
		m_work.reset(NULL);
		// Waiting for the I/O thread to exit.
		//�߳������й��������ɺ������˳��¼�ѭ��
		m_thread->join();
	}
private:
	asio::io_service m_ios;
	std::unique_ptr<boost::asio::io_service::work> m_work;
	std::unique_ptr<std::thread> m_thread;
};

void handler(const HTTPRequest& request,
	const HTTPResponse& response,
	const system::error_code& ec)
{
	if (ec.value() == 0) {
		std::cout << "Request #" << request.get_id()
			<< " has completed. Response: "
			<< response.get_response().rdbuf();
	}
	else if (ec == asio::error::operation_aborted) {
		std::cout << "Request #" << request.get_id()
			<< " has been cancelled by the user."
			<< std::endl;
	}
	else {
		std::cout << "Request #" << request.get_id()
			<< " failed! Error code = " << ec.value()
			<< ". Error message = " << ec.message()
			<< std::endl;
	}
	return;
}
//
int C5_Client_ASYN() {
	try {
		//https://api.bgm.tv/user/wz97315
		HTTPClient client;
		std::shared_ptr<HTTPRequest> request_one =
			client.create_request(1);
		//request_one->set_host("api.bgm.tv");
		//request_one->set_uri("/user/wz97315");
		request_one->set_host("127.0.0.1");
		request_one->set_uri("/index.html");
		request_one->set_port(3333);
		request_one->set_callback(handler);
		request_one->execute();
		//std::shared_ptr<HTTPRequest> request_two =
		//	client.create_request(1);
		//request_two->set_host("localhost");
		//request_two->set_uri("/example.html");
		//request_two->set_port(3333);
		//request_two->set_callback(handler);
		//request_two->execute();
		//request_two->cancel();
		// Do nothing for 15 seconds, letting the
		// request complete.
		std::this_thread::sleep_for(std::chrono::seconds(15));
		// Closing the client and exiting the application.
		client.close();
	}
	catch (system::system_error &e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}



//������ʵ��һ��HTTP������
//��ת3196��


//�ͻ���Ӧ�ó���ͨ��ʹ��SSL/TLS������������������,���ÿ���,��������
//SSL/TLS����ͻ��˶Է��������������֤����������
//�������������֤����ͻ���ȷ�������ݷ��͵�Ԥ�ڵ��ռ���

//���ݼ��ܱ�֤��ʹ����������ڵ����������;�б��ػ�,������Ҳ�޷�ʹ����
//ʹ��OpenSSL��ʵ��֧��SSL/TLSЭ���ͬ��TCP�ͻ���Ӧ�ó���

//�ڿ�ʼʹ��ʱ,��Ҫ�Ȱ�װOpenSSL��
//#include <boost/asio/ssl.hpp>
//class SyncSSLClient {
//public:
//	SyncSSLClient(const std::string& raw_ip_address,
//		unsigned short port_num) :
//		m_ep(asio::ip::address::from_string(raw_ip_address),
//			port_num),
//		//ָ��Ӧ�ó����ʹ�ÿͻ��˽�ɫ��ʹ��������,����ϣ��֧�ֶ����ȫЭ��,��������汾SSL/TLS
//		m_ssl_context(asio::ssl::context::sslv3_client),
//		//
//		m_ssl_stream(m_ios, m_ssl_context)
//	{
//		// Set verification mode and designate that
//		// we want to perform verification.
//		//�����û���֤ģʽ���趨Ϊasio::ssl::verify_peer
//		//����ζ���������ڼ�ִ�жԵ���֤
//		m_ssl_stream.set_verify_mode(asio::ssl::verify_peer);
//		// Set verification callback.
//		//Ȼ������һ����֤�ص�����,��֤��ӷ���������ʱ�����ø÷���
//		//�Է��������͵�֤�����е�ÿ��֤�����һ�λص�
//		m_ssl_stream.set_verify_callback([this](
//			bool preverified,
//			asio::ssl::verify_context& context)->bool {
//			return on_peer_verify(preverified, context);
//		});
//	}
//	void connect() {
//		// Connect the TCP socket.
//		m_ssl_stream.lowest_layer().connect(m_ep);
//		// Perform the SSL handshake.
//		m_ssl_stream.handshake(asio::ssl::stream_base::client);
//	}
//	void close() {
//		// We ignore any errors that might occur
//		// during shutdown as we anyway can't
//		// do anything about them.
//		boost::system::error_code ec;
//		// shutdown����������ͬ���Ĳ���������ֱ��SSL���ӹرջ�������
//		m_ssl_stream.shutdown(ec); // Shutdown SSL.
//								   // Shut down the socket.
//		m_ssl_stream.lowest_layer().shutdown(
//			boost::asio::ip::tcp::socket::shutdown_both, ec);
//		m_ssl_stream.lowest_layer().close(ec);
//	}
//	std::string emulate_long_computation_op(
//		unsigned int duration_sec) {
//		std::string request = "EMULATE_LONG_COMP_OP "
//			+ std::to_string(duration_sec)
//			+ "\n";
//		send_request(request);
//		return receive_response();
//	};
//private:
//	bool on_peer_verify(bool preverified,
//		asio::ssl::verify_context& context)
//	{
//		// Here the certificate should be verified and the
//		// verification result should be returned.
//		return true;
//	}
//	void send_request(const std::string& request) {
//		asio::write(m_ssl_stream, asio::buffer(request));
//	}
//	std::string receive_response() {
//		asio::streambuf buf;
//		asio::read_until(m_ssl_stream, buf, '\n');
//		std::string response;
//		std::istream input(&buf);
//		std::getline(input, response);
//		return response;
//	}
//private:
//	asio::io_service m_ios;
//	asio::ip::tcp::endpoint m_ep;
//	//һ����ʾSSL�����ĵĶ���
//	//�����ϣ�����OpenSSL�ⶨ���SSL_CTX���ݽṹ�İ�װ����
//	//�˶������ʹ��SSL / TLSЭ�����ͨ�ŵ���������ͺ�����ʹ�õ�ȫ�����úͲ���
//	asio::ssl::context m_ssl_context;
//	//���ʾ��װTCP�׽��ֶ���ʵ������SSL/TLSͨ�Ų�������
//	asio::ssl::stream<asio::ip::tcp::socket>m_ssl_stream;
//};

//����openSSL�������ⲻ����������,��ʱע��
//int Test_SSL_TLS_Client() {
//	const std::string raw_ip_address = "pariya.cc";
//	const unsigned short port_num = 80;
//	try {
//		SyncSSLClient client(raw_ip_address, port_num);
//		// Sync connect.
//		client.connect();
//		std::cout << "Sending request to the server... "
//			<< std::endl;
//		std::string response =
//			client.emulate_long_computation_op(10);
//		std::cout << "Response received: " << response
//			<< std::endl;
//		// Close the connection and free resources.
//		client.close();
//	}
//	catch (system::system_error &e) {
//		std::cout << "Error occured! Error code = " << e.code()
//			<< ". Message: " << e.what();
//		return e.code().value();
//	}
//	return 0;
//}
//p217
//p226

//====CH.6====
//���°����˺���Boost.Asio����
//Boost.Asio��ʱ����һ�ֹ���ǿ�������,���Բ���ʱ����
//���ҿ����ڽ���������޹ص���������

//�����ȡ�������׽���ѡ��Ĺ���Ҳʮ����Ҫ,
//Boost.Asio���װ�׽��ֲ�Ϊ���ṩ�������Ľӿ�,ʹ�����ܹ�
//�����򵥶����ŵķֲ�ʽӦ�ó���

//[ʹ�ø��ϻ��������з�ɢ/�ռ�����]
//�ڶ���I/O�����е�ʹ�ù̶�����I/O������,������΢�漰��ɢ/�ռ������͸��ϻ�����
//
//���ϻ�������������һ�����ӵĻ�����,�����������ֲ��ڽ��̵�ַ�ռ��ϵ�
//�򵥻�����(�������ڴ��)���
//���ֻ�������������±���ر𷽱�
//1.Ӧ�ó����ڽ���Ϣ���͵�Զ��Ӧ�ó���֮ǰ��Ҫ���������洢��Ϣ
//���߽���Զ��Ӧ�ó����͵���Ϣ,��������Ϣ�Ĵ�С̫��,������������Դ洢���ĵ����������������ܻ����ڽ��̵ĵ�ַ�ռ���Ƭ
//��ʧ��,��������·�������С�Ļ�����,���ܺ͵Ĵ�С���Ĵ洢����
//������������ڵ������ϻ���������һ���ܺõĽ������
//2.����Ӧ�ó�����Ƶ�������,Ҫ���͵�Զ��Ӧ�ó������Ϣ���ֳɼ������ֲ��洢��
//��ͬ�Ļ�������,�������Ҫ��Զ��Ӧ�ó�����յ���Ϣ��Ҫ�ֳɼ�������,ÿ����������Ӧ
//�洢�ڵ����Ļ��������Թ���һ������
//
//������������½������������ϵ�һ�����ϻ�������Ȼ��ʹ�÷�ɢ���ͻ��ռ����ղ�������
//���������ĺ÷���


//�����㷨���������׼����socketִ����������ķ���һ��ʹ�õĸ��ϻ�����
//����asio::ip::tcp::socket::send()�����ɺ���asio::write()
//1.������Ҫ���価���ܶ���ڴ滺������ִ����ͷ������,��ע��,�˲��費�漰Boost.Asio�е��κι��ܻ���������
//2.ʹ��Ҫ�����������仺����
//3.��������ConstBufferSequence��MultipleBufferSequence����Ҫ������ʵ��,��ʾ���ϻ�����
//4.���򵥻�������ӵ����ϻ�����.ÿ���򵥻�����Ӧ�ñ�ʾΪasio::const_buffer��asio::mutable_buffer���ʵ��
//5.���ϻ�������׼������Boost.Asio�������һ��ʹ��

int C6_Send_Buffer_1() {
	const char *part1 = "Hello ";
	const char *part2 = "my ";
	const char *part3 = "friend!";

	std::vector<asio::const_buffer> composite_buffer;

	composite_buffer.push_back(asio::const_buffer(part1, 6));
	composite_buffer.push_back(asio::const_buffer(part2, 3));
	composite_buffer.push_back(asio::const_buffer(part3, 7));


	return 0;
}

//�����㷨���������׼����socketִ����������ķ���һ��ʹ�õĸ��ϻ�����
//����asio::ip::tcp::socket::receive()�����ɺ���asio::read();
//1.������Ҫ���価���ܶ���ڴ滺������ִ����ͷ������
//��������С���ܺͱ�����ڻ����Ҫ����Щ�������н��յ�Ԥ����Ϣ�Ĵ�С��
//��ע�⣬�˲��費�漰Boost.Asio�е��κι��ܻ���������
//2.����һ�����ʵ������������MutableBufferSequence����ı�ʾ���ϻ�������Ҫ��
//3.���򵥻�������ӵ����ϻ�������
//ÿ���򵥻�����Ӧ�ñ�ʾΪasio::mutable_buffer���һ��ʵ����
//4.���ϻ�������׼������Boost.Asio�������һ��ʹ��


int C6_Receive_Buffer_1() {
	// Step 1. Allocate simple buffers.
	char part1[6];
	char part2[3];
	char part3[7];
	// Step 2. Create an object representing a composite buffer.
	std::vector<asio::mutable_buffer> composite_buffer;
	// Step 3. Add simple buffers to the composite buffer object.
	composite_buffer.push_back(asio::mutable_buffer(part1,
		sizeof(part1)));
	composite_buffer.push_back(asio::mutable_buffer(part2,
		sizeof(part2)));
	composite_buffer.push_back(asio::mutable_buffer(part3,
		sizeof(part3)));
	// Now composite_buffer can be used with Boost.Asio
	// input operation as if it was a simple buffer
	// represented by contiguous block of memory.
	return 0;
}
//
//Ӳ����ʱ�����ڲ���ʱ�������豸- ���κμ�����Ļ������,���������ִ�����ϵͳ���ṩ����
//Ӧ�ó���ʹ�����Ľӿ�,���������ʱ����صĵ�������
//1.�ٶ�Ӧ�ó�����֪����ǰʱ�䲢Ҫ�����ϵͳ�ҵ���
//2.��Ӧ�ó���Ҫ�����ϵͳ��һ��ʱ���ȥʱ֪ͨ��(ͨ�����ûص�)
//2����Ҫ,��Ϊ��ʱ����ʵ���첽�����ĳ�ʱ���Ƶ�Ψһ����

//Boost.Asio���ṩ������ʵ�ּ�ʱ����ģ����
//����һ����asio::basic_deadline_timer<>,����Boost.Asio 1.49�汾֮ǰΨһ����
//��1.49�汾֮�������˵ڶ�����ʱ��asio::basic_waitable_timer<>��ģ��
//asio::basic_deadline_timer<>��ģ��ּ����Boost.Chrono�����,�����ڲ�
//���������ṩ�Ĺ���,��ģ�����е��ʱ,�����޵Ĺ���,��˲�����ʹ����
//�෴��c++11 chrono����ݵĽ��µ�asio::basic_waitable_timer<>��ģ������,���ṩ���๦��
//
//typedef basic_waitable_timer< std::chrono::system_clock >
//system_timer;
//typedef basic_waitable_timer< std::chrono::steady_clock >
//steady_timer;
//typedef basic_waitable_timer< std::chrono::high_resolution_clock >
//high_resolution_timer;

//asio::system_timer�����std::chrono::system_clock��,����ʾϵͳ��Χ��ʵʱʱ��
//��ʱ��(�Լ���ʱ��)Ҳ�ܵ�ǰϵͳʱ����ⲿ�仯��Ӱ��
//���,��������Ҫ����һ����ʱ��ʱ,asio::system_timer��һ�������ѡ��
//�����ڴﵽĳ������ʱ���ʱ֪ͨ����(����:13H:15M:45S),ͬʱ���ǵ�ϵͳʱ��
//�ı仯,��ʱ�����ú�����,Ȼ��
//�ü�ʱ�����ó�����ʱ����(����,��������35��),��Ϊϵͳʱ����λ���ܵ��¼�ʱ��
//��ʵ�ʼ����ȥ֮ǰ��֮����
//asio::steady_timer�����std::chrono::steady_clock��,������һ������ϵͳʱ�ӱ仯Ӱ���
//�ȶ�ʱ��,��ζ��asio::steady_timer�ǲ�������Ĳ���ѡ��
//���һ����ʱ��asio::high_resolution_timer�����std::chrono::high_resolution_clock��
//������һ���߷ֱ��ʵ�ϵͳʱ��,����������Ҫ�߾���ʱ����������

//��ʹ��Boost.Asio��ʵ�ֵķֲ�ʽӦ�ó�����,��ʱ��ͨ������ʵ���첽�����ĳ�ʱʱ��
//���첽������ʼ֮��(����::asio::async_read())Ӧ�ó�������һ������Ϊ��һ��ʱ��
//����ڵļ�ʱ��,����ʱ������ʱ,Ӧ�ó������첽�����Ƿ������
//���û��,����Ϊ������ʱ��ȡ��

//����steady_timer����ϵͳʱ��ƫ�Ƶ�Ӱ��,������ʺ�ʵ�ֳ�ʱ����
/*��ע�⣬��ĳЩƽ̨�ϣ��ȶ�ʱ�Ӳ����ã����ұ�ʾstd :: chrono :: steady_clock����Ӧ����ֳ���std :: chrono :: stystem_ clock��ͬ����Ϊ������ζ�ž������һ����
����ϵͳʱ�ӱ仯��Ӱ�졣
����ο�ƽ̨�ĵ�����Ӧ��C ++��׼��ʵ�֣���ȷ���ȶ�ʱ���Ƿ�ʵ���ȶ�*/

int Test_Steady_Timer() {
	asio::io_service ios;

	asio::steady_timer t1(ios);
	//�˷�������ʱ���л����ǹ���״̬����������
	//������һ���������ò�����ʾʱ�����ĳ���ʱ�䣬֮���ʱ��Ӧ�õ��ڡ�
	//һ�����ڽ�֪ͨ���еȴ��˼�ʱ���ĵ����¼�����
	t1.expires_from_now(std::chrono::seconds(2));

	asio::steady_timer t2(ios);
	t2.expires_from_now(std::chrono::seconds(5));

	//ע��ص�
	t1.async_wait([&t2](boost::system::error_code ec) {
		if (ec.value() == 0) {
			std::cout << "Timer #2 has expired!" << std::endl;
		}
		else if (ec == asio::error::operation_aborted) {
			std::cout << "Timer #2 has been cancelled!"
				<< std::endl;
		}
		else {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message()
				<< std::endl;
		}
		//ȡ���ڶ�����ʱ���ᵼ�µ��õ��ڻص���֪ͨ��ʱ���ڵ���ǰ��ȡ��
		t2.cancel();
	});	t2.async_wait([](boost::system::error_code ec) {
		if (ec.value() == 0) {
			std::cout << "Timer #2 has expired!" << std::endl;
		}
		else if (ec == asio::error::operation_aborted) {
			std::cout << "Timer #2 has been cancelled!"
				<< std::endl;
		}
		else {
			std::cout << "Error occured! Error code = "
				<< ec.value()
				<< ". Message: " << ec.message()
				<< std::endl;
		}
	});	ios.run();
	return 0;
}

//����ͨ������������ֵ�����õ����Լ���Ϊ
//ʵ��������ʱ,��ѡ�����Ĭ��ֵ,����������,Ĭ����������õ��׽���
//��������,������������¿�����Ҫͨ��������ѡ���ֵ��΢���׽���
//��ʹ������Ӧ�õ�Ҫ��,�ڱ�����,���˽����ʹ��Boost.Asio��ȡ�������׽���ѡ��

//ÿ���׽���ѡ���ֵ����ͨ��Boost.Asio�ṩ�Ĺ������û��ã���һ�����������ʾ
//Boost.Asio��֧�������������׽���ѡ�
//Ҫ���û��ȡ�����׽���ѡ���ֵ��������Ա������Ҫͨ����ӱ�ʾ����ѡ���������չBoost.Asio��
//���������׽��ֵĽ��ջ������Ĵ�С�����ڵĴ�С������
//����������Ҫ��ȡ�������ĵ�ǰ��С��Ȼ�������2����󽫳˷����õ�ֵ����Ϊ�µĽ��ջ�������С

int Test_Set_Socket_Buffer() {
	try {
		asio::io_service ios;
		// Create and open a TCP socket.
		//�����ǿ��Ի�ȡ�������ض��׽��ֶ����ѡ��֮ǰ���������Ӧ���׽��֡�
		//��Ϊ�ڴ�Boost.Asio�׽��ֶ���֮ǰ����δ������Ӧ����ϵͳ�Ļ��������׽��ֶ��󣬲���û���κ�����ѡ�����л�ȡѡ��
		asio::ip::tcp::socket sock(ios, asio::ip::tcp::v4());
		// Create an object representing receive buffer
		// size option.
		asio::socket_base::receive_buffer_size cur_buf_size;
		// Get the currently set value of the option.
		sock.get_option(cur_buf_size);
		std::cout << "Current receive buffer size is "
			<< cur_buf_size.value() << " bytes."
			<< std::endl;
		// Create an object representing receive buffer
		// size option with new value.
		asio::socket_base::receive_buffer_size
			new_buf_size(cur_buf_size.value() * 2);
		// Set new value of the option.
		sock.set_option(new_buf_size);
		std::cout << "New receive buffer size is "
			<< new_buf_size.value() << " bytes."
			<< std::endl;
	}
	catch (system::system_error &e) {
		std::cout << "Error occured! Error code = " << e.code()
			<< ". Message: " << e.what();
		return e.code().value();
	}
	return 0;
}


//Boost.Asio�����asio :: ip :: tcp :: iostream��װ�࣬
//��ΪTCP�׽��ֶ����ṩ����I / O���Ľӿڣ����������Ǹ����������̼�ͨ�Ų���

int Test_Stream() {
	//ʹ�ù��캯�����������󣬸ù��캯�����ܷ�����DNS���ƺ�Э��˿ںţ����Զ����Խ���DNS���ƣ�Ȼ�������ӵ��÷�������
	//��ע�⣬�˿ںű�ʾΪ�ַ���������������
	//������Ϊ���ݸ��˹��캯��������������ֱ�����ڴ������������ѯ������Ҫ���˿ںű�ʾΪ�ַ���
	//���������ʱ�����ǿ���ͨ��ָ��DNS���ƺ�Э��˿ں�������connect�����������Ա�ִ�н����������׽���
	asio::ip::tcp::iostream stream("localhost", "3333");
	if (!stream) {
		std::cout << "Error occurred! Error code = "
			<< stream.error().value()
			<< ". Message = " << stream.error().message()
			<< std::endl;
		return -1;
	}

	stream << "Request.";
	//flush����������ȷ�������л�����������͵���������
	stream.flush();
	//�ȴ�һ��
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout << "[" << __func__ << "]Response: " << stream.rdbuf()<<std::endl;
	return 0;
}
//���ǲ�������ʹ��asio :: ip :: tcp :: iostream�����������ķ�ʽʵ�ֿͻ���I / O�����ǻ������ڷ�������ִ��I / O������
//����֮�⣬������������ָ�������ĳ�ʱ����ʹ�û�������I / O����ͨ��ͬ��I / O��������
//�������������������ɵġ�
int Test_Stream_receive() {
	//���´���Ƭ����ʾ�����ʹ��asio :: ip :: tcp :: iostream��
	//ʵ��ִ�л�������I / O�ļ򵥷�������

	asio::io_service io_service;
	asio::ip::tcp::acceptor acceptor(io_service,
		asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 3333));
	//asio::ip::tcp::endpoint(asio::ip::tcp::v4(),3333);
	//���߶� OK

	asio::ip::tcp::iostream stream;

	//stream�����rdbuf������������ָ���������������ָ��
	//�����������������һ�����ʵ�������̳���asio :: ip :: tcp :: socket�࣬����ζ��asio :: ip :: tcp :: iostream��Ķ���ʹ�õ���������
	//����������ɫ��һ����������������һ����socket
	acceptor.accept(*stream.rdbuf());
	//�ȴ�һ��
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "["<<__func__<<"]Request: " << stream.rdbuf() << std::endl;
	stream << "Response.";
	stream.flush();
	return 0;
}
//��ΪI / O������asio :: ip :: tcp :: stream��
//�ṩ����ִ�е��̣߳��������ǿ����������൱����ʱ�䣬
//���Ը����ṩ��һ�����ó�ʱʱ��ķ�����
//��������ʱ���ᵼ�µ�ǰ�����̵߳Ĳ����жϣ�����еĻ���
//��ʱ�������ͨ��asio::ip::tcp::stream���
//expires_from_now�����������á�


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
	//C4_SYN_TCP_Server();
	//Test_C3_C4_1();
	//C4_SYN_TCP_MServer();
	//Test_C3_C4_2();
	//C4_ASYN_TCP_MServer(); //�����˵�5�µĲ���HTTP
	//Test_C3_C4_3();

	//====CH.5====
	//C5_Client_ASYN();
	//HTTP_SERVER::Test_Server();
	//Test_SSL_TLS_Client();

	//====CH.6====
	//Test_Steady_Timer();
	//Test_Set_Socket_Buffer();
	
	//��ʵ�����߲��ܺܺù���,������ʹ���߳��й�,ԭ�����������̼�ͨ�ŵ�
	//std::thread test_stream(Test_Stream_receive);
	//std::thread test_stream1(Test_Stream);
	//Test_Stream();
	//test_stream1.join();
	//test_stream.join();
	std::system("pause");
}