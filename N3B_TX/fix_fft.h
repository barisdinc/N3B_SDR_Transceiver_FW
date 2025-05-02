#ifndef __FIX_FFT_H__
#define __FIX_FFT_H__

#define FFT_SIZE	1024				// Use this for buffer allocations
#define FFT_ORDER	10					// FFT_SIZE = 1 << FFT_ORDER

int fix_fft(int16_t *fr, int16_t *fi, bool inverse);

#endif
