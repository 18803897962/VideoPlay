#include "RTSPServer.h"
#include<rpc.h>
#pragma comment(lib, "rpcrt4.lib")
SocketIniter RTSPServer::m_initer;
RTSPServer::RTSPServer() :m_socket(true), m_status(0), m_pool(3) {
	m_threadMain.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));//threadWorker���ܴ���ͬ��
	m_h264.Open("./test.h264", 96);
}

RTSPServer::~RTSPServer() {
	Stop();
}

int RTSPServer::Init(const std::string& strIP, short port) {
	m_addr.Update(strIP, port);
	m_socket.Bind(m_addr);
	m_socket.Listen();
	return 0;
}

int RTSPServer::Invoke() {
	m_threadMain.Start();
	m_pool.Invoke();
	return 0;
}

void RTSPServer::Stop() {
	m_socket.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

int RTSPServer::threadWorker() {
	EAddress client_addr;
	ESocket client = m_socket.Accept(client_addr);
	if (client != INVALID_SOCKET) {//�ַ�һ���Ự
		RTSPSession session(client);
		m_lstsession.PushBack(session);//����session����Ӧ�ͻ��ˣ�����ͻ������ӶϿ�����֮ǰ��session�޷����������
		m_pool.dispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSession));
	}
	return 0;
}

int RTSPServer::ThreadSession() {//���ÿ���ͻ��˶�����һ���Ự
	//TODO:1����������  2����������  3��Ӧ������
	//TODO:���� �ͻ����׽���Ҫ��Session�������
	RTSPSession session;
	if (m_lstsession.PopFront(session)) {
		return session.PickRequestAndReply(RTSPServer::PlayCallBack,this);
	}
	return -1;
}

void RTSPServer::PlayCallBack(RTSPServer* thiz, RTSPSession& session){
	EAddress client_addr = session.GetUDPAddress();
	thiz->UDPWorker(client_addr);
}

void RTSPServer::UDPWorker(EAddress& client_addr){
	MyBuffer frame =  m_h264.ReadOneFrame();
	RTPFrame rtp;
	int ret = 0;
	while (frame.size() > 0) {
		ret = m_helper.SendMediaFrame(rtp, frame, client_addr);
		frame = m_h264.ReadOneFrame();
	}
}

RTSPSession::RTSPSession() {
	UUID uuid;//����m_id
	UuidCreate(&uuid);//����һ��128λ�����������������Ψһ�ģ���֤sessionID��ֵΨһ
	m_id.resize(8);
	snprintf((char*)m_id.c_str(), m_id.size(), "%u%u", uuid.Data1,uuid.Data2);
	m_id.ResetSize();
	m_port = 0;
}

RTSPSession::RTSPSession(const ESocket& client) : m_client(client) {
	UUID uuid;//����m_id
	UuidCreate(&uuid);//����һ��128λ�����������������Ψһ�ģ���֤sessionID��ֵΨһ
	m_id.resize(8);
	snprintf((char*)m_id.c_str(), m_id.size(), "%u%u", uuid.Data1, uuid.Data2);
	m_id.ResetSize();
	m_port = 0;
}

RTSPSession::RTSPSession(const RTSPSession& session) {
	m_id = session.m_id;
	m_client = session.m_client;
	m_port = session.m_port;
}

RTSPSession& RTSPSession::operator=(const RTSPSession& session) {
	if (&session != this) {
		m_id = session.m_id;
		m_client = session.m_client;
		m_port = session.m_port;
	}
	return *this;
}

