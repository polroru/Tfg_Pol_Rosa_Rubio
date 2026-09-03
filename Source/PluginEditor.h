/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Globals/Parameters.h"

//==============================================================================
/**
*/
class RosaRosaAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RosaRosaAudioProcessorEditor (RosaRosaAudioProcessor&);
    ~RosaRosaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RosaRosaAudioProcessor& audioProcessor;



    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oversamplingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> adaaAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> parallelAttachment;


    juce::ComboBox oversampling_menu; //es crea combobox (desplegable grafic)
    juce::ComboBox distortion_menu; //es crea combobox (desplegable grafic)
    juce::ComboBox adaa_menu; //es crea combobox (desplegable grafic)
    juce::Slider gain_slider; //es crea slider (desplegable grafic)
	juce::Slider level_slider; //es crea slider (desplegable grafic)
    juce::Slider threshold_slider; //es crea slider (desplegable grafic)
    juce::ToggleButton parallel_button; // es crea button (botó grafic)



    void initialiseSliders(); //funcio per a inicialitzar els sliders
    void initialiseComboBoxes(); //funcio per a inicialitzar els combobox
    void initialiseButtons();//funcio per a inicialitzar els botons
    void setComboBoxProps(juce::ComboBox& box, const juce::StringArray& items); //funcio per a rebre el combobox a configurar i llista d'elements del menu
    void setSliderProps(juce::Slider& slider, const juce::String& name, double min, double max, double step, double defaultValue);
    void setButtonProps(juce::ToggleButton& button, const juce::String& text);
	void updateControls(); //funcio per evitar cambiar configuracions sense distortion


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RosaRosaAudioProcessorEditor)
};
