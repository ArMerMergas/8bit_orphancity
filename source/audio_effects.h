#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>

namespace AudioEffects {
	enum {
		EFF_NONE,
		EFF_MASKVOICE,
        EFF_REVERB,
        EFF_HIGHPASS,
        EFF_PROOT,
        EFF_CUSTOM
	};

    class DelayLine
    {
    public:
        float* buffer;
        int writeIndex;
        int bufferSize;

        DelayLine(int size = 480)
        {
            bufferSize = size;
            buffer = new float[bufferSize];
            memset(buffer, 0, sizeof(float) * bufferSize);
            writeIndex = 0;
        }

        void Write(float value)
        {
            buffer[writeIndex] = value;
            writeIndex = (writeIndex + 1) % bufferSize;
        }

        float Read(int delaySamples)
        {
            int readIndex = (writeIndex - delaySamples + bufferSize) % bufferSize;

            return buffer[readIndex];
        }
    };

    class CombFilter
    {
    public:
        DelayLine* delayLine;
        float feedbackGain; // How much signal feeds back (controls decay)
        float filterStore;  // Stores previous filtered value for smoothing
        float damping;      // Controls high-frequency damping

        CombFilter(int delaySamples, float feedback, float damping)
        {
            delayLine = new DelayLine(delaySamples);
            feedbackGain = feedback;
            this->damping = damping;
            filterStore = 0;
        }

        float Process(float input)
        {
            float delayedSample = delayLine->Read(delayLine->bufferSize);

            // Damping (low-pass filter on feedback)
            filterStore = (delayedSample * (1.0f - damping)) + (filterStore * damping);

            float output = input + filterStore * feedbackGain;
            delayLine->Write(output); // Feed processed signal back into delay
            return output;
        }
    };

    // Basic All-Pass Filter (smears phase, adds density)
    class AllPassFilter
    {
    public:
        DelayLine* delayLine;
        float feedbackGain; // Coefficient for feedback/feedforward

        AllPassFilter(int delaySamples, float gain)
        {
            delayLine = new DelayLine(delaySamples);
            feedbackGain = gain;
        }

        float Process(float input)
        {
            float delayedSample = delayLine->Read(delayLine->bufferSize);
            float output = -input * feedbackGain + delayedSample;
            delayLine->Write(input + output * feedbackGain); // Feedforward part
            return output;
        }
    };

    auto filter = CombFilter(1557, pow(0.001, 1557.0f / (1 * 24000)), 0.5);

    std::unordered_map<int, float> filterStore; // multiple players speaking will cause problems, so do that
    float damping2 = 0.95;

    void VoiceInMask(uint16_t* sampleBuffer, int samples, int uid, float damping = damping2) {
        for (int i = 0; i < samples; i++) {
            float signedSample = static_cast<float>(static_cast<int16_t>(sampleBuffer[i])); // samples ranging from -32768 to 32768
            //signedSample = (delayedSample * (1.0f - damping)) + (filterStore * damping);
            signedSample = (signedSample * (1.0f - damping)) + (filterStore[uid] * damping);
            filterStore[uid] = signedSample;
            sampleBuffer[i] = static_cast<uint16_t>(signedSample);

            //if (i < 10)
            //{
            //	Msg((std::to_string(i) + " = " + std::to_string(sampleBuffer[i]) + " 2\n").c_str());
            //}
        }
    }

    void Reverb(uint16_t* sampleBuffer, int samples) {
        for (int i = 0; i < samples; i++) {
            float signedSample = static_cast<float>(static_cast<int16_t>(sampleBuffer[i]));
            signedSample = filter.Process(signedSample);
            sampleBuffer[i] = static_cast<uint16_t>(signedSample);
        }
    }

    std::unordered_map<int, float> prevSample;
    std::unordered_map<int, float> prevOutput;

    std::unordered_map<int, CombFilter> prootReverbsMap;

