﻿/*
 ---------------------------------------------------------------------
 Tunefish 4  -  http://tunefish-synth.com
 ---------------------------------------------------------------------
 This file is part of Tunefish.

 Tunefish is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Tunefish is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Tunefish.  If not, see <http://www.gnu.org/licenses/>.
 ---------------------------------------------------------------------
 */

#include "tf4.hpp"

// ------------------------------------------------------------------------------------
// HELPER FUNCTIONS
// ------------------------------------------------------------------------------------

eBool eTfSignalMix(eF32 **master, eF32 **in, eU32 length, eF32 volume)
{
    eF32 *signal1 = master[0];
    eF32 *signal2 = master[1];
    eF32 *mix1 = in[0];
    eF32 *mix2 = in[1];

    if (volume <= 0.5f)
    {
        volume *= 2.0f;
        volume *= volume;
    }
    else
    {
        volume -= 0.5f;
        volume *= 20.0f;
        volume += 1.0f;
    }

    eF32x2 const_vol = eSimdSetAll(volume);
    eF32 hasSignal = 0.0f;

    while(length--)
    {
        hasSignal += eAbs(*mix1);
        hasSignal += eAbs(*mix2);

        eF32x2 val = eSimdAdd(
            eSimdMul(
                eSimdSet2(*mix1++, *mix2++),
                const_vol
            ),
            eSimdSet2(*signal1, *signal2)
            );

        eSimdStore2(val, *signal1, *signal2);

        signal1++;
        signal2++;
    }

    return hasSignal > 1.0f;
}

void eTfSignalToS16(eF32 **sig, eS16 *out, const eF32 gain, eU32 length)
{
    eS16 *dest = out;
    eF32 *srcLeft = sig[0];
    eF32 *srcRight = sig[1];

    eF32x2 const_gain = eSimdSetAll(gain);
    eF32x2 const_min = eSimdSetAll(-32768.0f);
    eF32x2 const_max = eSimdSetAll(32767.0f);

    while(length--)
    {
        eF32x2 val = eSimdSet2(*srcLeft++, *srcRight++);
        val = eSimdMin(eSimdMax(eSimdMul(val, const_gain), const_min), const_max);
        eF32 store_val[2];
        eSimdStore2(val, store_val[0], store_val[1]);
        *dest++ = eFtoL(store_val[0]);
        *dest++ = eFtoL(store_val[1]);
    }
}

void eTfSignalToPeak(eF32 **sig, eF32 *peak_left, eF32 *peak_right, eU32 length)
{
    eF32 *srcLeft = sig[0];
    eF32 *srcRight = sig[1];

    eF32x2 peak = eSimdSetAll(0.0f);
    eF32x2 div = eSimdSetAll((eF32)length);

    while(length--)
    {
        eF32 left = *srcLeft++;
        eF32 right = *srcRight++;
        eU32 left_i = *((eU32*)&left);
        eU32 right_i = *((eU32*)&right);
        left_i &= 0x7fffffff;
        right_i &= 0x7fffffff;
        left = *((eF32*)&left_i);
        right = *((eF32*)&right_i);

        peak = eSimdAdd(peak, eSimdSet2(left, right));
    }

    peak = eSimdDiv(peak, div);

    eSimdStore2(peak, *peak_left, *peak_right);
}

// ------------------------------------------------------------------------------------
// ENVELOPE
// ------------------------------------------------------------------------------------

void eTfEnvelopeReset(eTfEnvelope &state)
{
    state.phase = eTfEnvelope::FINISHED;
    state.volume = 0.0;
}

eBool eTfEnvelopeIsEnd(eTfEnvelope &state)
{
    return state.phase == eTfEnvelope::FINISHED;
}

void eTfEnvelopeNoteOn(eTfEnvelope &state)
{
    state.phase = eTfEnvelope::ATTACK;
    state.volume = 0.0f;
}

void eTfEnvelopeNoteOff(eTfEnvelope &state)
{
    state.phase = eTfEnvelope::RELEASE;
}

eF32 eTfEnvelopeProcess(eTfSynth &synth, eTfInstrument &instr, eTfEnvelope &envState, eF32 decayMod, eU32 paramOffset, eU32 frameSize)
{
    eF32 a = instr.params[paramOffset];
    eF32 d = instr.params[paramOffset+1];
    eF32 s = instr.params[paramOffset+2];
    eF32 r = instr.params[paramOffset+3];
    eF32 slope = instr.params[paramOffset+4];

    eF32 scale = 0.00050f * frameSize * (synth.sampleRate / 44100.0f);
    eF32 attack = -eLog10(eMax(0.000000001f, ePow(a, 3.f)) * .94f) * scale;
    d = eMax(0.000000001f, ePow(d * decayMod, 3.f));
    eF32 decay = eLog10(d * .94f) * 0.25f * scale;
    eF32 sustain = eMin(s, 0.99f);
    eF32 release = eLog10(eMax(ePow(r, 3.f), 0.000000001f) * .94f) * 0.25f * scale;
    eF32 volume = envState.volume;

    switch (envState.phase)
    {
    case eTfEnvelope::ATTACK:
        volume += attack;
        if (volume >= 1.0f)
        {
            volume = 1.0f;
            envState.phase = eTfEnvelope::DECAY;
        }
        break;
    case eTfEnvelope::DECAY:
        {
            if (volume < sustain)
            {
                volume = sustain;
                envState.phase = eTfEnvelope::SUSTAIN;
            }
            else
            {
                eF32 diff = 0.01f + (volume - sustain);
                eF32 range = 1.0f - sustain;
                eF32 pos = diff / range;
                eF32 slope_f = ePow(pos, slope);
                volume += decay * slope_f;

                if (volume <= sustain)
                {
                    volume = sustain;
                    envState.phase = eTfEnvelope::SUSTAIN;
                }
            }
        }
        break;
    case eTfEnvelope::SUSTAIN:
        if (volume < sustain)
        {
            volume -= decay;
            if (volume > sustain)
                volume = sustain;
        }
        else if (volume > sustain)
        {
            volume += decay;
            if (volume < sustain)
                volume = sustain;
        }
        break;
    case eTfEnvelope::RELEASE:
        {
            eF32 slope_f = ePow(volume, slope);
            volume += release * slope_f;

            if (volume <= 0.00001f)
            {
                volume = 0.0f;
                envState.phase = eTfEnvelope::FINISHED;
            }
        }
        break;
    case eTfEnvelope::FINISHED:
        break;
    }

    /*
    if (volume != volume) // NaN
        volume = 0.0f;

    volume = eClamp<eF32>(0.0f, volume, 1.0f);
    */

    envState.volume = volume;
    return volume;
}

// ------------------------------------------------------------------------------------
// LFO
// ------------------------------------------------------------------------------------

void eTfLfoReset(eTfLfo &state, eF32 phase)
{
    state.phase = phase;
}

