DPCCALL void TST_InitSelftest(PDATAPUMP_CONTEXT);
DPCCALL void TST_ClearSelftestResult(PDATAPUMP_CONTEXT);
DPCCALL void TST_StartSelftest(PDATAPUMP_CONTEXT, XCVR_SELFTEST_ID_TYPE selftest_id);
DPCCALL void TST_ExitSelftest(PDATAPUMP_CONTEXT);
DPCCALL UINT16 TST_GetCurTestDuration(PDATAPUMP_CONTEXT);
DPCCALL char *TST_GetCurSelftestStr(PDATAPUMP_CONTEXT);
DPCCALL UINT8 TST_GetCurSelftestState(PDATAPUMP_CONTEXT);
DPCCALL char *TST_GetCurSelftestStatusStr(PDATAPUMP_CONTEXT);
DPCCALL void TST_SetFixedDuration(PDATAPUMP_CONTEXT, UINT16 new_duration);
DPCCALL BOOLEAN TST_IsFixedDurationTestMode(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN TST_SysIsInTestMode(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN TST_SysIsInBurninMode(PDATAPUMP_CONTEXT);
