#include <iostream>
#include<Windows.h>
#include<conio.h>
#include"vlc.h"
//编码问题，多字节转utf-8  步骤为 多字节 -- unicode -- utf-8
std::string unicode2utf8(const std::wstring& strin) {
    std::string str;
    //当不确定转换后的编码大小时，可先不填写目标串，返回值即为转换后的编码大小
    int length = ::WideCharToMultiByte(CP_UTF8, 0, strin.c_str(), strin.size(), NULL, 0, NULL, NULL);
    str.resize(length);
    ::WideCharToMultiByte(CP_UTF8, 0, strin.c_str(), strin.size(), (LPSTR)str.c_str(), str.size(), NULL, NULL);
    return str;
}
int main()
{
    int argc = 1;
    char* argv[2];
    argv[0] = (char*)"--ignore-config";//忽略配置
    //获取vlc的instance
    libvlc_instance_t* vlc_ins = libvlc_new(argc,argv);
    //解决编码问题
    std::string path = unicode2utf8(L"股市讨论.mp4");//L类型直接为Unicode 将其转换为utf-8即可

    //获取媒体
    libvlc_media_t* media = libvlc_media_new_path(vlc_ins,path.c_str());
    //media = libvlc_media_new_location(vlc_ins,"file:///D:\\lab\\股市讨论.mp4");//另一种方式
    
    //获取player
    libvlc_media_player_t* player = libvlc_media_player_new_from_media(media);

    //开始播放
    do {
        int ret = libvlc_media_player_play(player);
        if (ret == -1) {
            printf("error found\r\n");
            break;
        }
        
        //等待媒体加载完成
        //获取音量
        int vol = -1;
        while (vol == -1) {
            vol = libvlc_audio_get_volume(player);
        }
        printf("vol = %d\r\n", vol);
        libvlc_audio_set_volume(player, 10);

        //获取时长
        libvlc_time_t tm = libvlc_media_player_get_length(player);
		printf("%02d:%02d:%02d\r\n", int(tm / 1000 / 3600), int((tm / 1000) / 60), int(tm / 1000));

        //获取宽和高
        int width = libvlc_video_get_width(player);
        int height = libvlc_video_get_height(player);
        printf("width = %d,height = %d\r\n", width, height);
        
        while (!_kbhit()) {
            printf("%f%%\r", 100.0 * libvlc_media_player_get_position(player));
            Sleep(500);
        }
        //按键操作 任意键暂停、继续、停止
        getchar();
        libvlc_media_player_pause(player);
        getchar();
        libvlc_media_player_play(player);
        getchar();
        libvlc_media_player_stop(player);
    } while (0);
    
    //释放资源
    libvlc_media_player_release(player);//释放player
    libvlc_media_release(media);//释放media
    libvlc_release(vlc_ins);//释放instance
}
