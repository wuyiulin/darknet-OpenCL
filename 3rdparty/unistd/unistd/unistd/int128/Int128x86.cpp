
// The code in this file is only used for x86 compilation.
// For x64 mode the file Int128x64.asm should be used instead.
// They execute much faster, because they use 64-bit registers,
// and division functions are coded entirely in assembler.

#ifndef _M_X64

#include "Int128.h"
typedef unsigned int UINT;

void int128add( void *dst, const void *x) {
  __asm {
    mov         esi, x
    mov         edi, dst
    mov         eax, dword ptr[esi]
    add         dword ptr[edi], eax
    mov         eax, dword ptr[esi+4]
    adc         dword ptr[edi+4], eax
    mov         eax, dword ptr[esi+8]
    adc         dword ptr[edi+8], eax
    mov         eax, dword ptr[esi+12]
    adc         dword ptr[edi+12], eax
  }
}

void int128sub(void *dst, const void *x) {
  __asm {
    mov         esi, x
    mov         edi, dst
    mov         eax, dword ptr[esi]
    sub         dword ptr[edi], eax
    mov         eax, dword ptr[esi+4]
    sbb         dword ptr[edi+4], eax
    mov         eax, dword ptr[esi+8]
    sbb         dword ptr[edi+8], eax
    mov         eax, dword ptr[esi+12]
    sbb         dword ptr[edi+12], eax
  }
}

void int128inc(void *x) {
  __asm {
    mov         edi, x
    add         dword ptr[edi], 1
    jnc         Done
    adc         dword ptr[edi+4], 0
    adc         dword ptr[edi+8], 0
    adc         dword ptr[edi+12], 0
Done:
  }
}

void int128dec(void *x) {
  __asm {
    mov         edi, x
    sub         dword ptr[edi], 1
    jnc         Done
    sbb         dword ptr[edi+4], 0
    sbb         dword ptr[edi+8], 0
    sbb         dword ptr[edi+12], 0
Done:
  }
}

#pragma warning(disable:4731)

void int128mul(void *dst, const void *x) {
  _int128       *dp = (_int128*)dst;
  const _int128 *b = (const _int128*)x;

  if (!(dp->s4.i[1] || b->s4.i[1] || dp->s4.i[2] || b->s4.i[2] || dp->s4.i[3] || b->s4.i[3])) {
    HI64(*dp) = 0;
    LO64(*dp) = __int64(dp->s4.i[0]) * b->s4.i[0]; // simple _int64 multiplication. int32 * int32
  }
  else {
    const _int128       a(*dp);
    __asm {
      push        ebp
      lea         ebx, a                       // ebx = &a
      mov         ecx, b                       // ecx = b
      mov         ebp, dst                     // ebp = &dp (dst)
      xor         eax, eax
      mov         dword ptr[ebp+8 ], eax       // dst[2..3] = 0
      mov         dword ptr[ebp+12], eax
      mov         eax, dword ptr[ebx]          // 1. round
      mul         dword ptr[ecx]               // [edx:eax] = x[0]*y[0]
      mov         dword ptr[ebp]  , eax
      mov         dword ptr[ebp+4], edx        // dst[0:1] = [edx:eax]

      mov         eax, dword ptr[ebx]          // 2. round
      mul         dword ptr[ecx+4]             // [edx:eax] = x[0]*y[1]
      mov         esi, eax                     //
      mov         edi, edx                     // [edi:esi] = [edx:eax]
      mov         eax, dword ptr[ebx+4]
      mul         dword ptr[ecx]               // [edx:eax] = x[1]*y[0]
      add         esi, eax                     //
      adc         edi, edx                     // [edi:esi] += [edx:eax]
      adc         dword ptr[ebp+12],0          // may be extra carry for dst[3]
      add         dword ptr[ebp+4], esi
      adc         dword ptr[ebp+8], edi        // dst[1:2] += [edi:esi]

      mov         eax, dword ptr[ebx]          // 3. round. Extra carries are overflow
      mul         dword ptr[ecx+8]             // [edx:eax] = x[0]*y[2]
      mov         esi, eax                     //
      mov         edi, edx                     // [edi:esi] = [edx:eax]
      mov         eax, dword ptr[ebx+4]
      mul         dword ptr[ecx+4]             // [edx:eax] = x[1]*y[1]
      add         esi, eax                     //
      adc         edi, edx                     // [edi:esi] += [edx:eax]
      mov         eax, dword ptr[ebx+8]
      mul         dword ptr[ecx]               // [edx:eax] = x[2]*y[0]
      add         esi, eax                     //
      adc         edi, edx                     // [edi:esi] += [edx:eax]
      add         dword ptr[ebp+8 ], esi
      adc         dword ptr[ebp+12], edi       // dst[2:3] += [edi:esi]

      mov         eax, dword ptr[ebx]          // 4. round. High end and carries are overflow
      mul         dword ptr[ecx+12]            // [edx:eax] = x[0]*y[3]
      mov         esi, eax                     // esi = eax
      mov         eax, dword ptr[ebx+4]        //
      mul         dword ptr[ecx+8]             // [edx:eax] = x[1]*y[2]
      add         esi, eax                     // esi += eax
      mov         eax, dword ptr[ebx+8]
      mul         dword ptr[ecx+4]             // [edx:eax] = x[2]*y[1]
      add         esi, eax                     // esi += eax
      mov         eax, dword ptr[ebx+12]
      mul         dword ptr[ecx]               // [edx:eax] = x[3]*y[0]
      add         esi, eax                     // esi += eax
      add         dword ptr[ebp+12], esi       // dst[3] += esi

      pop         ebp
    }
  }
}

