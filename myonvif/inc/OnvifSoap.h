#ifndef ONVIF_SOAP_H
#define ONVIF_SOAP_H

#include "soapH.h"
#include "wsaapi.h"
#include "wsddapi.h"
#include "wsseapi.h"
#include "wsdd.nsmap"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define SOAP_DBGLOG     printf
class OnvifSoap{
        struct soap mSoap;
        /************************************************************************
         * **_initProbeType
         * **功能：初始化探测设备的范围和类型
         * **参数：
         *         [in]  soap  - soap环境变量
         *         [out] probe - 填充要探测的设备范围和类型
         *         **返回：
         *         0表明探测到，非0表明未探测到
         *         **备注：
         *         1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
         *************************************************************************/
        void _initProbeType( struct wsdd__ProbeType *probe)
        {
            #define SOAP_ITEM       ""                                                      // 寻找的设备范围
            #define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型
            struct wsdd__ScopesType *scope = NULL;                                      // 用于描述查找哪类的Web服务

            assert(NULL != probe);

            scope = (wsdd__ScopesType*)soap_malloc(&mSoap,sizeof(wsdd__ScopesType));
            memset(scope,0,sizeof(wsdd__ScopesType));
            soap_default_wsdd__ScopesType(&mSoap, scope);                                 // 设置寻找设备的范围
            scope->__item = (char*)soap_malloc(&mSoap,strlen(SOAP_ITEM) + 1);
            strcpy(scope->__item, SOAP_ITEM);

            memset(probe, 0x00, sizeof(struct wsdd__ProbeType));
            soap_default_wsdd__ProbeType(&mSoap, probe);
            probe->Scopes = scope;
            probe->Types  = (char*)soap_malloc(&mSoap,strlen(SOAP_TYPES) + 1); // 设置寻找设备的类型
            strcpy(probe->Types, SOAP_TYPES);

            return;
        }
        void _deinitProbeType( struct wsdd__ProbeType *probe){
            soap_dealloc(&mSoap,probe->Types);
            soap_dealloc(&mSoap,probe->Scopes->__item);
            soap_dealloc(&mSoap,probe->Scopes);
        }

        void _initHeader(){
            // 设置消息头描述
            SOAP_ENV__Header* header = (SOAP_ENV__Header*)soap_malloc(&mSoap,sizeof(SOAP_ENV__Header));
            soap_default_SOAP_ENV__Header(&mSoap, header);
            header->wsa__MessageID = (char*)soap_wsa_rand_uuid(&mSoap);
            header->wsa__To        = (char*)soap_malloc(&mSoap,strlen(SOAP_TO) + 1); 
            header->wsa__Action    = (char*)soap_malloc(&mSoap,strlen(SOAP_ACTION) + 1);
            strcpy(header->wsa__To, SOAP_TO);
            strcpy(header->wsa__Action, SOAP_ACTION);
            mSoap.header = header;
        }
        
