#include<iostream>
#include<boost/asio.hpp>
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
	unsigned short port_num = 5555;
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