void int128shl(int shft, void *x) {
  __asm {
    mov         ecx, shft                      ; ecx = shift count
    mov         esi, x                         ; esi = x
    cmp         cl, 64
    jae         GE64
    cmp         cl, 32                         ; cl < 64
    jae         _32_63
    mov         eax, dword ptr[esi+8]          ; cl < 32
    shld        dword ptr[esi+12], eax, cl     ; shift s4.i[3] adding bits from s4.i[2]
    mov         eax, dword ptr[esi+4]
    shld        dword ptr[esi+8], eax, cl      ; shift s4.i[2] adding bits from s4.i[1]
    mov         eax, dword ptr[esi]
    shld        dword ptr[esi+4], eax, cl      ; shift s4.i[1] adding bits from s4.i[0]
    shl         dword ptr[esi], cl             ; shift s4.i[0]
    jmp         End

_32_63 :                                       ; 32 <= cl < 64
    and         cl, 1Fh                        ; cl %= 32
    mov         eax, dword ptr[esi+8]
    mov         dword ptr[esi+12], eax         ; s4.i[3] = s4.i[1]
    mov         eax, dword ptr[esi+4]
    shld        dword ptr[esi+12], eax, cl     ; shift s4.i[3] adding bits eax
    mov         dword ptr[esi+8], eax          ; s4.i[2] = s4.i[1]
    mov         eax, dword ptr[esi]            ; eax = s4.i[0]
    shld        dword ptr[esi+8], eax, cl      ; shift s4.i[2] adding bits eax
    shl         eax, cl
    mov         dword ptr[esi+4], eax          ; shift s4.i[1] eax << cl (old i[0] << cl)
    xor         eax, eax
    mov         dword ptr[esi], eax            ; s4.i[0] = 0
    jmp         End

GE64:                                          ; cl >= 64
    cmp         cl, 96
    jae         GE96
    and         cl, 1Fh                        ; cl < 96; cl %= 32
    mov         eax, dword ptr[esi+4]
    mov         dword ptr[esi+12], eax         ; s4.i[3] = s4.i[1]
    mov         eax, dword ptr[esi]            ; eax = s4.i[0]
    shld        dword ptr[esi+12], eax, cl     ; shift s4.i[3] adding bits eax
    shl         eax, cl
    mov         dword ptr[esi+8], eax          ; shift s4.i[2] eax << cl
    xor         eax, eax
    mov         dword ptr[esi], eax            ; s4.i[0] = 0
    mov         dword ptr[esi+4], eax          ; s4.i[1] = 0
    jmp         End

GE96:                                          ; cl >= 96
    cmp         cl, 128
    jae         GE128
    and         cl, 1Fh                        ; cl < 128; cl %= 32
    mov         eax, dword ptr[esi]
    shl         eax, cl
    mov         dword ptr[esi+12], eax         ; s4.i[3] = s4.i[0] << cl
    xor         eax, eax
    mov         dword ptr[esi], eax            ; s4.i[0] = 0
    mov         dword ptr[esi+4], eax          ; s4.i[1] = 0
    mov         dword ptr[esi+8], eax          ; s4.i[2] = 0
    jmp         End

GE128:                                         ; cl >= 128
    xor         eax, eax                       ; return 0
    mov         dword ptr[esi   ], eax
    mov         dword ptr[esi+4 ], eax
    mov         dword ptr[esi+8 ], eax
    mov         dword ptr[esi+12], eax
  }
End:;
}