eF32 eTfLfoProcess(eTfSynth &synth, eTfInstrument &instr, eTfLfo &lfoState, eU32 paramOffset, eU32 frameSize)
{
    eF32 freq = instr.params[paramOffset];
    eF32 depth = instr.params[paramOffset+1];
    eF32 shape = instr.params[paramOffset+2];

    eF32 result = 1.0f;
    depth = depth * depth;
    freq = (freq * freq) / synth.sampleRate * frameSize * 50.0f;

#ifdef eCFG_NO_TF_LFO_SINE
	if (shape < 0.2f)       { }
#else
    if (shape < 0.2f)       result = ((eSin(lfoState.phase) + 1.0f) / 2.0f);
#endif
#ifdef eCFG_NO_TF_LFO_SAWDOWN
    else if (shape < 0.4f)  { }
#else
	else if (shape < 0.4f)  result = eMod(lfoState.phase, eTWOPI) / eTWOPI;
#endif
#ifdef eCFG_NO_TF_LFO_SAWUP
	else if (shape < 0.6f)  { }
#else
	else if (shape < 0.6f)  result = 1.0f - (eMod(lfoState.phase, eTWOPI) / eTWOPI);
#endif
#ifdef eCFG_NO_TF_LFO_PULSE
	else if (shape < 0.8f)  { }
#else
	else if (shape < 0.8f)  result = (lfoState.phase < ePI) ? 1.0f : 0.0f;
#endif
#ifdef eCFG_NO_TF_LFO_NOISE
	else                    { }
#else
	else                    result = synth.lfoNoiseTable[eFtoL(lfoState.phase / (ePI*2) * TF_LFONOISETABLESIZE)];
#endif

    result = (result * depth) + (1.0f - depth);

    lfoState.phase += freq;
    if (lfoState.phase > (ePI*2))
        lfoState.phase -= ePI*2;

    lfoState.result = result;
    return result;
}

eBool eTfModMatrixIsActive(eTfModMatrix &state)
{
    for (eU32 i=0; i<TF_MODMATRIXENTRIES; i++)
    {
        switch(state.entries[i].src)
        {
        case eTfModMatrix::INPUT_ADSR1: if (!eTfEnvelopeIsEnd(state.envState[0])) return eTRUE; break;
        case eTfModMatrix::INPUT_ADSR2: if (!eTfEnvelopeIsEnd(state.envState[1])) return eTRUE; break;
        default:
            break;
        }
    }

    return eFALSE;
}

eBool eTfModMatrixProcess(eTfSynth &synth, eTfInstrument &instr, eTfModMatrix &state, eU32 frameSize)
{
    eBool playing1 = eFALSE;
	eBool playing2 = eFALSE;

    eBool adsr1_done = eFALSE;
    eBool adsr2_done = eFALSE;
    eBool lfo1_done = eFALSE;
    eBool lfo2_done = eFALSE;

    for(eU32 i=0;i<TF_MODMATRIXENTRIES;i++)
    {
        eF32 mod = instr.params[TF_MM1_MOD + i*3];
        if (mod <= 0.5f)
        {
            mod = mod * 2.0f;
        }
        else
        {
            mod = (mod - 0.5f) * 2.0f;
            mod *= mod;
            mod = 1.0f + mod * (TF_MM_MODRANGE-1.0f);
        }

        state.entries[i].src   = (eTfModMatrix::Input)eFtoL(eRoundNearest(instr.params[TF_MM1_SOURCE + i*3] * (eTfModMatrix::INPUT_COUNT-1)));
        state.entries[i].dst   = (eTfModMatrix::Output)eFtoL(eRoundNearest(instr.params[TF_MM1_TARGET + i*3] * (eTfModMatrix::OUTPUT_COUNT-1)));
        state.entries[i].mod   = mod;
        state.entries[i].result= 1.0f;

        switch(state.entries[i].src)
        {
        case eTfModMatrix::INPUT_LFO1:
            if (!lfo1_done)    state.values[eTfModMatrix::INPUT_LFO1] = eTfLfoProcess(synth, instr, state.lfoState[0], TF_LFO1_RATE, frameSize);
            lfo1_done = eTRUE;
            state.entries[i].result = state.entries[i].mod * state.values[eTfModMatrix::INPUT_LFO1] * state.modulation[i];
            break;

        case eTfModMatrix::INPUT_LFO2:
            if (!lfo2_done) state.values[eTfModMatrix::INPUT_LFO2] = eTfLfoProcess(synth, instr, state.lfoState[1], TF_LFO2_RATE, frameSize);
            lfo2_done = eTRUE;
            state.entries[i].result = state.entries[i].mod * state.values[eTfModMatrix::INPUT_LFO2] * state.modulation[i];
            break;

        case eTfModMatrix::INPUT_ADSR1:
            if (!adsr1_done)
            {
                eF32 mmo_decay = eTfModMatrixGet(state, eTfModMatrix::OUTPUT_ADSR1_DECAY);
				playing1 = !eTfEnvelopeIsEnd(state.envState[0]);
                state.values[eTfModMatrix::INPUT_ADSR1] = eTfEnvelopeProcess(synth, instr, state.envState[0], mmo_decay, TF_ADSR1_ATTACK, frameSize);
                adsr1_done = eTRUE;
            }

            state.entries[i].result = state.entries[i].mod * state.values[eTfModMatrix::INPUT_ADSR1] * state.modulation[i];
            break;

        case eTfModMatrix::INPUT_ADSR2:
            if (!adsr2_done)
            {
                eF32 mmo_decay = eTfModMatrixGet(state, eTfModMatrix::OUTPUT_ADSR2_DECAY);
				playing2 = !eTfEnvelopeIsEnd(state.envState[1]);
                state.values[eTfModMatrix::INPUT_ADSR2] = eTfEnvelopeProcess(synth, instr, state.envState[1], mmo_decay, TF_ADSR2_ATTACK, frameSize);
                adsr2_done = eTRUE;
            }

            state.entries[i].result = state.entries[i].mod * state.values[eTfModMatrix::INPUT_ADSR2] * state.modulation[i];
            break;
        default:
            break;
        }
    }

    // determine values for self-modulation of mod matrix
    for(eU32 i=0;i<TF_MODMATRIXENTRIES;i++)
    {
        state.modulation[i] = eTfModMatrixGet(state, (eTfModMatrix::Output)(eTfModMatrix::OUTPUT_MOD1 + i));
    }

	return playing1 || playing2;
}

eF32 eTfModMatrixGet(eTfModMatrix &state, eTfModMatrix::Output output)
{
    eF32 value = 1.0f;

    for(eU32 i=0; i<TF_MODMATRIXENTRIES; i++)
    {
        if (state.entries[i].dst == output)
            value *= state.entries[i].result;
    }

    return value;
}

void eTfModMatrixReset(eTfModMatrix &state)
{
    for(eU32 i=0;i<TF_MODMATRIXENTRIES;i++)
        state.modulation[i] = 1.0f;
}

void eTfModMatrixNoteOn(eTfModMatrix &state, eF32 lfoPhase1, eF32 lfoPhase2)
{
    eTfEnvelopeNoteOn(state.envState[0]);
    eTfEnvelopeNoteOn(state.envState[1]);

    eTfLfoReset(state.lfoState[0], lfoPhase1);
    eTfLfoReset(state.lfoState[1], lfoPhase2);
}

void eTfModMatrixNoteOff(eTfModMatrix &state)
{
    eTfEnvelopeNoteOff(state.envState[0]);
    eTfEnvelopeNoteOff(state.envState[1]);
}

void eTfModMatrixPanic(eTfModMatrix &state)
{
    eTfEnvelopeNoteOff(state.envState[0]);
    eTfEnvelopeNoteOff(state.envState[1]);
}

// ------------------------------------------------------------------------------------
// GENERATOR
// ------------------------------------------------------------------------------------

void eTfGeneratorReset(eTfGenerator &state)
{
    eRandom rand;
    rand.seedRandomly();

    for(eU32 i=0; i<TF_MAXUNISONO; i++)
	{
        eF32 base = rand.nextFloat();
		eF32 off = rand.nextFloat()*0.1f;
        state.phase[i*2] = base;
		state.phase[i*2+1] = base+off;
	}

    state.modulation = rand.nextFloat(0.0f, 100.0f);
    state.freq1 = state.freq2 = 0.0f;
}

