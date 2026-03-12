/*
* Modified from https://github.com/ffAudio/Frequalizer/blob/master/Source/FrequalizerEditor.cpp
*/

#pragma once

#include "ChordComponent.h"

class SpectogramComponent : public juce::Component, public juce::Timer
{
public:
    SpectogramComponent (PluginProcessor& p, ChordComponent& c)
        : processorRef (p),
          chordComponent (c)
    {
        startTimerHz (60);
    }

    ~SpectogramComponent() { stopTimer(); } 

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.setFont (12.0f);
        g.setColour (juce::Colours::silver);
        g.drawRoundedRectangle (plotFrame.toFloat(), 5, 2);
        for (int i = 0; i < 10; ++i)
        {
            g.setColour (juce::Colours::silver.withAlpha (0.3f));
            auto x = plotFrame.getX() + plotFrame.getWidth() * i * 0.1f;
            if (i > 0)
                g.drawVerticalLine (juce::roundToInt (x), float (plotFrame.getY()), float (plotFrame.getBottom()));

            auto freq = getFrequencyForPosition (i * 0.1f);
            g.drawFittedText ((freq < 1000) ? juce::String (freq) + " Hz"
                                            : juce::String (freq / 1000, 1) + " kHz",
                juce::roundToInt (x + 3),
                plotFrame.getBottom() - 18,
                50,
                15,
                juce::Justification::left,
                1);
        }

        g.setColour (juce::Colours::silver.withAlpha (0.3f));
        g.drawHorizontalLine (juce::roundToInt (plotFrame.getY() + 0.25 * plotFrame.getHeight()), float (plotFrame.getX()), float (plotFrame.getRight()));
        g.drawHorizontalLine (juce::roundToInt (plotFrame.getY() + 0.5 * plotFrame.getHeight()), float (plotFrame.getX()), float (plotFrame.getRight()));
        g.drawHorizontalLine (juce::roundToInt (plotFrame.getY() + 0.75 * plotFrame.getHeight()), float (plotFrame.getX()), float (plotFrame.getRight()));

        g.setColour (juce::Colours::silver);
        g.drawFittedText ("0 dB", plotFrame.getX() + 3, plotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
        g.drawFittedText (juce::String (processorRef.negInfinityDb / 4) + " dB", plotFrame.getX() + 3, juce::roundToInt (plotFrame.getY() + 2 + 0.25 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
        g.drawFittedText (juce::String (processorRef.negInfinityDb / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt (plotFrame.getY() + 2 + 0.5 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
        g.drawFittedText (juce::String (processorRef.negInfinityDb / 4 * 3) + " dB", plotFrame.getX() + 3, juce::roundToInt (plotFrame.getY() + 2 + 0.75 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);

        g.reduceClipRegion (plotFrame);

        juce::Colour colors[3] = {
            juce::Colour::fromRGBA (235, 87, 87, 120),
            juce::Colour::fromRGBA (235, 161, 87, 120),
            juce::Colour::fromRGBA (235, 230, 87, 120)
        };

        const auto factor = plotFrame.getWidth() / 10.0f;
        const int highlightWidth = 10;

        for (int i = 0; i < highlightedX.size(); ++i)
        {
            g.setColour (colors[i]);

            for (int j = 0; j < highlightedX[i].size(); ++j)
            {
                g.fillRect (
                    plotFrame.getX() - highlightWidth / 2 + static_cast<int> (std::round (factor * highlightedX[i][j])),
                    plotFrame.getY(),
                    highlightWidth,
                    plotFrame.getHeight()
                );
                    
            }
        }

        g.setColour (juce::Colour::fromRGB(36, 242, 160));
        g.strokePath (analyserPath, juce::PathStrokeType (2.0));

        float thresholdY = processorRef.getFFT().dbToY (processorRef.getDetectThreshold()->load(), plotFrame);
        
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 60));
        g.fillRect (
            plotFrame.getX(),
            static_cast<int> (std::round(thresholdY)),
            plotFrame.getWidth(),
            5
        );

        if (clicked)
        {
            g.setColour (juce::Colour::fromRGBA (36, 242, 160, 120));
            g.drawVerticalLine (mouseX, plotFrame.getY(), plotFrame.getY() + plotFrame.getHeight());
            g.drawFittedText ((clickedFreq < 1000) ? juce::String (clickedFreq, 2) + " Hz"
                                                   : juce::String (clickedFreq / 1000, 3) + " kHz",
                mouseX + 5,
                plotFrame.getHeight() / 2,
                50,
                15,
                juce::Justification::left,
                1);
        }
    }

    void resized() override
    {
        plotFrame = getLocalBounds().reduced (3, 3);
        plotFrame.reduce (3, 3);
    }

    void timerCallback() override
    {
        if (processorRef.getFFT().checkForNewData())
        {
            processorRef.createSpectogram (analyserPath, plotFrame);
            std::tie(chordComponent.chord, highlightedX) = processorRef.getChordDetect().detectChord();
            repaint();
            chordComponent.repaint();
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        clicked = true;
        mouseX = e.getMouseDownPosition().getX();

        float pos = ( mouseX - plotFrame.getX() ) / static_cast<float>( plotFrame.getWidth());
        clickedFreq = getFrequencyForPosition (pos);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        clicked = true;
        mouseX = e.getMouseDownPosition().getX();
        repaint ();
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        clicked = false;
    }

private:
    float getFrequencyForPosition (float pos)
    {
        return 20.0f * std::pow (2.0f, pos * 10.0f);
    }

    PluginProcessor& processorRef;
    ChordComponent& chordComponent;

    juce::Rectangle<int> plotFrame;
    juce::Path analyserPath;
    std::vector<std::vector<float>> highlightedX;

    bool clicked { false };
    int mouseX;
    float clickedFreq;
};