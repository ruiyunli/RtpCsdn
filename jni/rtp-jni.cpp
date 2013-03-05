
/*
 * RTP�������̣�
 * 1��������Ϣ
 * 	     �������ض˿ڣ�Ŀ��˿ڣ�Ŀ��ip�����͵����ݰ�����
 * 2��ip���ݽṹת��
 * 3��ʱ�������
 * 4�������Ự
 * 5�����ͼ��������ݰ�
 *
 */

#include <jni.h>
#include "rtpsession.h"				//������rtpsession��һЩʵ��
#include "rtpudpv4transmitter.h"	//������RTPSession�ĵڶ���������
#include "rtpipv4address.h"			//������rtpipv4address
#include "rtpsessionparams.h"		//������rtpsession�ĵ�һ��������
#include "rtperrors.h"				//������RTP�еĴ�����Ϣ
#include <netinet/in.h>	//not win32
#include <arpa/inet.h>	//not win32
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

//
#ifdef __cplusplus
extern "C" {
void Java_com_example_rtpcsdn_MainActivity_RtpTest(JNIEnv *, jobject);
}
#endif

/**** ����̨��� ****/
#include <android/log.h>
#define  LOG_TAG "RTP_JNI"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG, __VA_ARGS__)


//������Ϣ
#define LOCAL_PORT 10000
#define DEST_PORT 10000
#define DEST_IP "127.0.0.1"
#define PACKET_NUM 10
#define TIME_STAMP 1.0/10.0


void Java_com_example_rtpcsdn_MainActivity_RtpTest(JNIEnv* env, jobject thiz)
{
 	RTPSession sess;	//RTPSession����ʵ�����˴ε�RTP�Ự
	uint16_t portbase,destport;
	uint32_t destip;
	std::string ipstr;
	int status,i,num;

    //1��������Ϣ
	//�������ض˿ڣ�Ŀ��˿ڣ�Ŀ��ip�����͵����ݰ�����
	portbase=LOCAL_PORT;
    ipstr=DEST_IP;
    destport=DEST_PORT;
	num=PACKET_NUM;

	//2��ip���ݽṹת��
	//��ֱ��ʽת��Ϊ�����ֽ�����
	//a.b.c.d-->d*2^24+c*2^16+b*2^8+d*2^0
	destip = inet_addr(ipstr.c_str());
	if (destip == INADDR_NONE)
	{
		LOGE("BAD IP ADDR");
		return ;
	}
	//�����ֽ�����ת��Ϊ�����ֽ�����
	//network to host long
	destip = ntohl(destip);
	
	//���洴��һ��RTP�Ự�����ʹ�������ݰ�

	//����rtpsession�ĵڶ��������࣬���ĳ�Ա���������趨�����˿�
	RTPUDPv4TransmissionParams transparams;

	//����rtpsession�ĵ�һ�������࣬���ĳ�Ա������������ǡ����ʱ����Ԫ
	RTPSessionParams sessparams;
	
	//3��ʱ�������
	//����ǡ����ʱ����Ԫ��ÿ��������Ҫ����10�Σ��ʲ���Ϊ1.0/10
	sessparams.SetOwnTimestampUnit(TIME_STAMP);
	//���������ǲ��ǽ��������Զ�������ݰ�������ѡ��
	sessparams.SetAcceptOwnPackets(true);
	//���ñ����˿�
	transparams.SetPortbase(portbase);

	//4�������Ự
	status = sess.Create(sessparams,&transparams);	
//	checkerror(status);
	//���Ŀ�ĵ�ַ
	RTPIPv4Address addr(destip,destport);
	//����Ŀ�ĵ�ַ�����ӷ��͵�Ŀ�ĵ�ַ����Ȼ�������Ӻܶ��ַ��ɶಥ�Ĺ���
	//���⻹����ʹ��DeleteDestination()�� ClearDestinations()��ɾ�������Ŀ�ĵ�ַ
	//Ҳ����д�ɣ� unsigned long addr = ntohl��inet_addr("127.0.0.1")��;
	// sess.AddDestination(addr,6000);
	status = sess.AddDestination(addr);
//	checkerror(status);

	LOGD("LocalPort:%d",portbase);
	LOGD("DestPort:%d",destport);
	LOGD(DEST_IP);
	LOGD("ready!");

	//5�����ͼ��������ݰ�
	for (i = 1 ; i <= num ; i++)
	{
		//������ý������.
		//��һ�������Ƿ��͵����ݣ����������ݳ��ȣ�������rtp�������ͣ���ʶ��ʱ����
		//��ȻJRTPLIB�����������óɻỰ��Ĭ�ϲ�����
		//���ǵ���RTPSession���setDefaultpayloadtype(),setDefaultmark���ȷ�����
		//��ɵģ������������֮�����ǿ�����������������
		//status = sess.SendPacket((void *)"1234567890",10);
		status = sess.SendPacket((void *)"1234567890",10,0,false,10);
		
		LOGD("send packet:%d",i);

		//Ϊ�˱�֤source table������Դ�б�����Ķ�������GotoFirstSourceWithData�Ⱥ���
		//��ִ�б���Ҫ��BeginDataAccess��EndDataAccess֮�䣬������⺯������
		sess.BeginDataAccess();

		//�յ��ı��ģ���������Я�����ݵ�Դ����Ϊһ��rtp�Ự�����ж�������ߣ�Դ����
		if (sess.GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				
				while ((pack = sess.GetNextPacket()) != NULL)
				{
					// You can examine the data here
					LOGD("!!!!!Got packet!!!!!");
					// we don't longer need the packet, so we'll delete it
					sess.DeletePacket(pack);
				}
			} while (sess.GotoNextSourceWithData());
		}
		
		sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
		status = sess.Poll();
//		checkerror(status);
#endif // RTP_SUPPORT_THREAD
		
		RTPTime::Wait(RTPTime(1,0));
	}
	
	sess.BYEDestroy(RTPTime(10,0),0,0);


	return;
}