void eTfGeneratorNormalize(eF32 *buffer, eU32 frameSize)
{
    eF32 max = 0.0;
    eF32 *smp = buffer;
    eU32 len = frameSize;

	// normalize the signal
	// ------------------------------------------
    while(len--)
    {
        eF32 abs_smp = eAbs(*smp);
        if (abs_smp > max) max = abs_smp;
        smp += 2;
    }

    if (max<1e-5f) max=1e-5f;
    max = 1.0f/max;
    smp = buffer;
    len = frameSize;

	eF32 avg = 0.0f;
    while(len--)
    {
        *smp *= max;
		avg += *smp;
        smp += 2;
    }

	// center the signal
	// ------------------------------------------
	avg /= frameSize;
	smp = buffer;
	len = frameSize;

	while(len--)
	{
		*smp -= avg;
		smp += 2;
	}
}

void eTfGeneratorFft(eTfSynth &synth, eTfFftType type, eU32 frameSize, eF32 *fftBuffer)
{
    eInt sign = (eInt)type;
    eF32 wr, wi, arg, *p1, *p2, temp;
    eF32 tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
    eInt i, bitm, j, le, le2, k;
    eF32 fsign = (eF32)sign;
    eF32 sine, cosine;
    eInt count = eFtoL(eLog10((eF32)frameSize)/eLog10(2.0f));

    for (i = 2; i < (eInt)(2*frameSize-2); i += 2)
    {
        for (bitm = 2, j = 0; bitm < (eInt)(2*frameSize); bitm <<= 1)
        {
            if (i & bitm) j++;
            j <<= 1;
        }

        if (i < j)
        {
            p1 = fftBuffer+i; p2 = fftBuffer+j;
            temp = *p1; *(p1++) = *p2;
            *(p2++) = temp; temp = *p1;
            *p1 = *p2; *p2 = temp;
        }
    }
    for (k = 0, le = 2; k < count; k++)
    {
        le <<= 1;
        le2 = le>>1;
        ur = 1.0;
        ui = 0.0;
        arg = ePI / (le2>>1);

        eSinCos(arg, sine, cosine);
        wr = cosine;
        wi = fsign * sine;

        for (j = 0; j < le2; j += 2)
        {
            p1r = fftBuffer+j; p1i = p1r+1;
            p2r = p1r+le2; p2i = p2r+1;

            for (i = j; i < (eInt)(2*frameSize); i += le)
            {
                tr = *p2r * ur - *p2i * ui;
                ti = *p2r * ui + *p2i * ur;
                *p2r = *p1r - tr; *p2i = *p1i - ti;
                *p1r += tr; *p1i += ti;
                p1r += le; p1i += le;
                p2r += le; p2i += le;
            }

            tr = ur*wr - ui*wi;
            ui = ur*wi + ui*wr;
            ur = tr;
        }
    }
}

void eTfGeneratorUpdate(eTfSynth &synth, eTfInstrument &instr, eTfVoice &voice, eTfGenerator &generator, eF32 frequencyRange)
{
    eU32 frameSize = TF_IFFT_FRAMESIZE * 2;
    eU32 frameSizeHalf = frameSize / 2;
    eU32 genFrameSize = eFtoL(eF32(frameSizeHalf) * frequencyRange);

    if (genFrameSize < 4)
        genFrameSize = 4;

    eF32 harmonics  = instr.params[TF_GEN_NUMHARMONICS];
    eF32 bandwidth  = instr.params[TF_GEN_BANDWIDTH];
    eF32 scale      = instr.params[TF_GEN_SCALE]* 4.0f;
    eF32 damp       = instr.params[TF_GEN_DAMP];

    harmonics   *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_NUMHARMONICS);
    bandwidth   *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_BANDWIDTH);
    damp        *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_DAMP);
    scale       *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_SCALE);

    harmonics   = eClamp<eF32>(0.0f, harmonics, 1.0f);
    bandwidth   = eClamp<eF32>(0.0f, bandwidth, 1.0f);
    damp        = eClamp<eF32>(0.0f, damp, 1.0f);
    scale       = eClamp<eF32>(0.0f, scale, 4.0f);

    eU32 numHarmonics = 1+eMin((eU32)eFtoL(harmonics * TF_MAX_HARMONICS), TF_MAX_HARMONICS);

    if (generator.activeNumHarmonics != numHarmonics ||
        generator.activeGenSize != genFrameSize ||
        !eIsFloatZero(generator.activeDamp - damp) ||
        !eIsFloatZero(generator.activeScale - scale) ||
        !eIsFloatZero(generator.activeBandwidth - bandwidth))
    {
        eMemSet(generator.freqTable, 0, sizeof(eF32) * frameSize * 2);

        eF32 harmonicOffset[256];
        eF32 harmonicBandwidth[256];
        eF32 harmonicVolume[256];

        for (eU32 harmonicIndex=1; harmonicIndex < numHarmonics + 1; harmonicIndex++)
        {
            eF32 invHarmonicFrequency = (1.0f / TF_IFFT_FRAMESIZE) * harmonicIndex;
            harmonicOffset[harmonicIndex-1] = (((invHarmonicFrequency * frameSize) - 1.0f) * scale) + 1.0f;
            harmonicBandwidth[harmonicIndex-1] = 0.3f + (bandwidth * harmonicIndex);
            harmonicVolume[harmonicIndex-1] = 1.0f / ePow((eF32)harmonicIndex, 1.0f + damp);
        }

        eF32 *writePtr = generator.freqTable;
        for (eU32 i=0; i<genFrameSize; i++)
        {
            eF32 amp = 0.0f;

            eF32 *offsetPtr = harmonicOffset;
            eF32 *bandwidthPtr = harmonicBandwidth;
            eF32 *volumePtr = harmonicVolume;

            for (eU32 harmonicIndex=0; harmonicIndex < numHarmonics; harmonicIndex++)
            {
                eF32 dist = eAbs(*offsetPtr - (eF32)i);
                dist /= *bandwidthPtr;

                if (dist < 5.0f)
                {
                    eU32 expLookup = eFtoL(dist / 5.0f * (TF_MAXFRAMESIZE-1));
                    eF32 exp = synth.expBuffer[expLookup];
                    exp *= *volumePtr;
                    amp += exp;
                }

                offsetPtr++;
                bandwidthPtr++;
                volumePtr++;
            }

            *writePtr++ = amp;
            *writePtr++ = amp;
        }

        generator.freqTable[0] = 1.0f;
        generator.freqTable[1] = 0.0f;

        generator.activeBandwidth = bandwidth;
        generator.activeDamp = damp;
        generator.activeScale = scale;
        generator.activeNumHarmonics = numHarmonics;
        generator.activeGenSize = genFrameSize;
    }
}

eBool eTfGeneratorModulate(eTfSynth &synth, eTfInstrument &instr, eTfVoice &voice, eTfGenerator &generator)
{
    if (eIsFloatZero(generator.modulation))
        return eFALSE;

    eU32 frameSize = TF_IFFT_FRAMESIZE * 2;
    eU32 frameSizeHalf = frameSize / 2;

    eF32 random = instr.params[TF_GEN_MODULATION];
    eF32 modulation = ePow(random, 3);

    eF32 *readPtr = generator.freqTable;
    eF32 *writePtr = generator.freqModTable;
    eF32 *randPtr = synth.randomBuffer;

    for (eU32 i=0; i<frameSizeHalf; i++)
    {
        eF32 modulationStrength = (eF32)i / (eF32)frameSizeHalf;

        eF32 sineOffset = 0.0f;
        if (instr.params[TF_GEN_MODULATION])
            sineOffset = (generator.modulation * TF_FRAMESIZE) * (1.0f - (random * *randPtr++));

        eU32 sinLookup = eFtoL(sineOffset * modulationStrength) % TF_FRAMESIZE;
        eU32 cosLookup = (sinLookup + TF_FRAMESIZE/4) % TF_FRAMESIZE;

        *writePtr++ = *readPtr++ * synth.sinBuffer[sinLookup];
        *writePtr++ = *readPtr++ * synth.sinBuffer[cosLookup];
    }

    generator.freqModTable[0] = 1.0f;
    generator.freqModTable[1] = 0.0f;

    generator.modulation += modulation / 100.0f;

    return eTRUE;
}

