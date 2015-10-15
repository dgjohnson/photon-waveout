/* 
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PhotonWaveOut.h"

static WaveOut *current = NULL;

static void playback_handler(void) {
  int v = current->advance();
  // manual PWM, we map value to pulse-width
  if (v >= 0x80) {
    v = 0xFF-v;
    if (v > 0) {
      pinSetFast(current->pin_n);
      delayMicroseconds(v);
      pinResetFast(current->pin_n);
    }
  } else {
    if (v > 0) {
      pinSetFast(current->pin_p);
      delayMicroseconds(v);
      pinResetFast(current->pin_p);
    }
  }
}
    
bool WaveOut::play(char *wave, unsigned int wave_len, bool loop) {
  if (playing)
    stop();

  this->wave = wave;
  this->wave_len = wave_len;
  this->loop = loop;
  this->wave_ix = 0;
  current = this;
  // for 8000 Hz this should be 125, but we cheat a bit to be able to map the samples straight
  audio_clock->begin(playback_handler, 127, uSec);
  this->playing = true;
  return true;
}

void WaveOut::stop(void) {
  if (!playing)
    return;

  audio_clock->end();
  pinResetFast(pin_p);
  pinResetFast(pin_n);
  playing = false;
  current = NULL;
}

bool WaveOut::isPlaying(void) {
  return playing;
}

int WaveOut::advance(void) {
  int v = wave[wave_ix];
  wave_ix++;
  if (wave_ix >= wave_len) {
    if (loop) {
      wave_ix = 0;
    } else {
      stop();
    }
  }
  return v;
}

