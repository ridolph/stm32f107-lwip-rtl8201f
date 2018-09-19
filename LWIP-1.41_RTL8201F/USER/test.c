#include "stm32f10x.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h"
#include "stm32_eth.h"
#include <stdio.h>

void ETH_Reinit(void);

err_t ethernetif_init(struct netif *netif); //�����ļ��ں�������
err_t ethernetif_input(struct netif *netif);//�����ļ��ں�������

u8_t MACaddr[6]={0,0,0,0,0,1};              //MAC��ַ������Ψһ��

struct netif netif;             //����һ��ȫ�ֵ�����ӿڣ�ע����������Ҫ�õ�  
__IO uint32_t TCPTimer = 0;     //TCP��ѯ��ʱ�� 
__IO uint32_t ARPTimer = 0;     //ARP��ѯ��ʱ��
extern uint32_t LWipTime;       //LWip����ʱ��
volatile uint8_t  IGMP_UP_Time; //��������ʱ���
uint8_t EthInitStatus;          //��ʼ��״̬

#if LWIP_DHCP                   //�������DHCP
u32 DHCPfineTimer=0;	        //DHCP��ϸ�����ʱ��
u32 DHCPcoarseTimer=0;	        //DHCP�ֲڴ����ʱ��
#endif

void LWIP_Init(void)
{
  struct ip_addr ipaddr;     //����IP��ַ�ṹ��
  struct ip_addr netmask;    //������������ṹ��
  struct ip_addr gw;         //�������ؽṹ��

  mem_init();  //��ʼ����̬�ڴ��

  memp_init(); //��ʼ���ڴ��

  lwip_init(); //��ʼ��LWIP�ں�
	
#if LWIP_DHCP     //�������DHCP���Զ���ȡIP
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#else            //������DHCP��ʹ�þ�̬IP
  IP4_ADDR(&ipaddr, 192, 168, 3, 30);      //����IP��ַ
  IP4_ADDR(&netmask, 255, 255, 255, 0);   //������������
  IP4_ADDR(&gw, 192, 168, 3, 1);          //��������
#endif

  ETH_MACAddressConfig(ETH_MAC_Address0, MACaddr);  //����MAC��ַ

  //ע����������   ethernetif_init��������Ҫ�Լ�ʵ�֣���������netif����ֶΣ��Լ���ʼ���ײ�Ӳ����
  netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  //��ע����������ע�����������ΪĬ������
  netif_set_default(&netif);


#if LWIP_DHCP           //���������DHCP��λ
  dhcp_start(&netif);   //����DHCP
#endif

  //������
  netif_set_up(&netif);
}

//�������ݺ���
void LwIP_Pkt_Handle(void)
{
  //�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 
  ethernetif_input(&netif);
}

/*******************************
�������ܣ���������
********************************/
void Eth_Link_ITHandler(void)
{
/* Check whether the link interrupt has occurred or not */

   // if(((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR)) & PHY_BSR) != 0){/*������ж�*/
    if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status) == 0x00){
        uint16_t status = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR);
        if(status & (PHY_AutoNego_Complete | PHY_Linked_Status)){/*��⵽��������*/
            if(EthInitStatus != 1){/*if(EthInitStatus != ETH_SUCCESS)֮ǰδ�ɹ���ʼ����*/
                /*Reinit PHY*/
                ETH_Reinit();
            }
            else{/*֮ǰ�Ѿ��ɹ���ʼ��*/
                /*set link up for re link callbalk function*/
                netif_set_link_up(&netif);
            }
        }
        else{/*���߶Ͽ�*/\
            /*������������������callbalk����*/
            netif_set_link_down(&netif);
        }
        IGMP_UP_Time = 0;  //���߱�־λ
    } 
}

//LWIP����������
void lwip_periodic_handle()
{
#if LWIP_TCP
	//ÿ250ms����һ��tcp_tmr()����
  if (LWipTime - TCPTimer >= 250)
  {
    TCPTimer =  LWipTime;
    tcp_tmr();
  }
   #if LWIP_IGMP 
        if(IGMP_UP_Time>=2){
            IGMP_UP_Time=3;
            igmp_tmr();           
        } 
    #endif    
#endif
  //ARPÿ5s�����Ե���һ��
  if ((LWipTime - ARPTimer) >= ARP_TMR_INTERVAL)
  {
    ARPTimer =  LWipTime;
    etharp_tmr();
    IGMP_UP_Time++;
    Eth_Link_ITHandler();   //��������
  }

#if LWIP_DHCP //���ʹ��DHCP�Ļ�
  //ÿ500ms����һ��dhcp_fine_tmr()
  if (LWipTime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer =  LWipTime;
    dhcp_fine_tmr();
  }

  //ÿ60sִ��һ��DHCP�ֲڴ���
  if (LWipTime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
  {
    DHCPcoarseTimer =  LWipTime;
    dhcp_coarse_tmr();
  }  
#endif
}






