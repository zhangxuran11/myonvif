#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "soapH.h"
#include "wsaapi.h"
#include "wsddapi.h"
#include "wsseapi.h"
#include "wsdd.nsmap"
//#include "onvif_dump.h"

#define SOAP_ASSERT     assert
#define SOAP_DBGLOG     printf
#define SOAP_DBGERR     printf

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单秒秒）

void soap_perror(struct soap *soap, const char *str)
{
    if (NULL == str) {
        SOAP_DBGERR("[soap] error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    } else {
        SOAP_DBGERR("[soap] %s error: %d, %s, %s\n", str, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
    }
    return;
}

void* ONVIF_soap_malloc(struct soap *soap, unsigned int n)
{
    void *p = NULL;

    if (n > 0) {
        p = soap_malloc(soap, n);
        SOAP_ASSERT(NULL != p);
        memset(p, 0x00 ,n);
    }
    return p;
}

struct soap *ONVIF_soap_new(int timeout)
{
    struct soap *soap = NULL;                                                   // soap环境变量

    SOAP_ASSERT(NULL != (soap = soap_new()));

    soap_set_namespaces(soap, namespaces);                                      // 设置soap的namespaces
    soap->recv_timeout    = timeout;                                            // 设置超时（超过指定时间没有数据就退出）
    soap->send_timeout    = timeout;
    soap->connect_timeout = timeout;

#if defined(__linux__) || defined(__linux)                                      // 参考https://www.genivia.com/dev.html#client-c的修改：
    soap->socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
#endif

    soap_set_mode(soap, SOAP_C_UTFSTRING);                                      // 设置为UTF-8编码，否则叠加中文OSD会乱码

    return soap;
}

void ONVIF_soap_delete(struct soap *soap)
{
    soap_destroy(soap);                                                         // remove deserialized class instances (C++ only)
    soap_end(soap);                                                             // Clean up deserialized data (except class instances) and temporary data
    soap_done(soap);                                                            // Reset, close communications, and remove callbacks
    soap_free(soap);                                                            // Reset and deallocate the context created with soap_new or soap_copy
}

/************************************************************************
 * **函数：ONVIF_init_header
 * **功能：初始化soap描述消息头
 * **参数：
 *         [in] soap - soap环境变量
 *         **返回：无
 *         **备注：
 *             1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
 *             ************************************************************************/
void ONVIF_init_header(struct soap *soap)
{
    struct SOAP_ENV__Header *header = NULL;

    SOAP_ASSERT(NULL != soap);

    header = (struct SOAP_ENV__Header *)ONVIF_soap_malloc(soap, sizeof(struct SOAP_ENV__Header));
    soap_default_SOAP_ENV__Header(soap, header);
    header->wsa__MessageID = (char*)soap_wsa_rand_uuid(soap);
    header->wsa__To        = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TO) + 1);
    header->wsa__Action    = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ACTION) + 1);
    strcpy(header->wsa__To, SOAP_TO);
    strcpy(header->wsa__Action, SOAP_ACTION);
    soap->header = header;

    return;
}

/************************************************************************
 * **函数：ONVIF_init_ProbeType
 * **功能：初始化探测设备的范围和类型
 * **参数：
 *         [in]  soap  - soap环境变量
 *         [out] probe - 填充要探测的设备范围和类型
 *         **返回：
 *         0表明探测到，非0表明未探测到
 *         **备注：
 *         1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
 *************************************************************************/
void ONVIF_init_ProbeType(struct soap *soap, struct wsdd__ProbeType *probe)
{
    struct wsdd__ScopesType *scope = NULL;                                      // 用于描述查找哪类的Web服务

    SOAP_ASSERT(NULL != soap);
    SOAP_ASSERT(NULL != probe);

    scope = (struct wsdd__ScopesType *)ONVIF_soap_malloc(soap, sizeof(struct wsdd__ScopesType));
    soap_default_wsdd__ScopesType(soap, scope);                                 // 设置寻找设备的范围
    scope->__item = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_ITEM) + 1);
    strcpy(scope->__item, SOAP_ITEM);

    memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
    soap_default_wsdd__ProbeType(soap, probe);
    probe->Scopes = scope;
    probe->Types  = (char*)ONVIF_soap_malloc(soap, strlen(SOAP_TYPES) + 1);     // 设置寻找设备的类型
    strcpy(probe->Types, SOAP_TYPES);

    return;
}