void int128shr(int shft, void *x) {            // signed shift right
  __asm {
    mov         ecx, shft                      ; ecx = shift count
    mov         esi, x                         ; esi = x
    cmp         cl, 64
    jae         GE64
    cmp         cl, 32                         ; cl < 64
    jae         _32_63
    mov         eax, dword ptr[esi+4]
    shrd        dword ptr[esi], eax, cl        ; shift s4[0] new bits from s4[1] (eax)
    mov         eax, dword ptr[esi+8]
    shrd        dword ptr[esi+4], eax, cl      ; shift s4[1] new bits from s4[2] (eax)
    mov         eax, dword ptr[esi+12]
    shrd        dword ptr[esi+8], eax, cl      ; shift s4[2] new bits from s4[3] (eax)
    sar         dword ptr[esi+12], cl          ; shift s4[3]
    jmp         End

_32_63 :                                       ; 32 <= cl < 64
    and         cl, 1Fh                        ; cl %= 32
    mov         eax, dword ptr[esi+4]
    mov         dword ptr[esi], eax            ; s4[0] = eax (old s4[1])
    mov         eax, dword ptr[esi+8]          ; eax = s4[2]
    shrd        dword ptr[esi], eax, cl        ; shift s[0] new bits from s4[2] (eax)
    mov         dword ptr[esi+4], eax          ; s4[1] = eax (old s4[2])
    mov         eax, dword ptr[esi+12]         ; eax = s4[3]
    shrd        dword ptr[esi+4], eax, cl      ; shift s4[1] new bits from eax (old s4[3])
    mov         dword ptr[esi+8], eax          ; s4[2] = eax (old s4[3])
    mov         eax, dword ptr[esi+12]         ; eax = s4[3]
    sar         eax, 1fh                       ; eax contain signbit in all bits
    shrd        dword ptr[esi+8], eax, cl      ; shift s4[1] new bits from eax (sign of s4[3])
    mov         dword ptr[esi+12], eax         ; s4[3] = eax
    jmp         End

GE64:                                          ; cl >= 64
    cmp         cl, 96
    jae         GE96
    and         cl, 1Fh                        ; cl < 96; cl %= 32
    mov         eax, dword ptr[esi+8]
    mov         dword ptr[esi], eax            ; s4[0] = s4[2]
    mov         eax, dword ptr[esi+12]         ; eax = s4[3]
    shrd        dword ptr[esi], eax, cl        ; shift s4[0] new bits from eax (old s4[3])
    sar         eax, cl                        ; eax contain sign bit in all bits
    mov         dword ptr[esi+4], eax          ; s4[1] = eax (old s4[3])
    sar         eax, 1fh
    mov         dword ptr[esi+8], eax          ; s4[2] = sign of s4[3]
    mov         dword ptr[esi+12], eax         ; s4[3] = sign of s4[3]
    jmp         End

GE96:                                          ; cl >= 96
    cmp         cl, 128
    jae         GE128
    and         cl, 1Fh                        ; cl < 128; cl %= 32
    mov         eax, dword ptr[esi+12]
    sar         eax, cl
    mov         dword ptr[esi], eax            ; s4[0] = s4[3] >> cl (shift in signbit of s4[3])
    sar         eax, 1fh
    mov         dword ptr[esi+4 ], eax         ; s4[1] = sign of s4[3]
    mov         dword ptr[esi+8 ], eax         ; s4[2] = sign of s4[3]
    mov         dword ptr[esi+12], eax         ; s4[3] = sign of s4[3]
    jmp         End

GE128:                                         ; cl >= 128
    mov         eax, dword ptr[esi+12]
    sar         eax,1Fh
    mov         dword ptr[esi   ], eax
    mov         dword ptr[esi+4 ], eax
    mov         dword ptr[esi+8 ], eax
    mov         dword ptr[esi+12], eax
  }
End:;
}

