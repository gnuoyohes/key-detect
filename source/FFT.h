/*
* Modified from https://github.com/ffAudio/Frequalizer/blob/master/Source/Analyser.h
*/

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================

static const float MINFREQ = 20.0f;

class FFT : public juce::Thread
{
public:
    FFT() : juce::Thread ("FFT")
    {
        averager.clear();
    }

    ~FFT() override = default;

    void setup (int audioFifoSize, float sampleRateToUse, float negInfinityDb)
    {
        sampleRate = sampleRateToUse;
        audioFifo.setSize (1, audioFifoSize);
        abstractFifo.setTotalSize (audioFifoSize);
        negInfinity = negInfinityDb;

        startThread();
    }

    void addAudioData (const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels)
    {
        if (abstractFifo.getFreeSpace() < buffer.getNumSamples())
            return;

        int start1, block1, start2, block2;
        abstractFifo.prepareToWrite (buffer.getNumSamples(), start1, block1, start2, block2);
        audioFifo.copyFrom (0, start1, buffer.getReadPointer (startChannel), block1);
        if (block2 > 0)
            audioFifo.copyFrom (0, start2, buffer.getReadPointer (startChannel, block1), block2);

        for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
        {
            if (block1 > 0)
                audioFifo.addFrom (0, start1, buffer.getReadPointer (channel), block1);
            if (block2 > 0)
                audioFifo.addFrom (0, start2, buffer.getReadPointer (channel, block1), block2);
        }
        abstractFifo.finishedWrite (block1 + block2);
        waitForData.signal();
    }

    void run() override
    {
        while (!threadShouldExit())
        {
            if (abstractFifo.getNumReady() >= fft.getSize())
            {
                fftBuffer.clear();

                int start1, block1, start2, block2;
                abstractFifo.prepareToRead (fft.getSize(), start1, block1, start2, block2);
                if (block1 > 0)
                    fftBuffer.copyFrom (0, 0, audioFifo.getReadPointer (0, start1), block1);
                if (block2 > 0)
                    fftBuffer.copyFrom (0, block1, audioFifo.getReadPointer (0, start2), block2);
                abstractFifo.finishedRead ((block1 + block2) / 2);

                windowing.multiplyWithWindowingTable (fftBuffer.getWritePointer (0), size_t (fft.getSize()));
                fft.performFrequencyOnlyForwardTransform (fftBuffer.getWritePointer (0));

                juce::ScopedWriteLock lockedForWriting (lock);
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples(), -1.0f);
                averager.copyFrom (averagerPtr, 0, fftBuffer.getReadPointer (0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                averager.addFrom (0, 0, averager.getReadPointer (averagerPtr), averager.getNumSamples());
                if (++averagerPtr == averager.getNumChannels())
                    averagerPtr = 1;

                newDataAvailable.store (true);
            }
            else
            {
                waitForData.wait (100);
            }
        }
    }

    void createPath (juce::Path& p, const juce::Rectangle<int> bounds)
    {
        p.clear();
        p.preallocateSpace (8 + fftBuffer.getNumSamples() * 3);

        const juce::ScopedReadLock lockedForReading (lock);
        const auto* fftData = averager.getReadPointer (0);
        const auto factor = bounds.getWidth() / 10.0f;

        p.startNewSubPath (bounds.getX() + factor * freqToX (indexToFreq(0)), binToY (fftData[0], bounds));
        for (int i = 0; i < averager.getNumSamples(); ++i)
            p.lineTo (bounds.getX() + factor * freqToX (indexToFreq (i)), binToY (fftData[i], bounds));
    }

    bool checkForNewData()
    {
        auto available = newDataAvailable.load();
        newDataAvailable.store (false);
        return available;
    }

    float indexToFreq (float index)
    {
        return (sampleRate * index) / fft.getSize();
    }

    float freqToX (float freq)
    {
        return (freq > 0.01f) ? std::log (freq / MINFREQ) / std::log (2.0f) : 0.0f;
    }

    float binToY(float bin, const juce::Rectangle<int> bounds)
    {
        return dbToY (juce::Decibels::gainToDecibels (bin, negInfinity), bounds);
    }

    float dbToY (float db, const juce::Rectangle<int> bounds)
    {
        return juce::jmap (
            db,
            negInfinity,
            0.0f,
            static_cast<float> (bounds.getBottom()),
            static_cast<float> (bounds.getY()));
    }

    juce::AudioBuffer<float>& getAudio()
    {
        return averager;
    }

    juce::ReadWriteLock& getLock()
    {
        return lock;
    }

private:
    juce::WaitableEvent waitForData;
    juce::ReadWriteLock lock;

    float sampleRate {};
    float negInfinity;

    juce::dsp::FFT fft { 13 };
    juce::dsp::WindowingFunction<float> windowing { size_t (fft.getSize()), juce::dsp::WindowingFunction<float>::hann, true };
    juce::AudioBuffer<float> fftBuffer { 1, fft.getSize() * 2 };

    juce::AudioBuffer<float> averager { 5, fft.getSize() / 2 };
    int averagerPtr = 1;

    juce::AbstractFifo abstractFifo { 48000 };
    juce::AudioBuffer<float> audioFifo;

    std::atomic<bool> newDataAvailable;
    std::atomic<int> chordDetected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFT)
};