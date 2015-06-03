/*
 * Copyright 1994-2003 The MathWorks, Inc.
 *
 * File: ext_util.h     
 *
 * Abstract:
 * Headers for utility functions in ext_util.c.
 */

#ifndef EXT_CONVERT_H
#define EXT_CONVERT_H

extern void Copy32BitsToTarget(
    ExternalSim  *ES,
    char         *dst,
    const void   *src,
    const int    n);

extern void Copy32BitsFromTarget(
    ExternalSim *ES,
    void        *dst,
    const char  *src,
    const int    n);

extern void Double_TargetToHost(
    ExternalSim   *ES,
    void          *voidDst,
    const char    *src,
    const int     n,
    const int     dType);

extern void Double_HostToTarget(
    ExternalSim   *ES,/*lint !e952 Parameter 'ES' could be declared const */
    char          *dst,
    const void    * const voidSrc,
    const int     n,
    const int     dType);

extern void Generic_TargetToHost(
    ExternalSim  *ES,
    void         *dst,
    const char   * const src,
    const int    n,
    const int    dType);

extern void Generic_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const src,
    const int     n,
    const int     dType);

extern void Bool_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const voidSrc,
    const int     n,
    const int     dType);

extern void Bool_TargetToHost(
    ExternalSim   *ES,
    void          *voidDst,
    const char    *src,
    const int     n,
    const int     dType);

extern void Int33Plus_HostToTarget(
    ExternalSim   *ES,
    char          *dst,
    const void    * const src,
    const int     n,
    const int     dType);

extern void Int33Plus_TargetToHost(
    ExternalSim  *ES,
    void         *dst,
    const char   * const src,
    const int    n,
    const int    dType);

extern void ProcessConnectResponse1(ExternalSim *ES, PktHeader *pktHdr);

extern void ProcessTargetDataSizes(ExternalSim *ES, uint32_T *bufPtr);

extern void InstallIntegerOnlyDoubleConversionRoutines(ExternalSim *ES);


#endif /*EXT_CONVERT_H*/