void uint128shr(int shft, void *x) { // unsigned shift right
  __asm {
    mov         ecx, shft                      ; ecx = shift count
    mov         esi, x
    cmp         cl, 64
    jae         GE64
    cmp         cl, 32                         ; cl < 64
    jae         _32_63
    mov         eax, dword ptr[esi+4]
    shrd        dword ptr[esi], eax, cl       ; shift s4[0] new bits from s4[1] (eax)
    mov         eax, dword ptr[esi+8]
    shrd        dword ptr[esi+4], eax, cl     ; shift s4[1] new bits from s4[2] (eax)
    mov         eax, dword ptr[esi+12]
    shrd        dword ptr[esi+8], eax, cl     ; shift s4[2] new bits from s4[3] (eax)
    shr         dword ptr[esi+12], cl         ; shift s4[3]
    jmp         End

_32_63 :                                      ; 32 <= cl < 64
    and         cl, 1Fh                       ; cl %= 32
    mov         eax, dword ptr[esi+4]
    mov         dword ptr[esi], eax           ; s4[0] = eax (old s4[1])
    mov         eax, dword ptr[esi+8]         ; eax = s4[2]
    shrd        dword ptr[esi], eax, cl       ; shift s[0] new bits from s4[2] (eax)
    mov         dword ptr[esi+4], eax         ; s4[1] = eax (old s4[2])
    mov         eax, dword ptr[esi+12]        ; eax = s4[3]
    shrd        dword ptr[esi+4], eax, cl     ; shift s4[1] new bits from eax (old s4[3])
    shr         eax, cl
    mov         dword ptr[esi+8], eax         ; s4[2] = eax << cl
    xor         eax, eax
    mov         dword ptr[esi+12], eax        ; s4[3] = 0
    jmp         End

GE64:                                         ; cl >= 64
    cmp         cl, 96
    jae         GE96
    and         cl, 1Fh                       ; cl < 96; cl %= 32
    mov         eax, dword ptr[esi+8]
    mov         dword ptr[esi], eax           ; s4[0] = s4[2]
    mov         eax, dword ptr[esi+12]        ; eax = s4[3]
    shrd        dword ptr[esi], eax, cl       ; shift s4[0] new bits from eax (old s4[3])
    shr         eax, cl
    mov         dword ptr[esi+4], eax         ; s4[1] = eax >> cl (old s4[3] >> cl)
    xor         eax, eax
    mov         dword ptr[esi+8], eax         ; s4[2] = 0
    mov         dword ptr[esi+12], eax        ; s4[3] = 0
    jmp         End

GE96:                                         ; cl >= 96
    cmp         cl, 128
    jae         GE128
    and         cl, 1Fh                       ; cl < 128; cl %= 32
    mov         eax, dword ptr[esi+12]
    shr         eax, cl
    mov         dword ptr[esi], eax           ; s4[0] = s4[3] >> cl
    xor         eax, eax
    mov         dword ptr[esi+4 ], eax        ; s4[1] = 0
    mov         dword ptr[esi+8 ], eax        ; s4[2] = 0
    mov         dword ptr[esi+12], eax        ; s4[3] = 0
    jmp         End

GE128:                                        ; cl >= 128
    xor         eax,eax                       ; return 0
    mov         dword ptr[esi   ], eax
    mov         dword ptr[esi+4 ], eax
    mov         dword ptr[esi+8 ], eax
    mov         dword ptr[esi+12], eax
  }
End:;
}