int RTSPSession::PickRequestAndReply(RTSPPLAYCB cb, RTSPServer* thiz) {
	//1��Pick 
	//2��Analyse
	//3��Reply
	int count = 0;
	count++;
	int ret = -1;
	do {
		MyBuffer buffer;
		Pick(buffer);
		if (buffer.size() <= 0) 
			return -1;
		RTSPRequest request = AnalyseRequest(buffer);
		if (abs(request.method() - 2) > 2) {
			TRACE("error at :[%s]:(%d)(%s)\r\n", __FILE__, __LINE__, __FUNCTION__);
			TRACE("buffer:%s\r\n", buffer);
			return -2;
		}
		
		RTSPReply reply = MakeReply(request);
		std::string str = (char*)reply.toBuffer();
		printf("%s\r\n", str.c_str());
		ret = m_client.Send(reply.toBuffer());

		if (request.method() == 2) {
			m_port = (unsigned short)atoi(request.port());
		}
		if (request.method() == 3) {//����ǲ���
			cb(thiz, *this);
		}
	} while (ret >= 0);
	if (ret < 0) return ret;
	return 0;
}

EAddress RTSPSession::GetUDPAddress(){
	EAddress client_addr;
	int len = client_addr.Size();
	getsockname(m_client,client_addr,&len);
	client_addr.Fresh();
	client_addr = m_port;
	return client_addr;
}

MyBuffer RTSPSession::PickOneLine(MyBuffer& buffer) {
	if (buffer.size() <= 0) return MyBuffer();
	MyBuffer line;
	size_t i = 0;
	for (; i < buffer.size(); i++) {
		line.push_back(buffer.at(i));
		if (line.size() >= 2&&line.substr(i - 1, 2) == "\r\n") {
			i++;
			break;
		}
	}
	MyBuffer tmp = (char*)buffer + i;
	buffer = tmp;//ÿȡһ�У���Ҫ��ȡ������������
	return line;//���ص�ǰ��
}

void RTSPSession::Pick(MyBuffer& result){
	//����Э�齻����ʽ��ÿһ�����ĵ������\r\n\r\n������ж��Ƿ���һ����ȷ�ı���
	int ret = 1;
	MyBuffer buf(1);//���ν��յ�����  //һ���ֽ�һ���ֽڶ�ȡ����ֹ�ְ����ͣ������޷�ʶ����ȷ�İ�
	while (ret > 0) {
		buf.Zero();//���㣬�����ı仺������С
		ret = m_client.Recv(buf);
		if (ret > 0) {
			result += buf;
			if (result.size() >= 4) {
				UINT val = *(UINT*)(result.size() - 4 + (char*)result.c_str());
				if (val == *(UINT*)"\r\n\r\n") {//�ж�ĩβ�Ƿ��Ǳ�־λ\r\n
					break;
				}
			}
		}
	}
}


RTSPRequest RTSPSession::AnalyseRequest(const MyBuffer& buffer) {//��������ʱ�ͻ��˷����Ľ�����Ϣ
	//����buffer
	RTSPRequest request;
	MyBuffer method(32), url(512),version(16),seq(64);
	MyBuffer data = buffer;
	MyBuffer line = PickOneLine(data);//ÿ�δ���ѡ��һ��
	if (sscanf(line, "%s %s %s\r\n", (char*)method, (char*)url, (char*)version) < 3) {//�ӵ�һ�н�����method��url��version
		TRACE("error at :[%s]:(%d)(%s)\r\n", __FILE__, __LINE__, __FUNCTION__);
		return request;
	}
	line = PickOneLine(data);
	if (sscanf(line, "CSeq: %s\r\n", (char*)seq) < 1) {
		TRACE("error at :[%s]:(%d)(%s)\r\n", __FILE__, __LINE__, __FUNCTION__);
		return request;
	}
	method.ResetSize();
	url.ResetSize();
	version.ResetSize();
	seq.ResetSize();
	request.SetMethod(method);
	request.SetUrl(url);
	request.SetSequence(seq);
	TRACE("%s\r\n", (char*)buffer);
	if (strcmp(method, "OPTIONS") == 0 || strcmp(method, "DESCRIBE") == 0) {
		return request;
	}
	else if (strcmp(method, "SETUP") == 0) {
		line = PickOneLine(data);
		while (strstr(line.c_str(), "client_port=") == NULL) {
			line = PickOneLine(data);
		}
		int port[2] = { 0 };
		if (sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", &port[0], &port[1]) != 2) {
			TRACE("error at :[%s]:(%d)(%s)\r\n", __FILE__, __LINE__, __FUNCTION__);
			return request;
		}
		request.SetClientPort(port);
		return request;
	}
	else if (strcmp(method, "PLAY") == 0 || strcmp(method, "TEARDOWN") == 0) {
		line = PickOneLine(data);
		MyBuffer session(64);
		if (sscanf(line, "Session: %s\r\n", (char*)session) == 1) {
			//session.ResetSize();
			request.SetSession(session);
			return request;
		}
	}
	return request;
}

