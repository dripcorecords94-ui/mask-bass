#include "PluginProcessor.h"
#include "PluginEditor.h"

static constexpr double kRateBeats[4] = { 0.5, 1.0, 2.0, 4.0 };   // 1/8, 1/4, 1/2, 1/1

juce::AudioProcessorValueTreeState::ParameterLayout MaskBassProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
                                                  NormalisableRange<float> (0.f, 1.f), 0.8f));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "rate", 1 }, "Rate",
                                                   StringArray { "1/8", "1/4", "1/2", "1/1" }, 1));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Trigger",
                                                   StringArray { "Sync", "MIDI", "Audio" }, 0));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "band", 1 }, "Band", false));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "xover", 1 }, "Crossover",
                                                  NormalisableRange<float> (40.f, 500.f, 1.f, 0.4f), 150.f));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "listen", 1 }, "Listen", false));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { "curve", 1 }, "Curve", 0, mb::kNumCurves - 1, 2));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "thresh", 1 }, "Audio Threshold",
                                                  NormalisableRange<float> (-60.f, 0.f, 0.1f), -30.f));
    return l;
}

MaskBassProcessor::MaskBassProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                          .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pMix    = apvts.getRawParameterValue ("mix");
    pRate   = apvts.getRawParameterValue ("rate");
    pMode   = apvts.getRawParameterValue ("mode");
    pBand   = apvts.getRawParameterValue ("band");
    pXover  = apvts.getRawParameterValue ("xover");
    pListen = apvts.getRawParameterValue ("listen");
    pCurve  = apvts.getRawParameterValue ("curve");
    pThresh = apvts.getRawParameterValue ("thresh");
}

void MaskBassProcessor::prepareToPlay (double sr, int blockSize)
{
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockSize, 2 };
    crossover.prepare (spec);
    crossover.reset();

    smoothCoef = 1.f - std::exp (-1.f / (0.001f * (float) sr));   // ~1 ms anti-click
    envRelease = std::exp (-1.f / (0.005f * (float) sr));
    samplesSinceTrigger = 1.0e12;
    smoothedGain = 1.f;
    env = 0.f;
    armed = true;
}

bool MaskBassProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo() && out != juce::AudioChannelSet::mono()) return false;
    if (layouts.getMainInputChannelSet() != out) return false;

    if (layouts.inputBuses.size() > 1)
    {
        const auto sc = layouts.getChannelSet (true, 1);
        if (! sc.isDisabled() && sc != juce::AudioChannelSet::stereo() && sc != juce::AudioChannelSet::mono())
            return false;
    }
    return true;
}

void MaskBassProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    auto main = getBusBuffer (buffer, true, 0);
    auto sc   = getBusBuffer (buffer, true, 1);       // 0 canales si esta desactivado
    const int numCh = juce::jmin (2, main.getNumChannels());
    const int scCh  = sc.getNumChannels();

    const float mix    = pMix->load();
    const int   rate   = juce::jlimit (0, 3, (int) pRate->load());
    const int   mode   = (int) pMode->load();
    const bool  band   = pBand->load() > 0.5f;
    const bool  listen = pListen->load() > 0.5f && band;
    const int   curve  = (int) pCurve->load();
    const float thrLin = juce::Decibels::decibelsToGain (pThresh->load());

    crossover.setCutoffFrequency (pXover->load());

    // --- info del host ---
    double bpm = 120.0, ppq = 0.0;
    bool playing = false, havePpq = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = juce::jmax (20.0, *b);
            playing = pos->getIsPlaying();
            if (auto p = pos->getPpqPosition()) { ppq = *p; havePpq = true; }
        }

    const double sr = getSampleRate();
    const double beatsPerSample = bpm / (60.0 * sr);
    const double cycleBeats = kRateBeats[rate];
    const double cycleSamples = cycleBeats / beatsPerSample;
    const double minGap = 0.02 * sr;

    // --- disparos MIDI ---
    int midiTrig[128]; int nMidi = 0, midiIdx = 0;
    if (mode == 1)
        for (const auto meta : midi)
            if (meta.getMessage().isNoteOn() && nMidi < 128)
                midiTrig[nMidi++] = meta.samplePosition;

    float* ch[2] = { nullptr, nullptr };
    for (int c = 0; c < numCh; ++c) ch[c] = main.getWritePointer (c);

    float lastPhase = 0.f; bool active = false;

    for (int i = 0; i < numSamples; ++i)
    {
        float g = 1.f; active = false;

        if (mode == 0)
        {
            if (playing && havePpq)
            {
                const double beat = ppq + (double) i * beatsPerSample;
                lastPhase = (float) std::fmod (beat / cycleBeats, 1.0);
                g = mb::curveValue (curve, lastPhase);
                active = true;
            }
        }
        else
        {
            if (mode == 1)
            {
                while (midiIdx < nMidi && midiTrig[midiIdx] <= i) { samplesSinceTrigger = 0.0; ++midiIdx; }
            }
            else if (scCh > 0)
            {
                float lvl = 0.f;
                for (int c = 0; c < scCh; ++c) lvl = juce::jmax (lvl, std::abs (sc.getSample (c, i)));
                env = lvl > env ? lvl : env * envRelease;

                if (armed && env > thrLin)                         { samplesSinceTrigger = 0.0; armed = false; }
                else if (! armed && env < thrLin * 0.5f && samplesSinceTrigger > minGap) armed = true;
            }

            const double ph = samplesSinceTrigger / cycleSamples;
            if (ph < 1.0) { lastPhase = (float) ph; g = mb::curveValue (curve, lastPhase); active = true; }
            samplesSinceTrigger += 1.0;
        }

        const float duck = 1.f - mix * (1.f - g);
        smoothedGain += (duck - smoothedGain) * smoothCoef;

        for (int c = 0; c < numCh; ++c)
        {
            const float x = ch[c][i];
            float low, high;
            crossover.processSample (c, x, low, high);

            ch[c][i] = band ? (listen ? low * smoothedGain : low * smoothedGain + high)
                            : x * smoothedGain;
        }
    }

    uiPhase.store (lastPhase);
    uiActive.store (active);
}

juce::AudioProcessorEditor* MaskBassProcessor::createEditor() { return new MaskBassEditor (*this); }

void MaskBassProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void MaskBassProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MaskBassProcessor(); }