static UINT getFirst16(const _uint128 &n, int &expo2) {
  UINT result, expo;
  __asm {
    pushf
    mov         ecx , 4
    mov         edi , n
    add         edi , 12
    xor         eax , eax
    std
    repe        scasd
    jnz         SearchBit
    xor         edx , edx
    jmp         End1Result

SearchBit:                                     ; assume ecx hold the index of integer with first 1-bit
    mov         edx , dword ptr [edi+4]
    bsr         eax , edx                      ; eax holds index of highest 1 bit
    cmp         eax , 15
    je          End2Results
    jg          TooManyBits
    test        ecx , ecx                      ; Get some bits from previous, if we got some
    je          End2Results

    shl         ecx , 5
    add         ecx , eax
    mov         expo, ecx

    mov         edi , dword ptr[edi]           ; edi = previous int
    mov         ecx , 15
    sub         ecx , eax                      ; ecx = 15 - index of higest 1-bit
    shld        edx , edi, cl                  ; Shift edx left adding new bits from previous digit (edi)
    jmp         End1Result

TooManyBits:                                   ; assume eax = index of higest 1-bit in edx (>15)
    shl         ecx , 5
    add         ecx , eax
    mov         expo, ecx
    mov         ecx , eax
    sub         ecx , 15
    shr         edx , cl
    jmp         End1Result

End2Results:
    shl         ecx , 5
    add         ecx , eax
    mov         expo, ecx
End1Result:
    mov         result, edx
    popf
  }
  expo2 = expo;
  return result;
}

static UINT getFirst32(const _uint128 &n, int &expo2) {
  UINT result, expo;
  __asm {
    pushf
    mov         ecx, 4
    mov         edi, n
    add         edi, 12
    xor         eax, eax
    std
    repe        scasd
    jnz         SearchBit
    xor         edx, edx
    jmp         End1Result

SearchBit:                                     ; assume ecx hold the index of integer with first 1-bit
    mov         edx, dword ptr[edi+4]
    bsr         eax, edx                       ; eax holds index of highest 1 bit
    cmp         eax, 31
    je          End2Results
    test        ecx, ecx                       ; Get some bits from next, if we got some
    je          End2Results

    shl         ecx, 5
    add         ecx, eax
    mov         expo, ecx

    mov         edi, dword ptr[edi]            ; edi = previous int
    mov         ecx, 31
    sub         ecx, eax                       ; ecx = 31 - bits already in edx
    shld        edx, edi, cl                   ; Shift edx left adding new bits from previous digit (edi)
    jmp         End1Result

End2Results:
    shl         ecx   , 5
    add         ecx   , eax
    mov         expo  , ecx

End1Result:
    mov         result, edx
    popf
  }
  expo2 = expo;
  return result;
}

static inline int getExpo2(UINT x) {
  UINT result;
  _asm {
    bsr         eax, x
    mov         result, eax
  }
  return result;
}

// Special class to perform fast multiplication of _uint128 and UINT
// no need to do any accumulation of "cross-multiplications" cause 2. operand
// is only 32 bits
class _uint128FastMul : public _uint128 {
public:
  inline _uint128FastMul &operator=(const _uint128 &src) {
    HI64(*this) = HI64(src); LO64(*this) = LO64(src);
    return *this;
  }
  inline _uint128FastMul &operator*=(UINT k) {
    const _uint128FastMul copy(*this);
    _asm {
      mov edi, this
      lea esi, copy
      mov eax, dword ptr[esi]
      mul k
      mov dword ptr[edi], eax
      mov dword ptr[edi+4], edx
      mov eax, dword ptr[esi+4]
      mul k
      add dword ptr[edi+4], eax
      mov dword ptr[edi+8], edx
      adc dword ptr[edi+8], 0
      mov eax, dword ptr[esi+8]
      mul k
      add dword ptr[edi+8], eax
      mov dword ptr[edi+12], edx
      adc dword ptr[edi+12], 0
      mov eax, dword ptr[esi+12]
      mul k
      add dword ptr[edi+12], eax
    }
    return *this;
  }
};

