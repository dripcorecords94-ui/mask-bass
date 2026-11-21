#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace mbui
{
    inline const juce::Colour kBg        { 0xff151518 };
    inline const juce::Colour kPanel     { 0xff0b0b0d };
    inline const juce::Colour kAccent    { 0xffa43dff };   // morado principal
    inline const juce::Colour kAccentHi  { 0xffc98bff };   // morado claro
    inline const juce::Colour kAccentDim { 0xff5b2a8a };   // morado oscuro
    inline const juce::Colour kCell      { 0xff141417 };
    inline const juce::Colour kGrey      { 0xff8c8c96 };

    void drawPill (juce::Graphics&, juce::Rectangle<float>, const juce::String&, bool on, bool over, bool fillOff);

    // Componente generico clicable con dibujo personalizado
    struct Box : public juce::Component
    {
        using DrawFn = std::function<void (juce::Graphics&, juce::Rectangle<float>, bool, bool)>;
        DrawFn draw;
        std::function<void()> onClick;
        bool on = false, over = false;

        Box() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
        void paint (juce::Graphics& g) override { if (draw) draw (g, getLocalBounds().toFloat(), on, over); }
        void mouseEnter (const juce::MouseEvent&) override { over = true;  repaint(); }
        void mouseExit  (const juce::MouseEvent&) override { over = false; repaint(); }
        void mouseUp (const juce::MouseEvent& e) override { if (e.mouseWasClicked() && onClick) onClick(); }
        void setOn (bool v) { if (on != v) { on = v; repaint(); } }
    };

    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        int getSliderThumbRadius (juce::Slider&) override { return 10; }
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                               float startAngle, float endAngle, juce::Slider&) override;
        void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                               float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    };

    class ChoiceGroup
    {
    public:
        ChoiceGroup (juce::Component& parent, juce::RangedAudioParameter& p, const juce::StringArray& labels);
        std::vector<std::unique_ptr<Box>> boxes;
    private:
        juce::ParameterAttachment attachment;
    };

    class CurveView : public juce::Component, private juce::Timer
    {
    public:
        explicit CurveView (MaskBassProcessor& p) : proc (p) { startTimerHz (30); }
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override { repaint(); }
        MaskBassProcessor& proc;
    };

    class CurveSelector : public juce::Component
    {
    public:
        explicit CurveSelector (juce::AudioProcessorValueTreeState& apvts);
        void paint (juce::Graphics&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void step (int delta);
    private:
        juce::Rectangle<float> cellBounds (int i) const;
        int current = 2;
        juce::RangedAudioParameter& param;
        juce::ParameterAttachment attachment;
    };
}

class MaskBassEditor : public juce::AudioProcessorEditor
{
public:
    explicit MaskBassEditor (MaskBassProcessor&);
    ~MaskBassEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MaskBassProcessor& proc;
    mbui::LookAndFeel laf;

    juce::Slider mix, xover, thresh;
    mbui::ChoiceGroup rate, mode;
    mbui::Box band, listen, prevRow, prev, next, nextRow;
    mbui::CurveView view;
    mbui::CurveSelector selector;

    juce::ParameterAttachment bandAtt, listenAtt;
    juce::AudioProcessorValueTreeState::SliderAttachment mixAtt, xoverAtt, threshAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaskBassEditor)
};