    //auto prootReverb = CombFilter(600, pow(0.001, 512.0f / (0.2 * 24000)), 0.3);

    void ProotFilter(uint16_t* sampleBuffer, int samples, int uid, float cutOff, float gain) {
        auto& reverb = prootReverbsMap.try_emplace(uid, 600, pow(0.001, 512.0f / (0.2 * 24000)), 0.3).first->second;

        for (int i = 0; i < samples; i++) {
            float signedSample = static_cast<float>(static_cast<int16_t>(sampleBuffer[i]));

            if (signedSample > 0)
            {
                signedSample *= 0.2;
            }

            float output = cutOff * (prevOutput[uid] + signedSample - prevSample[uid]);

            prevSample[uid] = signedSample;
            prevOutput[uid] = output;

            signedSample = (signedSample * 0.3) + (output * gain);
            signedSample = reverb.Process(signedSample);
            sampleBuffer[i] = static_cast<uint16_t>(signedSample);
        }
    }

    struct CustomParams {
        float pitch = 1.0f;
        float lowpass = 0.0f;
        float reverb = 0.0f;
    };

    std::unordered_map<int, CustomParams> customParamsMap;

    class GranularPitchShifter
    {
    public:
        float buffer[4096];
        int writePos;
        float phase;
        float shift;
        int windowSize;

        GranularPitchShifter(float shiftFactor = 1.0f) {
            memset(buffer, 0, sizeof(buffer));
            writePos = 0;
            phase = 0;
            shift = 1.0f - shiftFactor;
            windowSize = 800;
        }

        float Process(float input) {
            buffer[writePos] = input;
            
            float readPos1 = writePos - phase;
            if (readPos1 < 0) readPos1 += 4096;
            if (readPos1 >= 4096) readPos1 -= 4096;

            float phase2 = phase + windowSize / 2.0f;
            if (phase2 >= windowSize) phase2 -= windowSize;

            float readPos2 = writePos - phase2;
            if (readPos2 < 0) readPos2 += 4096;
            if (readPos2 >= 4096) readPos2 -= 4096;
            
            float out1 = buffer[(int)readPos1];
            float out2 = buffer[(int)readPos2];

            float t = phase / windowSize;
            float w1 = 1.0f - std::abs(t * 2.0f - 1.0f);
            
            float t2 = phase2 / windowSize;
            float w2 = 1.0f - std::abs(t2 * 2.0f - 1.0f);

            phase += shift;
            if (phase >= windowSize) phase -= windowSize;
            if (phase < 0) phase += windowSize;

            writePos = (writePos + 1) % 4096;
            return (out1 * w1 + out2 * w2) / (w1 + w2 + 0.0001f);
        }
    };

    std::unordered_map<int, GranularPitchShifter> customShiftersMap;

    void CustomFilter(uint16_t* sampleBuffer, int samples, int uid) {
        CustomParams params = customParamsMap[uid];
        
        auto& shifter = customShiftersMap.try_emplace(uid, params.pitch).first->second;
        shifter.shift = 1.0f - params.pitch; 
        
        auto& reverbEffect = prootReverbsMap.try_emplace(uid, 600, pow(0.001, 512.0f / (0.2 * 24000)), 0.3).first->second;

        for (int i = 0; i < samples; i++) {
            float signedSample = static_cast<float>(static_cast<int16_t>(sampleBuffer[i]));
            
            if (params.pitch != 1.0f) {
                signedSample = shifter.Process(signedSample);
            }
            
            if (params.lowpass > 0.0f) {
                signedSample = (signedSample * (1.0f - params.lowpass)) + (filterStore[uid] * params.lowpass);
                filterStore[uid] = signedSample;
            }
            
            if (params.reverb > 0.0f) {
                signedSample = reverbEffect.Process(signedSample) * params.reverb + signedSample * (1.0f - params.reverb);
            }
            
            sampleBuffer[i] = static_cast<uint16_t>(signedSample);
        }
    }
}