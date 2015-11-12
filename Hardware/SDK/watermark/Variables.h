
#define FREQB_BIN_NUM	(BIN_PER_BIT + MONITOR_CLUSTER_NUM + EGY_CLUSTER_WIDTH - 1 + DEVIATION_NUM) // no reserve for deviation correction, share with FREQD_BIN_NUM
#define FREQC_BIN_NUM	(BIN_PER_BIT + DEVIATION_NUM)	// share with FREQA/FREQC/FREQE
#define DEVIATION_NUM	1
#define MONITOR_CLUSTER_NUM		(7)	// before is 9
//#define EGY_CLUSTER_WIDTH	(4)	// before is 11
#define EGY_CLUSTER_WIDTH	(5)	// before is 11
#define BIN_PER_BIT		10	//(SAMPLE_PER_BIT/(32 * BIN_DECIMATE_ORDER)) // 1920/(32*6) @40ms, sample 10 bin from 60 bin @40ms


short FtredADISRBuf[WM_FRAMELEN ] ; //32 is enough
unsigned short FreqACosCoefIdx,FreqBCosCoefIdx,FreqCCosCoefIdx,FreqDCosCoefIdx,FreqECosCoefIdx	;
unsigned long ulFreqATotEgy,ulFreqBTotEgy,ulFreqCTotEgy,ulFreqDTotEgy,ulFreqETotEgy	;
long FreqACosAccEgy,FreqSinAccEgy,FreqBCosAccEgy,FreqBSinAccEgy,FreqCCosAccEgy,FreqCSinAccEgy,FreqDCosAccEgy,FreqDSinAccEgy,FreqECosAccEgy,FreqESinAccEgy	;
unsigned long ulEgyFreqBBinAry[FREQB_BIN_NUM],ulEgyFreqDBinAry[FREQB_BIN_NUM]; // must with same dimension
unsigned long ulEgyFreqCBinAry[FREQC_BIN_NUM];	
unsigned long ulSyncFreqDClusterEgy[2][MONITOR_CLUSTER_NUM]; // for sync bit 1 and 0
unsigned long ulSyncFreqBClusterEgy[2][MONITOR_CLUSTER_NUM];
long lSyncClusterEgy16k4Data1, lSyncClusterEgy16k4Data0;
unsigned long u32WMFreqCEgyShortTerm,u32WMFreqBEgyShortTerm,u32WMFreqCEgyLongTerm,u32WMFreqBEgyLongTerm	;

