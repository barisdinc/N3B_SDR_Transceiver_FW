#ifndef _HMC830_H
#define _HMC830_H

#ifdef __cplusplus
extern "C" {
#endif
   
//#include "main.h"
#include "Arduino.h"
#include <stdbool.h>

// CONFIG MODE
#define HMC830_HMC_MODE     0
// TODO HMC830_OPEN_MODE
//#define HMC830_OPEN_MODE  1

#define HMC830_SEN_Pin  13
#define HMC830_SCK_Pin  14
#define HMC830_SDI_Pin  15
#define HMC830_SDO_Pin  17


// CONFIG GPIO
#define HMC830_SCK(x)   digitalWrite(HMC830_SCK_Pin, x)//((x)==0 ? 0 : 1))
#define HMC830_SEN(x)   digitalWrite(HMC830_SEN_Pin, x)//((x)==0 ? 0 : 1))
#define HMC830_SDI(x)   digitalWrite(HMC830_SDI_Pin, x)//((x)==0 ? 0 : 1))
#define HMC830_SDO      ((digitalRead(HMC830_SDO_Pin) == 0)? 0 : 1)

// HMC830 Mode
#define HMC830_INTEGER_MODE                                 0
#define HMC830_FRACTIONAL_MODE                              1
#define HMC830_OUTPUT_DIFFERENTIAL_MODE                     0
#define HMC830_OUTPUT_SINGLE_ENDED_MODE                     1
#define HMC830_UNLOCKED                                     0
#define HMC830_LOCKED                                       1

// HMC830 REGISTER MAP
#define HMC830_REG00H_ID                                    0x00
#define HMC830_REG00H_CHIP_ID_MASK                          0x00FFFFFF
#define HMC830_REG00H_CHIP_ID_OFFSET                        0
#define HMC830_REG00H_READ_ADDRESS_MASK                     0x0000001F
#define HMC830_REG00H_READ_ADDRESS_OFFSET                   0
#define HMC830_REG00H_SOFT_RESET_MASK                       0x00000020
#define HMC830_REG00H_SOFT_RESET_OFFSET                     5
#define HMC830_REG00H_CHIP_ID                               0xA7975

#define HMC830_REG01H_ENABLE                                0x01
#define HMC830_REG01H_CHIPEN_PIN_SELECT_MASK                0x00000001
#define HMC830_REG01H_CHIPEN_FROM_SPI_MASK                  0x00000002
#define HMC830_REG01H_KEEP_BIAS_ON_MASK                     0x00000004
#define HMC830_REG01H_KEEP_PD_ON_MASK                       0x00000008
#define HMC830_REG01H_KEEP_CP_ON_MASK                       0x00000010
#define HMC830_REG01H_KEEP_REF_BUF_ON_MASK                  0x00000020
#define HMC830_REG01H_KEEP_VCO_ON_MASK                      0x00000040
#define HMC830_REG01H_KEEP_GPO_DRIVER_ON_MASK               0x00000080

#define HMC830_REG02H_REFDIV                                0x02
#define HMC830_REG02H_RDIV_MASK                             0x00003FFF
#define HMC830_REG02H_RDIV_OFFSET                           0
#define HMC830_REG02H_RDIV_MIN                              1
#define HMC830_REG02H_RDIV_MAX                              16383

#define HMC830_REG03H_INTEGER_PART                          0x03
#define HMC830_REG03H_INTG_MASK                             0x0007FFFF
#define HMC830_REG03H_INTG_OFFSET                           0
#define HMC830_REG03H_INTG_FACTIONAL_MIN                    20
#define HMC830_REG03H_INTG_FACTIONAL_MAX                    524284
#define HMC830_REG03H_INTG_INTEGER_MIN                      16
#define HMC830_REG03H_INTG_INTEGER_MAX                      524287

#define HMC830_REG04H_FRACTIONAL_PART                       0x04
#define HMC830_REG04H_FRAC_MASK                             0x00FFFFFF
#define HMC830_REG04H_FRAC_OFFSET                           0
#define HMC830_REG04H_FRAC_MIN                              0
#define HMC830_REG04H_FRAC_MAX                              16777215

#define HMC830_REG05H_VCO_SPI                               0x05
#define HMC830_REG05H_VCO_SUBSYSTEM_ID_MASK                 0x00000007
#define HMC830_REG05H_VCO_SUBSYSTEM_ID_OFFSET               0
#define HMC830_REG05H_VCO_SUBSYSTEM_REG_ADDRESS_MASK        0x00000078
#define HMC830_REG05H_VCO_SUBSYSTEM_REG_ADDRESS_OFFSET      3
#define HMC830_REG05H_VCO_SUBSYSTEM_DATA_MASK               0x0000FF80
#define HMC830_REG05H_VCO_SUBSYSTEM_DATA_OFFSET             7
#define HMC830_REG05H_VCO_SUBSYSTEM_ID_WB                   0x0

#define HMC830_REG06H_SD_CFG                                0x06
#define HMC830_REG06H_SEED_MASK                             0x00000003
#define HMC830_REG06H_ORDER_MASK                            0x0000000C
#define HMC830_REG06H_FRAC_BYPASS_MASK                      0x00000080
#define HMC830_REG06H_AUTOSEED_MASK                         0x00000100
#define HMC830_REG06H_CLKRQ_REFDIV_SEL_MASK                 0x00000200
#define HMC830_REG06H_SD_MODULATOR_CLK_SELECT_MASK          0x00000400
#define HMC830_REG06H_SD_ENABLE_MASK                        0x00000800
#define HMC830_REG06H_BIST_ENABLE_MASK                      0x00040000
#define HMC830_REG06H_RDIV_BIST_CYCLES_MASK                 0x00180000
#define HMC830_REG06H_AUTO_CLOCK_CONFIG_MASK                0x00200000

#define HMC830_REG07H_LOCK_DETECT                           0x07
#define HMC830_REG07H_LKD_WINCNT_MAX_MASK                   0x00000007
#define HMC830_REG07H_ENABLE_INTERNAL_LOCK_DETECT_MASK      0x00000008
#define HMC830_REG07H_LOCK_DETECT_WINDOW_TYPE_MASK          0x00000040
#define HMC830_REG07H_LD_DIGITAL_WINDOW_DURATION_MASK       0x00000380
#define HMC830_REG07H_LD_DIGITAL_TIMER_FREQ_CTRL_MASK       0x00000C00
#define HMC830_REG07H_LD_TIMER_TEST_MODE_MASK               0x00001000
#define HMC830_REG07H_AUTO_RELOCK_ONE_TRY_MASK              0x00002000

#define HMC830_REG08H_ANALOG_EN                             0x08
#define HMC830_REG08H_BIAS_EN_MASK                          0x00000001
#define HMC830_REG08H_CP_EN_MASK                            0x00000002
#define HMC830_REG08H_PD_EN_MASK                            0x00000004
#define HMC830_REG08H_REFBUF_EN_MASK                        0x00000008
#define HMC830_REG08H_VCOBUF_EN_MASK                        0x00000010
#define HMC830_REG08H_GPO_PAD_EN_MASK                       0x00000020
#define HMC830_REG08H_VCO_DIV_CLK_TO_DIG_EN_MASK            0x00000080
#define HMC830_REG08H_PRESCALER_CLOCK_EN_MASK               0x00000200
#define HMC830_REG08H_VCO_BUFFER_AND_PRESCALER_BIAS_EN_MASK 0x00000400
#define HMC830_REG08H_CHARGE_PUMP_INTERNAL_OPAMP_EN_MASK    0x00000800
#define HMC830_REG08H_HIGH_FREQUENCY_REFERENCE_MASK         0x00200000

#define HMC830_REG09H_CHARGE_PUMP                           0x09
#define HMC830_REG09H_CP_DN_GAIN_MASK                       0x0000007F
#define HMC830_REG09H_CP_DN_GAIN_OFFSET                     0
#define HMC830_REG09H_CP_UP_GAIN_MASK                       0x00003F80
#define HMC830_REG09H_CP_UP_GAIN_OFFSET                     7
#define HMC830_REG09H_OFFSET_MAGNITUDE_MASK                 0x001FC000
#define HMC830_REG09H_OFFSET_MAGNITUDE_OFFSET               14
#define HMC830_REG09H_OFFSET_UP_ENABLE_MASK                 0x00200000
#define HMC830_REG09H_OFFSET_UP_ENABLE_OFFSET               21
#define HMC830_REG09H_OFFSET_DN_ENABLE_MASK                 0x00400000
#define HMC830_REG09H_OFFSET_DN_ENABLE_OFFSET               22
#define HMC830_REG09H_HIKCP_MASK                            0x00800000
#define HMC830_REG09H_HIKCP_OFFSET                          23
#define HMC830_REG09H_CP_GAIN_MIN                           0.00
#define HMC830_REG09H_CP_GAIN_MAX                           2.54
#define HMC830_REG09H_OFFSET_MAGNITUDE_MIN                  0
#define HMC830_REG09H_OFFSET_MAGNITUDE_MAX                  635

#define HMC830_REG0AH_VCO_AUTOCAL_CFG                       0x0A
#define HMC830_REG0AH_VTUNE_RESOLUTION_MASK                 0x00000007
#define HMC830_REG0AH_VCO_CURVE_ADJUSTMENT_MASK             0x00000038
#define HMC830_REG0AH_WAIT_STATE_SET_UP_MASK                0x000000C0
#define HMC830_REG0AH_NUM_OF_SAR_BITS_IN_VCO_MASK           0x00000300
#define HMC830_REG0AH_FORCE_CURVE_MASK                      0x00000400
#define HMC830_REG0AH_BYPASS_VCO_TUNING_MASK                0x00000800
#define HMC830_REG0AH_NO_VSPI_TRIGGER_MASK                  0x00001000
#define HMC830_REG0AH_FSM_VSPI_CLOCK_SELECT_MASK            0x00006000
#define HMC830_REG0AH_XTAL_FALLING_EDGE_FOR_FSM_MASK        0x00008000
#define HMC830_REG0AH_FORCE_RDIVIDER_BYPASS_MASK            0x00010000

#define HMC830_REG0BH_PD                                    0x0B
#define HMC830_REG0BH_PD_DEL_SEL_MASK                       0x00000007
#define HMC830_REG0BH_SHORT_PD_INPUTS_MASK                  0x00000008
#define HMC830_REG0BH_PD_PHASE_SEL_MASK                     0x00000010
#define HMC830_REG0BH_PD_UP_EN_MASK                         0x00000020
#define HMC830_REG0BH_PD_DN_EN_MASK                         0x00000040
#define HMC830_REG0BH_CSP_MODE_MASK                         0x00000180
#define HMC830_REG0BH_FORCE_CP_UP_MASK                      0x00000200
#define HMC830_REG0BH_FORCE_CP_DN_MASK                      0x00000400
#define HMC830_REG0BH_FORCE_CP_MID_RAIL_MASK                0x00000800
#define HMC830_REG0BH_CP_INTERNAL_OPAMP_BIAS_MASK           0x00018000
#define HMC830_REG0BH_MCOUNTER_CLOCK_GATING_MASK            0x00060000

#define HMC830_REG0CH_FINE_FREQUENCY_CORRECTION             0x0C
#define HMC830_REG0CH_NUMBER_OF_CHANNELS_PER_FPD_MASK       0x00003FFF

#define HMC830_REG0FH_GPO_SPI_RDIV                          0x0F
#define HMC830_REG0FH_GPO_SELECT_MASK                       0x0000001F
#define HMC830_REG0FH_GPO_TEST_DATA_MASK                    0x00000020
#define HMC830_REG0FH_PREVENT_AUTOMUX_SDO_MASK              0x00000040
#define HMC830_REG0FH_LDO_DRIVER_ALWAYS_ON_MASK             0x00000080
#define HMC830_REG0FH_DISABLE_PFET_MASK                     0x00000100
#define HMC830_REG0FH_DISABLE_NFET_MASK                     0x00000200

#define HMC830_REG10H_VCO_TUNE                              0x10
#define HMC830_REG10H_VCO_SWITCH_SETTING_MASK               0x000000FF
#define HMC830_REG10H_AUTOCAL_BUSY_MASK                     0x00000100

#define HMC830_REG11H_SAR                                   0x11
#define HMC830_REG11H_SAR_ERROR_MAG_COUNTS_MASK             0x0007FFFF
#define HMC830_REG11H_SAR_ERROR_SIGN_MASK                   0x00080000

#define HMC830_REG12H_GPO2                                  0x12
#define HMC830_REG12H_GPO_MASK                              0x00000001
#define HMC830_REG12H_LOCK_DETECT_MASK                      0x00000002
#define HMC830_REG12H_LOCK_DETECT_OFFSET                    1

#define HMC830_REG13H_BIST                                  0x13
#define HMC830_REG13H_BIST_SIGNATURE_MASK                   0x0000FFFF
#define HMC830_REG13H_BIST_BUSY_MASK                        0x00010000

// HMC830 VCO SUBSYSTEM REGISTER MAP
#define HMC830_VCO_REG00H_TURNING                           0x0
#define HMC830_VCO_REG00H_CAL_MASK                          0x0001
#define HMC830_VCO_REG00H_CAPS_MASK                         0x01FE

#define HMC830_VCO_REG01H_ENABLES                           0x1
#define HMC830_VCO_REG01H_MASTER_ENABLE_VCO_SUBSYSTEM_MASK  0x0001
#define HMC830_VCO_REG01H_MANUAL_PLL_BUFFER_ENABLE_MASK     0x0002
#define HMC830_VCO_REG01H_MANUAL_RF_BUFFER_ENABLE_MASK      0x0004
#define HMC830_VCO_REG01H_MANUAL_DIVIDE_BY_1_ENABLE_MASK    0x0008
#define HMC830_VCO_REG01H_MANUAL_RF_DIVIDER_ENABLE_MASK     0x0010

#define HMC830_VCO_REG02H_BIASES                            0x2
#define HMC830_VCO_REG02H_RF_DIVIDE_RATIO_MASK              0x003F
#define HMC830_VCO_REG02H_RF_DIVIDE_RATIO_OFFSET            0
#define HMC830_VCO_REG02H_RF_OUTPUT_BUF_GAIN_CTL_MASK       0x00C0
#define HMC830_VCO_REG02H_RF_OUTPUT_BUF_GAIN_CTL_OFFSET     6
#define HMC830_VCO_REG02H_DIV_OUTPUT_STAGE_GAIN_CTL_MASK    0x0100
#define HMC830_VCO_REG02H_DIV_OUTPUT_STAGE_GAIN_CTL_OFFSET  8
#define HMC830_VCO_REG02H_KDIV_MIN                          0
#define HMC830_VCO_REG02H_KDIV_MAX                          62
#define HMC830_VCO_REG02H_GAIN_3                            0x3
#define HMC830_VCO_REG02H_GAIN_2                            0x2
#define HMC830_VCO_REG02H_GAIN_1                            0x1
#define HMC830_VCO_REG02H_GAIN_0                            0x0

#define HMC830_VCO_REG03H_CONFIG                            0x3
#define HMC830_VCO_REG03H_CONFIG_DEFAULT                    0x051
#define HMC830_VCO_REG03H_RF_BUFFER_SE_ENABLE_MASK          0x0001
#define HMC830_VCO_REG03H_RF_BUFFER_SE_ENABLE_OFFSET        0
#define HMC830_VCO_REG03H_MANUAL_RFO_MODE_MASK              0x0004
#define HMC830_VCO_REG03H_RF_BUFFER_BIAS_MASK               0x0018

#define HMC830_VCO_REG04H                                   0x4
#define HMC830_VCO_REG04H_RECOMMAND                         0x60A0

#define HMC830_VCO_REG05H                                   0x5
#define HMC830_VCO_REG05H_RECOMMAND                         0x1628

#define HMC830_VCO_REG06H                                   0x6
#define HMC830_VCO_REG06H_DEFAULT                           0x7FB0


// FUNCTION PROTOTYPES
void HMC830_Delay(void);                                    // Delay
void HMC830_Init(uint8_t MODE);                             // Initial (Mode Select)
void HMC830_HMC_Write(uint8_t address,uint32_t data);       // HMC Mode Write
uint32_t HMC830_HMC_Read(uint8_t address);                  // HMC Mode Read

void HMC830_HMC_VCO_Write(uint8_t vco_address,uint32_t data);   // HMC Mode VCO SubSystem Write

void HMC830_HMC_Write_REFDIV(uint16_t refdiv);
void HMC830_HMC_Write_NDIV(double ndiv);
void HMC830_HMC_Write_VCO_General_Setting(uint8_t kdiv, uint8_t GAIN);
void HMC830_HMC_Write_PFD_General_Setting(uint8_t PFD_MODE);
void HMC830_HMC_Write_Charge_Pump_Current(float Icp, uint8_t PFD_MODE, uint16_t fVCO, uint16_t fPFD);
void HMC830_HMC_Write_Output_Mode(uint8_t OUTPUT_MODE);

uint32_t HMC830_HMC_Read_Chip_ID(void);
uint8_t HMC830_HMC_Read_Lock_Detect(void);
uint16_t HMC830_HMC_Read_REFDIV(void);
double HMC830_HMC_Read_NDIV(void);

void HMC830_HMC_Write_Freq(double fREF, uint16_t REFDIV, double fOUT, float Icp);

void HMC830_HMC_Test_Dump_Register(uint32_t * dump);
void HMC830_HMC_Test_REF50M_35M(void);
void HMC830_HMC_Test_REF50M_100M(void);
void HMC830_HMC_Test_REF50M_425M(void);
void HMC830_HMC_Test_REF50M_650M(void);


#ifdef __cplusplus
}
#endif
#endif /* _ADF4360_H */
