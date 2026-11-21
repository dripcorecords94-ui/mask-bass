#pragma once
#include <JuceHeader.h>
#include "Curves.h"

class MaskBassProcessor : public juce::AudioProcessor
{
public:
    MaskBassProcessor();
    ~MaskBassProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Mask Bass"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // para la interfaz (cabezal de reproduccion)
    std::atomic<float> uiPhase { 0.f };
    std::atomic<bool>  uiActive { false };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::atomic<float> *pMix, *pRate, *pMode, *pBand, *pXover, *pListen, *pCurve, *pThresh;

    juce::dsp::LinkwitzRileyFilter<float> crossover;

    double samplesSinceTrigger = 1.0e12;
    float  smoothedGain = 1.f, smoothCoef = 0.01f;
    float  env = 0.f, envRelease = 0.999f;
    bool   armed = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaskBassProcessor)
};