void ONVIF_DetectDevice(void (*cb)(char *DeviceXAddr))
{
    int i;
    int result = 0;
    unsigned int count = 0;                                                     // 搜索到的设备个数
    struct soap *soap = NULL;                                                   // soap环境变量
    struct wsdd__ProbeType      req;                                            // 用于发送Probe消息
    struct __wsdd__ProbeMatches rep;                                            // 用于接收Probe应答
    struct wsdd__ProbeMatchType *probeMatch;

    SOAP_ASSERT(NULL != (soap = ONVIF_soap_new(SOAP_SOCK_TIMEOUT)));

    ONVIF_init_header(soap);                                                    // 设置消息头描述
    ONVIF_init_ProbeType(soap, &req);                                           // 设置寻找的设备的范围和类型
    result = soap_send___wsdd__Probe(soap, SOAP_MCAST_ADDR, NULL, &req);        // 向组播地址广播Probe消息
    while (SOAP_OK == result)                                                   // 开始循环接收设备发送过来的消息
    {
        memset(&rep, 0x00, sizeof(rep));
        result = soap_recv___wsdd__ProbeMatches(soap, &rep);
        if (SOAP_OK == result) {
            if (soap->error) {
                soap_perror(soap, "ProbeMatches");
            } else {                                                  // 成功接收到设备的应答消息
                //dump__wsdd__ProbeMatches(&rep);

                if (NULL != rep.wsdd__ProbeMatches) {
                    count += rep.wsdd__ProbeMatches->__sizeProbeMatch;
                    for(i = 0; i < rep.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
                        probeMatch = rep.wsdd__ProbeMatches->ProbeMatch + i;
                        if (NULL != cb) {
                            cb(probeMatch->XAddrs);                             // 使用设备服务地址执行函数回调
                        }
                    }
                }
            }
        } else if (soap->error) {
            break;
        }
    }

    SOAP_DBGLOG("\ndetect end! It has detected %d devices!\n", count);

    if (NULL != soap) {
        ONVIF_soap_delete(soap);
    }

    return ;
}
static void cb(char *DeviceXAddr){
    printf("%s\n",DeviceXAddr);

}
#define USERNAME    "admin"
#define PASSWORD    "hidoo123"

