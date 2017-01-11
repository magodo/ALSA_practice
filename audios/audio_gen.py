#!/usr/bin/env python
# -*- coding: utf-8 -*-

#########################################################################
# Author: Zhaoting Weng
# Created Time: Sun 04 Sep 2016 07:55:33 PM CST
# Description:
#########################################################################

import struct
import math
import numpy as np

def audio_gen(fs, audio_len, audio_type, audio_format = "S16_LE", audio_channel = 1, freq = 1000, out_file=""):
    '''Generate audio file for the use of a certain sample rate.

    @fs:                this audio file is to be used as input for a certain sample rate(in Hz)
    @freq:              frequency of the generated audio(in Hz)
    @audio_type:        one of [pulse, sin, wnoise]
    @audio_format:      one of [S8 U8 S16_LE S16_BE U16_LE U16_BE S32_LE S32_BE U32_LE U32_BE]
    @audio_channel:     count of audio channels
    @audio_len:         length of audio(in second)'''

    content = []

    # parse format
    is_LE = 'LE' in audio_format
    is_unsigned = 'U' in audio_format
    if '8' in audio_format:
        struct_type = 'c'
        peak = 1 << 8 - 1
    elif '16' in audio_format:
        struct_type = 'h'
        peak = 1 << 16 - 1
    elif '32' in audio_format:
        struct_type = 'l'
        peak = 1 << 32 - 1

    # parse audio type type
    if audio_type == 'pulse':
        if is_unsigned:
            peak = peak
            bottom = 0
        else:
            peak = (peak + 1) / 2 - 1
            bottom = -peak

        period = fs / freq
        toggle = 1

        for sample_idx in range(audio_len * fs):
            if sample_idx % (period/2) == 0:
                toggle = 0 if toggle else 1
            for _ in range(audio_channel):
                if toggle:
                    content.append(struct.pack('<%s'%struct_type, peak))
                else:
                    content.append(struct.pack('<%s'%struct_type, bottom))

    elif audio_type == 'sin':
        peak = (peak+1)/2 - 1
        phase = 0
        step = 2 * math.pi * freq / fs
        for sample_idx in range(audio_len * fs):
            phase = (phase + step) % (2*math.pi)
            for _ in range(audio_channel):
                magnitude = peak * math.sin(phase)
                magnitude += (peak + 1) if is_unsigned else 0
                content.append(struct.pack('<%s'%struct_type, magnitude))

    elif audio_type == "wnoise":
        signal = np.random.normal(size = audio_len * fs) # mean: 0.0, standard deviation: 1.0
        max_number = max(abs(signal))
        signal = [(i * peak / 2) / max_number for i in signal]
        if is_unsigned:
            signal = [i + peak / 2 for i in signal]
        for s in signal:
            for _ in range(audio_channel):
                content.append(struct.pack('<%s'%struct_type, s))

    if not out_file:
        if audio_type == "wnoise":
            out_file = "Fs_%d_Channel_%d_Format_%s_Len_%d_Wave_%s.pcm"%(
                    fs,audio_channel, audio_format, audio_len, audio_type)
        else:
            out_file = "Fs_%d_Freq_%d_Channel_%d_Format_%s_Len_%d_Wave_%s.pcm"%(
                    fs, freq, audio_channel, audio_format, audio_len, audio_type)

    with open(out_file, 'w') as f:
        f.write(''.join(content))

if "__main__" == __name__:
    #audio_gen(16000, 5, 'sin', 'S16_LE', 1)
    audio_gen(16000, 30, 'sin')

