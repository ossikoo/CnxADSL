#ifndef _DIAGNOS_H_
#define _DIAGNOS_H_
#define MAX_LOG_SIZE     100  // max number of diagnostic event we can log
                             // this number can be modified without affecting
                             // the operation
DPCCALL void DIAG_AddDiagnostic(PDATAPUMP_CONTEXT pDatapump, DIAG_ENTRY_TYPE, DIAG_SUBCODE_TYPE);
DPCCALL DIAGNOSTIC_TYPE * DIAG_GetDiagnostic(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL void DIAG_InitDiagnosticLog(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void DIAG_TraceWrite(PDATAPUMP_CONTEXT pDatapump, UINT8 Offset, DWORD Value);
DPCCALL void DIAG_AddResetCount(PDATAPUMP_CONTEXT pDatapump, DIAG_SUBCODE_TYPE subcode);

#endif
