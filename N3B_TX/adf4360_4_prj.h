#ifndef _ADF4360_4_PRJ_H
#define _ADF4360_4_PRJ_H



struct ADF4360_InitialSettings ADF4360_st = 
{
    10000000,   // refIn (Hz)
    
    /* Control Latch */
    2,   //prescaler 32/33
    0,   // powerDownMode;
    1, //7,   // currentSetting2;
    1, //7,   // currentSetting1;
    2, //0,   // outPowerLevel; //minimum power
    0,   // muteTillLd;
    1,   // cpGain;
    0,   // cpThreeState;
    1,   // phaseDetectorPolarity;
    1,   // muxControl; //was 0 , now digital lock detect
    0, //3,//2   // corePowerLevel; was 0
    
    /* N Counter Latch */
    0,  // divideBy2Select. Not available for ADF4360-8 and ADF4360-9
    0,  // divideBy2. Not available for ADF4360-8 and ADF4360-9
    
    /* R Counter Latch */
    0,  // lockDetectPrecision;
    0,  // antiBacklash;
};


struct ADF4360_Specifications ADF4360_part[10]=
{
    /* ADF4360-0 */
    {
        2400000000,   // vcoMinFreq (Hz)
        2725000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-1 */
    {
        2050000000,   // vcoMinFreq (Hz)
        2450000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-2 */
    {
        1850000000,   // vcoMinFreq (Hz)
        2150000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-3 */
    {
        1600000000,   // vcoMinFreq (Hz)
        1950000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-4 */
    {
        1450000000,   // vcoMinFreq (Hz)
        1750000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32            // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-5 */
    {
        1200000000,   // vcoMinFreq (Hz)
        1400000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-6 */
    {
        1050000000,   // vcoMinFreq (Hz)
        1250000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        32              // maxPrescalerVal (8/9, 16/17 and 32/33 are available)
    },
    
    /* ADF4360-7 */
    {
        350000000,    // vcoMinFreq (Hz)
        1800000000,   // vcoMaxFreq (Hz)
        300000000,    // countersMaxFreq (Hz)
        16              // maxPrescalerVal (8/9 and 16/17 are available)
    },
    
    /* ADF4360-8 */
    {
        65000000,     // vcoMinFreq (Hz)
        400000000,    // vcoMaxFreq (Hz)
        400000000,    // countersMaxFreq (Hz)
        1               // maxPrescalerVal (not available)
    },
    
    /* ADF4360-9 */
    {
        65000000,     // vcoMinFreq (Hz)
        400000000,    // vcoMaxFreq (Hz)
        400000000,    // countersMaxFreq (Hz)
        1               // maxPrescalerVal (not available)
    }
};

#endif /* _ADF4360_4_PRJ_H */