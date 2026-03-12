#pragma once

#include "FFT.h"

static const std::string CHORDLABELS[] = {
    "C Major",
    "C Minor",
    "C# Major",
    "C# Minor",
    "D Major",
    "D Minor",
    "Eb Major",
    "Eb Minor",
    "E Major",
    "E Minor",
    "F Major",
    "F Minor",
    "F# Major",
    "F# Minor",
    "G Major",
    "G Minor",
    "Ab Major",
    "Ab Minor",
    "A Major",
    "A Minor",
    "Bb Major",
    "Bb Minor",
    "B Major",
    "B Minor"
};

static const int CHORDMINFREQ = 40;
static const int CHORDMAXFREQ = 10000;

static const float ROOTFACTOR = 1.0f;
static const float THIRDFACTOR = 0.7f;
static const float FIFTHFACTOR = 0.6f;

static const int FREQFACTOREXPO = 4;

//==============================================================================

class ChordDetect
{
public:
    ChordDetect (FFT& fftRef, std::atomic<float>* thresholdPtr)
        : fft (fftRef),
          threshold (thresholdPtr)
    {}
    ~ChordDetect() {}

    void setup(float negInfinityDb)
    {
        negInfinity = negInfinityDb;
    }

    std::tuple<juce::String, std::vector<std::vector<float>>> detectChord()
    {
        std::array<float, 24> chordScores = { 0 };
        std::array<float, 12> pitchClassScores = { 0 };
        std::vector<std::vector<float>> highlightedX (12, std::vector<float>());

        juce::ScopedReadLock lockedForReading (fft.getLock());
        juce::AudioBuffer<float>& audio = fft.getAudio();
        const auto* fftData = audio.getReadPointer (0);

        auto thresholdVal = threshold->load();

        //juce::Range< float> range = averager.findMinMax (0, 0, averager.getNumSamples());
        //DBG (juce::String (range.getStart()) + ", " + juce::String (range.getEnd()));
        for (int i = 1; i < audio.getNumSamples(); ++i)
        {
            float freq = fft.indexToFreq (i);

            if (freq < CHORDMINFREQ)
                continue;

            if (freq > CHORDMAXFREQ)
                break;

            float freqFactor = std::pow (FREQFACTOREXPO, -1 * juce::jmap (i, 0, audio.getNumSamples(), 0, 1));
            int midiNote = static_cast<int> (std::round (12.0f * std::log2f (freq / 440.0f) + 69.0f));

            //int octave = midiNote / 12 - 1;
            int pitchClass = midiNote % 12;

            float bin = fftData[i];
            //bin = juce::Decibels::gainToDecibels (bin) + 100.0f;

            if (juce::Decibels::gainToDecibels (bin, negInfinity) > thresholdVal)
            {
                pitchClassScores[pitchClass] += bin * freqFactor;
                highlightedX[pitchClass].push_back (fft.freqToX (freq));
            }

            //if (bin > threshold)
            //{
            //    // Add root scores
            //    //int index = pitchClass * 2;
            //    //chordScores[index] += bin * freqFactor * ROOTFACTOR;
            //    //chordScores[index + 1] += bin * freqFactor * ROOTFACTOR;

            //    // Add third scores
            //    //index = ((pitchClass + 8) % 12) * 2;
            //    //chordScores[index] += bin * freqFactor * THIRDFACTOR;
            //    //chordScores[(index + 3) % 24] += bin * freqFactor * THIRDFACTOR;
            //
            //    // Add fifth scores
            //    //index = ((pitchClass + 5) % 12) * 2;
            //    //chordScores[index] += bin * freqFactor * fifthFactor;
            //    //chordScores[index + 1] += bin * freqFactor * fifthFactor;
            //}
        }

        size_t pcSize = pitchClassScores.size();
        int maxIndex = 0;
        float maxScore = 0;
        int rootIndex = 0, thirdIndex = 0, fifthIndex = 0;

        for (int j = 0; j < pcSize; ++j)
        {
            // Major triad
            int third = (j + 4) % pcSize, fifth = (j + 7) % pcSize;

            float score = ROOTFACTOR * pitchClassScores[j]
                          + THIRDFACTOR * pitchClassScores[third]
                          + FIFTHFACTOR * pitchClassScores[fifth];
            if (score > maxScore)
            {
                maxIndex = j * 2;
                maxScore = score;
                rootIndex = j;
                thirdIndex = third;
                fifthIndex = fifth;
            }

            // Minor triad
            third = (j + 3) % pcSize;

            score = ROOTFACTOR * pitchClassScores[j]
                    + THIRDFACTOR * pitchClassScores[third]
                    + FIFTHFACTOR * pitchClassScores[fifth];
            if (score > maxScore)
            {
                maxIndex = j * 2 + 1;
                maxScore = score;
                rootIndex = j;
                thirdIndex = third;
                fifthIndex = fifth;
            }
        }

        //int maxIndex = 0;
        //float maxScore = 0;
        //for (int j = 0; j < chordScores.size(); ++j)
        //{
        //    if (chordScores[j] > maxScore)
        //    {
        //        maxIndex = j;
        //        maxScore = chordScores[j];
        //    }
        //}
        // 
        //chordDetected.store (maxIndex);
        juce::String chordLabel = CHORDLABELS[maxIndex];

        std::vector<std::vector<float>> chordX = { highlightedX[rootIndex], highlightedX[thirdIndex], highlightedX[fifthIndex] };

        return std::make_tuple (chordLabel, chordX);
    }

private:
    FFT& fft;

    float negInfinity;
    std::atomic<float>* threshold;
};