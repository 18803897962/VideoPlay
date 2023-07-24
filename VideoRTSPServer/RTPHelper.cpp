#include "RTPHelper.h"
#include<Windows.h>
#define RTP_MAX_PACKEG_SIZE 1300
RTPHelper::RTPHelper()
    :m_timestamp(0),m_udp(false)
{
    m_udp.Bind(EAddress("0.0.0.0",(short)55000));
    m_file = fopen("./out.bin", "wb+");
}
RTPHelper::~RTPHelper(){
    fclose(m_file);
}

////���ķְ�����
//int RTPHelper::SendMediaFrame(RTPFrame& rtpframe, MyBuffer& frame, const EAddress& client_addr){
//    int sepsize = GetFrameSepSize(frame);
//    size_t framesize = frame.size()- sepsize;
//    BYTE* prealframe = (BYTE*)frame + sepsize;//��ȥ�������ָ֡��
//    if (framesize > RTP_MAX_PACKEG_SIZE) {//��Ҫ��Ƭ����
//        size_t index = 0;//�ѷ��͵�size
//        rtpframe.m_pyload.resize(RTP_MAX_PACKEG_SIZE);
//        do {
//            ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//���õ�һ���ֽڵ�����  
//			if (index == 0)((BYTE*)rtpframe.m_pyload)[1] = 0x80 | (prealframe[0] & 0x1F);//���õڶ����ֽ� ��ʼ
//			else if (framesize - index <= RTP_MAX_PACKEG_SIZE) ((BYTE*)rtpframe.m_pyload)[1] = 0x40 | (prealframe[0] & 0x1F);//���õڶ����ֽ� ����
//            else ((BYTE*)rtpframe.m_pyload)[1] = (prealframe[0] & 0x1F);//���õڶ����ֽ� �м�
//            size_t sendsize = framesize - index > RTP_MAX_PACKEG_SIZE - 2 ? RTP_MAX_PACKEG_SIZE - 2 : framesize - index;
//            if (sendsize < RTP_MAX_PACKEG_SIZE - 2) rtpframe.m_pyload.resize(sendsize + 2);
//            memcpy(2 + (BYTE*)rtpframe.m_pyload.c_str(), prealframe + index, sendsize);
//            //TODO�������ܳ���ΪRTP_MAX_PACKEG_SIZE,���а����ְ��������ֽ���Ϣ��
//            //TODO:send
//            SendFrame(rtpframe,client_addr);//���һ�����ĳ��ȿ�����Ҫ���ģ���Ϊ��һ���ﵽ�������ȣ�������������������
//            rtpframe.m_head.serial++;
//            index += sendsize;
//        } while (index < framesize);
//    }
//    else {
//        rtpframe.m_pyload.resize(frame.size()-sepsize);
//		memcpy(rtpframe.m_pyload, prealframe, rtpframe.m_pyload.size());
//        //TODO������RTP��header
//        //������ۼӵģ���ʱ���һ���Ǽ�������ģ���0��ʼ��ÿ֡׷�� ʱ��Ƶ��90000/ÿ��24֡
//        SendFrame(rtpframe, client_addr);
//        rtpframe.m_head.serial++;
//    }
//    rtpframe.m_head.timestamp += 90000 / 24;
//    //udp������Ҫ������ߣ���Ϊudp��������̫�󣬿��ܻᵼ�º����صĶ������⣬���������Ҫ���Ʒ����ٶ�
//    Sleep(1000 / 30);
//    return 0;
//}