eBool eTfGeneratorProcess(eTfSynth &synth, eTfInstrument &instr, eTfVoice &voice, eTfGenerator &generator, eF32 velocity, eF32 **signal, eU32 frameSize)
{
    eF32 vol = instr.params[TF_GEN_VOLUME] * 4.0f * velocity;
    eF32 maxCurrentVolume = eMax(voice.lastVolL, voice.lastVolR);
    eF32 maxFrameVolume = eMax(maxCurrentVolume, vol);

    if (maxFrameVolume > 0.0f)
    {
        eF32 freq       = instr.params[TF_GEN_FREQ];
        eF32 panning    = instr.params[TF_GEN_PANNING];
        eF32 drive      = instr.params[TF_GEN_DRIVE];
        eF32 detune     = instr.params[TF_GEN_DETUNE];
        eF32 octave     = instr.params[TF_GEN_OCTAVE];
        eF32 spread     = instr.params[TF_GEN_SPREAD];
        eU32 unisono    = eFtoL(eRoundNearest(instr.params[TF_GEN_UNISONO] * (TF_MAXUNISONO-1))) + 1;
        eBool notefreq  = freq < 0.00001f;

        // process modulation matrix
        // -------------------------------------------------
        vol     *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_VOLUME);
        detune  *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_DETUNE);
        drive   *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_DRIVE);
        spread  *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_SPREAD);
        eF32 freqMod = eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_FREQ);

        maxFrameVolume = eMax(maxCurrentVolume, vol);
        if (maxFrameVolume < 0.001f)
            return eFALSE;

        // scale some of the values
        // -------------------------------------------------
        detune  = ePow(detune, 3.0f) * 10.0f;
        drive   *= 32.0f;
        drive   += 1.0f;
        freq    = ePow(freq, 2.0f) * 1000.0f;
        spread  = ePow(spread, 4.0f) / (eF32)synth.sampleRate * 10.0f;

        // calculate octave multiplicator
        // -------------------------------------------------
        eU32 ioctave = eFtoL(octave * (TF_MAXOCTAVES-1));
        eF32 octave_mul = TF_OCTAVES[ioctave];

        // calculate final frequency
        // -------------------------------------------------
        generator.freq1 = voice.currentFreq * octave_mul + detune;
        generator.freq2 = voice.currentFreq * octave_mul - detune;

        if (!notefreq)
        {
            generator.freq1 = freq;
            generator.freq2 = freq;
        }

        generator.freq1 = generator.freq1 * freqMod / synth.sampleRate;
        generator.freq2 = generator.freq2 * freqMod / synth.sampleRate;

        // calculate volume & panning
        // -------------------------------------------------
        vol *= 0.1f;
        eF32 vol_left = panning <= 0.5f ? vol : vol * (1.0f - ((panning-0.5f)*2.0f));
        eF32 vol_right = panning >= 0.5f ? vol : vol * (panning*2.0f);

        // calculate signal
        // -------------------------------------------------
        eF32x2 mdrive = eSimdSetAll(drive);
        eF32x2 mmin = eSimdSetAll(-1.0f);
        eF32x2 mmax = eSimdSetAll(1.0f);

        for (eU32 j=0; j<unisono; j++)
        {
            eF32 *sig1 = signal[0];
            eF32 *sig2 = signal[1];
            eF32 *phase1 = &generator.phase[j*2];
            eF32 *phase2 = &generator.phase[j*2+1];

			eF32x2 mvol = eSimdSet2(voice.lastVolL, voice.lastVolR);
			eF32x2 mvol_step = eSimdSet2((vol_left - voice.lastVolL) / frameSize,
				                         (vol_right - voice.lastVolR) / frameSize);

            // make sure we do not get in negative value (for example with LFO modulation) or we crash
            if (generator.freq1 < 0.0f) generator.freq1 *= -1.0f;
            if (generator.freq2 < 0.0f) generator.freq2 *= -1.0f;

            eU32 len = frameSize;
            while(len--)
            {
                eU32 off1 = eFtoL(*phase1 * (TF_IFFT_FRAMESIZE-1)) * 2;
                eU32 off2 = eFtoL(*phase2 * (TF_IFFT_FRAMESIZE-1)) * 2;

                eF32 val1 = generator.resultTable[off1];
                eF32 val2 = generator.resultTable[off2];

                eF32x2 mval = eSimdMul(
                    eSimdMax(
                        eSimdMin(
                            eSimdMul(
                                eSimdSet2(val1, val2),
                                mdrive),
                            mmax),
                        mmin),
                    mvol);

                eF32 store_l, store_r;
                eSimdStore2(mval, store_l, store_r);
                *sig1++ += store_l;
                *sig2++ += store_r;

                *phase1 += generator.freq1;
                while (*phase1 > 1.0f) { *phase1 -= 1.0f; }

                *phase2 += generator.freq2;
                while (*phase2 > 1.0f) { *phase2 -= 1.0f; }

				mvol = eSimdAdd(mvol, mvol_step);
            }

            generator.freq1 += spread;
            generator.freq2 -= spread;
        }

		voice.lastVolL = vol_left;
		voice.lastVolR = vol_right;

        return eTRUE;
    }

    return eFALSE;
}

// ------------------------------------------------------------------------------------
// NOISE
// ------------------------------------------------------------------------------------

void eTfNoiseReset(eTfNoise &state)
{
    eRandom rand;
    rand.seedRandomly();

    state.offset1 = rand.nextInt(0, TF_NOISETABLESIZE/2);
    state.offset2 = rand.nextInt(0, TF_NOISETABLESIZE/2);
    state.filterOn = eFALSE;
    state.amount = 0.0f;
}

void eTfNoiseUpdate(eTfSynth &synth, eTfInstrument &instr, eTfNoise &state, eTfModMatrix &modMatrix, eF32 velocity)
{
    eF32 bw = instr.params[TF_NOISE_BW];
    eF32 noise = instr.params[TF_NOISE_AMOUNT];

    state.filterOn = bw < 0.99f;
    state.amount = noise * velocity * eTfModMatrixGet(modMatrix, eTfModMatrix::OUTPUT_NOISE_AMOUNT);

    if (state.filterOn && state.amount > 0.0f)
    {
        eF32 f = instr.params[TF_NOISE_FREQ];
        eTfFilterUpdate(synth, *state.filterHP, f - bw, 0.05f, eTfFilter::FILTER_HP);
        eTfFilterUpdate(synth, *state.filterLP, f + bw, 0.05f, eTfFilter::FILTER_LP);
    }
}