static void unsignedQuotRemainder(const _uint128 &a, const _uint128 &y, _uint128 *quot, _uint128 &rem) {
  _uint128FastMul p128;
  int             lastShift = 0;
  rem = a;
  if(quot) *quot = 0;
  if(y < 0x8000) {
    int                yExpo2 = getExpo2((UINT)y);
    const int          yScale = 15 - yExpo2;
    const UINT         y16    = (UINT)y << yScale;
    for(int count = 0; rem >= y16; count++) {
      int remExpo2;
      const UINT         rem32      = getFirst32(rem, remExpo2);
      const int          rem32Expo2 = getExpo2(rem32);
      UINT               q32;
      int                shift;
      p128 = y;
      if((shift = remExpo2 - rem32Expo2 + yScale) < 0) { // >= -31
        q32  = (rem32 / (y16+1)) >> -shift;
        shift = 0;
        switch(q32) {
        case 0 : q32 = 1    ; break;
        case 1 :              break;
        default: p128 *= q32; break;
        }
      } else {
        q32 = rem32 / (y16 + 1);
        switch(q32) {
        case 0 : q32 = 1    ; break;
        case 1 :              break;
        default: p128 *= q32; break;
        }
        if(shift) int128shl(shift, &p128);
      }
      if(quot) { // do we want the quot. If its NULL there's no need to do this
        if(count) {
          if(lastShift > shift) {
            int128shl(lastShift - shift, quot);
          }
          __asm {
            mov eax, q32
            mov esi, quot
            add dword ptr[esi], eax
            jnc AddDone1
            adc dword ptr[esi+4] ,0
            adc dword ptr[esi+8] ,0
            adc dword ptr[esi+12],0
          }
        } else {
          *quot = q32;
        }
AddDone1:
        lastShift = shift;
      }
      rem -= p128;
    }

    // rem < 0xffff
    if(quot) {
      if(lastShift) {
        int128shl(lastShift, quot);
      }
      *quot += (UINT)rem / (UINT)y;
    }
    rem = (UINT)rem % (UINT)y;
  } else {
    int                yExpo2;
    const UINT         y16    = getFirst16(y, yExpo2);
    const int          yScale = yExpo2 - getExpo2(y16);
    for(int count = 0; rem >= y; count++) {
      int remExpo2;
      const UINT         rem32      = getFirst32(rem, remExpo2);
      const int          rem32Expo2 = getExpo2(rem32);
      UINT               q32;
      int                shift;
      p128 = y;
      if((shift = remExpo2 - rem32Expo2 - yScale) < 0) { // >= -31
        q32  = (rem32 / (y16+1)) >> -shift;
        shift = 0;
        switch(q32) {
        case 0 : q32 = 1    ; break;
        case 1 :              break;
        default: p128 *= q32; break;
        }
      } else {
        q32 = rem32 / (y16 + 1);
        switch(q32) {
        case 0 : q32 = 1    ; break;
        case 1 :              break;
        default: p128 *= q32; break;
        }
        if(shift) int128shl(shift, &p128);
      }
      if(quot) { // do we want the quot. If its NULL there's no need to do this
        if(count) {
          if(lastShift > shift) {
            int128shl(lastShift - shift, quot);
          }
          __asm {
            mov eax, q32
            mov esi, quot
            add dword ptr[esi], eax
            jnc AddDone2
            adc dword ptr[esi+4] ,0
            adc dword ptr[esi+8] ,0
            adc dword ptr[esi+12],0
          }
        } else {
          *quot = q32;
        }
AddDone2:
        lastShift = shift;
      }
      rem -= p128;
    }
    if(lastShift && quot) {
      int128shl(lastShift, quot);
    }
  }
}

