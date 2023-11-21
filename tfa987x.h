#pragma once
#define BIT(x) (1 << (x))
#define BIT_ULL(nr) (1ULL << (nr))
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (sizeof(long) * 8 - 1 - (h))))
#define GENMASK_ULL(h, l)                                                      \
        (((~0ULL) << (l)) & (~0ULL >> (sizeof(long long) * 8 - 1 - (h))))
#define LSB_GET(value) ((value) & -(value))

#define TFA987X_SYS_CTRL0               0x00
#define TFA987X_SYS_CTRL0_PWDN_MSK      BIT(0)
#define TFA987X_SYS_CTRL0_I2CR_MSK      BIT(1)
#define TFA987X_SYS_CTRL0_AMPE_MSK      BIT(3)
#define TFA987X_SYS_CTRL0_DCDC_MSK      BIT(4)

#define TFA987X_SYS_CTRL1               0x01
#define TFA987X_SYS_CTRL1_MANSCONF_MSK  BIT(2)
#define TFA987X_SYS_CTRL1_MANSAOOSC_MSK BIT(4)

#define TFA987X_SYS_CTRL2               0x02
#define TFA987X_SYS_CTRL2_AUDFS_MSK     GENMASK(3, 0)
#define TFA987X_SYS_CTRL2_FRACTDEL_MSK  GENMASK(10, 5)

#define TFA987X_REV                     0x03
#define TFA987X_CLK_GATING_CTRL         0x05

#define TFA987X_TDM_CFG0                0x20
#define TFA987X_TDM_CFG0_FSBCLKS_MSK    GENMASK(15, 12)
#define TFA987X_TDM_CFG1                0x21
#define TFA987X_TDM_CFG1_NSLOTS_MSK     GENMASK(3,  0)
#define TFA987X_TDM_CFG1_SLOTBITS_MSK   GENMASK(8,  4)
#define TFA987X_TDM_CFG2                0x22
#define TFA987X_TDM_CFG2_SWIDTH_MSK     GENMASK(6,  2)
#define TFA987X_TDM_CFG3                0x23
#define TFA987X_TDM_CFG3_SPKE_MSK       BIT(0)
#define TFA987X_TDM_CFG3_DCE_MSK        BIT(1)
#define TFA987X_TDM_CFG3_CSE_MSK        BIT(3)
#define TFA987X_TDM_CFG3_VSE_MSK        BIT(4)
#define TFA987X_TDM_CFG6                0x26
#define TFA987X_TDM_CFG6_SPKS_MSK       GENMASK(3,  0)
#define TFA987X_TDM_CFG6_DCS_MSK        GENMASK(4,  7)
#define TFA987X_TDM_CFG6_CSS_MSK        GENMASK(15, 12)
#define TFA987X_TDM_CFG7                0x27
#define TFA987X_TDM_CFG7_VSS_MSK        GENMASK(3,  0)

#define TFA987X_AUDIO_CTRL              0x51
#define TFA987X_AUDIO_CTRL_BSSS_MSK     BIT(0)
#define TFA9872_AUDIO_CTRL_INTSMUTE_MSK BIT(1)
#define TFA987X_AUDIO_CTRL_HPFBYP_MSK   BIT(5)
#define TFA987X_AUDIO_CTRL_DPSA_MSK     BIT(7)

#define TFA987X_AMP_CFG                 0x52
#define TFA987X_AMP_CFG_CLIPCTRL_MSK    GENMASK(4, 2)
#define TFA987X_AMP_CFG_GAIN_MSK        GENMASK(12, 5)
#define TFA987X_AMP_CFG_SLOPEE_MSK      BIT(13)
#define TFA987X_AMP_CFG_SLOPESET_MSK    BIT(14)

#define TFA987X_KEY1_PWM_CFG            0x58
#define TFA987X_TDM_CFG8                0x61
#define TFA987X_TDM_CFG8_DCG_MSK        GENMASK(5, 2)
#define TFA987X_TDM_CFG8_SPKG_MSK       GENMASK(9, 6)

#define TFA987X_LOW_NOISE_GAIN1         0x62
#define TFA987X_LOW_NOISE_GAIN2         0x63
#define TFA987X_MODE1_DET1              0x64
#define TFA987X_MODE1_DET1_LPM1MODE_MSK GENMASK(15, 14)
#define TFA987X_MODE1_DETECTOR2         0x65
#define TFA987X_TDM_SRC                 0x68
#define TFA987X_CURSENSE_COMP           0x6f
#define TFA987X_DCDC_CTRL0              0x70
#define TFA9872_DCDC_CTRL0_DCVOS_MSK    GENMASK(2,  0)
#define TFA987X_DCDC_CTRL0_MCC_MSK      GENMASK(6,  3)
#define TFA987X_DCDC_CTRL0_DCIE_MSK     BIT(9)
#define TFA987X_DCDC_CTRL1              0x71
#define TFA987X_DCDC_CTRL4              0x74
#define TFA9872_DCDC_CTRL4_DCVOF_MSK    GENMASK(2,  0)
#define TFA987X_DCDC_CTRL4_DCTRIP_MSK   GENMASK(8,  4)
#define TFA987X_DCDC_CTRL5              0x75
#define TFA987X_DCDC_CTRL5_DCTRIP2_MSK  GENMASK(7,  3)
#define TFA987X_DCDC_CTRL6              0x76
#define TFA9874_DCDC_CTRL6_DCVOF_MSK    GENMASK(8,  3)
#define TFA9874_DCDC_CTRL6_DCVOS_MSK    GENMASK(14,  9)
#define FIELD_PREP(mask, value) (((value) * LSB_GET(mask)) & (mask))