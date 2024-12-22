/*任务结构体的定义*/
typedef struct task_t
{
    bool occupy;     // 是否已占用：true表示当前有任务；false表示当前没有任务
    uint16_t task;   // 任务内容
    uint16_t expire; // 等待时间
} task_t;

/*任务池的定义*/
#define TASK_LIST_SIZE 5
static task_t taskList_g[TASK_LIST_SIZE];

/*任务池初始化*/
static void taskListInit()
{
    // 把所有任务都设置为未使用
    for (uint16_t i = 0; i < TASK_LIST_SIZE; i++)
        taskList_g[i].occupy = false;
}

/*往任务池中添加任务*/
static void addTask(uint16_t expire, uint16_t task)
{
    for (uint16_t i = 0; i < TASK_LIST_SIZE; i++)
    {
        if (taskList_g[i].occupy)
            continue;

        taskList_g[i].task = task;
        taskList_g[i].expire = expire;
        taskList_g[i].occupy = true;

        break;
    }
}

/*轮询*/
static void polling()
{
    for (uint16_t i = 0; i < (sizeof(taskList_g) / sizeof(taskList_g[0])); i++)
        if (taskList_g[i].occupy && (--taskList_g[i].expire == 0))
        {
            // 执行任务
            taskHandler(taskList_g[i].task);
            // 释放任务池中的空间
            taskList_g[i].occupy = false;
        }
}

/*协调器创建网络*/
bdb_StartCommissioning(                    // 组建网络
    BDB_COMMISSIONING_MODE_NWK_FORMATION | // 支持Network Formation
    BDB_COMMISSIONING_MODE_FINDING_BINDING // 支持Finding and Binding（F & B）
);

/*路由器或终端设备加入网络*/
bdb_StartCommissioning(                    // 设备入网
    BDB_COMMISSIONING_MODE_NWK_STEERING |  // 支持Network Steering
    BDB_COMMISSIONING_MODE_FINDING_BINDING // 支持Finding and Binding（F & B）
);

/*AF层数据发送API*/
/*
 * @param dstAddr 目标设备地址，包含网络地址和端点号
 * @param srcEP 发送设备的简单描述符
 * @param cID Cluster ID，后续章节将会详细讲解
 * @param len 待发送数据的长度
 * @param buf 待发送的数据
 * @param transID 传输ID，可以用来给每一次发送的数据包编一个号
 * @param options 附加选项，可以用来给这次数据发送添加一些说明
 * @param radius 最大的路由跳转级数
 */
afStatus_t AF_DataRequest(
    afAddrType_t *dstAddr,
    endPointDesc_t *srcEP,
    uint16 cID,
    uint16 len,
    uint8 *buf,
    uint8 *transID,
    uint8 options,
    uint8 radius);

/*点对点通信*/
/*
 * @param destNwkAddr 目标设备的网络地址
 * @param cid Cluster ID，后续课程将会详细讲解
 * @param len 数据长度
 * @param data 数据内容
 */
static void zclSampleSw_AF_P2P(
    uint16 destNwkAddr,
    uint16 cid,
    uint8 len,
    uint8 *data)
{
    afAddrType_t dstAddr;        // 寻址信息配置
    static uint8 transferId = 0; // 传输ID，是数据包的标识符

    /* Destination */
    dstAddr.addrMode = afAddr16Bit;       // 设置目标地址模式为16位网络地址，表示使用P2P的通信方式
    dstAddr.addr.shortAddr = destNwkAddr; // 目标设备的网络地址
    dstAddr.endPoint = SAMPLESW_ENDPOINT; // 目标设备的端点号

    transferId++;
    AF_DataRequest(&dstAddr,
                   &sampleSw_TestEp, // 已经创建好的简单描述符
                   cid,
                   len,
                   data,
                   &transferId,
                   AF_DISCV_ROUTE,     // 进行路由扫描操作，用于建立发送数据报文的通信路径。关于这个参数，暂时保持例程默认的代码就可以了
                   AF_DEFAULT_RADIUS); // 指定最大的路由跳转级数
}

/*广播通信*/
/*
 * @param cid Cluster ID
 * @param len 待发送数据的长度
 * @param *data 待发送数据的内容
 */
static void zclSampleSw_AF_Broadcast(
    uint16 cid,
    uint8 len,
    uint8 *data)
{
    afAddrType_t dstAddr;
    static uint8 transferId = 0;

    /* Destination */
    dstAddr.addrMode = afAddrBroadcast;   // 使用广播模式
    dstAddr.addr.shortAddr = 0xFFFF;      // 广播地址
    dstAddr.endPoint = SAMPLESW_ENDPOINT; // 目标设备的端点号

    /* Transfer id */
    transferId++;

    /* Send */
    AF_DataRequest(
        &dstAddr,
        &sampleSw_TestEp, // 已经创建好的简单描述符
        cid,
        len,
        data,
        &transferId,
        AF_TX_OPTIONS_NONE,
        AF_DEFAULT_RADIUS); // 指定了最大的路由跳转级数
}

