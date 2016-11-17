#!/usr/bin/env python
# -*- coding: utf-8 -*-

#########################################################################
# Author: Zhaoting Weng
# Created Time: Sun 04 Sep 2016 07:55:33 PM CST
# Description:
#########################################################################

import struct
import math
import re
import numpy as np
import argparse

def audio_gen(fs, duration, audio_type, audio_format, audio_channel, freq, out_file):
    '''Generate audio file for the use of a certain sample rate.

    @fs:                this audio file is to be used as input for a certain sample rate(in Hz)
    @freq:              frequency of the generated audio(in Hz)
    @audio_type:        one of [pulse, sin, wnoise]
    @audio_format:      one of [S8 U8 S16_LE S16_BE U16_LE U16_BE S32_LE S32_BE U32_LE U32_BE]
    @audio_channel:     count of audio channels
    @duration:          length of audio(in second)'''

    content = []

    # - parse format -
    #   get 3 info:
    #      * little endiant or big endiant -> fmt_endiant
    #      * signed or unsigned -> peak, fmt_chr
    #      * sample depth -> peak, fmt_chr

    s_fmt_struct_dict = {'8': 'b', '16': 'h', '32': 'l'}
    us_fmt_struct_dict = {'8': 'B', '16': 'H', '32': 'L'}

    fmt_endiant = '<' if 'LE' in audio_format else '>'
    is_unsigned = 'U' in audio_format
    smp_depth = re.match(r'[^\d]*(\d+)[^\d]*', audio_format).group(1)

    if is_unsigned:
        peak = ((1 << int(smp_depth)) - 1)
        fmt_chr = us_fmt_struct_dict[smp_depth]
    else:
        peak = (1 << int(smp_depth)) / 2 - 1
        fmt_chr = s_fmt_struct_dict[smp_depth]

    # - parse audio type type -

    if audio_type == 'pulse':
        bottom = 0 if is_unsigned else (-peak)
        sample_per_period = fs / freq
        toggle = 1

        for sample_idx in range(duration * fs):
            if sample_idx % (sample_per_period/2) == 0:
                toggle = 0 if toggle else 1
            for _ in range(audio_channel):
                if toggle:
                    content.append(struct.pack('%s%s'%(fmt_endiant,fmt_chr ), peak))
                else:
                    content.append(struct.pack('%s%s'%(fmt_endiant,fmt_chr ), bottom))

    elif audio_type == 'sin':
        peak = (1 << int(smp_depth))/2 - 1  # re-assign `peak`
        phase = 0
        step = 2 * math.pi * freq / fs
        for sample_idx in range(int(duration * fs)):
            phase = (phase + step) % (2*math.pi)
            for _ in range(audio_channel):
                magnitude = peak * math.sin(phase)
                magnitude += (peak) if is_unsigned else 0  # lift magnitude by half of max value if format is unsigned
                content.append(struct.pack('%s%s'%(fmt_endiant, fmt_chr), magnitude))

    elif audio_type == "wnoise":
        signal = np.random.normal(size = duration * fs) # mean: 0.0, standard deviation: 1.0
        max_number = max(abs(signal))
        signal = [(i * peak) / max_number for i in signal]
        for s in signal:
            for _ in range(audio_channel):
                content.append(struct.pack('%s%s'%(fmt_endiant, fmt_chr), s))

    if not out_file:
        if audio_type == "wnoise":
            out_file = "Fs_%d_Channel_%d_Format_%s_Len_%d_Wave_%s.pcm"%(
                    int(fs), audio_channel, audio_format, int(duration), audio_type)
        else:
            out_file = "Fs_%d_Freq_%d_Channel_%d_Format_%s_Len_%d_Wave_%s.pcm"%(
                    int(fs), int(freq), audio_channel, audio_format, int(duration), audio_type)

    with open(out_file, 'w') as f:
        f.write(''.join(content))

if "__main__" == __name__:
    parser = argparse.ArgumentParser(prog = 'Audio Generator')
    parser.add_argument('-r', '--rate', dest='rate', default=8000, type=float, help='sample rate in Hz (default: 8000)')
    parser.add_argument('-d', '--duration', dest='duration', default=5, type=float, help = 'audio length in second (default: 5)')
    parser.add_argument('-t', '--type', dest='type', default='sin', help = 'one of [pulse, sin, wnoise] (default: sin)')
    parser.add_argument('-f', '--format', dest='format', default='S16_LE', help='sample format one of [S8 U8 S16_LE S16_BE U16_LE U16_BE S32_LE S32_BE U32_LE U32_BE] (default: S16_LE)')
    parser.add_argument('-c', '--channel', dest='channel', default=1, type=int ,help='sample channel count (default: 1)')
    parser.add_argument('--freq', dest='freq', default=1000,type=float, help='"sin" wave frequency in Hz (default: 1000)')
    parser.add_argument('-o', '--out', dest='out', default='', help='output filename')
    args = parser.parse_args()
    audio_gen(args.rate, args.duration, args.type, args.format, args.channel, args.freq, args.out)

    #audio_gen(16000, 10, 'sin', freq = 4000)
    #audio_gen(16000, 30, 'wnoise')
