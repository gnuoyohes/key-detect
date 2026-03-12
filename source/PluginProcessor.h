#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "FFT.h"
#include "ChordDetect.h"


#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void createSpectogram (juce::Path&, const juce::Rectangle<int>);

    juce::AudioProcessorValueTreeState& getState() { return state; }
    juce::UndoManager& getUndoManager() { return undoManager; }
    std::atomic<float>* getDetectThreshold() { return detectThreshold; }
    FFT& getFFT() { return fft; }
    ChordDetect& getChordDetect() { return cd; }

    inline static float negInfinityDb { -80.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    juce::AudioProcessorValueTreeState state;
    juce::UndoManager undoManager;

    std::atomic<float>* detectThreshold;

    FFT fft;
    ChordDetect cd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
