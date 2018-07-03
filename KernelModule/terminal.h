#ifndef TERMINAL_H
#define TERMINAL_H
/*****                     Global functions                           ******/
DPCCALL void TRM_InputTerminal(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE mode, char inputkey);
DPCCALL void TRM_ResetTerminal(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE mode);
DPCCALL void TRM_PeriodicTerminalRefresh(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void TRM_DisplayWholeScreen(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE mode);
DPCCALL void TRM_AutoUpdateDisable(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE mode, BOOLEAN disable);
DPCCALL void TRM_TerminalInit(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void TRM_ForcedLogon(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN TRM_LockWorkArea(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE logon_mode);
DPCCALL void TRM_UnlockWorkArea(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE logon_mode);
DPCCALL void TRM_XmitStartupLog(PDATAPUMP_CONTEXT pDatapump, const char * msg, UINT8 data_cnt, SINT16 *data);
DPCCALL void TRM_DisplayNextXcvr(PDATAPUMP_CONTEXT pDatapump, SYS_LOGON_MODE_TYPE logon_mode, GEN_SINT direction);
#endif
