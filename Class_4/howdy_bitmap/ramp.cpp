
#include "ramp.h"

void AudioRamp::update(void)
{
  audio_block_t *block;
  uint32_t i;

  block = allocate();
  if (block) {
    for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      if (mode == 1) {
        slope *= rise_rate;
      }
      if (mode == 2) {
        slope *= fall_rate;
      }
      if (slope > 32700) {
        slope = 32700;
        mode = 2;
      }
      if (slope < 1) {
        slope = 0;
        mode = 0;
      }

      block->data[i] = uint32_t(slope);
    }

    transmit(block);
    release(block);
  }

}