RTSPReply RTSPSession::MakeReply(const RTSPRequest& request) {
	RTSPReply reply;
	reply.SetSequence(request.sequence());//���
	if (request.session().size() > 0) {//session
		reply.SetSession(request.session());
	}
	else {
		reply.SetSession(m_id);
	}
	reply.SetMethod(request.method());
	switch (request.method())
	{
	case 0://OPTION
		reply.SetOptions("Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n");
		break;
	case 1://DESCRIBE
		//TODO:sdp������
	{
		MyBuffer sdp;
		sdp << "v=0\r\n";
		sdp << "o=- " << (char*)m_id << " 1 IN IP4 127.0.0.1\r\n";
		sdp << "t=0 0\r\n" << "a=control:*\r\n" << "m=video 0 RTP/AVP 96\r\n";
		sdp << "a=framerate:24\r\n";
		sdp << "a=rtpmap:96 H264/90000\r\n" << "a=control:track0\r\n";
		reply.SetSdp(sdp);
	}
	break;
	case 2://SETUP
		reply.SetClientPort(request.port(0), request.port(1));
		reply.SetServerPort("55000", "55001");
		reply.SetSession(m_id);
		break;
	case 3://PLAY
	case 4://TEARDOWN
	default:
		break;
	}
	return reply;
}

RTSPRequest::RTSPRequest() {
	m_method = -1;
}

RTSPRequest::RTSPRequest(const RTSPRequest& rsp) {
	m_method = rsp.m_method;
	m_url = rsp.m_url;
	m_session = rsp.m_session;
	m_seq = rsp.m_seq;
	m_client_port[0] = rsp.m_client_port[0];
	m_client_port[1] = rsp.m_client_port[1];

}

RTSPRequest& RTSPRequest::operator=(const RTSPRequest& rsp)
{
	if (&rsp != this) {
		m_method = rsp.m_method;
		m_url = rsp.m_url;
		m_session = rsp.m_session;
		m_seq = rsp.m_seq;
		m_client_port[0] = rsp.m_client_port[0];
		m_client_port[1] = rsp.m_client_port[1];
	}
	return *this;
}

RTSPRequest::~RTSPRequest() {
	m_method = -1;
}

void RTSPRequest::SetMethod(const MyBuffer& method) {
	if (method == "OPTIONS") {
		m_method = 0;
	}
	else if (method == "DESCRIBE") {
		m_method = 1;
	}
	else if (method == "SETUP") {
		m_method = 2;
	}
	else if (method == "PLAY") {
		m_method = 3;
	}
	else if (method == "TEARDOWN") {
		m_method = 4;
	}
}

void RTSPRequest::SetUrl(const MyBuffer& url) {
	m_url = url;
}

void RTSPRequest::SetSession(const MyBuffer& session) {
	m_session = session;
}

void RTSPRequest::SetSequence(const MyBuffer& seq) {
	m_seq = seq;
}

void RTSPRequest::SetClientPort(int ports[]) {
	//m_client_port[0] = std::to_string(ports[0]).c_str();
	//m_client_port[1] = std::to_string(ports[1]).c_str();
	m_client_port[0] << ports[0];
	m_client_port[1] << ports[1];
}

