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

////最大的分包长度
//int RTPHelper::SendMediaFrame(RTPFrame& rtpframe, MyBuffer& frame, const EAddress& client_addr){
//    int sepsize = GetFrameSepSize(frame);
//    size_t framesize = frame.size()- sepsize;
//    BYTE* prealframe = (BYTE*)frame + sepsize;//除去定界符的帧指针
//    if (framesize > RTP_MAX_PACKEG_SIZE) {//需要分片发送
//        size_t index = 0;//已发送的size
//        rtpframe.m_pyload.resize(RTP_MAX_PACKEG_SIZE);
//        do {
//            ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//设置第一个字节的内容  
//			if (index == 0)((BYTE*)rtpframe.m_pyload)[1] = 0x80 | (prealframe[0] & 0x1F);//设置第二个字节 开始
//			else if (framesize - index <= RTP_MAX_PACKEG_SIZE) ((BYTE*)rtpframe.m_pyload)[1] = 0x40 | (prealframe[0] & 0x1F);//设置第二个字节 结束
//            else ((BYTE*)rtpframe.m_pyload)[1] = (prealframe[0] & 0x1F);//设置第二个字节 中间
//            size_t sendsize = framesize - index > RTP_MAX_PACKEG_SIZE - 2 ? RTP_MAX_PACKEG_SIZE - 2 : framesize - index;
//            if (sendsize < RTP_MAX_PACKEG_SIZE - 2) rtpframe.m_pyload.resize(sendsize + 2);
//            memcpy(2 + (BYTE*)rtpframe.m_pyload.c_str(), prealframe + index, sendsize);
//            //TODO：发送总长度为RTP_MAX_PACKEG_SIZE,其中包含分包的两个字节信息。
//            //TODO:send
//            SendFrame(rtpframe,client_addr);//最后一个包的长度可能需要更改，因为不一定达到最大包长度？？？？？？？？？？
//            rtpframe.m_head.serial++;
//            index += sendsize;
//        } while (index < framesize);
//    }
//    else {
//        rtpframe.m_pyload.resize(frame.size()-sepsize);
//		memcpy(rtpframe.m_pyload, prealframe, rtpframe.m_pyload.size());
//        //TODO：处理RTP的header
//        //序号是累加的，而时间戳一般是计算出来的，从0开始，每帧追加 时钟频率90000/每秒24帧
//        SendFrame(rtpframe, client_addr);
//        rtpframe.m_head.serial++;
//    }
//    rtpframe.m_head.timestamp += 90000 / 24;
//    //udp发送需要添加休眠，因为udp发送速率太大，可能会导致很严重的丢包问题，因此我们需要控制发送速度
//    Sleep(1000 / 30);
//    return 0;
//}

//最大的分包长度
int RTPHelper::SendMediaFrame(RTPFrame& rtpframe, MyBuffer& frame, const EAddress& client_addr) {
    int sepsize = GetFrameSepSize(frame);
    size_t frame_size = frame.size() - sepsize;
    BYTE* pFrame = (BYTE*)frame + sepsize;//除去定界符的帧指针
    if (frame_size > RTP_MAX_PACKEG_SIZE) {//需要分片发送
        size_t index = 0;//已发送的size
        rtpframe.m_pyload.resize(RTP_MAX_PACKEG_SIZE + 2);
        ((BYTE*)rtpframe.m_pyload)[0] = 0x60 | 28;//设置第一个字节的内容  

        while(index < frame_size) {
            ((BYTE*)rtpframe.m_pyload)[1] = pFrame[0] & 0x1F;//设置第二个字节 中间   这句话需要放在里面，因为每次进来都可能发生变化
			if (index == 0)((BYTE*)rtpframe.m_pyload)[1] |= 0x80;//设置第二个字节 开始
			else if (frame_size - index <= RTP_MAX_PACKEG_SIZE) ((BYTE*)rtpframe.m_pyload)[1] |= 0x40;//设置第二个字节 结束
            size_t sendsize = (frame_size - index) > RTP_MAX_PACKEG_SIZE ? RTP_MAX_PACKEG_SIZE : (frame_size - index);            
            if (sendsize < RTP_MAX_PACKEG_SIZE) rtpframe.m_pyload.resize(sendsize + 2);
			memcpy(2 + (BYTE*)rtpframe.m_pyload, pFrame + index + 1, sendsize);
            //发送总长度为RTP_MAX_PACKEG_SIZE,外加分包的两个字节信息。
            SendFrame(rtpframe, client_addr);
            rtpframe.m_head.serial++;
            index += sendsize;
        }
    }
    else {
        rtpframe.m_pyload.resize(frame.size() - sepsize);
        memcpy(rtpframe.m_pyload, pFrame, rtpframe.m_pyload.size());
        //TODO：处理RTP的header
        //序号是累加的，而时间戳一般是计算出来的，从0开始，每帧追加 时钟频率90000/每秒24帧
        SendFrame(rtpframe, client_addr);
        rtpframe.m_head.serial++;
    }
    rtpframe.m_head.timestamp += 90000 / 24;
    //udp发送需要添加休眠，因为udp发送速率太大，可能会导致很严重的丢包问题，因此我们需要控制发送速度
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
    BYTE buf[] = { 0,0,0,1 };//定位到当前帧起始定界符是00 00 00 01 还是 00 00 01，返回定界符长度，用于去除定界符
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
    // TODO: 在此处插入 return 语句
    if (&header != this) {
        int size = 12 + header.csrccount * 4;
        memcpy(this, &header, size);
    }
    return *this;
}

RTPHeader::operator MyBuffer(){
    RTPHeader header = *this;
    header.serial = htons(serial);
    header.timestamp = htonl(timestamp);//主机字节序改网络字节序
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
