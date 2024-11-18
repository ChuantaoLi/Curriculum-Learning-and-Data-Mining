/**************************************************************************************************
  Filename:       zcl_samplesw.c
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $

  Description:    Zigbee Cluster Library - sample switch application.


  Copyright 2006-2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
  This application implements a ZigBee On/Off Switch, based on Z-Stack 3.0.

  This application is based on the common sample-application user interface. Please see the main
  comment in zcl_sampleapp_ui.c. The rest of this comment describes only the content specific for
  this sample applicetion.
  
  Application-specific UI peripherals being used:

  - none (LED1 is currently unused by this application).

  Application-specific menu system:

    <TOGGLE LIGHT> Send an On, Off or Toggle command targeting appropriate devices from the binding table.
      Pressing / releasing [OK] will have the following functionality, depending on the value of the 
      zclSampleSw_OnOffSwitchActions attribute:
      - OnOffSwitchActions == 0: pressing [OK] will send ON command, releasing it will send OFF command;
      - OnOffSwitchActions == 1: pressing [OK] will send OFF command, releasing it will send ON command;
      - OnOffSwitchActions == 2: pressing [OK] will send TOGGLE command, releasing it will not send any command.

*********************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "MT_SYS.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_samplesw.h"
#include "zcl_diagnostic.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_adc.h"

#if defined (OTA_CLIENT) && (OTA_CLIENT == TRUE)
#include "zcl_ota.h"
#include "hal_ota.h"
#endif

#include "bdb.h"
#include "bdb_interface.h"

#include <stdio.h>

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclSampleSw_TaskID;


/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t sampleSw_TestEp =
{
  SAMPLESW_ENDPOINT,                  // endpoint
  0,
  &zclSampleSw_TaskID,
  (SimpleDescriptionFormat_t *)&zclSampleSw_SimpleDesc,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);


// Cluster
#define CLUSTER_P2P             0
#define CLUSTER_BROADCAST       1
#define CLUSTER_GROUPCAST       2

// GroupId
#define GROUP_ID                21

// P2P
static void zclSampleSw_AF_P2P(uint16 destNwkAddr,
                               uint16 cid,
                               uint8 len, uint8 *data);
// Broadcase
static void zclSampleSw_AF_Broadcast(uint16 cid,
                                     uint8 len, uint8 *data);
// Groupcast
static void zclSampleSw_AF_Groupcast(uint16 groupId,
                                     uint16 cid,
                                     uint8 len, uint8 *data);

// Process incoming message
static void zclSampleSw_AF_RxProc(afIncomingMSGPacket_t *MSGpkt);


/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * REFERENCED EXTERNALS
 */

/*********************************************************************
 * ZCL General Profile Callback table
 */