/*组播通信*/
/*
 * @param groupId 组ID
 * @param cid ClusterID，后续章节将会详细讲解
 * @param len 待发送数据的长度
 * @param data 待发送数据的内容
 */
static void zclSampleSw_AF_Groupcast(
    uint16 groupId,
    uint16 cid,
    uint8 len,
    uint8 *data)
{
    afAddrType_t dstAddr;
    static uint8 transferId = 0;

    /* Destination */
    dstAddr.addrMode = afAddrGroup; // 使用组播通信模式
    dstAddr.addr.shortAddr = groupId;
    dstAddr.endPoint = SAMPLESW_ENDPOINT; // 组中的设备的端点号

    /* Transfer id */
    transferId++;

    /* Send */
    AF_DataRequest(&dstAddr,
                   &sampleSw_TestEp, // 已经创建好的简单描述符
                   cid,
                   len,
                   data,
                   &transferId,
                   AF_TX_OPTIONS_NONE,
                   AF_DEFAULT_RADIUS); // 指定最大的路由跳转级数，暂时可忽略
}

/*组建网络*/
void zclSampleSw_Init(byte task_id) // 初始化函数
{
    zclSampleSw_TaskID = task_id;

    // 注册所有关键事件-这个应用程序将处理所有关键事件
    RegisterForKeys(zclSampleSw_TaskID);

    bdb_RegisterCommissioningStatusCB(zclSampleSw_ProcessCommissioningStatus);

    // 注册一个测试端点
    afRegister(&sampleSw_TestEp);

#ifdef ZDO_COORDINATOR
    // 如果角色选择协调器，则执行以下代码
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_FORMATION |
                           BDB_COMMISSIONING_MODE_FINDING_BINDING); // 建立网络

    // 允许其他设备加入到由本协调器创建的网络中,参数255表示一直允许，如果改为0则表示一直不允许；如果改为1254表示在1254秒内允许。
    NLME_PermitJoiningRequest(255);

    // 一段时间后（SAMPLEAPP_BROADCAST_PERIOD毫秒）向 zclSampleSw_TaskID 指定的任务发送广播事件
    osal_start_timerEx(zclSampleSw_TaskID,
                       SAMPLEAPP_BROADCAST_EVT,

                       SAMPLEAPP_BROADCAST_PERIOD);
    // 一段时间后（SAMPLEAPP_GROUPCAST_PERIOD毫秒）向 zclSampleSw_TaskID 指定的任务发送组播事件
    osal_start_timerEx(zclSampleSw_TaskID,
                       SAMPLEAPP_GROUPCAST_EVT,
                       SAMPLEAPP_GROUPCAST_PERIOD);
#else
    // 如果节点不是协调器
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING |
                           BDB_COMMISSIONING_MODE_FINDING_BINDING); // 加入网络

    // Add group添加小组
    aps_Group_t group = {
        .ID = GROUP_ID,
        .name = "",
    };
    aps_AddGroup(SAMPLESW_ENDPOINT, &group);

    // 一段时间后发送P2P事件
    osal_start_timerEx(zclSampleSw_TaskID,
                       SAMPLEAPP_P2P_EVT,
                       SAMPLEAPP_P2P_PERIOD);
#endif
}

/*入网状态检测与重新入网*/
static void zclSampleSw_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
    switch (bdbCommissioningModeMsg->bdbCommissioningMode)
    {
    case BDB_COMMISSIONING_FORMATION:                                                     // 检查建网是否成功
        if (bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS) // 假如建网成功
        {
            // After formation, perform nwk steering again plus the remaining commissioning modes that has not been processed yet
            bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
            printf("I have formation.\n"); // 在控制台打印建网成功信息
        }
        break;
    case BDB_COMMISSIONING_NWK_STEERING:                                                  // 检查入网是否成功
        if (bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS) // 假如入网成功
        {
            printf("I have joined.\n"); // 在控制台打印入网成功信息
        }
        else
        {
            printf("Fail to join!") // 入网失败，在控制台打印入网失败信息以此判断状态
        }
        break;
    }
}

