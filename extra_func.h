#ifndef EXTRA_FUNC_H
#define EXTRA_FUNC_H

#include <stdint.h>
#include <math.h>
#include <stddef.h>

#define HAS_NOTE(mask)           (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x01)))
#define HAS_INSTRUMENT(mask)     (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x02)))
#define HAS_VOLUME(mask)         (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x04)))
#define HAS_EFFECT_TYPE(mask)    (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x08)))
#define HAS_EFFECT_PARAM(mask)   (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x10)))
#define IS_COMPRESSED_MODE(mask) ((mask) & 0x80)
#define IS_ORIGINAL_MODE(mask)   (!((mask) & 0x80))

#define hexToDecimalTens(num) (((num) >> 4) & 0x0F)
#define hexToDecimalOnes(num) ((num) & 0x0F)
#define hexToRow(num) (hexToDecimalTens(num) * 10 + hexToDecimalOnes(num))

#define PORTUP16(f, t) (f + ((f * t) >> 8))
#define PORTUP64(f, t) (f + ((f * t) >> 10))

#define PORTDOWN16(f, t) (f - ((f * t) >> 8))
#define PORTDOWN64(f, t) (f - ((f * t) >> 10))

void parse_vol_cmd(uint8_t vol_cmd, char* mnemonic, uint8_t* val);
size_t encode_dpcm_8bit(int8_t* pcm_data, int8_t* dpcm_data, size_t num_samples);
size_t encode_dpcm_16bit(int16_t* pcm_data, int16_t* dpcm_data, size_t num_samples);
void decode_dpcm_8bit(int8_t* dpcm_data, int8_t* pcm_data, size_t num_samples);
void decode_dpcm_16bit(int16_t* dpcm_data, int16_t* pcm_data, size_t num_samples);
float calc_sample_rate(int8_t rel_tone, int8_t fine_tune);
void xm_note_to_str(uint8_t note, char output[4]);
float noteToFrequency(float bassfreq, int note);
size_t bpmToTicksize(uint16_t bpm, uint32_t smp_rate);

extern const char* note_table[12];
extern const float C4_FREQ;
extern const float SEMITONE_RATIO;

#endif // EXTRA_FUNC_H