        int _getProfiles(const char* mediaAddr,std::string& profiles,const char*userName,const char*pwd){
            struct _trt__GetProfiles 			getProfiles;
            struct _trt__GetProfilesResponse		response;	
            //第二步：获取profile,需要鉴权	
            //自动鉴权
            soap_wsse_add_UsernameTokenDigest(&mSoap, NULL, userName, pwd);
            
            //获取profile
            if(soap_call___trt__GetProfiles(&mSoap,mediaAddr,NULL,&getProfiles,response)==SOAP_OK)
            {
                profiles = response.Profiles[0]->token;
                printf("get profile succeed \n");		
                printf("profile====%s\n",profiles.c_str());	
                return 0;
            }
            else
            {
                printf("get profile failed \n");
                printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
                                                    mSoap.error, *soap_faultcode(&mSoap), 
                                                    *soap_faultstring(&mSoap));  
                return -1;
            }	
        }
        int _getAddr(const char* ip,std::string& mediaXAddr,std::string& ptzXAddr){
            struct _tds__GetCapabilities            	req;
            struct _tds__GetCapabilitiesResponse    	rep;
            req.Category.resize(1);
            req.Category[0] = tt__CapabilityCategory__All;

            //第一步：获取capability
            char endpoint[255];
            memset(endpoint, '\0', 255);
            
            sprintf(endpoint, "http://%s/onvif/device_service", ip);
            soap_call___tds__GetCapabilities(&mSoap, endpoint, NULL, &req, rep);
            if (mSoap.error)  
            {  
                printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
                                                    mSoap.error, *soap_faultcode(&mSoap), 
                                                    *soap_faultstring(&mSoap));
                return -1;
            } 
            else
            {
                printf("get capability success\n");
                //printf("Dev_XAddr====%s\n",rep.Capabilities->Device->XAddr);
                printf("Med_XAddr====%s\n",rep.Capabilities->Media->XAddr.c_str());
                printf("PTZ_XAddr====%s\n",rep.Capabilities->PTZ->XAddr.c_str());
                mediaXAddr = rep.Capabilities->Media->XAddr;
                ptzXAddr = rep.Capabilities->PTZ->XAddr;
                return 0;
            }	

        }
        int _absoluteMove(const char* profile,const char* ptzXAddr,const char*userName,const char*pwd)
        {
            float pan = 1;
            float panSpeed = 1;
            float tilt = 1;
            float tiltSpeed = 0.5;
            float zoom = 0;
            float zoomSpeed = 0.5;
            
            struct _tptz__AbsoluteMove           absoluteMove;
            struct _tptz__AbsoluteMoveResponse   absoluteMoveResponse;
            //第三步：PTZ结构体填充      
            
            absoluteMove.ProfileToken = profile;
            //setting pan and tilt
            absoluteMove.Position = soap_new_tt__PTZVector(&mSoap, -1);
            absoluteMove.Position->PanTilt = soap_new_tt__Vector2D(&mSoap, -1);
            absoluteMove.Speed = soap_new_tt__PTZSpeed(&mSoap, -1);
            absoluteMove.Speed->PanTilt = soap_new_tt__Vector2D(&mSoap, -1);
            //pan
            absoluteMove.Position->PanTilt->x = pan;
            absoluteMove.Speed->PanTilt->x = panSpeed;
            //tilt
            absoluteMove.Position->PanTilt->y = tilt;
            absoluteMove.Speed->PanTilt->y = tiltSpeed;
            //setting zoom
            absoluteMove.Position->Zoom = soap_new_tt__Vector1D(&mSoap, -1);
            absoluteMove.Speed->Zoom = soap_new_tt__Vector1D(&mSoap, -1);
            absoluteMove.Position->Zoom->x = zoom;
            absoluteMove.Speed->Zoom->x = zoomSpeed;
            
            //第四步：执行绝对位置控制指令，需要再次鉴权
            soap_wsse_add_UsernameTokenDigest(&mSoap, NULL, userName, pwd);
            if(soap_call___tptz__AbsoluteMove(&mSoap, ptzXAddr, NULL, &absoluteMove, 
                                                    absoluteMoveResponse)==SOAP_OK)	
            {
                printf("get tptz__AbsoluteMove succeed \n");		
                //printf("profile====%s\n",profile);	
            }
            else
            {
                printf("get tptz__AbsoluteMove failed \n");
                printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
                                                    mSoap.error, *soap_faultcode(&mSoap), 
                                                    *soap_faultstring(&mSoap));  
            }		 

            return 0;
        }
        //opt   0:up    1:down  2:left  3:right 4:room out  5:room in   6:stop
        int _continuousMove(const char* profile,const char* ptzXAddr,const char*userName,const char*pwd,int opt)
        {   
            struct _tptz__ContinuousMove           continuousMove;
            struct _tptz__ContinuousMoveResponse   continuousMoveResponse;
            //第三步：PTZ结构体填充      
            
            continuousMove.ProfileToken = profile;
            //setting pan and tilt
            continuousMove.Velocity  = soap_new_tt__PTZSpeed(&mSoap, -1);
            continuousMove.Velocity->PanTilt = soap_new_tt__Vector2D(&mSoap, -1);
            continuousMove.Velocity->PanTilt->space = new std::string("http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace");
            continuousMove.Velocity->Zoom = soap_new_tt__Vector1D(&mSoap, -1);
            float speed = 10;

            #define UP 0
            #define DOWN (UP+1)
            #define LEFT     (DOWN+1)
            #define RIGHT     (LEFT+1)
            #define ZOOMOUT     (RIGHT+1)
            #define ZOOMIN     (ZOOMOUT+1)

            switch (opt)
            {
            case LEFT:
                continuousMove.Velocity->PanTilt->x = -((float)speed / 10);
                continuousMove.Velocity->PanTilt->y = 0;
                break;
            case RIGHT:
                continuousMove.Velocity->PanTilt->x = ((float)speed / 10);
                continuousMove.Velocity->PanTilt->y = 0;
                break;
            case UP:
                continuousMove.Velocity->PanTilt->x = 0;
                continuousMove.Velocity->PanTilt->y = ((float)speed / 10);
                break;
            case DOWN:
                continuousMove.Velocity->PanTilt->x = 0;
                continuousMove.Velocity->PanTilt->y = -((float)speed / 10);
                break;
            case ZOOMIN:
                continuousMove.Velocity->Zoom->x = ((float)speed / 10);
                break;
            case ZOOMOUT:
                continuousMove.Velocity->Zoom->x = -((float)speed / 10);
                break;
            default:
                break;
            }
            //第四步：执行绝对位置控制指令，需要再次鉴权
            soap_wsse_add_UsernameTokenDigest(&mSoap, NULL, userName, pwd);
            if (soap_call___tptz__ContinuousMove(&mSoap,ptzXAddr,NULL,&continuousMove,continuousMoveResponse) == SOAP_OK){
                //转动成功
                printf("get tptz__ContinuousMove succeed \n");		

            }
            else{
                printf("get tptz__ContinuousMove failed \n");
                printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
                                                    mSoap.error, *soap_faultcode(&mSoap), 
                                                    *soap_faultstring(&mSoap));
            }		 

            return 0;
        }
        int _stopMove(const char* profile,const char* ptzXAddr,const char*userName,const char*pwd)
        {
            _tptz__Stop stop;
            _tptz__StopResponse stopResponse;
            //memset(&stop,0,sizeof(_tptz__Stop));
            //memset(&stopResponse,0,sizeof(_tptz__StopResponse));
            stop.ProfileToken = profile;

            bool* pantilt = new bool;
            stop.PanTilt = pantilt;
            *(stop.PanTilt) = true;
            bool* zoom = new bool;
            stop.Zoom = zoom;
            *(stop.Zoom) = true;

            soap_wsse_add_UsernameTokenDigest(&mSoap, NULL, userName, pwd);
            if (SOAP_OK == soap_call___tptz__Stop(&mSoap,ptzXAddr,NULL,&stop,stopResponse)){
                //停止成功
                printf("soap_call___tptz__Stop succeed \n");		
                return 0;
            }
            else{
                printf("oap_call___tptz__Stop failed \n");
                printf("[%s][%d]--->>> soap result: %d, %s, %s\n", __func__, __LINE__, 
                                                    mSoap.error, *soap_faultcode(&mSoap), 
                                                    *soap_faultstring(&mSoap));  
                return -1;
            }
                
        }
    public:
        OnvifSoap(int timeout){
            soap_init(&mSoap);
            soap_set_namespaces(&mSoap, namespaces);                                      // 设置soap的namespaces
            mSoap.recv_timeout    = timeout;                                            // 设置超时（超过指定时间没有数据就退出）
            mSoap.send_timeout    = timeout;
            mSoap.connect_timeout = timeout;

        #if defined(__linux__) || defined(__linux)                                      // 参考https://www.genivia.com/dev.html#client-c的修改：
            mSoap.socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
        #endif
            soap_set_mode(&mSoap, SOAP_C_UTFSTRING);                                      // 设置为UTF-8编码，否则叠加中文OSD会乱码

            
        }
        
        ~OnvifSoap(){
            soap_destroy(&mSoap); // clean up class instances
            soap_end(&mSoap); // clean up everything and close socket, // userid and passwd were deallocated
            soap_done(&mSoap); // close master socket and detach context
        }
        void detectDevice(void (*cb)(char *DeviceXAddr))
        {
            #define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址
            int i;
            int result = 0;
            unsigned int count = 0;                                                     // 搜索到的设备个数
            struct wsdd__ProbeType      req;                                            // 用于发送Probe消息
            struct __wsdd__ProbeMatches rep;                                            // 用于接收Probe应答
            struct wsdd__ProbeMatchType *probeMatch;

            _initHeader();
            _initProbeType( &req);                                           // 设置寻找的设备的范围和类型
            result = soap_send___wsdd__Probe(&mSoap, SOAP_MCAST_ADDR, NULL, &req);        // 向组播地址广播Probe消息
            while (SOAP_OK == result)                                                   // 开始循环接收设备发送过来的消息
            {
                memset(&rep, 0x00, sizeof(rep));
                result = soap_recv___wsdd__ProbeMatches(&mSoap, &rep);
                if (SOAP_OK == result) {
                    if (mSoap.error) {
                        soap_perror( "ProbeMatches");
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
                } else if (mSoap.error) {
                    break;
                }
            }
            _deinitProbeType( &req);

            SOAP_DBGLOG("\ndetect end! It has detected %d devices!\n", count);

            return ;
        }
        void soap_perror(const char *str)
        {
            #define SOAP_DBGERR     printf
            if (NULL == str) {
                SOAP_DBGERR("[soap] error: %d, %s, %s\n", mSoap.error, *soap_faultcode(&mSoap), *soap_faultstring(&mSoap));
            } else {
                SOAP_DBGERR("[soap] %s error: %d, %s, %s\n", str, mSoap.error, *soap_faultcode(&mSoap), *soap_faultstring(&mSoap));
            }
            return;
        }

        int absoluteMove(const char* ip,const char*userName,const char*pwd)
        {
            std::string profile,mediaXAddr,ptzXAddr;
            _getAddr(ip,mediaXAddr,ptzXAddr);
            _getProfiles(mediaXAddr.c_str(),profile,userName,pwd);
            return _absoluteMove(profile.c_str(),ptzXAddr.c_str(),userName,pwd);
        }

        int continuousMove(const char* ip,const char*userName,const char*pwd,int opt)
        {
            std::string profile,mediaXAddr,ptzXAddr;
            int ret = _getAddr(ip,mediaXAddr,ptzXAddr);
            if( ret != 0)
                return ret;
            ret = _getProfiles(mediaXAddr.c_str(),profile,userName,pwd);
            if(ret != 0 )
                return ret;
            if(opt == 6)
                return _stopMove(profile.c_str(),ptzXAddr.c_str(),userName,pwd);
            else
                return _continuousMove(profile.c_str(),ptzXAddr.c_str(),userName,pwd,opt);

        }
        
};

#endif//ONVIF_SOAP_H