//���ķְ�����
int RTPHelper::SendMediaFrame(RTPFrame& rtpframe, MyBuffer& frame, const EAddress& client_addr) {
    int sepsize = GetFrameSepSize(frame);
    size_t frame_size = frame.size() - sepsize;
    BYTE* pFrame = (BYTE*)frame + sepsize;//��ȥ�������ָ֡��
    if (frame_size > RTP_MAX_PACKEG_SIZE) {//��Ҫ��Ƭ����
        size_t index = 0;//�ѷ��͵�size
        rtpframe.m_pyload.resize(RTP_MAX_PACKEG_SIZE + 2);
        ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//���õ�һ���ֽڵ�����  

        while(index < frame_size) {
            ((BYTE*)rtpframe.m_pyload)[1] = pFrame[0] & 0x1F;//���õڶ����ֽ� �м�   ��仰��Ҫ�������棬��Ϊÿ�ν��������ܷ����仯
			if (index == 0)((BYTE*)rtpframe.m_pyload)[1] |= 0x80;//���õڶ����ֽ� ��ʼ
			else if (frame_size - index <= RTP_MAX_PACKEG_SIZE) ((BYTE*)rtpframe.m_pyload)[1] |= 0x40;//���õڶ����ֽ� ����
            size_t sendsize = (frame_size - index) > RTP_MAX_PACKEG_SIZE ? RTP_MAX_PACKEG_SIZE : (frame_size - index);            
            if (sendsize < RTP_MAX_PACKEG_SIZE) rtpframe.m_pyload.resize(sendsize + 2);
			memcpy(2 + (BYTE*)rtpframe.m_pyload, pFrame + index + 1, sendsize);
            //�����ܳ���ΪRTP_MAX_PACKEG_SIZE,��ӷְ��������ֽ���Ϣ��
            SendFrame(rtpframe, client_addr);
            rtpframe.m_head.serial++;
            index += sendsize;
        }
    }
    else {
        rtpframe.m_pyload.resize(frame.size() - sepsize);
        memcpy(rtpframe.m_pyload, pFrame, rtpframe.m_pyload.size());
        //TODO������RTP��header
        //������ۼӵģ���ʱ���һ���Ǽ�������ģ���0��ʼ��ÿ֡׷�� ʱ��Ƶ��90000/ÿ��24֡
        SendFrame(rtpframe, client_addr);
        rtpframe.m_head.serial++;
    }
    rtpframe.m_head.timestamp += 90000 / 24;
    //udp������Ҫ������ߣ���Ϊudp��������̫�󣬿��ܻᵼ�º����صĶ������⣬���������Ҫ���Ʒ����ٶ�
    Sleep(1000 / 30);
    return 0;
}


int RTPHelper::SendFrame(const MyBuffer& frame, const EAddress& client_addr){
	fwrite(frame, 1, frame.size(), m_file);
    fwrite("00000000", 1, 8, m_file);
    fflush(m_file);
    int ret = sendto(m_udp,(char*)frame,frame.size(),0,client_addr,client_addr.Size());
    printf("send ret=%d framesize=%d ip=%s port=%d\r\n", ret, frame.size(), client_addr.IP().c_str(), client_addr.port());
    return ret;
}

int RTPHelper::GetFrameSepSize(const MyBuffer& frame){
    BYTE buf[] = { 0,0,0,1 };//��λ����ǰ֡��ʼ�������00 00 00 01 ���� 00 00 01�����ض�������ȣ�����ȥ�������
    if (memcmp((const void*)frame.c_str(), (const void*)buf, sizeof(buf)) == 0) {
        return 4;
    }
    return 3;
}

RTPHeader::RTPHeader(){
    csrccount = 0;
    extension = 0;
    padding = 0;
    version = 2;
    pytype = 96;
    mark = 0;
    serial = 0;
    timestamp = 0;
    ssrc = 0x98765432;
    memset(csrc, 0, sizeof(csrc));
}

RTPHeader::RTPHeader(const RTPHeader& header){
    int size = 12 + header.csrccount * 4;
    memcpy(this, &header, size);
}

RTPHeader& RTPHeader::operator=(const RTPHeader& header){
    // TODO: �ڴ˴����� return ���
    if (&header != this) {
        int size = 12 + header.csrccount * 4;
        memcpy(this, &header, size);
    }
    return *this;
}

RTPHeader::operator MyBuffer(){
    RTPHeader header = *this;
    header.serial = htons(serial);
    header.timestamp = htonl(timestamp);//�����ֽ���������ֽ���
	header.ssrc = htonl(ssrc);
    int size = 12 + 4 * csrccount;
    MyBuffer result(size);
    memcpy((BYTE*)result.c_str(), &header, size);
    return result;
}

RTPFrame::operator MyBuffer(){
    MyBuffer result;
    result += (MyBuffer)m_head;
    result += m_pyload;
    return result;
}
