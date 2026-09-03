/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RosaRosaAudioProcessorEditor::RosaRosaAudioProcessorEditor (RosaRosaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::ignoreUnused(audioProcessor);

    initialiseComboBoxes(); //es crida la funcio per a inicialitzar els combobox
    initialiseSliders(); //es crida la funcio per a inicialitzar els sliders
    initialiseButtons(); //es crida la funcio per inicialitzar el boto

    distortion_menu.onChange = [this]()
        {
            updateControls();
        };

    updateControls();

    setSize(1000, 600);
}

RosaRosaAudioProcessorEditor::~RosaRosaAudioProcessorEditor()
{
}

//==============================================================================
void RosaRosaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(16.0f);

    const int labelX = 20;
    const int labelWidth = 120;
    const int labelHeight = 30;

    const int startY = 20;
    const int spacing = 50;

    g.drawText("Distortion", labelX, startY + 0 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("Oversampling", labelX, startY + 1 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("Parallel", labelX, startY + 2 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("ADAA", labelX, startY + 3 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("Threshold", labelX, startY + 4 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("Drive", labelX, startY + 5 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);

    g.drawText("Level", labelX, startY + 6 * spacing,
        labelWidth, labelHeight, juce::Justification::centredLeft);


}

void RosaRosaAudioProcessorEditor::resized()
{
    const int controlX = 150;
    const int startY = 20;
    const int spacing = 50;

    distortion_menu.setBounds(controlX, startY + 0 * spacing, 180, 30);
    oversampling_menu.setBounds(controlX, startY + 1 * spacing, 180, 30);
    parallel_button.setBounds(controlX, startY + 2 * spacing, 180, 30);
    adaa_menu.setBounds(controlX, startY + 3 * spacing, 180, 30);
    threshold_slider.setBounds(controlX, startY + 4 * spacing, 300, 30);
    gain_slider.setBounds(controlX, startY + 5 * spacing, 300, 30);
    level_slider.setBounds(controlX, startY + 6 * spacing, 300, 30);
}




//==============================================================================
//                              FUNCIONS PROPIES
//==============================================================================

void RosaRosaAudioProcessorEditor::initialiseComboBoxes()
{
    // Distortion
    const juce::StringArray distortion_items = { "Off", "Hard Clip", "Tanh", "Cubic", "Arctan", "Asimetric"}; //array de items amb les opcions de distorsio
    setComboBoxProps(distortion_menu, distortion_items); //es crida la funcio per a configurar el combobox de distorsio amb les opcions


    distortionAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::distortionTypeID,
            distortion_menu);

    // Oversampling
    const juce::StringArray oversampling_items = { "Off", "2X", "4X", "8X", "16X" }; //array de items amb les opcions de oversampling
    setComboBoxProps(oversampling_menu, oversampling_items); //es crida la funcio per a configurar el combobox de oversampling amb les opcions

    oversamplingAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::oversamplingID,
            oversampling_menu);

    // ADAA
    const juce::StringArray adaa_items = {"Off", "ADAA1", "ADAA2"};
    setComboBoxProps(adaa_menu, adaa_items);
    adaaAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::adaaID,
            adaa_menu);

}



void RosaRosaAudioProcessorEditor::initialiseSliders()
{
    // Gain
    setSliderProps(gain_slider, "gain", 1.0, 10.0, 0.01, 1.0);

    gainAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::gainID,
            gain_slider);


    // Level
    setSliderProps(level_slider, "Level", 0.0, 2.0, 0.01, 1.0);
    levelAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::levelID,
            level_slider);


    // Threshold
    setSliderProps(threshold_slider, "Threshold", 0.0, 1.0, 0.01, 0.5);
    thresholdAttachment = std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::thresholdID,
            threshold_slider);

}


void RosaRosaAudioProcessorEditor::initialiseButtons()
{
    setButtonProps(parallel_button, "Enable");

    parallelAttachment =
        std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            audioProcessor.getTreeState(),
            viator::parameters::parallelID,
            parallel_button);

}





void RosaRosaAudioProcessorEditor::setComboBoxProps(juce::ComboBox& box, const juce::StringArray& items)
{
    box.clear();

    box.addItemList(items, 1);

    box.setSelectedId(1, juce::dontSendNotification);

    box.setEnabled(true);

    addAndMakeVisible(box);
}

void RosaRosaAudioProcessorEditor::setSliderProps(juce::Slider& slider, const juce::String& name, double min, double max, double step, double defaultValue)
{
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    slider.setRange(min, max, step);
    slider.setValue(defaultValue);
    addAndMakeVisible(slider);
}

void RosaRosaAudioProcessorEditor::setButtonProps(juce::ToggleButton& button,
    const juce::String& text)
{
    button.setButtonText(text);
    addAndMakeVisible(button);
}



void RosaRosaAudioProcessorEditor::updateControls()
{
    const int distortionType =
        distortion_menu.getSelectedItemIndex();

    const bool distortionEnabled =
        distortionType != 0;

    const bool isTanh =
        distortionType == 2;


    // Activem els controls només si hi ha una distorsió seleccionada.
    parallel_button.setEnabled(distortionEnabled);
    oversampling_menu.setEnabled(distortionEnabled);
    adaa_menu.setEnabled(distortionEnabled);


    // ADAA2 correspon a l'ID 3 del ComboBox.
    // Si la distorsió és Tanh, aquesta opció queda desactivada.
    adaa_menu.setItemEnabled(3, !isTanh);


    // Si ADAA2 estava seleccionat i canviem a Tanh,
    // canviem automàticament a ADAA1.
    if (isTanh && adaa_menu.getSelectedId() == 3)
    {
        adaa_menu.setSelectedId(2,juce::sendNotificationSync);
    }
}

