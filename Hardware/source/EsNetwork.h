/*********************************************************************#
//	文件名		：EsNetwork.h
//	文件功能	：识别命令词网络数据结构的定义
//	作者		：Truman
//	创建时间	：2007年7月10日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#ifndef IFLY_ES_TEAM__2007_07_30__ESRNETWORK__H
#define IFLY_ES_TEAM__2007_07_30__ESRNETWORK__H

#include "EsFrontParam.h"

#define DW_LEXHEADER_CHECK	(0x20080114)

/* 整个网络存储时保证所有叶子节点靠后存储，这样方便知道哪些点可以传动啊EndFiller上去,省去nResult字段;整体存储采用倒序存储，pLexRoot指向最后一个节点 */
typedef struct tagLexiconNode	 
{	
	ivUInt16 iParent;			    /* 从1开始，0表示无父节点 */
	
	ivUInt16 iSModeIndex;		    /* 该节点对应的模型状态ID */
	ivUInt16 nStateCount;           /* 叶子节点上存储的是该命令词的state个数.非叶子节点=0 */
}TLexNode, ivPtr PLexNode;

/* Lexicon */
typedef struct tagLexiconHeader
{
	ivUInt16 nTotalNodes;		
	ivUInt16 nExtendNodes;		/* 起始的若干节点，层次存储，用来标记当VAD认为语音未开始时可扩展的节点 */
	ivUInt32 dwCheck;			
	ivUInt32 nCmdNum;			/* 命令词个数,用于检测ESRSetCmdActive时nID的有效性的 20101022 */
	ivUInt32 nLexRootOffset;	
	ivUInt32 nCmdDescOffset;	
}TLexiconHeader,ivPtr PLexiconHeader;
typedef TLexiconHeader ivCPtr PCLexiconHeader;

typedef struct tagCmdDesc
{
	ivUInt16 nID;							
	ivInt16  nCMThresh;     /* 拒识门限 */		
	ivUInt16 bTag;		    /* 是否是voicetag */
}TCmdDesc, ivPtr PCmdDesc;

#endif /* IFLY_ES_TEAM__2007_07_30__ESRNETWORK__H */