static void signedQuotRemainder(const _int128 &a, const _int128 &b, _int128 *quot, _int128 &rem) {
  const bool aNegative = a.isNegative();
  const bool bNegative = b.isNegative();
  if(!aNegative && !bNegative) {
    unsignedQuotRemainder(*(const _uint128*)&a, *(const _uint128*)&b, (_uint128*)quot, (_uint128&)rem);
  } else {
    unsignedQuotRemainder((_uint128)(aNegative?-a:a), (_uint128)(bNegative?-b:b), (_uint128*)quot, (_uint128&)rem);
    if(quot && (aNegative != bNegative)) int128neg(quot);
    if(aNegative) int128neg(&rem);
  }
}

void int128div(void *dst, void *x) {
  const _int128  a(*(_int128*)dst);
  _int128 rem;
  signedQuotRemainder(a, *(const _int128*)x, (_int128*)dst, rem);
}

void int128rem(void *dst, void *x) {
  const _int128  a(*(_int128*)dst);
  signedQuotRemainder(a, *(const _int128*)x, NULL, *(_int128*)dst);
}

void uint128div(void *dst, const void *x) {
  const _uint128  a(*(_uint128*)dst);
  _uint128 rem;
  unsignedQuotRemainder(a, *(const _uint128*)x, (_uint128*)dst, rem);
}

void uint128rem(void *dst, const void *x) {
  const _uint128  a(*(_uint128*)dst);
  unsignedQuotRemainder(a, *(const _uint128*)x, NULL, *(_uint128*)dst);
}

void int128neg(void *x) {
  __asm {
    mov esi, x
    not dword ptr[esi   ]
    not dword ptr[esi+ 4]
    not dword ptr[esi+ 8]
    not dword ptr[esi+12]
    add dword ptr[esi   ], 1
    adc dword ptr[esi+ 4], 0
    adc dword ptr[esi+ 8], 0
    adc dword ptr[esi+12], 0
  }
}

int int128cmp(const void *x1, const void *x2) {
  int result;
  __asm {
    mov         esi, x1
    mov         edi, x2
    mov         eax, dword ptr[esi+12]
    cmp         eax, dword ptr[edi+12]
    jl          lessthan                       ; signed compare of high int
    jg          greaterthan
    mov         eax, dword ptr[esi+8]
    cmp         eax, dword ptr[edi+8]
    jb          lessthan                       ; unsigned compare of the 3 others
    ja          greaterthan
    mov         eax, dword ptr[esi+4]
    cmp         eax, dword ptr[edi+4]
    jb          lessthan
    ja          greaterthan
    mov         eax, dword ptr[esi]            ;
    cmp         eax, dword ptr[edi]            ;
    jb          lessthan
    ja          greaterthan
    mov         result, 0                      ; they are equal
    jmp End
greaterthan:
    mov result, 1
    jmp End
lessThan:
    mov result, -1
  }
End:
  return result;
}

int uint128cmp(const void *x1, const void *x2) {
  int result;
  __asm {
    mov         esi, x1
    mov         edi, x2
    mov         eax, dword ptr[esi+12]
    cmp         eax, dword ptr[edi+12]
    jb          lessthan                       ; unsigned compare of all integers
    ja          greaterthan
    mov         eax, dword ptr[esi+8]
    cmp         eax, dword ptr[edi+8]
    jb          lessthan
    ja          greaterthan
    mov         eax, dword ptr[esi+4]
    cmp         eax, dword ptr[edi+4]
    jb          lessthan
    ja          greaterthan
    mov         eax, dword ptr[esi]            ;
    cmp         eax, dword ptr[edi]            ;
    jb          lessthan
    ja          greaterthan
    mov         result, 0                      ; they are equal
    jmp End
greaterthan:
    mov result, 1
    jmp End
lessThan:
    mov result, -1
  }
End:
  return result;
}

#endif // _M_X64
