/*********************************************************************#
//	文件名		：EsrKernel.h
//	文件功能	：Include file for standard system include files
//	作者		：Truman
//	创建时间	：2007年7月30日
//	项目名称	：EsKernel
//	备注		：
/---------------------------------------------------------------------#
//	历史记录：
//	编号	日期		作者	备注
#**********************************************************************/

#if !defined(ES_TEAM__2007_07_10__ESKERNEL_ESRKERNEL__H)
#define ES_TEAM__2007_07_10__ESKERNEL_ESRKERNEL__H


#if _MSC_VER > 1000
#pragma once
#endif /* _MSC_VER > 1000 */


#if 1
//#if !(defined(DEBUG) || defined(_DEBUG))
#define EsFrontInit                           AitalkMini6983DA04861D5FBEA8BC3031B0E4FDA8
#define EsFrontReset                          AitalkMini6983DA03761D5E6EA8BC3031B0E4FDA8
#define EsFrontAppendData                     AitalkMini837A62AC32B84B58FDAF33ACA28B682D
#define EsFrontGetFeature                     AitalkMini022E41F49DAC4CF0BCC85778473A7E5F
#define EsFrontUpdateMean                     AitalkMini35355BEEBC4D4A84148ADDC67987B9A1
#define EsFrontCalcDynamicMFCC                AitalkMini022E43F49DAC4CA0BRB8C778473A7E5E
#define EsInit                                AitalkMini2F6A41AB4BEA418CB9CE1FDFB679AEDF
#define EsValidate                            AitalkMini0CE383A817684C9CA7B801E08BCC5E1E
#define EsReset                               AitalkMini0A624AE3651C4E2AA0A6229825B8E320
#define EsRunStep                             AitalkMini2F6A46AB6BEA414CB9BE1FDFB679AEDF
#define EsOutputResult                        AitalkMini0CE383A817684C9CA7B701E08BDC5E1D
#define EsAlloc                               AitalkMini681C72D235E04AAB07BE4B52CFC2455C
#define EsSearchInit                          AitalkMiniE9C2ABF6705D4FD0B6A2C972CE8F39E7
#define EsSetLexicon                          AitalkMini837A62AC32B84C58FDAF33ACA28B682E
#define EsSearchReset                         AitalkMiniC9ECDFD47290497B968175832FABCA76
#define EsSearchFrameStep                     AitalkMini022E41F49DAC4CF0BBB85778473A7E5D
#define LAdd                                  AitalkMiniD6D5D04A8B9C45A4C98CF08A6D1330A9
#define simple_table_ln                       AitalkMini6983DA03761D4E6EA1BE30A1B0E4FDA8
#define ivMakeCRC                             AitalkMini35355BEEBE4D4A84148ADDC67987B9A0
#define ivMemCopy                             AitalkMiniE7A155C159D246B460A706C7A3C9B295
#define ivMemZero                             AitalkMini4AA4C536F264474D5D8749CDCFC8D266
#define EsFrontCheckVoice                     AitalkMiniA28D16C38C124EB7CDBB49872F94A921
#define EsFrontCalcStaticMFCC                 AitalkMini2E87D039917F40EF0FA2C2447EEA46CC
#define EsFrontFilterBankAndToMFCC            AitalkMini4AFEE9E6B23D4568D38D2304C9C89072
#define EsCMScoreCDFMatch                     AitalkMini615D7073AFF841DDA68C1189A54C6D84
#define EsCalcFiller                          AitalkMiniDBA23BC86C1A49147BA321F1DCD3A9A4
/* #define EsCalcOutLike                         AitalkMini2F40AC703DF9494A149CC067D7C59AD3 */
/* #define EsCal1GaussOutlike                    AitalkMini0835CA9850B5435A6B99EFA6FDDF01E3 */
/* #define EsUpdateAllScore                      AitalkMini7E288E5FBC0E41007D97CAD42998367F */
#define EsSPICalcOutLike                      AitalkMini1696B9805F6C4ab6BA720FD90FDB1C2E
#define EsSPICal1GaussOutlike                 AitalkMini0DA53730A7C04d2784CC64D0A164432F
#define EsSPIUpdateAllScore                   AitalkMini6F4D364F240A48feA707ECE7BF4869A0
#define EsUpdateEndFiller                     AitalkMiniF44AF17344B541F643A3659C7A84C21C
#define EsGetPruneThreshByBucket              AitalkMiniA0B2D2AC82B642ACE4BA11683F52C66A
#define EsGetPruneThresh                      AitalkMiniA0B2D2CC82B642ACE4BA12684F52C66A
#define g_bEnhanceVAD                         AitalkMini4178E873CA101486161704102408A5E3
#define norm_l                                AitalkMini702C1573CA101486161704102408A5E3
#define g_pwFFTCosTable                       AitalkMini749BA873CA101486161704102408A5E3
#define g_pwFilterbankInfo                    AitalkMini75F8CA73CA101486161704102408A5E3
#define g_s16CMNCoef                          AitalkMini7752E773CA101486161704102408A5E3
#define g_s16CMNCoefMax                       AitalkMini78AC1B73CA101486161704102408A5E3
#define g_s16CMNCoefMin                       AitalkMini7A066D73CA101486161704102408A5E3
#define g_s16CMScore                          AitalkMini7B61BD73CA101486161704102408A5E3
#define g_s16MeanVarTable                     AitalkMini7CBE8C73CA101486161704102408A5E3
#define g_s16SimpleLnTable                    AitalkMini7E189A73CA101486161704102408A5E3
#define g_sHamWindow                          AitalkMini7F72D573CA101486161704102408A5E3
#define g_ucReverseTable                      AitalkMini8226EA73CA101486161704102408A5E3
#define g_wFBToMFCCTable                      AitalkMini83835773CA101486161704102408A5E3
#define LAddQ10                               AitalkMini84DE7373CA101486161704102408A5E3
#define g_bChkAmbientNoise                    AitalkMini8638C573CA101486161704102408A5E3
#define g_ps16MeanCoefQ                       AitalkMini8ACB9773CA101486161704102408A5E3
#define g_ps16MeanCoef                        AitalkMini8D83A073CA101486161704102408A5E3
/* #define g_iPCMEnd                          AitalkMini8791D473CA101486161704102408A5E3 */
/* #define g_pPCMBuffer                       AitalkMini8C285773CA101486161704102408A5E3 */
/* #define g_iPCMStart                        AitalkMini896C3B73CA101486161704102408A5E3 */
/* #define g_s32MeanSum                       AitalkMini8EDDAE73CA101486161704102408A5E3 */
#define EsSwitch1Decorder                     AitalkMiniAD52ED2DE2CB4dcbB7669FBF3C5D19DF
#define EsTrySwitch2Decorder                  AitalkMini4CC846493E404f40A24F1DC7ECF0D09F
#define EsFrontCheckEngery                    AitalkMini916B563DF3A64e7aAA63B4ADD5614468
#define EsTagOutputResult                     AitalkMiniDF6F17612273426880A244AC021F1DA6
#define EsTagSearchFrameStep                  AitalkMiniE8E970B771C445aeB12A788C1C9DFCE2
#define EsTagUpdateAllScore                   AitalkMini183C331715914ea8BA0D35A51D734D62
#define EsUpdateNBest                         AitalkMini276BE7BCAEFA46ffB65D23138C90A771
#define EsAddTagToLex                         AitalkMiniD3A31D81350842d9AAE0E47ADD4AE7E1
#define EsReverserOrder                       AitalkMiniBA887D688C17466e940CF677D926A98C
#define EsDelTagFromLex                       AitalkMini80F5EAB41D36454aA1002280B040DA94

#endif


#include "ivDebug.h"
#include "ivMemory.h"
#include "ivEsErrorCode.h"
#include "ivESR.h"

#endif /* !defined(ES_TEAM__2007_07_10__ESKERNEL_ESRKERNEL__H) */