static void ONVIF_PTZ()
{
    struct soap soap;
	soap_init(&soap);
			
    const char * ip;
    char Mediaddr[256]="";
    char profile[256]="";
    float pan = 1;
    float panSpeed = 1;
    float tilt = 1;
    float tiltSpeed = 0.5;
    float zoom = 0;
    float zoomSpeed = 0.5;
    struct _tds__GetCapabilities            	req;
    struct _tds__GetCapabilitiesResponse    	rep;
    struct _trt__GetProfiles 			getProfiles;
    struct _trt__GetProfilesResponse		response;	
    struct _tptz__AbsoluteMove           absoluteMove;
    struct _tptz__AbsoluteMoveResponse   absoluteMoveResponse;
	       	
    //req.Category = (enum tt__CapabilityCategory *)soap_malloc(&soap, sizeof(int));
    //req.__sizeCategory = 1;
    //*(req.Category) = (enum tt__CapabilityCategory)0;
    req.Category.resize(1);
    req.Category[0] = tt__CapabilityCategory__All;
       
    //第一步：获取capability
    char endpoint[255];
    memset(endpoint, '\0', 255);
    
    ip = "192.168.10.66"; 
    sprintf(endpoint, "http://%s/onvif/device_service", ip);    
    soap_call___tds__GetCapabilities(&soap, endpoint, NULL, &req, rep);
    if (soap.error)  
    {  
        printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
	                                        soap.error, *soap_faultcode(&soap), 
	                                        *soap_faultstring(&soap));  	 
    } 
    else
	{
        printf("get capability success\n");
        //printf("Dev_XAddr====%s\n",rep.Capabilities->Device->XAddr);
        printf("Med_XAddr====%s\n",rep.Capabilities->Media->XAddr.c_str());
        //printf("PTZ_XAddr====%s\n",rep.Capabilities->PTZ->XAddr);
        strcpy(Mediaddr,rep.Capabilities->Media->XAddr.c_str());
    }	
    printf("\n");
	
    //第二步：获取profile,需要鉴权	
    //自动鉴权
    soap_wsse_add_UsernameTokenDigest(&soap, NULL, USERNAME, PASSWORD);
	
    //获取profile
    if(soap_call___trt__GetProfiles(&soap,Mediaddr,NULL,&getProfiles,response)==SOAP_OK)
    {
        strcpy(profile, response.Profiles[0]->token.c_str());
        printf("get profile succeed \n");		
	    printf("profile====%s\n",profile);	
    }
    else
    {
        printf("get profile failed \n");
	    printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
	                                        soap.error, *soap_faultcode(&soap), 
	                                        *soap_faultstring(&soap));  
    }
    printf("\n");	
		
    //第三步：PTZ结构体填充
    char PTZendpoint[255];
    memset(PTZendpoint, '\0', 255);
    sprintf(PTZendpoint, "http://%s/onvif/PTZ", ip);
    printf("PTZendpoint is %s \n", PTZendpoint);        
    
    absoluteMove.ProfileToken = profile;
    //setting pan and tilt
    absoluteMove.Position = soap_new_tt__PTZVector(&soap, -1);
    absoluteMove.Position->PanTilt = soap_new_tt__Vector2D(&soap, -1);
    absoluteMove.Speed = soap_new_tt__PTZSpeed(&soap, -1);
    absoluteMove.Speed->PanTilt = soap_new_tt__Vector2D(&soap, -1);
    //pan
    absoluteMove.Position->PanTilt->x = pan;
    absoluteMove.Speed->PanTilt->x = panSpeed;
    //tilt
    absoluteMove.Position->PanTilt->y = tilt;
    absoluteMove.Speed->PanTilt->y = tiltSpeed;
    //setting zoom
    absoluteMove.Position->Zoom = soap_new_tt__Vector1D(&soap, -1);
    absoluteMove.Speed->Zoom = soap_new_tt__Vector1D(&soap, -1);
    absoluteMove.Position->Zoom->x = zoom;
    absoluteMove.Speed->Zoom->x = zoomSpeed;
    
    //第四步：执行绝对位置控制指令，需要再次鉴权
    soap_wsse_add_UsernameTokenDigest(&soap, NULL, USERNAME, PASSWORD);
    if(soap_call___tptz__AbsoluteMove(&soap, PTZendpoint, NULL, &absoluteMove, 
	                                        absoluteMoveResponse)==SOAP_OK)	
    {
        printf("get tptz__AbsoluteMove succeed \n");		
	    //printf("profile====%s\n",profile);	
    }
    else
    {
        printf("get tptz__AbsoluteMove failed \n");
	    printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
	                                        soap.error, *soap_faultcode(&soap), 
	                                        *soap_faultstring(&soap));  
    }		 
    //第五步：清除结构体
    soap_destroy(&soap); // clean up class instances
    soap_end(&soap); // clean up everything and close socket, // userid and passwd were deallocated
    soap_done(&soap); // close master socket and detach context
    printf("\n");	
		
    return ;
}

int main(int argc, char **argv)
{
    //ONVIF_DetectDevice(cb);
    ONVIF_PTZ();

    return 0;
}
