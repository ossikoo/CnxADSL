#ifndef _AFEINTF_H_
#define _AFEINTF_H_


typedef enum CNXT_EEPROM_AFE_ID_E
{
	FETCHER_AFE_ID		= 0x01,
	AFE2B_AFE_ID		= 0x0F
} CNXT_EEPROM_AFE_ID_T;

// The EEPROM_AFE_ID_CHECK_VALUE should be the result of XORing the AFE Id value
// read from EEPROM and the AFE Id Check value read from EEPROM.
//
#define EEPROM_AFE_ID_CHECK_VALUE		0x5A

// FETCHER_AFE_ID(0x01) XOR FETCHER_AFE_ID_CHECK_VALUE(0x5B) => EEPROM_AFE_ID_CHECK_VALUE(0x5A)
// AFE2B_AFE_ID  (0x0F) XOR AFE2B_AFE_ID_CHECK_VALUE  (0x55) => EEPROM_AFE_ID_CHECK_VALUE(0x5A)
//
#define FETCHER_AFE_ID_CHECK_VALUE		(FETCHER_AFE_ID  ^ EEPROM_AFE_ID_CHECK_VALUE)
#define AFE2B_AFE_ID_CHECK_VALUE		(AFE2B_AFE_ID    ^ EEPROM_AFE_ID_CHECK_VALUE)

// AFE value stored in EEPROM is the AFE Id and the AFE Id Check Value, read
// as a WORD, the AFE Id is the LSB and the AFE Id Check Value is the MSB.
//
#define TIGRIS_EEPROM_FETCHER_AFE_ID	((FETCHER_AFE_ID_CHECK_VALUE << 8) + FETCHER_AFE_ID)
#define TIGRIS_EEPROM_AFE2B_AFE_ID		((AFE2B_AFE_ID_CHECK_VALUE << 8)   + AFE2B_AFE_ID)
#define NON_TIGRIS_EEPROM_AFE_ID		0


typedef enum 
{
    UNKNOWN_AFE,
    AFE2B,
    FETCHER,
    AFE3
}CNXT_AFE_TYPE;

/* Conexant AFE Register names */
typedef enum 
{
    UNUSED_REG_ADRS         = 0,
    REV_ID_AND_MODE_ADRS    = 1,
    SLEEP_ADRS              = 2,
    DIGITAL_ENABLE_ADRS     = 3,
    ANALOG_ENABLE1_ADRS     = 4,
    ANALOG_ENABLE2_ADRS     = 5,
    TX_GAIN_ADRS            = 6, 
    SHORT_LOOP_CNTL_ADRS    = TX_GAIN_ADRS,
    RX_GAIN_ADRS            = 7,
    CAL_CONTROL_ADRS        = 8,
    CAL_STATUS_ADRS         = 9,
    TONE_DETECT1_ADRS       =10,
    TONE_DETECT2_ADRS       =11,
    TONE_DETECT_TEST_ADRS   =12,
    TEST1_ADRS              =13,
    TEST2_ADRS              =14,
    ANALOG_TEST1_ADRS       =15,
    ANALOG_TEST2_ADRS       =16,
    CAL_TX_ADRS             =17,
    CAL_RX_ADRS             =18,
    CAL_AAF_ADRS            =19,
    CAL_DOWN_TX_ADRS        =20,
    PAD_CONTROL_ADRS        =21 ,
    NUM_AFE_REG_ADRS
} CLNCLNCLN;

typedef struct
{
    GEN_SINT   ADC_gain;
    GEN_SINT   rx_gain;
    GEN_SINT   hybrid_gain;
    GEN_SINT   hybrid_pregain;
}AFE_RX_GAIN_SETTING_TYPE;

typedef struct
{
    AFE_RX_GAIN_SETTING_TYPE rx_gain_setting;
    CNXT_AFE_TYPE cnxt_afe_rev;  // default to 0 (i.e. UNKNOWN_AFE)
    UINT8 afe_register_image[NUM_AFE_REG_ADRS]; // Contains values written to AFE.
    #ifdef CO
    GEN_SINT rx_setting;
    #endif
    GEN_SINT tx_setting;
}LOCAL_AFEINTF_TYPE;

DPCCALL void AFE_Calibrate(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_SetTxGain(PDATAPUMP_CONTEXT pDatapump, GEN_SINT gain);
DPCCALL BOOLEAN AFE_SetRxGain(PDATAPUMP_CONTEXT pDatapump, GEN_SINT gain);
DPCCALL void AFE_Init(PDATAPUMP_CONTEXT pDatapump);
DPCCALL GEN_SINT AFE_GetTxGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL GEN_SINT AFE_GetRxGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN AFE_SetHybridGain(PDATAPUMP_CONTEXT pDatapump, GEN_SINT overall_gain);
DPCCALL GEN_SINT AFE_GetHybridGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_SetADCGain(PDATAPUMP_CONTEXT pDatapump, GEN_SINT gain);
DPCCALL GEN_SINT AFE_GetADCGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL GEN_SINT AFE_GetRxOnlyGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL GEN_SINT AFE_GetHybridPreGain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_UpdateRxGainRegister(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_BypassRxFilter(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_EnableRxFilter(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_BypassTxCTF(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AFE_IncludeTxCTF(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void  AFE_BypassTxFIR(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void  AFE_IncludeTxFIR(PDATAPUMP_CONTEXT pDatapump);
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/afeintf.h $
 * 
 * 5     7/13/01 8:06a Davidsdj
 * PROBLEM:
 * Compile error.
 * 
 * SOLUTION:
 * Changed XOR to correct C syntax ^.
 * 
 * 4     7/13/01 7:03a Davidsdj
 * PROBLEM:
 * Recent changes to DP controller files in the Windows drivers required
 * changes to driver files not shared with Linux drivers.
 * 
 * SOLUTION:
 * Function added to "CardALTigrs.c" in Windows drivers needs added for
 * Linux drivers.
 * 
 * Added defines for AFE Id and AFE Id Check Value values stored in
 * EEPROM.
 * 
 * 
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 3     7/12/01 12:42p Lauds
 * Cleanup afeintf.c and add code to recognize Fetcher and utilize extra
 * hybrid gain in Fetcher
 * 
 * 2     7/10/01 12:53p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 **************  end of file $Workfile: afeintf.h $          ****************/