/*非协调器节点获取地址信息*/
// P2P Event: 检查当前事件是否包含P2P事件标志
if (events & SAMPLEAPP_P2P_EVT)
{
    NodeAddressInfo nodeInfo; // 定义一个结构体变量用于存储节点地址信息

    // 获取本节点短地址
    nodeInfo.shortAddr = NLME_GetShortAddr(); // 调用NLME_GetShortAddr()函数获取节点的16位短地址并赋值给nodeInfo.shortAddr

    // 注意：这里调用了NLME_GetExtAddr()，但返回值未被使用。
    // 此行可能是为了填充全局变量saveExtAddr，后续会将其复制到nodeInfo.extAddr中。
    NLME_GetExtAddr();

    // 将之前保存的扩展地址复制到nodeInfo.extAddr中
    memcpy(nodeInfo.extAddr, saveExtAddr, sizeof(nodeInfo.extAddr));

    // 获取父节点的短地址（通常是协调器或路由器）
    nodeInfo.parentShortAddr = NLME_GetCoordShortAddr(); // 调用NLME_GetCoordShortAddr()获取父节点的16位短地址

    // 获取父节点的扩展地址
    NLME_GetCoordExtAddr(nodeInfo.parentExtAddr); // 调用NLME_GetCoordExtAddr()并将结果直接存储在nodeInfo.parentExtAddr中

    // 序列化数据：准备将NodeAddressInfo结构体的数据发送出去
    uint8 buffer[sizeof(NodeAddressInfo)];              // 定义一个缓冲区用于存放序列化的数据
    memcpy(buffer, &nodeInfo, sizeof(NodeAddressInfo)); // 将nodeInfo结构体内容复制到buffer中

    // 发送序列化后的数据
    zclSampleSw_AF_P2P(0x0000, CLUSTER_P2P, sizeof(buffer), buffer); // 通过zclSampleSw_AF_P2P函数发送数据，参数分别为目的地地址、集群ID、数据长度和数据指针

    // 设置定时器，使SAMPLEAPP_P2P_EVT事件再次触发
    osal_start_timerEx(zclSampleSw_TaskID,    // 启动定时器，参数为任务ID
                       SAMPLEAPP_P2P_EVT,     // 事件类型
                       SAMPLEAPP_P2P_PERIOD); // 定时周期

    // 返回剩余的事件，去除已经处理过的P2P事件标志
    return (events ^ SAMPLEAPP_P2P_EVT);
}

/*数据接收*/
static void zclSampleSw_AF_RxProc(afIncomingMSGPacket_t *MSGpkt)
{
    static uint8 p2pCnt = 0; // 记录P2P消息的数量
    static uint8 bcCnt = 0;  // 记录广播消息的数量
    static uint8 gcCnt = 0;  // 记录组播消息的数量

    switch (MSGpkt->clusterId)
    {
    case CLUSTER_P2P: // 如果消息属于P2P集群
        p2pCnt++;     // 增加P2P消息计数

        // 将消息数据指针转换为NodeAddressInfo结构体指针
        NodeAddressInfo *nodeInfo = (NodeAddressInfo *)MSGpkt->cmd.Data;

        // 遍历networkNodes数组，寻找空位或者已存在的节点信息
        for (int i = 0; i < sizeof(networkNodes) / sizeof(networkNodes[0]); i++)
        { // 注意：这里应使用正确的数组大小计算方法
            if (networkNodes[i] == 0)
            {                                          // 如果找到空位
                networkNodes[i] = nodeInfo->shortAddr; // 添加新节点的短地址到数组中

                // 打印节点信息
                printf("Node Info:\n");
                printf("Short Address: 0x%X\n", nodeInfo->shortAddr);
                printf("Extended Address: ");
                for (int j = 7; j >= 0; j--)
                {                                          // 使用不同的循环变量避免混淆
                    printf("%02X ", nodeInfo->extAddr[j]); // 格式化输出扩展地址
                }
                printf("\nParent Short Address: 0x%X\n", nodeInfo->parentShortAddr);
                printf("Parent Extended Address: ");
                for (int j = 7; j >= 0; j--)
                {                                                // 使用不同的循环变量避免混淆
                    printf("%02X ", nodeInfo->parentExtAddr[j]); // 格式化输出父节点扩展地址
                }
                printf("\n");

                break;
            }
            // 如果在数组中找到已存在的节点信息，则不再继续添加
            else if (networkNodes[i] == nodeInfo->shortAddr)
            {
                break;
            }
        }

        break;
        // 其他集群ID的处理逻辑...
    }
}

/*修改引脚*/
#define HAL_LCD_SPI_CS_PORT 1
#define HAL_LCD_SPI_CS_PIN 3
#define HAL_LCD_SPI_DS_PORT 0
#define HAL_LCD_SPI_DS_PIN 0
#define HAL_DHT11_PORT 0
#define HAL_DHT11_PIN 4

/*获取温湿度和光照*/
// 获取温湿度并检测数值是否有效
dht11Dat = halDHT11GetData();
if (!(dht11Dat.ok) || (dht11Dat.temp == 0 && dht11Dat.humi == 0))
{
    return;
} // 这里的接口需要找到对应的文件hal_dht11.h，通过原理电路图判断接口号

