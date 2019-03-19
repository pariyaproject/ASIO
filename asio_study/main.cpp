#include<iostream>
#define BOOST_ASIO_ENABLE_CANCELIO
#include<boost/asio.hpp>
#include<thread>
#include<mutex>
#include<memory>
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
	s->sock -> async_write_some(
		asio::buffer(s->buf),
		std::bind(callback,
			std::placeholders::_1,
			std::placeholders::_2,
			s));

}
//���ڻص�Step 3
int First_ASYN_TCP_Write(){
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
	std::cout << x <<std::endl;

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