eBool eTfNoiseProcess(eTfSynth &synth, eTfInstrument &instr, eTfNoise &state, eF32 **signal, eU32 frameSize)
{
    eF32 *signal1 = signal[0];
    eF32 *signal2 = signal[1];

    if (state.amount > 0.01f)
    {
        eU32 len = frameSize;
        while(len--)
        {
            *signal1++ = synth.whiteNoiseTable[state.offset1++] * state.amount;
            *signal2++ = synth.whiteNoiseTable[state.offset2++] * state.amount;

            if (state.offset1 >= TF_NOISETABLESIZE) state.offset1 = 0;
            if (state.offset2 >= TF_NOISETABLESIZE) state.offset2 = 0;
        }

        if (state.filterOn)
        {
            eTfFilterProcess(*state.filterLP, eTfFilter::FILTER_LP, signal, frameSize);
            eTfFilterProcess(*state.filterHP, eTfFilter::FILTER_HP, signal, frameSize);
        }

        return eTRUE;
    }
    else
    {
        eMemSet(signal1, 0, sizeof(eF32) * frameSize);
        eMemSet(signal2, 0, sizeof(eF32) * frameSize);

        return eFALSE;
    }
}

// ------------------------------------------------------------------------------------
// FILTER
// ------------------------------------------------------------------------------------

void eTfFilterUpdate(eTfSynth &synth, eTfFilter &state, eF32 f, eF32 q, eTfFilter::Type type)
{
    f = eClamp<eF32>(0.0f, f, 1.0f);
    q = eClamp<eF32>(0.0f, q, 0.85f);

    if (type == eTfFilter::FILTER_LP)
    {
        f = f * f * 20000.0f + 30.0f;
        f = 2.0f * f / synth.sampleRate; //[0 - 1]
        state.k = 3.6f*f - 1.6f*f*f -1.0f; //(Empirical tunning)
        state.p = (state.k+1.0f)*0.5f;
        eF32 scale = ePow(eEXPONE, ((1.0f-state.p)*1.386249f));
        state.r = q*scale;
    }
    else if (type == eTfFilter::FILTER_NT)
    {
        f = f * f * 20000.0f + 30.0f;

        eF32 z1x = eCos(2.0f * ePI * f / synth.sampleRate);

        state.b0 = (1.0f-q)*(1.0f-q)/(2.0f*(eAbs(z1x)+1.0f)) + q;
        state.b2 = state.b0;
        state.b1 = -2.0f * z1x * state.b0;
        state.a1 = -2.0f * z1x * q;
        state.a2 = q*q;
    }
    else
    {
        f = f * f * 10000.0f + 30.0f;
        eF32 A = 1.059253725f;
        eF32 w0 = 2.0f * ePI * f / synth.sampleRate;

        const eF32 cos_w0 = eCos(w0);
        const eF32 sin_w0 = eSin(w0);

        eF32 alpha = sin_w0 * eSinH( eLog10(2.0f)/2.0f * (1.0f - q) * w0/sin_w0 );

        switch(type)
        {
            case eTfFilter::FILTER_HP:
                state.b0 =  (1.0f + cos_w0)/2.0f;
                state.b1 = -(1.0f + cos_w0);
                state.b2 =  state.b0;
                break;

            case eTfFilter::FILTER_BP:
                state.b0 =  sin_w0 / 2.0f;
                state.b1 =  0.0f;
                state.b2 = -state.b0;
                break;

            /*case eTfFilter::FILTER_NT:
                state.b0 =  1.0f;
                state.b1 = -2.0f * cos_w0;
                state.b2 =  1.0f;
                break;*/
        }

        state.a0 =   1.0f + alpha;
        state.a1 =  -2.0f * cos_w0;
        state.a2 =   1.0f - alpha;

        state.b0 /= state.a0;
        state.b1 /= state.a0;
        state.b2 /= state.a0;
        state.a1 /= state.a0;
        state.a2 /= state.a0;
    }
}

void eTfFilterProcess(eTfFilter &state, eTfFilter::Type type, eF32 **signal, eU32 frameSize)
{
    eASSERT_ALIGNED16(&state);

    eF32 *signal1 = signal[0];
    eF32 *signal2 = signal[1];
    eU32 len = frameSize;

    if (type == eTfFilter::FILTER_LP)
    {
        eF32x2 p = eSimdSetAll(state.p);
        eF32x2 r = eSimdSetAll(state.r);
        eF32x2 k = eSimdSetAll(state.k);

        eF32x2 x;
        eF32x2 const_6 = eSimdSetAll(1.0f / 6.0f);

        while (len--)
        {
            eF32x2 in = eSimdSet2((eF32)*signal1, (eF32)*signal2);

            // x = in - r * y4
            x = eSimdNfma(in, r, state.y4);

            // state->y1_l=x_l*state->p + state->oldx_l*state->p - state->k*state->y1_l;
            state.y1 = eSimdNfma(eSimdFma(eSimdMul(state.oldx, p), x, p), k, state.y1);

            //state->y2_l=state->y1_l*state->p+state->oldy1_l*state->p - state->k*state->y2_l;
            state.y2 = eSimdNfma(eSimdFma(eSimdMul(state.oldy1, p), state.y1, p), k, state.y2);

            //state->y3_l=state->y2_l*state->p+state->oldy2_l*state->p - state->k*state->y3_l;
            state.y3 = eSimdNfma(eSimdFma(eSimdMul(state.oldy2, p), state.y2, p), k, state.y3);

            //state->y4_l=state->y3_l*state->p+state->oldy3_l*state->p - state->k*state->y4_l;
            state.y4 = eSimdNfma(eSimdFma(eSimdMul(state.oldy3, p), state.y3, p), k, state.y4);

            //out_l = state->y4_l - (state->y4_l*state->y4_l*state->y4_l)/6;
            eF32x2 out = eSimdNfma(state.y4, eSimdMul(eSimdMul(state.y4, state.y4), state.y4), const_6);

            state.oldx = x;
            state.oldy1 = state.y1;
            state.oldy2 = state.y2;
            state.oldy3 = state.y3;

            eSimdStore2(out, *signal1, *signal2);
            signal1++;
            signal2++;
        }

        state.y1 = state.oldy1;
        state.y2 = state.oldy2;
        state.y3 = state.oldy3;
    }
    else if (type == eTfFilter::FILTER_NT)
    {
        eF32x2 b0 = eSimdSetAll(state.b0);
        eF32x2 b1 = eSimdSetAll(state.b1);
        eF32x2 b2 = eSimdSetAll(state.b2);
        eF32x2 a1 = eSimdSetAll(state.a1);
        eF32x2 a2 = eSimdSetAll(state.a2);

        while(len--)
        {
            eF32x2 out  = eSimdSub(
                            eSimdSub(
                                eSimdAdd(
                                    eSimdAdd(
                                        eSimdMul(b0, state.in0),
                                        eSimdMul(b1, state.in1)),
                                    eSimdMul(b2, state.in2)),
                                eSimdMul(a1, state.out1)),
                            eSimdMul(a2, state.out2));

            state.out2 = state.out1;
            state.out1 = out;
            state.in2 = state.in1;
            state.in1 = state.in0;

            state.in0 = eSimdSet2(*signal1, *signal2);

            eSimdStore2(out, *signal1, *signal2);

            signal1++;
            signal2++;
        }
    }
    else
    {
        eF32x2 b0 = eSimdSetAll(state.b0);
        eF32x2 b1 = eSimdSetAll(state.b1);
        eF32x2 b2 = eSimdSetAll(state.b2);
        eF32x2 a1 = eSimdSetAll(state.a1);
        eF32x2 a2 = eSimdSetAll(state.a2);

        while(len--)
        {
            eF32x2 in = eSimdSet2(*signal1, *signal2);

            eF32x2 out = eSimdNfma(
                            eSimdNfma(
                                eSimdFma(
                                    eSimdFma(
                                        eSimdMul(b0, in),
                                        b1, state.in1
                                        ),
                                    b2, state.in2
                                    ),
                                a1, state.out1
                                ),
                            a2, state.out2
                         );

            eSimdStore2(out, *signal1, *signal2);

            signal1++;
            signal2++;

            state.in2 = state.in1;
            state.in1 = in;
            state.out2 = state.out1;
            state.out1 = out;
        }
    }
}

