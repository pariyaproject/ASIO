#include<iostream>
#include<boost/asio.hpp>
using namespace boost;
//basic
//�ڴ����˵㣨endpoint��֮ǰ���ͻ���Ӧ�ó�������ȡԭʼIP��ַ��ָ������֮ͨ�ŵķ�������Э��˿ں�
//��һ���棬������Ӧ�ó���ͨ����������IP��ַ�ϵĴ�����Ϣ��ֻ��Ҫ��ȡҪ�����Ķ˿ں�
//ԭ�򣺷����������ж��IP��ַ��֮�������ӣ�������Ҫ�������е�IP��ַ

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

	//���ڴ洢���������еĴ���
	boost::system::error_code ec;

	//step 5
	asio::ip::tcp::resolver::iterator it =
		resolver.resolve(resolver_query, ec);
	asio::ip::tcp::endpoint ep = *it;
	std::cout << ep.address() << std::endl;
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

}
int main() {
	if (Resolving_DNS() != 0) {
		return 1;
	}
	std::system("pause");
}