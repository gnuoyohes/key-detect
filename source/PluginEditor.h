#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "SpectogramComponent.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    //std::unique_ptr<melatonin::Inspector> inspector;
    juce::UndoManager& undoManager;

    //juce::TextButton inspectButton { "Inspect the UI" };

    //juce::TextButton undoButton { juce::String::fromUTF8 ("↶") };
    //juce::TextButton redoButton { juce::String::fromUTF8 ("↷") };

    ChordComponent chord;
    SpectogramComponent spectogram;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