// ------------------------------------------------------------------------------------
// VOICE
// ------------------------------------------------------------------------------------

void eTfVoiceReset(eTfVoice &state)
{
    state.noteIsOn = eFALSE;
    state.playing = eFALSE;
    eTfModMatrixReset(state.modMatrix);
    eTfGeneratorReset(state.generator);
    eTfNoiseReset(state.noiseGen);
}

void eTfVoiceNoteOn(eTfVoice &state, eS32 note, eS32 velocity, eF32 lfoPhase1, eF32 lfoPhase2)
{
    eRandom rand;
    rand.seedRandomly();

    state.currentNote = note;
    state.currentVelocity = velocity;
    state.currentSlop = rand.nextFloat(-1.0f, 1.0f);
    state.noteIsOn = eTRUE;
    state.time = 0;
	state.lastVolL = 0.0f;
	state.lastVolR = 0.0f;

    eTfModMatrixNoteOn(state.modMatrix, lfoPhase1, lfoPhase2);
    eTfGeneratorReset(state.generator);
    eTfNoiseReset(state.noiseGen);
}

void eTfVoiceNoteOff(eTfVoice &state)
{
    state.noteIsOn = eFALSE;
    eTfModMatrixNoteOff(state.modMatrix);
}

void eTfVoicePitchBend(eTfVoice &state, eF32 semitones, eF32 cents)
{
    state.pitchBendSemitones = semitones;
    state.pitchBendCents = cents;
}

void eTfVoicePanic(eTfVoice &state)
{
    state.noteIsOn = eFALSE;
    state.playing = eFALSE;
    eTfModMatrixPanic(state.modMatrix);
}

// ------------------------------------------------------------------------------------
// INSTRUMENT
// ------------------------------------------------------------------------------------

void eTfInstrumentInit(eTfSynth &synth, eTfInstrument &instr)
{
    instr.lfo1Phase = instr.lfo2Phase = 0.0f;

    for(eU32 i=0; i<TF_MAXEFFECTS; i++)
    {
        instr.effects[i] = nullptr;
        instr.effectIndex[i] = 0;
    }

    for(eU32 i=0; i<TF_MAXVOICES; i++)
        eTfVoiceReset(instr.voice[i]);
}

eF32 eTfInstrumentProcess(eTfSynth &synth, eTfInstrument &instr, eF32 **outputs, long frameSize)
{
    eSimdSetArithmeticFlags(eSAF_FTZ);
    eASSERT(frameSize <= TF_MAXFRAMESIZE);

    eF32 *tempBuffers[2];
    tempBuffers[0] = instr.tempBuffers[0];
    tempBuffers[1] = instr.tempBuffers[1];

    for(eU32 k=0;k<TF_MAXVOICES;k++)
    {
        eTfVoice &voice = instr.voice[k];

        if (voice.noteIsOn || voice.playing)
        {
            instr.effectsInactiveTime = 0.0f;
            voice.time++;

            //  RUN MOD MATRIX
            // -------------------------------------------------------------------------------
            eBool has_mm_active = eTfModMatrixProcess(synth, instr, voice.modMatrix, frameSize);
            instr.lfo1Phase = voice.modMatrix.lfoState[0].phase;
            instr.lfo2Phase = voice.modMatrix.lfoState[1].phase;

            //  CALCULATE VELOCITY
            // -------------------------------------------------------------------------------
            eF32 velocity = (eF32)voice.currentVelocity / 128.0f;
            if (!has_mm_active && !voice.noteIsOn)
            {
                velocity = 0.0f;
            }

            //  RUN NOISE GEN
            // -------------------------------------------------------------------------------
            eTfNoiseUpdate(synth, instr, voice.noiseGen, voice.modMatrix, velocity);
            eTfNoiseProcess(synth, instr, voice.noiseGen, tempBuffers, frameSize);

            //  CALCULATE FREQUENCY
            // -------------------------------------------------------------------------------
            eF32 baseFreq = synth.freqTable[voice.currentNote & 0x7f];
            eF32 prevFreq = baseFreq;
            eF32 nextFreq = baseFreq;

            // Pitch wheel calculation
            // -------------------------------------------------------------------------------
            eU32 semiTonesUp = eFtoL(instr.params[TF_PITCHWHEEL_UP] * 23.0f + 1.0f);
            eU32 semiTonesDown = eFtoL(instr.params[TF_PITCHWHEEL_DOWN] * 23.0f + 1.0f);
            while (semiTonesUp--) { nextFreq *= TF_12TH_ROOT_OF_2; }
            while (semiTonesDown--) { prevFreq *= (1.0f / TF_12TH_ROOT_OF_2); }
            baseFreq = eLerp(prevFreq, baseFreq, eClamp<eF32>(0.0f, voice.pitchBendSemitones + 1.0f, 1.0f));
            baseFreq = eLerp(baseFreq, nextFreq, eClamp<eF32>(0.0f, voice.pitchBendSemitones, 1.0f));

            // SLOP CALCULATION
            // -------------------------------------------------------------------------------
            eF32 slop = ePow(instr.params[TF_GEN_SLOP], 3);
            baseFreq += voice.currentSlop * slop * 8.0f;

            // GLIDE CALCULATION
            // -------------------------------------------------------------------------------
            eF32 glide = instr.params[TF_GEN_GLIDE];
            if (glide > 0.0f && voice.currentFreq > 0.0f)
            {
                eF32 freqDiff = baseFreq - voice.currentFreq;
                freqDiff /= glide * 10.0f + 1.0f;
                voice.currentFreq += freqDiff;
            }
            else
                voice.currentFreq = baseFreq;

            //  RUN GENERATOR
            // -------------------------------------------------------------------------------
            if (voice.time % 4 == 1) // reduce cpu hit a bit. recalculate not more than every 4th frame
            {
                eF32 freqRange = eClamp<eF32>(0.0f, (voice.currentFreq-8.0f) / 2000.0f, 1.0f);
                eF32 invFreqRange = 1.0f - freqRange;
                invFreqRange = ePow(invFreqRange, 3.0f);
                eTfGeneratorUpdate(synth, instr, voice, voice.generator, invFreqRange);

                if (eTfGeneratorModulate(synth, instr, voice, voice.generator))
                    eMemCopy(voice.generator.resultTable, voice.generator.freqModTable, TF_IFFT_FRAMESIZE * sizeof(eF32) * 2);
                else
                    eMemCopy(voice.generator.resultTable, voice.generator.freqTable, TF_IFFT_FRAMESIZE * sizeof(eF32) * 2);

                eTfGeneratorFft(synth, IFFT, TF_IFFT_FRAMESIZE, voice.generator.resultTable);
                eTfGeneratorNormalize(voice.generator.resultTable, TF_IFFT_FRAMESIZE);
            }

            eTfGeneratorProcess(synth, instr, voice, voice.generator, velocity, tempBuffers, frameSize);

            //  RUN VOICE SYNTHESIS
            // -------------------------------------------------------------------------------
            //eTfVocSynGenerate(synth, *synth.vocSyn, voice.vocState, (voice.time / 100) % 5, velocity, tempBuffers, frameSize);

            //  RUN LOWPASS FILTER
            // -------------------------------------------------------------------------------
            if (instr.params[TF_LP_FILTER_ON] > 0.5f)
            {
                eF32 lpCutoff = instr.params[TF_LP_FILTER_CUTOFF];
                eF32 lpResonance = instr.params[TF_LP_FILTER_RESONANCE];

                lpCutoff *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_LP_FILTER_CUTOFF);
                lpResonance *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_LP_FILTER_RESONANCE);

                eTfFilterUpdate(synth, *voice.filterLP, lpCutoff, lpResonance, eTfFilter::FILTER_LP);
                eTfFilterProcess(*voice.filterLP, eTfFilter::FILTER_LP, tempBuffers, frameSize);
            }

            //  RUN HIGHPASS FILTER
            // -------------------------------------------------------------------------------
            if (instr.params[TF_HP_FILTER_ON] > 0.5f)
            {
                eF32 hpCutoff = instr.params[TF_HP_FILTER_CUTOFF];
                eF32 hpResonance = instr.params[TF_HP_FILTER_RESONANCE];

                hpCutoff *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_HP_FILTER_CUTOFF);
                hpResonance *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_HP_FILTER_RESONANCE);

                eTfFilterUpdate(synth, *voice.filterHP, hpCutoff, hpResonance, eTfFilter::FILTER_HP);
                eTfFilterProcess(*voice.filterHP, eTfFilter::FILTER_HP, tempBuffers, frameSize);
            }

            //  RUN BANDPASS FILTER
            // -------------------------------------------------------------------------------
            if (instr.params[TF_BP_FILTER_ON] > 0.5f)
            {
                eF32 bpCutoff = instr.params[TF_BP_FILTER_CUTOFF];
                eF32 bpQ = instr.params[TF_BP_FILTER_Q];

                bpCutoff *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_BP_FILTER_CUTOFF);
                bpQ *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_BP_FILTER_Q);

                eTfFilterUpdate(synth, *voice.filterBP, bpCutoff, bpQ, eTfFilter::FILTER_BP);
                eTfFilterProcess(*voice.filterBP, eTfFilter::FILTER_BP, tempBuffers, frameSize);
            }

            //  RUN NOTCH FILTER
            // -------------------------------------------------------------------------------
            if (instr.params[TF_NT_FILTER_ON] > 0.5f)
            {
                eF32 ntCutoff = instr.params[TF_NT_FILTER_CUTOFF];
                eF32 ntQ = instr.params[TF_NT_FILTER_Q];

                ntCutoff *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_NT_FILTER_CUTOFF);
                ntQ *= eTfModMatrixGet(voice.modMatrix, eTfModMatrix::OUTPUT_NT_FILTER_Q);

                eTfFilterUpdate(synth, *voice.filterNT, ntCutoff, ntQ, eTfFilter::FILTER_NT);
                eTfFilterProcess(*voice.filterNT, eTfFilter::FILTER_NT, tempBuffers, frameSize);
            }

            // MIX SIGNAL
            // ------------------------------------------------------------------------------
            eF32 gain = instr.params[TF_GLOBAL_GAIN];
            voice.playing = eTfSignalMix(outputs, tempBuffers, frameSize, gain);
        }
    }

    //    RUN EFFECTS
    // ------------------------------------------------------------------------------
    if (instr.effectsInactiveTime < TF_EFFECT_SWITCHOFF_TIME)
    {
        for(eU32 i=0;i<TF_MAXEFFECTS;i++)
        {
            eTfEffect *fx = instr.effects[i];

            eU32 oldFxIndex = instr.effectIndex[i];
            eF32 fxVal = instr.params[TF_EFFECT_1 + i];
            eU32 fxIndex = eFtoL(eRoundNearest(fxVal * (FX_COUNT-1)));

            if (fxIndex != oldFxIndex && oldFxIndex != 0)
            {
                s_effectDelete[oldFxIndex](fx);
                instr.effects[i] = fx = nullptr;
                instr.effectIndex[i] = 0;
            }

            if (fxIndex != 0 && fx == nullptr)
            {
				if (s_effectCreate[fxIndex]) {
					instr.effects[i] = fx = s_effectCreate[fxIndex]();
					instr.effectIndex[i] = fxIndex;
				}
            }

            if (fx != nullptr)
                s_effectProcess[fxIndex](fx, synth, instr, outputs, frameSize);
        }
    }

    eF32 peak_left = 0.0f;
    eF32 peak_right = 0.0f;
    eTfSignalToPeak(outputs, &peak_left, &peak_right, frameSize);
    eF32 peak = (peak_left + peak_right) / 2.0f;

    if (eIsFloatZero(peak))
        instr.effectsInactiveTime += (eF32)frameSize / synth.sampleRate;

    return peak;
}

