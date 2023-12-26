#ifndef DG10107_TABLE_H__
#define DG10107_TABLE_H__

#define SET_D1                      (s_u32DispBuf[0] |= 0x00000001)    //(s_u32DispBuf[4]|= (1<<10))
#define CLR_D1                      (s_u32DispBuf[0] &= 0xfffffffe)    //(s_u32DispBuf[4]&= ~(1<<10))
#define SET_H1                      (s_u32DispBuf[0] |= 0x00000002)    //(s_u32DispBuf[4]|= (1<<11))
#define CLR_H1                      (s_u32DispBuf[0] &= 0xfffffffd)    //(s_u32DispBuf[4]&= ~(1<<11))
#define SET_D2                      (s_u32DispBuf[0] |= 0x00000004)    //(s_u32DispBuf[4]|= (1<<8))
#define CLR_D2                      (s_u32DispBuf[0] &= 0xfffffffb)    //(s_u32DispBuf[4]&= ~(1<<8))
#define SET_H2                      (s_u32DispBuf[0] |= 0x00000008)    //(s_u32DispBuf[4]|= (1<<9))
#define CLR_H2                      (s_u32DispBuf[0] &= 0xfffffff7)    //(s_u32DispBuf[4]&= ~(1<<9))
#define SET_D3                      (s_u32DispBuf[0] |= 0x00000010)    //(s_u32DispBuf[0]|= (1<<0))
#define CLR_D3                      (s_u32DispBuf[0] &= 0xffffffef)    //(s_u32DispBuf[0]&= ~(1<<0))
#define SET_H3                      (s_u32DispBuf[0] |= 0x00000020)    //(s_u32DispBuf[0]|= (1<<1))
#define CLR_H3                      (s_u32DispBuf[0] &= 0xffffffdf)    //(s_u32DispBuf[0]&= ~(1<<1))
#define SET_D4                      (s_u32DispBuf[0] |= 0x00000040)    //(s_u32DispBuf[0]|= (1<<2))
#define CLR_D4                      (s_u32DispBuf[0] &= 0xffffffbf)    //(s_u32DispBuf[0]&= ~(1<<2))
#define SET_H4                      (s_u32DispBuf[0] |= 0x00000080)    //(s_u32DispBuf[0]|= (1<<3))
#define CLR_H4                      (s_u32DispBuf[0] &= 0xffffff7f)    //(s_u32DispBuf[0]&= ~(1<<3))
#define SET_D5                      (s_u32DispBuf[0] |= 0x00000100)    //(s_u32DispBuf[0]|= (1<<4))
#define CLR_D5                      (s_u32DispBuf[0] &= 0xfffffeff)    //(s_u32DispBuf[0]&= ~(1<<4))
#define SET_H5                      (s_u32DispBuf[0] |= 0x00000200)    //(s_u32DispBuf[0]|= (1<<5))
#define CLR_H5                      (s_u32DispBuf[0] &= 0xfffffdff)    //(s_u32DispBuf[0]&= ~(1<<5))
#define SET_D6                      (s_u32DispBuf[0] |= 0x00000400)    //(s_u32DispBuf[0]|= (1<<6))
#define CLR_D6                      (s_u32DispBuf[0] &= 0xfffffbff)    //(s_u32DispBuf[0]&= ~(1<<6))
#define SET_H6                      (s_u32DispBuf[0] |= 0x00000800)    //(s_u32DispBuf[0]|= (1<<7))
#define CLR_H6                      (s_u32DispBuf[0] &= 0xfffff7ff)    //(s_u32DispBuf[0]&= ~(1<<7))
#define SET_D7                      (s_u32DispBuf[0] |= 0x00001000)    //(s_u32DispBuf[0]|= (1<<8))
#define CLR_D7                      (s_u32DispBuf[0] &= 0xffffefff)    //(s_u32DispBuf[0]&= ~(1<<8))
#define SET_H7                      (s_u32DispBuf[0] |= 0x00002000)    //(s_u32DispBuf[0]|= (1<<9))
#define CLR_H7                      (s_u32DispBuf[0] &= 0xffffdfff)    //(s_u32DispBuf[0]&= ~(1<<9))
#define SET_D8                      (s_u32DispBuf[0] |= 0x00004000)    //(s_u32DispBuf[0]|= (1<<10))
#define CLR_D8                      (s_u32DispBuf[0] &= 0xffffbfff)    //(s_u32DispBuf[0]&= ~(1<<10))
#define SET_NULL1                   (s_u32DispBuf[0] |= 0x00008000)    //(s_u32DispBuf[0]|= (1<<11))
#define CLR_NULL1                   (s_u32DispBuf[0] &= 0xffff7fff)    //(s_u32DispBuf[0]&= ~(1<<11))
#define SET_BZ                      (s_u32DispBuf[0] |= 0x00010000)    //(s_u32DispBuf[0]|= (1<<12))
#define CLR_BZ                      (s_u32DispBuf[0] &= 0xfffeffff)    //(s_u32DispBuf[0]&= ~(1<<12))
#define SET_HZ7                     (s_u32DispBuf[0] |= 0x00020000)    //(s_u32DispBuf[0]|= (1<<13))
#define CLR_HZ7                     (s_u32DispBuf[0] &= 0xfffdffff)    //(s_u32DispBuf[0]&= ~(1<<13))
#define SET_KW                      (s_u32DispBuf[0] |= 0x00040000)    //(s_u32DispBuf[0]|= (1<<14))
#define CLR_KW                      (s_u32DispBuf[0] &= 0xfffbffff)    //(s_u32DispBuf[0]&= ~(1<<14))
#define SET_NULL2                   (s_u32DispBuf[0] |= 0x00080000)    //(s_u32DispBuf[0]|= (1<<15))
#define CLR_NULL2                   (s_u32DispBuf[0] &= 0xfff7ffff)    //(s_u32DispBuf[0]&= ~(1<<15))
#define SET_E1                      (s_u32DispBuf[1] |= 0x00000001)    //(s_u32DispBuf[4]|= (1<<22))
#define CLR_E1                      (s_u32DispBuf[1] &= 0xfffffffe)    //(s_u32DispBuf[4]&= ~(1<<22))
#define SET_C1                      (s_u32DispBuf[1] |= 0x00000002)    //(s_u32DispBuf[4]|= (1<<23))
#define CLR_C1                      (s_u32DispBuf[1] &= 0xfffffffd)    //(s_u32DispBuf[4]&= ~(1<<23))
#define SET_E2                      (s_u32DispBuf[1] |= 0x00000004)    //(s_u32DispBuf[4]|= (1<<20))
#define CLR_E2                      (s_u32DispBuf[1] &= 0xfffffffb)    //(s_u32DispBuf[4]&= ~(1<<20))
#define SET_C2                      (s_u32DispBuf[1] |= 0x00000008)    //(s_u32DispBuf[4]|= (1<<21))
#define CLR_C2                      (s_u32DispBuf[1] &= 0xfffffff7)    //(s_u32DispBuf[4]&= ~(1<<21))
#define SET_E3                      (s_u32DispBuf[1] |= 0x00000010)    //(s_u32DispBuf[1]|= (1<<0))
#define CLR_E3                      (s_u32DispBuf[1] &= 0xffffffef)    //(s_u32DispBuf[1]&= ~(1<<0))
#define SET_C3                      (s_u32DispBuf[1] |= 0x00000020)    //(s_u32DispBuf[1]|= (1<<1))
#define CLR_C3                      (s_u32DispBuf[1] &= 0xffffffdf)    //(s_u32DispBuf[1]&= ~(1<<1))
#define SET_E4                      (s_u32DispBuf[1] |= 0x00000040)    //(s_u32DispBuf[1]|= (1<<2))
#define CLR_E4                      (s_u32DispBuf[1] &= 0xffffffbf)    //(s_u32DispBuf[1]&= ~(1<<2))
#define SET_C4                      (s_u32DispBuf[1] |= 0x00000080)    //(s_u32DispBuf[1]|= (1<<3))
#define CLR_C4                      (s_u32DispBuf[1] &= 0xffffff7f)    //(s_u32DispBuf[1]&= ~(1<<3))
#define SET_E5                      (s_u32DispBuf[1] |= 0x00000100)    //(s_u32DispBuf[1]|= (1<<4))
#define CLR_E5                      (s_u32DispBuf[1] &= 0xfffffeff)    //(s_u32DispBuf[1]&= ~(1<<4))
#define SET_C5                      (s_u32DispBuf[1] |= 0x00000200)    //(s_u32DispBuf[1]|= (1<<5))
#define CLR_C5                      (s_u32DispBuf[1] &= 0xfffffdff)    //(s_u32DispBuf[1]&= ~(1<<5))
#define SET_E6                      (s_u32DispBuf[1] |= 0x00000400)    //(s_u32DispBuf[1]|= (1<<6))
#define CLR_E6                      (s_u32DispBuf[1] &= 0xfffffbff)    //(s_u32DispBuf[1]&= ~(1<<6))
#define SET_C6                      (s_u32DispBuf[1] |= 0x00000800)    //(s_u32DispBuf[1]|= (1<<7))
#define CLR_C6                      (s_u32DispBuf[1] &= 0xfffff7ff)    //(s_u32DispBuf[1]&= ~(1<<7))
#define SET_E7                      (s_u32DispBuf[1] |= 0x00001000)    //(s_u32DispBuf[1]|= (1<<8))
#define CLR_E7                      (s_u32DispBuf[1] &= 0xffffefff)    //(s_u32DispBuf[1]&= ~(1<<8))
#define SET_C7                      (s_u32DispBuf[1] |= 0x00002000)    //(s_u32DispBuf[1]|= (1<<9))
#define CLR_C7                      (s_u32DispBuf[1] &= 0xffffdfff)    //(s_u32DispBuf[1]&= ~(1<<9))
#define SET_E8                      (s_u32DispBuf[1] |= 0x00004000)    //(s_u32DispBuf[1]|= (1<<10))
#define CLR_E8                      (s_u32DispBuf[1] &= 0xffffbfff)    //(s_u32DispBuf[1]&= ~(1<<10))
#define SET_C8                      (s_u32DispBuf[1] |= 0x00008000)    //(s_u32DispBuf[1]|= (1<<11))
#define CLR_C8                      (s_u32DispBuf[1] &= 0xffff7fff)    //(s_u32DispBuf[1]&= ~(1<<11))
#define SET_HZ2                     (s_u32DispBuf[1] |= 0x00010000)    //(s_u32DispBuf[1]|= (1<<12))
#define CLR_HZ2                     (s_u32DispBuf[1] &= 0xfffeffff)    //(s_u32DispBuf[1]&= ~(1<<12))
#define SET_HZ8                     (s_u32DispBuf[1] |= 0x00020000)    //(s_u32DispBuf[1]|= (1<<13))
#define CLR_HZ8                     (s_u32DispBuf[1] &= 0xfffdffff)    //(s_u32DispBuf[1]&= ~(1<<13))
#define SET_H                       (s_u32DispBuf[1] |= 0x00040000)    //(s_u32DispBuf[1]|= (1<<14))
#define CLR_H                       (s_u32DispBuf[1] &= 0xfffbffff)    //(s_u32DispBuf[1]&= ~(1<<14))
#define SET_M3                      (s_u32DispBuf[1] |= 0x00080000)    //(s_u32DispBuf[1]|= (1<<15))
#define CLR_M3                      (s_u32DispBuf[1] &= 0xfff7ffff)    //(s_u32DispBuf[1]&= ~(1<<15))
#define SET_G1                      (s_u32DispBuf[2] |= 0x00000001)    //(s_u32DispBuf[5]|= (1<<2))
#define CLR_G1                      (s_u32DispBuf[2] &= 0xfffffffe)    //(s_u32DispBuf[5]&= ~(1<<2))
#define SET_B1                      (s_u32DispBuf[2] |= 0x00000002)    //(s_u32DispBuf[5]|= (1<<3))
#define CLR_B1                      (s_u32DispBuf[2] &= 0xfffffffd)    //(s_u32DispBuf[5]&= ~(1<<3))
#define SET_G2                      (s_u32DispBuf[2] |= 0x00000004)    //(s_u32DispBuf[5]|= (1<<0))
#define CLR_G2                      (s_u32DispBuf[2] &= 0xfffffffb)    //(s_u32DispBuf[5]&= ~(1<<0))
#define SET_B2                      (s_u32DispBuf[2] |= 0x00000008)    //(s_u32DispBuf[5]|= (1<<1))
#define CLR_B2                      (s_u32DispBuf[2] &= 0xfffffff7)    //(s_u32DispBuf[5]&= ~(1<<1))
#define SET_G3                      (s_u32DispBuf[2] |= 0x00000010)    //(s_u32DispBuf[2]|= (1<<0))
#define CLR_G3                      (s_u32DispBuf[2] &= 0xffffffef)    //(s_u32DispBuf[2]&= ~(1<<0))
#define SET_B3                      (s_u32DispBuf[2] |= 0x00000020)    //(s_u32DispBuf[2]|= (1<<1))
#define CLR_B3                      (s_u32DispBuf[2] &= 0xffffffdf)    //(s_u32DispBuf[2]&= ~(1<<1))
#define SET_G4                      (s_u32DispBuf[2] |= 0x00000040)    //(s_u32DispBuf[2]|= (1<<2))
#define CLR_G4                      (s_u32DispBuf[2] &= 0xffffffbf)    //(s_u32DispBuf[2]&= ~(1<<2))
#define SET_B4                      (s_u32DispBuf[2] |= 0x00000080)    //(s_u32DispBuf[2]|= (1<<3))
#define CLR_B4                      (s_u32DispBuf[2] &= 0xffffff7f)    //(s_u32DispBuf[2]&= ~(1<<3))
#define SET_G5                      (s_u32DispBuf[2] |= 0x00000100)    //(s_u32DispBuf[2]|= (1<<4))
#define CLR_G5                      (s_u32DispBuf[2] &= 0xfffffeff)    //(s_u32DispBuf[2]&= ~(1<<4))
#define SET_B5                      (s_u32DispBuf[2] |= 0x00000200)    //(s_u32DispBuf[2]|= (1<<5))
#define CLR_B5                      (s_u32DispBuf[2] &= 0xfffffdff)    //(s_u32DispBuf[2]&= ~(1<<5))
#define SET_G6                      (s_u32DispBuf[2] |= 0x00000400)    //(s_u32DispBuf[2]|= (1<<6))
#define CLR_G6                      (s_u32DispBuf[2] &= 0xfffffbff)    //(s_u32DispBuf[2]&= ~(1<<6))
#define SET_B6                      (s_u32DispBuf[2] |= 0x00000800)    //(s_u32DispBuf[2]|= (1<<7))
#define CLR_B6                      (s_u32DispBuf[2] &= 0xfffff7ff)    //(s_u32DispBuf[2]&= ~(1<<7))
#define SET_G7                      (s_u32DispBuf[2] |= 0x00001000)    //(s_u32DispBuf[2]|= (1<<8))
#define CLR_G7                      (s_u32DispBuf[2] &= 0xffffefff)    //(s_u32DispBuf[2]&= ~(1<<8))
#define SET_B7                      (s_u32DispBuf[2] |= 0x00002000)    //(s_u32DispBuf[2]|= (1<<9))
#define CLR_B7                      (s_u32DispBuf[2] &= 0xffffdfff)    //(s_u32DispBuf[2]&= ~(1<<9))
#define SET_G8                      (s_u32DispBuf[2] |= 0x00004000)    //(s_u32DispBuf[2]|= (1<<10))
#define CLR_G8                      (s_u32DispBuf[2] &= 0xffffbfff)    //(s_u32DispBuf[2]&= ~(1<<10))
#define SET_B8                      (s_u32DispBuf[2] |= 0x00008000)    //(s_u32DispBuf[2]|= (1<<11))
#define CLR_B8                      (s_u32DispBuf[2] &= 0xffff7fff)    //(s_u32DispBuf[2]&= ~(1<<11))
#define SET_HZ3                     (s_u32DispBuf[2] |= 0x00010000)    //(s_u32DispBuf[2]|= (1<<12))
#define CLR_HZ3                     (s_u32DispBuf[2] &= 0xfffeffff)    //(s_u32DispBuf[2]&= ~(1<<12))
#define SET_LOCK                    (s_u32DispBuf[2] |= 0x00020000)    //(s_u32DispBuf[2]|= (1<<13))
#define CLR_LOCK                    (s_u32DispBuf[2] &= 0xfffdffff)    //(s_u32DispBuf[2]&= ~(1<<13))
#define SET_HZ1                     (s_u32DispBuf[2] |= 0x00040000)    //(s_u32DispBuf[2]|= (1<<14))
#define CLR_HZ1                     (s_u32DispBuf[2] &= 0xfffbffff)    //(s_u32DispBuf[2]&= ~(1<<14))
#define SET_H_SLASH                 (s_u32DispBuf[2] |= 0x00080000)    //(s_u32DispBuf[2]|= (1<<15))
#define CLR_H_SLASH                 (s_u32DispBuf[2] &= 0xfff7ffff)    //(s_u32DispBuf[2]&= ~(1<<15))
#define SET_F1                      (s_u32DispBuf[3] |= 0x00000001)    //(s_u32DispBuf[5]|= (1<<14))
#define CLR_F1                      (s_u32DispBuf[3] &= 0xfffffffe)    //(s_u32DispBuf[5]&= ~(1<<14))
#define SET_A1                      (s_u32DispBuf[3] |= 0x00000002)    //(s_u32DispBuf[5]|= (1<<15))
#define CLR_A1                      (s_u32DispBuf[3] &= 0xfffffffd)    //(s_u32DispBuf[5]&= ~(1<<15))
#define SET_F2                      (s_u32DispBuf[3] |= 0x00000004)    //(s_u32DispBuf[5]|= (1<<12))
#define CLR_F2                      (s_u32DispBuf[3] &= 0xfffffffb)    //(s_u32DispBuf[5]&= ~(1<<12))
#define SET_A2                      (s_u32DispBuf[3] |= 0x00000008)    //(s_u32DispBuf[5]|= (1<<13))
#define CLR_A2                      (s_u32DispBuf[3] &= 0xfffffff7)    //(s_u32DispBuf[5]&= ~(1<<13))
#define SET_F3                      (s_u32DispBuf[3] |= 0x00000010)    //(s_u32DispBuf[3]|= (1<<0))
#define CLR_F3                      (s_u32DispBuf[3] &= 0xffffffef)    //(s_u32DispBuf[3]&= ~(1<<0))
#define SET_A3                      (s_u32DispBuf[3] |= 0x00000020)    //(s_u32DispBuf[3]|= (1<<1))
#define CLR_A3                      (s_u32DispBuf[3] &= 0xffffffdf)    //(s_u32DispBuf[3]&= ~(1<<1))
#define SET_F4                      (s_u32DispBuf[3] |= 0x00000040)    //(s_u32DispBuf[3]|= (1<<2))
#define CLR_F4                      (s_u32DispBuf[3] &= 0xffffffbf)    //(s_u32DispBuf[3]&= ~(1<<2))
#define SET_A4                      (s_u32DispBuf[3] |= 0x00000080)    //(s_u32DispBuf[3]|= (1<<3))
#define CLR_A4                      (s_u32DispBuf[3] &= 0xffffff7f)    //(s_u32DispBuf[3]&= ~(1<<3))
#define SET_F5                      (s_u32DispBuf[3] |= 0x00000100)    //(s_u32DispBuf[3]|= (1<<4))
#define CLR_F5                      (s_u32DispBuf[3] &= 0xfffffeff)    //(s_u32DispBuf[3]&= ~(1<<4))
#define SET_A5                      (s_u32DispBuf[3] |= 0x00000200)    //(s_u32DispBuf[3]|= (1<<5))
#define CLR_A5                      (s_u32DispBuf[3] &= 0xfffffdff)    //(s_u32DispBuf[3]&= ~(1<<5))
#define SET_F6                      (s_u32DispBuf[3] |= 0x00000400)    //(s_u32DispBuf[3]|= (1<<6))
#define CLR_F6                      (s_u32DispBuf[3] &= 0xfffffbff)    //(s_u32DispBuf[3]&= ~(1<<6))
#define SET_A6                      (s_u32DispBuf[3] |= 0x00000800)    //(s_u32DispBuf[3]|= (1<<7))
#define CLR_A6                      (s_u32DispBuf[3] &= 0xfffff7ff)    //(s_u32DispBuf[3]&= ~(1<<7))
#define SET_F7                      (s_u32DispBuf[3] |= 0x00001000)    //(s_u32DispBuf[3]|= (1<<8))
#define CLR_F7                      (s_u32DispBuf[3] &= 0xffffefff)    //(s_u32DispBuf[3]&= ~(1<<8))
#define SET_A7                      (s_u32DispBuf[3] |= 0x00002000)    //(s_u32DispBuf[3]|= (1<<9))
#define CLR_A7                      (s_u32DispBuf[3] &= 0xffffdfff)    //(s_u32DispBuf[3]&= ~(1<<9))
#define SET_F8                      (s_u32DispBuf[3] |= 0x00004000)    //(s_u32DispBuf[3]|= (1<<10))
#define CLR_F8                      (s_u32DispBuf[3] &= 0xffffbfff)    //(s_u32DispBuf[3]&= ~(1<<10))
#define SET_A8                      (s_u32DispBuf[3] |= 0x00008000)    //(s_u32DispBuf[3]|= (1<<11))
#define CLR_A8                      (s_u32DispBuf[3] &= 0xffff7fff)    //(s_u32DispBuf[3]&= ~(1<<11))
#define SET_T1                      (s_u32DispBuf[3] |= 0x00010000)    //(s_u32DispBuf[3]|= (1<<12))
#define CLR_T1                      (s_u32DispBuf[3] &= 0xfffeffff)    //(s_u32DispBuf[3]&= ~(1<<12))
#define SET_TEMP                    (s_u32DispBuf[3] |= 0x00020000)    //(s_u32DispBuf[3]|= (1<<13))
#define CLR_TEMP                    (s_u32DispBuf[3] &= 0xffffdfff)    //(s_u32DispBuf[3]&= ~(1<<13))
#define SET_HZ5                     (s_u32DispBuf[3] |= 0x00040000)    //(s_u32DispBuf[3]|= (1<<14))
#define CLR_HZ5                     (s_u32DispBuf[3] &= 0xfffdffff)    //(s_u32DispBuf[3]&= ~(1<<14))
#define SET_HZ4                     (s_u32DispBuf[3] |= 0x00080000)    //(s_u32DispBuf[3]|= (1<<15))
#define CLR_HZ4                     (s_u32DispBuf[3] &= 0xfff7ffff)    //(s_u32DispBuf[3]&= ~(1<<15))

#endif




