/*********************************************************************
 * @fn          zclSampleSw_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclSampleSw_Init( byte task_id ) // 初始化函数
{
  zclSampleSw_TaskID = task_id; // 任务ID

  // 注册指定任务以接收按键事件。通过调用该函数，任务会被配置成处理按键输入事件，用于处理用户界面的交互或设备操作
  RegisterForKeys( zclSampleSw_TaskID );

  // 注册一个回调函数，用于处理和报告 Zigbee 设备的网络调试和设置状态
  bdb_RegisterCommissioningStatusCB( zclSampleSw_ProcessCommissioningStatus );
  
  // 注册一个应用程序端点，应用程序被加入到 Zigbee 网络中，以处理从网络上接收的消息或发送消息给其他设备
  afRegister( &sampleSw_TestEp );
  
#ifdef ZDO_COORDINATOR
  // 如果设备是协调器，启动网络组建模式（NWK_FORMATION），并开始寻找并绑定模式（FINDING_BINDING）。此操作会建立网络并允许其他设备加入
  bdb_StartCommissioning( BDB_COMMISSIONING_MODE_NWK_FORMATION |
                          BDB_COMMISSIONING_MODE_FINDING_BINDING );
  
  NLME_PermitJoiningRequest(255); // 启用网络的设备加入功能，允许设备在 255 秒内加入网络
  
  // Broadcast
  // 启动一个定时器来触发广播事件。SAMPLEAPP_BROADCAST_EVT 是事件类型，SAMPLEAPP_BROADCAST_PERIOD 是事件触发的周期时间
  osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_BROADCAST_EVT, 
                     SAMPLEAPP_BROADCAST_PERIOD);
  // groupcast
  // 启动一个定时器来触发组播事件
  osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_GROUPCAST_EVT, 
                     SAMPLEAPP_GROUPCAST_PERIOD);
#else
// 如果设备不是网络协调器，启动网络引导（NWK_STEERING）和寻找并绑定（FINDING_BINDING）模式。网络引导允许设备加入现有网络
  bdb_StartCommissioning( BDB_COMMISSIONING_MODE_NWK_STEERING |
                          BDB_COMMISSIONING_MODE_FINDING_BINDING );
  // 定义一个组 group
  aps_Group_t group = {
    .ID = GROUP_ID,
    .name = "",
  };
  aps_AddGroup(SAMPLESW_ENDPOINT, &group); // 将该组添加到指定的端点 SAMPLESW_ENDPOINT
  
  // P2P
  osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_P2P_EVT, 
                     SAMPLEAPP_P2P_PERIOD); // 启动一个定时器来触发点对点（P2P）通信事件
#endif
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclSampleSw_event_loop( uint8 task_id, uint16 events ) // 事件循环函数
// uint8 task_id: 任务标识符，用于标识当前处理的任务
// uint16 events: 事件标志，用于指示当前任务需要处理的事件
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter


  if ( events & SYS_EVENT_MSG ) // 检查 events 是否包含 SYS_EVENT_MSG 标志位。如果包含，该任务有消息需要处理
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclSampleSw_TaskID )) ) // 使用 osal_msg_receive 函数接收消息
    {
      switch ( MSGpkt->hdr.event ) // 根据 MSGpkt->hdr.event 类型判断如何处理消息
      {
        case AF_INCOMING_MSG_CMD:
          zclSampleSw_AF_RxProc( MSGpkt );
          break;

        default:
          break;
      }

      // 处理完 SYS_EVENT_MSG 事件后，将该事件从 events 中移除
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // 返回剩余的未处理事件
    return (events ^ SYS_EVENT_MSG);
  }
 
  
#ifdef ZDO_COORDINATOR // 如果是协调器
  // 广播事件
  if ( events & SAMPLEAPP_BROADCAST_EVT ) // 检测该任务是否有消息需要处理
  {
    // 发送广播消息
    // CLUSTER_BROADCAST: 指定广播的目标群集 ID
    // 10: 数据长度
    // "Broadcast": 要发送的字符串消息
    zclSampleSw_AF_Broadcast(CLUSTER_BROADCAST,
                             10, "Broadcast");
    // 设置一个定时器，以 SAMPLEAPP_BROADCAST_PERIOD 为周期，触发 SAMPLEAPP_BROADCAST_EVT，以便定期发送广播消息
    osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_BROADCAST_EVT, 
                     SAMPLEAPP_BROADCAST_PERIOD);
    
    // 处理完此事件后，将 SAMPLEAPP_BROADCAST_EVT 从 events 中清除，返回剩余的未处理事件
    return ( events ^ SAMPLEAPP_BROADCAST_EVT );
  }
  
  // 组播事件
  if ( events & SAMPLEAPP_GROUPCAST_EVT )
  {
    zclSampleSw_AF_Groupcast(GROUP_ID,
                             CLUSTER_GROUPCAST,
                             10, "Groupcast");
    
    osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_GROUPCAST_EVT, 
                     SAMPLEAPP_GROUPCAST_PERIOD);
    
    return ( events ^ SAMPLEAPP_GROUPCAST_EVT );
  }
#else
  // 如果设备不是网络协调器，启动网络引导（NWK_STEERING）和寻找并绑定（FINDING_BINDING）模式。网络引导允许设备加入现有网络
  if ( events & SAMPLEAPP_REJOIN_EVT )
  {
   bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING |
                      BDB_COMMISSIONING_MODE_FINDING_BINDING );
    
    return ( events ^ SAMPLEAPP_REJOIN_EVT );
  }
  
  // P2P Event 点对点
  if ( events & SAMPLEAPP_P2P_EVT )
  {
    // 0x0000: 目标地址
    // CLUSTER_P2P: 群集 ID
    // 4: 数据长度
    // "P2P": 要发送的字符串消息
    zclSampleSw_AF_P2P(0x0000,
                       CLUSTER_P2P,
                       4, "P2P");
    
    osal_start_timerEx(zclSampleSw_TaskID, 
                     SAMPLEAPP_P2P_EVT, 
                     SAMPLEAPP_P2P_PERIOD);
    
    return ( events ^ SAMPLEAPP_P2P_EVT );
  }
#endif
  
  // Discard unknown events
  return 0;
}

  
/*********************************************************************
 * @fn      zclSampleSw_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg) // 处理调试或网络加入过程的状态报告
// 接收一个 bdbCommissioningModeMsg_t 结构体参数，报告当前调试过程的状态，处理网络加入的不同阶段（如网络组网、网络引导等）的成功或失败状态
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  // 根据 bdbCommissioningModeMsg->bdbCommissioningMode 的值，switch 语句决定如何处理不同的调试阶段
  {
    case BDB_COMMISSIONING_FORMATION: // 组网阶段
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        // 如果组网成功，继续尝试引导其他设备加入网络，并进行绑定等操作
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
      }
    break;
    case BDB_COMMISSIONING_NWK_STEERING: // 网络引导阶段
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
      }
      else
      {
        #ifdef ZDO_COORDINATOR
        #else
        // 如果网络引导失败，则会在非协调器设备上启动一个定时器，以尝试重新加入网络
        osal_start_timerEx(zclSampleSw_TaskID, 
                           SAMPLEAPP_REJOIN_EVT, 
                           SAMPLEAPP_REJOIN_PERIOD);
        #endif
      }
    break;
  }
}

/**
 * @fn      zclSampleSw_AF_P2P
 *
 * @brief   P2P
 *
 * @param   destNwkAddr - destination address
 *          cid - cluster id
 *          len - data length
 *          data - data
 *
 * @return  none
 */

