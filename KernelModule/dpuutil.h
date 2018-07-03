
/*****                     Global functions                           ******/
DPCCALL void    DPUU_NullRoutine(DPU_SCR_INFO *work_area);
DPCCALL void    DPUU_InitWorkarea(DPU_SCR_INFO *);
DPCCALL void    DPUU_StoreUserInput(DPU_SCR_INFO *, UINT8, UINT8);
DPCCALL void    DPUU_BackSpace(DPU_SCR_INFO *work_area);
DPCCALL void    DPUU_GetActiveAlarmStr(PDATAPUMP_CONTEXT pDatapump, char *output_buf, UINT8 data_stream);
DPCCALL void    DPUU_GetLineMeasStr(PDATAPUMP_CONTEXT pDatapump, char *output_buf, SINT16 line_meas);
DPCCALL void    DPUU_SetupForDownload(DPU_SCR_INFO *work_area);
DPCCALL void    DPUU_ClearHistoryDatabase(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8   DPUU_Get24HrTimeStampIndex(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8   DPUU_HexToDec(UINT8 char_input);
DPCCALL char    DPUU_ConvIpAddr(char *, UINT8 *);
DPCCALL const char * DPUU_GetEnableOrDisableStr(BOOLEAN flag);
DPCCALL const char * DPUU_GetOnOrOffStr(BOOLEAN flag);
DPCCALL void    DPUU_SetDefaultFactoryCfg(DPU_SCR_INFO *work_area,UINT8 default_config);
DPCCALL BOOLEAN DPUU_SetNVRAMDefaultCfg(PDATAPUMP_CONTEXT pDatapump, UINT8 default_config);

