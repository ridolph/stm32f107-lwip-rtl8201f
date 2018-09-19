#include "stm32f10x.h"
#include "stm32_eth.h"

__IO uint32_t LWipTime = 0;     //LWipʱ�����ڼ�����

void NVIC_Configuration(void);      //����������̫���ж�����
void GPIO_Configuration(void);      //�����õ���GPIO��
void Ethernet_Configuration(void);  //��ʼ����̫��
void LWIP_Init(void);
void lwip_periodic_handle();



//extern struct netif netif; 
//extern volatile uint8_t  IGMP_UP_Time; //��������ʱ���
extern uint8_t EthInitStatus;   

int main()
{
	RCC_ClocksTypeDef RCC_Clocks;//���ļ��ṹ�壬����һ��ʱ�Ӳ����Ľṹ�屸��

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | 
						  RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);	                                            //�⺯����ʹ��MAC��MAC���ͺ�MAC����ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
						  RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO , ENABLE);	    //�⺯����ʹ��GPIOʱ��
	NVIC_Configuration();       //�Զ��庯����������̫��NVIC�ж�
	GPIO_Configuration();       //�Զ��庯������ʼ���õ���GPIO
	Ethernet_Configuration();   //�Զ��庯����������̫��
	RCC_GetClocksFreq(&RCC_Clocks);                   //�⺯������ȡ��ǰϵͳ��ʱ�Ӳ���
	SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 100);//�⺯��������SysTickʱ�ӣ�����һ��10ms���ж�
	NVIC_SetPriority (SysTick_IRQn, 1);               //�⺯��������SysTick���ж����ȼ�
	
	LWIP_Init();
	
	while (1)
	{    
		lwip_periodic_handle();
	}

}


void NVIC_Configuration(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;            //���ļ��ڣ�NVIC�ṹ��

  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);     //�⺯����������������λ�ú�ƫ�ƣ���������������FLASH������ƫ�ơ�

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   //�⺯�����������ȼ����飺��ռ���ȼ��ʹ����ȼ�
                                                    //NVIC_PriorityGroup_2����ռ���ȼ�2λ �����ȼ�2λ
  
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;           //�����ж�ͨ��=��̫���ж�
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//������ռ���ȼ�=2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //���ô����ȼ�=0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          //ʹ���ж�
  NVIC_Init(&NVIC_InitStructure);                          //�⺯��������NVIC_InitStructure�ṹ���ڵĲ�����ʼ��NVIC�Ĵ���
}
void GPIO_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure; //���ļ����ڳ�ʼ��GPIO�õ��Ľṹ��
    /* Configure PA2 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure PC1, PC2 and PC3 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Input (Reset Value):
    - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
    - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PA7
    - ETH_MII_RXD0 / ETH_RMII_RXD0: PC4
    - ETH_MII_RXD1 / ETH_RMII_RXD1: PC5
    */

    /* Configure PA1,PA7 as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    /* Configure PC4, PC5,  as input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure); 

    /* MCO pin configuration */
    /* Configure MCO (PA8) as alternate function push-pull */
    //������ܳ�ʼ����50Mʱ��ʹ��RTL8021F�����ʱ�ӣ�
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
}

/*******************************
�������ܣ�������ʼ��
********************************/
void ETH_Reinit(void)
{
	ETH_InitTypeDef ETH_InitStructure;  //���ļ�����ʼ����̫���õ��Ľṹ��
	ETH_StructInit(&ETH_InitStructure);//��ʼ���ṹ��ETH_InitStructure������ԱΪĬ��ֵ

	//ETH_InitStructure�ṹ���ڳ�Ա�϶࣬�����Ĭ��ֵ���޸�һЩ�ؼ���Ա������Ӧ��ǰ����Ҫ
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			//������������Ӧ����
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					//�رշ���
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		//�ر��ش�����
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	//�ر��Զ�ȥ��PDA/CRC���� 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						//�رս������е�֡
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//�����������й㲥֡
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			//�رջ��ģʽ�ĵ�ַ����  
	//ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//�����鲥��ַʹ��������ַ���� 
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_None;     //�����鲥��ַʹ��������ַ����      
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	  //�Ե�����ַʹ��������ַ���� 
    
	#if  !CHECKSUM_GEN_ICMP    //�ж��Ƿ���Ӳ��У��,�ر�����У��
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			//����ipv4��TCP/UDP/ICMP��֡У���ж��   
	#endif
	
	//������ʹ��֡У���ж�ع��ܵ�ʱ��һ��Ҫʹ�ܴ洢ת��ģʽ,�洢ת��ģʽ��Ҫ��֤����֡�洢��FIFO��,
	//����MAC�ܲ���/ʶ���֡У��ֵ,����У����ȷ��ʱ��DMA�Ϳ��Դ���֡,����Ͷ�������֡
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //��������TCP/IP����֡
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;     //�����������ݵĴ洢ת��ģʽ    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;   //�����������ݵĴ洢ת��ģʽ  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;     	//��ֹת������֡  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//��ת����С�ĺ�֡ 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		//�򿪴����ڶ�֡����
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	//����DMA����ĵ�ַ���빦��
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			//�����̶�ͻ������    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		//DMA���͵����ͻ������Ϊ32������   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			//DMA���յ����ͻ������Ϊ32������
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	EthInitStatus = ETH_Init(&ETH_InitStructure,PHY_ADDRESS);		        //����ETH    
}


void Ethernet_Configuration(void)
{ 

//ѡ��ӿ���MII����RMII������ʹ��RMII
#ifdef MII_MODE //ѡ��ʹ��MII�ӿ� MII�ӿ���Ҫʱ��25M
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);//�ú���������̫���ӿ�ΪMII

	RCC_MCOConfig(RCC_MCO_HSE);//����MCOʱ����� RCC_MCO_HSE���������ⲿ����ʱ����ΪMCO��������İ��õ�25M�������ṩMII��25Mʱ��

#elif defined RMII_MODE //ѡ��ʹ��RMII�ӿ� RMII�ӿ���Ҫʱ��50M
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);//�ú���������̫���ӿ�ΪRMII

	//����˼·����PLL3ʱ��ΪMCO�ṩRMII��Ҫ��50Mʱ��
	//ϵͳ��ʼ��STM32��ʱ��Ϊ72M��ʱ��PLL3������Ϊ���ⲿ25M����5��Ƶ 25/5=5M
	RCC_PLL3Config(RCC_PLL3Mul_10);//PLL3��Ƶ10�� 5*10=50M

	RCC_PLL3Cmd(ENABLE);//ʹ��PLL3ʱ��

	while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)//�ȴ�PLL3ʹ�ܳɹ�
	{}
	  
	RCC_MCOConfig(RCC_MCO_PLL3CLK);//ѡ��MCO��ʱ��ԴΪPLL3
#endif

	ETH_DeInit();       //��λ��̫������Ĵ�����Ĭ��ֵ����ʹ�ܣ��ڽ�ֹ���ѻָ�Ĭ��ֵ

	ETH_SoftwareReset();//������λ��ͨ������ETH->DMABMR�Ĵ�����λ0ʵ��

	while (ETH_GetSoftwareResetStatus() == SET)//�ȴ�����λ�ɹ�
    {    
        
    }
    
    ETH_Reinit();//������ʼ��
	ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);  	//ʹ����̫�������ж�	

}
