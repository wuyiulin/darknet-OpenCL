// sys/prctl.h
// Copyright 2016 Robin.Rowe@cinepaint.org
// License MIT open source

#ifndef prctl_h
#define prctl_h

#include "../portable/stub.h"

#ifdef __cplusplus
extern "C" {
#else
#define inline __inline
#endif

inline
int prctl(int option, unsigned long arg2, unsigned long arg3,unsigned long arg4, unsigned long arg5)
{   STUB_NEG(prctl);
}

enum
{	PR_CAP_AMBIENT,
	PR_CAP_AMBIENT_RAISE,
	PR_CAP_AMBIENT_LOWER,
	PR_CAP_AMBIENT_IS_SET,
	PR_CAP_AMBIENT_CLEAR_ALL,
	PR_CAPBSET_READ,
	PR_CAPBSET_DROP,
	PR_SET_CHILD_SUBREAPER,
	PR_GET_CHILD_SUBREAPER,
	PR_SET_DUMPABLE,
	PR_GET_DUMPABLE,
	PR_SET_ENDIAN,
	PR_GET_ENDIAN,
	PR_SET_FPEMU,
	PR_GET_FPEMU,
	PR_SET_FPEXC,
	PR_GET_FPEXC,
	PR_SET_KEEPCAPS,
	PR_GET_KEEPCAPS,
	PR_MCE_KILL,
	PR_MCE_KILL_GET,
	PR_SET_MM,
	PR_SET_MM_START_CODE,
	PR_SET_MM_END_CODE,
	PR_SET_MM_START_DATA,
	PR_SET_MM_END_DATA,
	PR_SET_MM_START_STACK,
	PR_SET_MM_START_BRK,
	PR_SET_MM_BRK,
	PR_SET_MM_ARG_START,
	PR_SET_MM_ARG_END,
	PR_SET_MM_ENV_START,
	PR_SET_MM_ENV_END,
	PR_SET_MM_AUXV,
	PR_SET_MM_EXE_FILE,
	PR_SET_NAME,
	PR_GET_NAME,
	PR_SET_NO_NEW_PRIVS,
	PR_GET_NO_NEW_PRIVS,
	PR_SET_PDEATHSIG,
	PR_GET_PDEATHSIG,
	PR_SET_PTRACER,
	PR_SET_SECCOMP,
	PR_GET_SECCOMP,
	PR_SET_SECUREBITS,
	PR_GET_SECUREBITS,
	PR_SET_THP_DISABLE,
	PR_TASK_PERF_EVENTS_DISABLE,
	PR_TASK_PERF_EVENTS_ENABLE,
	PR_GET_THP_DISABLE,
	PR_GET_TID_ADDRESS,
	PR_SET_TIMERSLACK,
	PR_GET_TIMERSLACK,
	PR_SET_TIMING,
	PR_GET_TIMING,
	PR_SET_TSC,
	PR_GET_TSC,
	PR_SET_UNALIGN,
	PR_GET_UNALIGN
};

#ifdef __cplusplus
}
#endif

#endif