void eTfInstrumentNoteOn(eTfInstrument &instr, eS32 note, eS32 velocity)
{
    eF32 lfoPhase1 = 0.0f;
    eF32 lfoPhase2 = 0.0f;

    eU32 voice = eTfInstrumentAllocateVoice(instr);

    if (instr.params[TF_LFO1_SYNC] < 0.5f)
        lfoPhase1 = instr.lfo1Phase;

    if (instr.params[TF_LFO2_SYNC] < 0.5f)
        lfoPhase2 = instr.lfo2Phase;

    eTfVoiceNoteOn(instr.voice[voice], note, velocity, lfoPhase1, lfoPhase2);
    instr.latestTriggeredVoice = &instr.voice[voice];
}

eBool eTfInstrumentNoteOff(eTfInstrument &instr, eS32 note)
{
	eBool killed = eFALSE;

    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (instr.voice[i].currentNote == note && instr.voice[i].noteIsOn)
		{
            eTfVoiceNoteOff(instr.voice[i]);
			killed = eTRUE;
		}
    }

	return killed;
}

void eTfInstrumentAllNotesOff(eTfInstrument &instr)
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (instr.voice[i].noteIsOn)
            eTfVoiceNoteOff(instr.voice[i]);
    }
}

void eTfInstrumentPitchBend(eTfInstrument &instr, eF32 semitones, eF32 cents)
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        eTfVoicePitchBend(instr.voice[i], semitones, cents);
    }
}

void eTfInstrumentPanic(eTfInstrument &instr)
{
    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (instr.voice[i].noteIsOn)
            eTfVoicePanic(instr.voice[i]);
    }
}

eU32 eTfInstrumentGetPolyphony(eTfInstrument &instr)
{
    eU32 count = 0;

    for(eU32 i=0;i<TF_MAXVOICES;i++)
    {
        if (instr.voice[i].playing)
            count++;
    }

    return count;
}

eU32 eTfInstrumentAllocateVoice(eTfInstrument &instr)
{
    eU32 poly = eFtoL(instr.params[TF_GEN_POLYPHONY] * (TF_MAXVOICES-1) + 1);

    eU32 time = 0;
    eS32 chosen = -1;

    for(eU32 i=0;i<poly;i++) {
        if (!instr.voice[i].playing && !instr.voice[i].noteIsOn) {
            return i;
        } else {
            if (chosen == -1 || instr.voice[i].time > time)    {
                chosen = i;
                time = instr.voice[i].time;
            }
        }
    }

    return (eU32)chosen;
}

// ------------------------------------------------------------------------------------
// SYNTH
// ------------------------------------------------------------------------------------