int RTSPRequest::method() const {
	return m_method;
}

const MyBuffer& RTSPRequest::url() const {
	return m_url;
}

const MyBuffer& RTSPRequest::session() const
{
	return m_session;
}

const MyBuffer& RTSPRequest::sequence() const {
	return m_seq;
}

const MyBuffer& RTSPRequest::port(int index) const {

	return index ? m_client_port[1] : m_client_port[0];
}

RTSPReply::RTSPReply(){
	m_method = -1;
	m_client_port[0] = -1;
	m_client_port[1] = -1;
	m_server_port[0] = -1;
	m_server_port[1] = -1;
}

RTSPReply::RTSPReply(const RTSPReply& rsp){
	m_method = rsp.m_method;
	m_client_port[0] = rsp.m_client_port[0];
	m_client_port[1] = rsp.m_client_port[1];
	m_server_port[0] = rsp.m_server_port[0];
	m_server_port[1] = rsp.m_server_port[1];
	m_sdp = rsp.m_sdp;
	m_options = rsp.m_options;
	m_session = rsp.m_session;
	m_seq = rsp.m_seq;
}

RTSPReply& RTSPReply::operator=(const RTSPReply& rsp){
	if (&rsp != this) {
		m_method = rsp.m_method;
		m_client_port[0] = rsp.m_client_port[0];
		m_client_port[1] = rsp.m_client_port[1];
		m_server_port[0] = rsp.m_server_port[0];
		m_server_port[1] = rsp.m_server_port[1];
		m_sdp = rsp.m_sdp;
		m_options = rsp.m_options;
		m_session = rsp.m_session;
		m_seq = rsp.m_seq;
	}
	return *this;
}

RTSPReply::~RTSPReply(){
	m_method = -1;
}

MyBuffer RTSPReply::toBuffer(){
	MyBuffer result;
	result << "RTSP/1.0 200 OK\r\n" << "CSeq: " << (char*)m_seq << "\r\n";
	switch (m_method)
	{
	case 0://OPTIONS
		result << "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n\r\n";
		break;
	case 1://DESCRIBE
		result << "Content-Base: 127.0.0.1\r\n";
		result << "Content-type: application/sdp\r\n";
		result << "Content-length: " << m_sdp.size() << "\r\n\r\n";
		result << (char*)m_sdp;
		break;

		break;
	case 2://SETUP
		result << "Transport: RTP/AVP;unicast;client_port=" << m_client_port[0] << "-" << m_client_port[1];
		result<< ";server_port="<<m_server_port[0]<<"-"<<m_server_port[1]<<"\r\n";	
		result << "Session: " << (char*)m_session << "\r\n\r\n";
		break;
	case 3://PLAY
		result << "Range: npt=0.000-\r\n";
		result << "Session: " << (char*)m_session << "\r\n\r\n";
		//result<<"; timeout = 60\r\n";
		break;
	case 4://TEARDOWN
		result << "Session: " << (char*)m_session << "\r\n\r\n";
		break;
	default:
		break;
	}
	return result;
}

void RTSPReply::SetOptions(const MyBuffer& option){
	m_options = option;
}

void RTSPReply::SetSequence(const MyBuffer& seq){
	m_seq = seq;
}

void RTSPReply::SetSdp(const MyBuffer& sdp){
	m_sdp = sdp;
}

void RTSPReply::SetClientPort(const MyBuffer& port0, const MyBuffer& port1){
	port0 >> m_client_port[0];
	port1 >> m_client_port[1];
}

void RTSPReply::SetServerPort(const MyBuffer& port0, const MyBuffer& port1){
	port0 >> m_server_port[0];
	port1 >> m_server_port[1];
}

void RTSPReply::SetSession(const MyBuffer& session){
	m_session = session;
}

void RTSPReply::SetMethod(int method){
	m_method = method;
}

int RTSPReply::GetMethod(){
	return m_method;
}
