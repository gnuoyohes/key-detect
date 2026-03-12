#pragma once

class ChordComponent : public juce::Component
{
public:
    ChordComponent (PluginProcessor& p)
        : processorRef (p)
    {
        slider.setSliderStyle (juce::Slider::Rotary);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 80, 30);
        slider.setTextBoxIsEditable (true);
        slider.setTextValueSuffix (" Db");
        addAndMakeVisible (slider);

        label.setFont (juce::Font (14.0f, juce::Font::bold));
        label.setText ("Detect Threshold", juce::dontSendNotification);
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType (juce::Justification::centredTop);
        label.attachToComponent (&slider, false);
        addAndMakeVisible (label);
        label.toBack();

        sliderAttachment = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.getState(), "detectThreshold", slider);
    }

    ~ChordComponent() {}

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        g.setFont (60.0f);
        g.setColour (juce::Colours::silver);
        g.drawFittedText (chord, 0, 0, getWidth(), getHeight(), juce::Justification::centred, 1);
    }

    void resized() override
    {
        const int sliderSize = 130;

        slider.setBounds (getRight() - sliderSize - 50, 50, sliderSize, sliderSize);
        label.setBounds (slider.getX(), slider.getY() - 15, sliderSize, sliderSize);
    }

    juce::String chord;

private:
    PluginProcessor& processorRef;

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
};