// 点对点通讯
// destNwkAddr：目标设备的网络地址（16位地址），即你希望发送数据的设备的网络地址。
// cid：集群ID（Cluster ID），指示消息所属的特定应用集群。
// len：数据的长度，表示数据块的字节数。
// data：要发送的数据内容，通常是一个指向字节数组的指针。
static void zclSampleSw_AF_P2P(uint16 destNwkAddr,
                               uint16 cid,
                               uint8 len, uint8 *data)
{
  afAddrType_t dstAddr;
  static uint8 transferId = 0;
 
  /* Destination */
  dstAddr.addrMode = afAddr16Bit; // 设置地址模式为16位地址模式
  dstAddr.addr.shortAddr = destNwkAddr; // 设置目标设备的短地址为 destNwkAddr
  dstAddr.endPoint = SAMPLESW_ENDPOINT; // 设置目标设备的端点（Endpoint）为 SAMPLESW_ENDPOINT，这通常是在设备上运行应用程序的端点
  
  /* Transfer id */
  transferId++; // 增加传输ID，以便为每个传输操作提供唯一标识符
  
  // 发送数据
  AF_DataRequest( &dstAddr, &sampleSw_TestEp,
                  cid,
                  len, data,
                  &transferId,
                  AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
                  
}
/*
&dstAddr：目标设备的地址（包括地址模式、短地址和端点）。
&sampleSw_TestEp：当前设备的端点信息。
cid：集群ID。
len：数据长度。
data：数据内容。
&transferId：传输ID。
AF_DISCV_ROUTE：表示使用动态路由选择。
AF_DEFAULT_RADIUS：默认传输半径，通常表示消息的传输范围。
*/

/**
 * @fn      zclSampleSw_AF_Broadcast
 *
 * @brief   Broadcase
 *
 * @param   cid - cluster id
 *          len - data length
 *          data - data
 *
 * @return  none
 */
/*
cid：集群ID，指示广播消息的应用层集群类型。
len：广播数据的长度。
data：广播的数据内容，通常是一个字节数组。
*/
static void zclSampleSw_AF_Broadcast(uint16 cid,
                                     uint8 len, uint8 *data)
{
  afAddrType_t dstAddr;
  static uint8 transferId = 0;
 
  /* Destination */
  dstAddr.addrMode = afAddrBroadcast; // 设置地址模式为广播模式
  dstAddr.addr.shortAddr = 0xFFFF; // 目标地址
  dstAddr.endPoint = SAMPLESW_ENDPOINT; // 目标设备的端点
  
  /* Transfer id */
  transferId++;
  
  /* Send */
  AF_DataRequest( &dstAddr, &sampleSw_TestEp,
                  cid,
                  len, data,
                  &transferId,
                  AF_TX_OPTIONS_NONE, AF_DEFAULT_RADIUS );
}

/**
 * @fn      zclSampleSw_AF_Groupcast
 *
 * @brief   Groupcast
 *
 * @param   groupId - group id
 *          cid - cluster id
 *          len - data length
 *          data - data
 *
 * @return  none
 */
/*
groupId：组ID，表示要发送数据的设备组。
cid：集群ID，表示数据的集群类型。
len：数据的长度。
data：要发送的组播数据。
*/
static void zclSampleSw_AF_Groupcast(uint16 groupId,
                                     uint16 cid,
                                     uint8 len, uint8 *data)
{
  afAddrType_t dstAddr;
  static uint8 transferId = 0;
 
  /* Destination */
  dstAddr.addrMode = afAddrGroup; // 组播模式
  dstAddr.addr.shortAddr = groupId;
  dstAddr.endPoint = SAMPLESW_ENDPOINT;
  
  /* Transfer id */
  transferId++;
  
  /* Send */
  AF_DataRequest( &dstAddr, &sampleSw_TestEp,
                  cid,
                  len, data,
                  &transferId,
                  AF_TX_OPTIONS_NONE, AF_DEFAULT_RADIUS );
}

/**
 * @fn      zclSampleSw_AF_RxProc
 *
 * @brief   Process incoming message
 *
 * @param   MSGpkt - incoming message
 *
 * @return  none
 */

// 根据接收到的消息的集群 ID（clusterId）进行分类处理，并执行特定的动作，处理接收到的AF消息
static void zclSampleSw_AF_RxProc(afIncomingMSGPacket_t *MSGpkt) // MSGpkt是消息包
{
/*
p2pCnt：用于跟踪接收到的点对点通信消息的数量。
bcCnt：用于跟踪接收到的广播消息的数量。
gcCnt：用于跟踪接收到的组播消息的数量。
*/
  static uint8 p2pCnt = 0;
  static uint8 bcCnt = 0;
  static uint8 gcCnt = 0;
  
  switch( MSGpkt->clusterId )
  {
  case CLUSTER_P2P: // 点对点类型的消息
    p2pCnt++;
    
    HalLcdWriteStringValue( (char *)MSGpkt->cmd.Data, p2pCnt, 10, 3); // 显示消息
    HalLedSet(HAL_LED_1, HAL_LED_MODE_TOGGLE); // 切换LED状态
  break;
    
  case CLUSTER_BROADCAST:
    bcCnt++;
    
    HalLcdWriteStringValue( (char *)MSGpkt->cmd.Data, bcCnt, 10, 3);
    HalLedSet(HAL_LED_2, HAL_LED_MODE_TOGGLE);
  break;
  
  case CLUSTER_GROUPCAST:
    gcCnt++;
    
    HalLcdWriteStringValue( (char *)MSGpkt->cmd.Data, gcCnt, 10, 4);
    HalLedSet(HAL_LED_3, HAL_LED_MODE_TOGGLE);
  break;
  
  default:
  break;
  }
}