void eTfSynthInit(eTfSynth &synth)
{
    eRandom rand;
    rand.seedRandomly();

    for (eU32 i=0; i<TF_MAXFRAMESIZE; i++)
    {
        synth.randomBuffer[i] = eSin(rand.nextFloat(0.0f, eTWOPI));
        synth.sinBuffer[i] = eSin((eF32)i / TF_MAXFRAMESIZE * 2 * ePI);
        synth.expBuffer[i] = eExp(-(5.0f / TF_MAXFRAMESIZE * i));
    }

    // make frequency (Hz) table
    eF64 a = 6.875;    // a
    a *= TF_12TH_ROOT_OF_2;    // b
    a *= TF_12TH_ROOT_OF_2;    // bb
    a *= TF_12TH_ROOT_OF_2;    // c, frequency of midi note 0

    for (eU32 i = 0; i < TF_NUMFREQS; i++)    // 128 midi notes
    {
        synth.freqTable[i] = (eF32)a;
        a *= TF_12TH_ROOT_OF_2;
    }

    for (eU32 i=0;i<TF_LFONOISETABLESIZE;i++)
    {
        synth.lfoNoiseTable[i] = rand.nextFloat(0.0f, 1.0f);
    }

    for (eU32 i=0;i<TF_NOISETABLESIZE;i++)
    {
        const static eInt q = 15;
        const static eF32 c1 = (1 << q) - 1;
        const static eF32 c2 = (eF32)(eFtoL(c1 / 3)) + 1;
        const static eF32 c3 = 1.f / c1;

        eF32 random = rand.nextFloat(0.0f, 1.0f);
        synth.whiteNoiseTable[i] = (2.f * ((random * c2) + (random * c2) + (random * c2)) - 3.f * (c2 - 1.f)) * c3;
    }

    for(eU32 j=0; j<TF_MAX_INSTR; j++)
        synth.instr[j] = nullptr;

}
// ------------------------------------------------------------------------------------
// SYNTH
// ------------------------------------------------------------------------------------

void eTfPlayerInit(eTfPlayer &player, eU32 sampleRate)
{
    eTfSynthInit(player.synth);
    player.synth.sampleRate = sampleRate;
    player.song.instrCount = 0;
    player.playing = eFALSE;
    player.volume = 0.1f;
}

void eTfPlayerLoadSong(eTfPlayer &player, const eU8 *data, eU32 len, eF32 delay)
{
    eTfPlayerUnloadSong(player);
    if (!len) return;

    eDataStream stream(data, len);

    eTfSong &song = player.song;
    eTfSynth &synth = player.synth;

    eU16 eventCounts[TF_MAX_INSTR];
    song.instrCount = stream.readU16();
    song.tempo = stream.readU16();

    // calculate speed values
    const eU32 rows_per_beat = stream.readU16();
    const eU32 rows_per_min = song.tempo * rows_per_beat;
    const eF32 rows_per_sec = (eF32)rows_per_min / 60.0f;
    const eF32 secs_per_row = 1.0f / rows_per_sec;

    //  init instruments & event arrays
    // -----------------------------------------------------------------------------------------
    for (eU32 i=0; i<song.instrCount; i++)
    {
        synth.instr[i] = new eTfInstrument;
        eTfInstrumentInit(synth, *synth.instr[i]);
        eventCounts[i] = stream.readU16();
        song.events[i].resize(eventCounts[i] * sizeof(eTfEvent));
    }

    //  read instruments
    // -----------------------------------------------------------------------------------------
    eU32 tagInst = stream.readU32();
    eASSERT(eMemEqual(&tagInst, "INST", 4));

  // grouped by instrument
  for (eU32 j=0; j<song.instrCount; j++)
    {
        for (eU8 i=0; i<TF_PARAM_COUNT; i++)
        {
            eF32 p = (eF32)stream.readU8()/100.0f;
            synth.instr[j]->params[i] = p;
        }
    }
  /*
  // grouped by paramindex
  for (eU8 i=0; i<TF_PARAM_COUNT; i++)
    {
        for (eU32 j=0; j<song.instrCount; j++)
        {
            eF32 p = (eF32)stream.readU8()/100.0f;
            synth.instr[j]->params[i] = p;
        }
    }
  */

    //  read song
    // -----------------------------------------------------------------------------------------
    eU32 tagSong = stream.readU32();
    eASSERT(eMemEqual(&tagSong, "SONG", 4));

    for (eU32 j=0; j<song.instrCount; j++)
    {
        eArray<eTfEvent> &events = song.events[j];

        // read times
        eU32 row = 0;
        for(eU32 i=0; i<eventCounts[j]; i++)
        {
            eU32 diff = stream.readU16();
            row += diff;
            events[i].time = (eF32)row * secs_per_row + delay;
        }

        // read notes
        for(eU32 i=0; i<eventCounts[j]; i++)
        {
            events[i].note = stream.readU8();
        }

        // read velocities
        for(eU32 i=0; i<eventCounts[j]; i++)
        {
            events[i].velocity = stream.readU8();
        }
    }

    eU32 tagEnd = stream.readU32();
    eASSERT(eMemEqual(&tagEnd, "ENDS", 4));
}

void eTfPlayerUnloadSong(eTfPlayer &player)
{
    for (eU32 i=0; i<TF_MAX_INSTR; i++)
    {
        player.song.events[i].clear();
        player.song.instrCount = 0;
        eDelete(player.synth.instr[i]);
    }
}

void eTfPlayerProcess(eTfPlayer &player, const eU8 **output)
{
    ePROFILER_FUNC();

    if (!player.playing)
        return;

    eF32 timeStep = (eF32)TF_FRAMESIZE / player.synth.sampleRate;
    eF32 nextTime = player.time + timeStep;

    eTfSong &song = player.song;
    eTfSynth &synth = player.synth;

    for (eU32 j=0; j<song.instrCount; j++)
    {
        eArray<eTfEvent> &events = song.events[j];

        for(eU32 i=0; i<events.size(); i++)
        {
            eTfEvent &ev = events[i];

            if (ev.time >= player.time && ev.time < nextTime)
            {
                if (ev.note && ev.instr >= 0)
                {
                    eTfInstrument *instr = synth.instr[j];

                    if (instr)
                    {
                        if (!ev.velocity)
                            eTfInstrumentNoteOff(*instr, ev.note);
                        else
                            eTfInstrumentNoteOn(*instr, ev.note, ev.velocity);
                    }
                }
            }
        }
    }

    eF32 *tempSignals[2];
    tempSignals[0] = &player.tempSignal[0];
    tempSignals[1] = &player.tempSignal[TF_FRAMESIZE];

    eF32 *signals[2];
    signals[0] = &player.outputSignal[0];
    signals[1] = &player.outputSignal[TF_FRAMESIZE];

    eMemSet(player.outputSignal, 0, sizeof(eF32)*TF_FRAMESIZE*2);

    //eU32 polyPhony = 0;
    for (eU32 i=0; i<TF_MAX_INSTR; i++)
    {
        eTfInstrument *instr = player.synth.instr[i];

        if (instr)
        {
            eMemSet(player.tempSignal, 0, sizeof(eF32)*TF_FRAMESIZE*2);
            eTfInstrumentProcess(player.synth, *instr, tempSignals, TF_FRAMESIZE);
            //polyPhony += eTfInstrumentGetPolyphony(*instr);
            eTfSignalMix(signals, tempSignals, TF_FRAMESIZE, 1.0f);
        }
    }

    eTfSignalToS16(signals, player.outputFinal, 10000.0f * TF_MASTER_VOLUME * player.volume, TF_FRAMESIZE);
    *output = (const eU8*)player.outputFinal;

    player.time = nextTime;
}

void eTfPlayerStart(eTfPlayer &player, eF32 time)
{
    player.time = time;
    player.playing = eTRUE;
}

void eTfPlayerStop(eTfPlayer &player)
{
    player.playing = eFALSE;

  for (eU32 i=0; i<TF_MAX_INSTR; i++)
    {
        eTfInstrument *instr = player.synth.instr[i];

        if (instr)
        {
      eTfInstrumentAllNotesOff(*instr);
    }
  }
}

