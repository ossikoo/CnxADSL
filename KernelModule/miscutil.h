#ifndef _MISCUTIL_H_
#define _MISCUTIL_H_
typedef struct
{
    UINT32 saved_store;
    UINT16 teq_gain_scale;
    UINT16 saved_tx_filter_gain;
}LOCAL_MISCUTIL_TYPE;

DPCCALL void    MISC_SelectFrameBoundary(PDATAPUMP_CONTEXT pDatapump, SINT16 *);
DPCCALL UINT8   MISC_DetectTone(const SINT32 *);
DPCCALL SINT16  MISC_RxPwrdB(PDATAPUMP_CONTEXT pDatapump, SINT32 fft_out_pwr,SINT16 RXgaindB);
DPCCALL BOOLEAN MISC_AdjustRcvGain(PDATAPUMP_CONTEXT pDatapump, const SINT16 *);
DPCCALL void    MISC_ScaleTEQFilter(PDATAPUMP_CONTEXT pDatapump, SINT16 *);
DPCCALL BOOLEAN   MISC_ScaleTEQFilter2(PDATAPUMP_CONTEXT pDatapump, SINT16 *);
DPCCALL BOOLEAN MISC_ComputeNSR(PDATAPUMP_CONTEXT pDatapump, UINT32 *errpwr, MARGIN_TYPE snr_margin,
                     GEN_UINT chnl_start, GEN_UINT chnl_end);
DPCCALL void    MISC_Convert2AdjSqrtNSR(PDATAPUMP_CONTEXT pDatapump, UINT32 *, ADJ_SQRT_NSR_TYPE *);
DPCCALL UINT8   MISC_RxBitPreAllocation(PDATAPUMP_CONTEXT pDatapump,const UINT32 *, const UINT16 *, GEN_UINT, GEN_UINT trellis_overhead);
DPCCALL BOOLEAN MISC_RxBitAllocation(PDATAPUMP_CONTEXT pDatapump, const ADJ_SQRT_NSR_TYPE *, BITS_GAIN_TYPE *, GEN_UINT, BOOLEAN);

DPCCALL BOOLEAN MISC_SetupTxBitGainInfo(PDATAPUMP_CONTEXT pDatapump, const BITS_GAIN_TYPE *);
DPCCALL void    MISC_SetupRxBitGainInfo(PDATAPUMP_CONTEXT pDatapump, const BITS_GAIN_TYPE *);

DPCCALL UINT16  MISC_UpdateCRC(UINT16, UINT8);

DPCCALL UINT32  MISC_GetRcvPwr(const SINT16 *, UINT8, UINT8);
DPCCALL UINT32  MISC_GetRcvPwr_sint32(const SINT32 *, UINT8, UINT8);

DPCCALL CHNL_INFO_TYPE_RETURN MISC_GetChnlInfo(PDATAPUMP_CONTEXT pDatapump, UINT8 tone); // get channl info of status,bit,
                                           // bit capacity of each subchannel
DPCCALL SINT16  MISC_GetSysMargin(PDATAPUMP_CONTEXT pDatapump, const SINT16 *margin);

DPCCALL SINT16 MISC_GetLineAttenuation(PDATAPUMP_CONTEXT pDatapump, const SINT16 *, SINT16, BOOLEAN, UINT8, UINT8);
DPCCALL BOOLEAN MISC_ProgClk(PDATAPUMP_CONTEXT pDatapump,  GEN_UINT,  GEN_UINT);

DPCCALL void MISC_SetAllRxGains(PDATAPUMP_CONTEXT pDatapump );

DPCCALL SINT8 MISC_SetFFTAFETxAttenuation(PDATAPUMP_CONTEXT pDatapump,SINT8 attn);

DPCCALL void MISC_PRBSDET_IntrHandler(PDATAPUMP_CONTEXT pDatapump);

DPCCALL UINT8 MISC_MatchProfile (PDATAPUMP_CONTEXT pDatapump,  const XCVR_DATABASE_TYPE * xcvr_db);

DPCCALL void MISC_SendTxFilter(PDATAPUMP_CONTEXT pDatapump, const SINT16 * filter, UINT16 Gain);

DPCCALL UINT16 MISC_ComputeTxPwr (GEN_UINT direction, SINT8, const BITS_GAIN_TYPE *);

DPCCALL void    MISC_GenerateRxBitToneTbl(const BITS_GAIN_TYPE *,const UINT8 *, RX_BIT_TONE_TYPE *);

DPCCALL void MISC_RedefinePRBSChnl(PDATAPUMP_CONTEXT pDatapump, GEN_UINT num_active_chnls,
              const BITS_GAIN_TYPE * bits_gain_tbl,
              TONE_INFO_TYPE  * rx_tone_info);

DPCCALL GEN_UINT MISC_EstimateBitCapacity(const UINT32 *NSR, const BIT_CAPACITY_INFO_TYPE * bc_info, UINT8 * bc);

DPCCALL UINT32 MISC_Calc1stTo3rdHarmRatio( UINT32 *PowerArray, GEN_UINT FirstHarmIndex );

DPCCALL GEN_UINT MISC_DetAdslHeadEndEnvToUse( PDATAPUMP_CONTEXT pDatapump );

DPCCALL BOOLEAN MISC_ProgClk(PDATAPUMP_CONTEXT pDatapump,  GEN_UINT path,  GEN_UINT clk_input);
DPCCALL BOOLEAN MISC_AdjustHybridGain(PDATAPUMP_CONTEXT pDatapump, const SINT16 *average_pwr);
DPCCALL GEN_UINT MISC_Estimate26AWGkFt( UINT16 PowerArray[] );
DPCCALL void MISC_ComputeSnrMarginTbl(PDATAPUMP_CONTEXT pDatapump, const SINT32 * err_pwr, SINT16 * snr_margin_table, MARGIN_TYPE coding_gain);
DPCCALL void InitDatapumpStructure(PDATAPUMP_CONTEXT pDatapump, void * pExternalContext);
DPCCALL void MISC_SetupLoopTiming(PDATAPUMP_CONTEXT pDatapump, LOOP_TIMING_MODE mode);
DPCCALL GEN_UINT MISC_DetectSlicerError(PDATAPUMP_CONTEXT pDatapump, GEN_UINT tone, GEN_UINT bit);
DPCCALL near void NullBreakPointroutine(PDATAPUMP_CONTEXT pDatapump);
extern far const UINT16 dB2PwrScale[16];
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/miscutil.h $
 * 
 * 7     8/22/01 5:47p Lauds
 * change output parameter (i.e. 3rd) to UINT8 * instead
 * 
 * 5     8/17/01 11:40a Lauds
 * start preparing code to utilize Rushmore bit error detector
 * 
 * 4     7/16/01 7:02p Lauds
 * change sync gain for Alcatel DSLAM from tone 16 to average B&G gain
 * 
 * 3     7/12/01 8:29a Davidsdj
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 2     7/10/01 1:02p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 *************  end of file $Workfile: miscutil.h $          ****************/