// 读取P07的ADC值，即光照强度值
lightAdc = HalAdcRead(HAL_ADC_CHN_AIN5, HAL_ADC_RESOLUTION_8); // HAL_ADC_CHN_AIN5是光照硬件接口对应变量，从原理电路图判断此处应选择几号接口
// 读取土壤湿度
wetAdc = HalAdcRead(HAL_ADC_CHN_AIN6, HAL_ADC_RESOLUTION_8);

/* 优化光照度和土壤湿度值 */
#define LIGHT_ADC_MAX_VALUE 120 // 光照度最大值
#define WET_ADC_MAX_VALUE 120   // 土壤湿度最大值
float lightAdcFloat;
float wetAdcFloat;

// 裁剪光照度值
if (lightAdc > LIGHT_ADC_MAX_VALUE)
{
    lightAdc = LIGHT_ADC_MAX_VALUE;
}
lightAdcFloat = (float)lightAdc * 100 / LIGHT_ADC_MAX_VALUE;

// 处理土壤湿度值
if (wetAdc > WET_ADC_MAX_VALUE)
{
    wetAdc = WET_ADC_MAX_VALUE;
}
wetAdcFloat = (float)wetAdc * 100 / WET_ADC_MAX_VALUE;

// 将浮点数转换为整数部分和小数部分
lightAdcFloatInt = (int)lightAdcFloat;
lightAdcFloatDec = (int)((lightAdcFloat - lightAdcFloatInt) * 10);

wetAdcFloatInt = (int)wetAdcFloat;
wetAdcFloatDec = (int)((wetAdcFloat - wetAdcFloatInt) * 10);

/*修改硬件接口*/
/** @brief   DHT11 GPIO. */
// 此处应根据硬件情况进行更改，通过看硬件原理图得知接口号为多少，而粤嵌的板子为4号
#define HAL_DHT11_PORT 0 //!< Port0.（端口号）
#define HAL_DHT11_PIN 4  //!< Pin4.（接口号）

/*路由器收集到数据，发送给协调器*/
/* 往协调器发送数据 */
char msg[64]; // 定义一个长度为64的字符数组来存储消息
sprintf(msg, "D%sT%d.%dH%d.%dL%d.%dW%d.%d#",
        zb_port_get_local_mac(), // 设备本地MAC地址
        tempInt,                 // %d - 温度整数部分
        tempDec,                 // %d - 温度小数部分
        humInt,                  // %d - 湿度整数部分
        humDec,                  // %d - 湿度小数部分
        lightAdcFloatInt,        // %d - 光照度整数部分
        lightAdcFloatDec,        // %d - 光照度小数部分
        wetAdcFloatInt,          // %d - 土壤湿度整数部分
        wetAdcFloatDec           // %d - 土壤湿度小数部分
);

/*路由器获取自身物理地址部分*/
/*
 * 用于获取本地MAC地址
 * @return MAC地址
 */
static const char *zb_port_get_local_mac(void)
{
    uint8 i;
    uint8 *xad;
    static uint8 localMacStr[Z_EXTADDR_LEN * 2 + 1];

    xad = aExtendedAddress + Z_EXTADDR_LEN - 1;

    for (i = 0; i < Z_EXTADDR_LEN * 2; xad--)
    {
        uint8 ch;
        ch = (*xad >> 4) & 0x0F;
        localMacStr[i++] = ch + ((ch < 10) ? '0' : 'A' - 10);
        ch = *xad & 0x0F;
        localMacStr[i++] = ch + ((ch < 10) ? '0' : 'A' - 10);
    }
    // 打印自身地址
    // printf("%s\n", (const char *)localMacStr);
    return (const char *)localMacStr;
}

/*路由器入网后，协调器收到路由器发送的数据*/
/*
 * 接收到ZigBee消息事的处理函数
 *
 *@param nwkaddr 发送者的网络地址
 *@param cluster Cluster
 *@param data 数据内容
 *@param len 数据长度
 */
void zb_port_on_priv_msg(uint16_t nwkaddr,
                         uint16_t cluster,
                         const uint8_t *data,
                         uint16_t len)
{
#if ZG_BUILD_COORDINATOR_TYPE // 协调器
    if (cluster == PRIV_CID_MESSAGE)
    {
        HalLcdWriteString("get data", 3);
        // 打印接收到的数据（可以在此处添加一个打印语句查看是否正确获取）
    }
#elif ZG_BUILD_RTRONLY_TYPE
#else
#endif
}

/*协调器上传到PC端*/
#if ZG_BUILD_COORDINATOR_TYPE
/*
 *从串口中获取到数据时的处理函数
 * @param data 数据内容
 * @param len 数据长度
 */
void zb_on_msg(const uint8_t *data, uint8_t len)
{
    // 可加一个打印语句查看数据
}
#endif
