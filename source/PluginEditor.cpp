#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      undoManager (processorRef.getUndoManager()),
      chord (processorRef),
      spectogram(processorRef, chord)
{
    //addAndMakeVisible (inspectButton);

    // this chunk of code instantiates and opens the melatonin inspector
    //inspectButton.onClick = [&] {
    //    if (!inspector)
    //    {
    //        inspector = std::make_unique<melatonin::Inspector> (*this);
    //        inspector->onClose = [this]() { inspector.reset(); };
    //    }

    //    inspector->setVisible (true);
    //};

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

    //undoButton.onClick = [this] { undoManager.undo(); };
    //addAndMakeVisible (undoButton);

    //redoButton.onClick = [this] { undoManager.redo(); };
    //addAndMakeVisible (redoButton);

    addAndMakeVisible (spectogram);
    addAndMakeVisible (chord);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll (juce::Colours::black);

}

void PluginEditor::resized()
{
    const int width = getWidth();
    const int height = getHeight();

    //inspectButton.setBounds (10, height - 40, 50, 30);

    // Undo and redo buttons
    //const int undoButtonSize = 30;
    //undoButton.setBounds (width - (undoButtonSize * 2 + 30), height - 50, undoButtonSize, undoButtonSize);
    //redoButton.setBounds (width - (undoButtonSize + 20), height - 50, undoButtonSize, undoButtonSize);

    spectogram.setBounds (10, 10, width - 20, height * 2 / 3 - 20);
    chord.setBounds(10, spectogram.getBottom(), width - 20, height - spectogram.getBottom());
}