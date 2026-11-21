#include "PluginEditor.h"

using namespace mbui;

static juce::Font boldFont (float h) { return juce::Font (juce::FontOptions (h, juce::Font::bold)); }

void mbui::drawPill (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& text,
                     bool on, bool over, bool fillOff)
{
    if (on)                    { g.setColour (kYellow); g.fillRoundedRectangle (r, r.getHeight() * 0.5f); }
    else if (over || fillOff)  { g.setColour (kCell);   g.fillRoundedRectangle (r, r.getHeight() * 0.5f); }

    g.setColour (on ? juce::Colour (0xff111111) : kGrey);
    g.setFont (boldFont (r.getHeight() * 0.55f));
    g.drawText (text, r, juce::Justification::centred);
}

// ---------------------------------------------------------------- LookAndFeel
void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                    float start, float end, juce::Slider&)
{
    auto b = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float arcR = r - 8.f;
    const float angle = start + pos * (end - start);

    juce::Path track;  track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, start, end, true);
    g.setColour (juce::Colour (0xff262626));
    g.strokePath (track, juce::PathStrokeType (12.f));

    juce::Path val;    val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, start, angle, true);
    g.setColour (kYellow);
    g.strokePath (val, juce::PathStrokeType (12.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto disc = [&] (float rad, juce::uint32 col)
    {
        g.setColour (juce::Colour (col));
        g.fillEllipse (c.x - rad, c.y - rad, rad * 2.f, rad * 2.f);
    };
    const float r1 = r - 28.f, r2 = r1 * 0.84f, r3 = r2 * 0.88f, r4 = r3 * 0.35f;
    disc (r1, 0xff2f2f2f);
    disc (r2, 0xff232323);
    disc (r3, 0xff333333);
    disc (r4, 0xff1b1b1b);

    const float dx = std::sin (angle), dy = -std::cos (angle);
    const float pr = r3 * 0.8f;
    g.setColour (juce::Colour (0xff8a8a8a));
    g.fillEllipse (c.x + dx * pr - 7.f, c.y + dy * pr - 7.f, 14.f, 14.f);
}

void LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                    float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto track = b.withSizeKeepingCentre (b.getWidth(), b.getHeight() * 0.75f);

    g.setColour (kCell);
    g.fillRoundedRectangle (track, track.getHeight() * 0.5f);

    auto filled = track.withRight (sliderPos);
    g.setColour (kYellow);
    g.fillRoundedRectangle (filled, track.getHeight() * 0.5f);

    const float d = b.getHeight() * 1.25f;
    const float cy = b.getCentreY();
    g.setColour (juce::Colour (0xff6a6a6a));
    g.fillEllipse (sliderPos - d * 0.5f, cy - d * 0.5f, d, d);
    g.setColour (kBg);
    g.fillEllipse (sliderPos - d * 0.14f, cy - d * 0.14f, d * 0.28f, d * 0.28f);
}

// ---------------------------------------------------------------- ChoiceGroup
ChoiceGroup::ChoiceGroup (juce::Component& parent, juce::RangedAudioParameter& p, const juce::StringArray& labels)
    : attachment (p, [this] (float v)
      {
          const int idx = (int) std::lround (v);
          for (int i = 0; i < (int) boxes.size(); ++i) boxes[(size_t) i]->setOn (i == idx);
      })
{
    for (int i = 0; i < labels.size(); ++i)
    {
        auto b = std::make_unique<Box>();
        const juce::String text = labels[i];
        b->draw = [text] (juce::Graphics& g, juce::Rectangle<float> r, bool on, bool over)
                  { drawPill (g, r, text, on, over, false); };
        b->onClick = [this, i] { attachment.setValueAsCompleteGesture ((float) i); };
        parent.addAndMakeVisible (*b);
        boxes.push_back (std::move (b));
    }
    attachment.sendInitialUpdate();
}

// ---------------------------------------------------------------- CurveView
void CurveView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (kPanel);
    g.fillRoundedRectangle (b, 6.f);

    const int   idx = (int) proc.apvts.getRawParameterValue ("curve")->load();
    const float mix = proc.apvts.getRawParameterValue ("mix")->load();
    const float top = 14.f, bot = b.getHeight() - 14.f;

    juce::Path p;
    for (int px = 0; px <= (int) b.getWidth(); px += 2)
    {
        const float gain = mb::curveValue (idx, (float) px / b.getWidth());
        const float duck = 1.f - mix * (1.f - gain);
        const float y = bot - duck * (bot - top);
        if (px == 0) p.startNewSubPath (0.f, y); else p.lineTo ((float) px, y);
    }
    g.setColour (kYellow);
    g.strokePath (p, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (proc.uiActive.load())
    {
        const float x = proc.uiPhase.load() * b.getWidth();
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRect (x, 0.f, b.getWidth() - x, b.getHeight());
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.fillRect (x - 1.f, 0.f, 2.f, b.getHeight());
    }
}

// ---------------------------------------------------------------- CurveSelector
CurveSelector::CurveSelector (juce::AudioProcessorValueTreeState& a)
    : param (*a.getParameter ("curve")),
      attachment (param, [this] (float v) { current = (int) std::lround (v); repaint(); })
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    attachment.sendInitialUpdate();
}

juce::Rectangle<float> CurveSelector::cellBounds (int i) const
{
    const float gap = 6.f;
    const float cw = ((float) getWidth() - 7.f * gap) / 8.f;
    const float ch = ((float) getHeight() - gap) / 2.f;
    const int col = i % 8, row = i / 8;
    return { (float) col * (cw + gap), (float) row * (ch + gap), cw, ch };
}

void CurveSelector::paint (juce::Graphics& g)
{
    for (int i = 0; i < mb::kNumCurves; ++i)
    {
        auto r = cellBounds (i);
        const bool sel = (i == current);
        g.setColour (sel ? kYellow : kCell);
        g.fillRoundedRectangle (r, 4.f);

        auto a = r.reduced (r.getWidth() * 0.16f, r.getHeight() * 0.22f);
        juce::Path p;
        for (int k = 0; k <= 30; ++k)
        {
            const float t = (float) k / 30.f;
            const float x = a.getX() + t * a.getWidth();
            const float y = a.getBottom() - mb::curveValue (i, t) * a.getHeight();
            if (k == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (sel ? juce::Colour (0xff111111) : juce::Colour (0xff7a7a7a));
        g.strokePath (p, juce::PathStrokeType (2.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

void CurveSelector::mouseUp (const juce::MouseEvent& e)
{
    if (! e.mouseWasClicked()) return;
    for (int i = 0; i < mb::kNumCurves; ++i)
        if (cellBounds (i).contains (e.position))
        {
            attachment.setValueAsCompleteGesture ((float) i);
            return;
        }
}

void CurveSelector::step (int delta)
{
    int n = (current + delta) % mb::kNumCurves;
    if (n < 0) n += mb::kNumCurves;
    attachment.setValueAsCompleteGesture ((float) n);
}

// ---------------------------------------------------------------- Editor
static Box::DrawFn arrowDraw (bool outer, bool left)
{
    return [outer, left] (juce::Graphics& g, juce::Rectangle<float> r, bool, bool over)
    {
        g.setColour (over ? juce::Colour (0xff343434) : kCell);
        g.fillRoundedRectangle (r, 4.f);
        const auto c = r.getCentre();
        const float s = left ? 1.f : -1.f;
        juce::Path p;
        g.setColour (kGrey);
        if (outer)
        {
            p.addTriangle (c.x + 9.f * s, c.y - 12.f, c.x - 9.f * s, c.y, c.x + 9.f * s, c.y + 12.f);
            g.fillPath (p);
        }
        else
        {
            p.startNewSubPath (c.x + 6.f * s, c.y - 12.f);
            p.lineTo (c.x - 6.f * s, c.y);
            p.lineTo (c.x + 6.f * s, c.y + 12.f);
            g.strokePath (p, juce::PathStrokeType (3.f));
        }
    };
}

MaskBassEditor::MaskBassEditor (MaskBassProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p),
      rate (*this, *p.apvts.getParameter ("rate"), { "1/8", "1/4", "1/2", "1/1" }),
      mode (*this, *p.apvts.getParameter ("mode"), { "SYNC", "MIDI", "AUDIO" }),
      view (p), selector (p.apvts),
      bandAtt   (*p.apvts.getParameter ("band"),   [this] (float v) { band.setOn (v > 0.5f); }),
      listenAtt (*p.apvts.getParameter ("listen"), [this] (float v) { listen.setOn (v > 0.5f); }),
      mixAtt (p.apvts, "mix", mix), xoverAtt (p.apvts, "xover", xover), threshAtt (p.apvts, "thresh", thresh)
{
    // knob
    mix.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    mix.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mix.setRotaryParameters (juce::degreesToRadians (210.f), juce::degreesToRadians (510.f), true);
    mix.setLookAndFeel (&laf);
    mix.onValueChange = [this] { repaint(); };

    for (auto* s : { &xover, &thresh })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s->setLookAndFeel (&laf);
        s->onValueChange = [this] { repaint(); };
    }

    // botones
    band.draw = [] (juce::Graphics& g, juce::Rectangle<float> r, bool on, bool over)
                { drawPill (g, r, "BAND", on, over, true); };
    band.onClick = [this] { bandAtt.setValueAsCompleteGesture (band.on ? 0.f : 1.f); };

    listen.draw = [] (juce::Graphics& g, juce::Rectangle<float> r, bool on, bool)
    {
        const auto c = r.getCentre();
        g.setColour (on ? kYellow : kGrey);
        juce::Path arc;
        arc.addCentredArc (c.x, c.y + 2.f, 11.f, 11.f, 0.f, -juce::MathConstants<float>::halfPi,
                           juce::MathConstants<float>::halfPi, true);
        g.strokePath (arc, juce::PathStrokeType (3.f));
        g.fillRoundedRectangle (c.x - 15.f, c.y + 1.f, 7.f, 12.f, 3.f);
        g.fillRoundedRectangle (c.x + 8.f,  c.y + 1.f, 7.f, 12.f, 3.f);
    };
    listen.onClick = [this] { listenAtt.setValueAsCompleteGesture (listen.on ? 0.f : 1.f); };

    prevRow.draw = arrowDraw (true,  true);   prevRow.onClick = [this] { selector.step (-8); };
    prev.draw    = arrowDraw (false, true);   prev.onClick    = [this] { selector.step (-1); };
    next.draw    = arrowDraw (false, false);  next.onClick    = [this] { selector.step (1);  };
    nextRow.draw = arrowDraw (true,  false);  nextRow.onClick = [this] { selector.step (8);  };

    for (auto* c : std::initializer_list<juce::Component*> { &mix, &xover, &thresh, &band, &listen,
                                                              &prevRow, &prev, &next, &nextRow, &view, &selector })
        addAndMakeVisible (*c);

    bandAtt.sendInitialUpdate();
    listenAtt.sendInitialUpdate();
    setSize (1208, 616);
}

MaskBassEditor::~MaskBassEditor()
{
    mix.setLookAndFeel (nullptr);
    xover.setLookAndFeel (nullptr);
    thresh.setLookAndFeel (nullptr);
}

void MaskBassEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBg);

    g.setColour (juce::Colour (0xff4a4a4a));
    g.setFont (boldFont (22.f));
    g.drawText ("SIDECHAIN SHAPER", 62, 34, 320, 26, juce::Justification::left);

    g.setColour (kYellow);
    g.setFont (boldFont (54.f));
    g.drawFittedText ("MASK BASS", 60, 60, 320, 66, juce::Justification::left, 1);

    g.setColour (kGrey);
    g.setFont (boldFont (34.f));
    g.drawText ("MIX " + juce::String (juce::roundToInt (mix.getValue() * 100.0)) + "%",
                62, 464, 308, 40, juce::Justification::centred);

    g.setFont (boldFont (18.f));
    g.drawText (juce::String ((int) xover.getValue()) + " Hz", 184, 528, 140, 38, juce::Justification::left);

    g.setFont (boldFont (15.f));
    g.drawText ("AUDIO TRIGGER THRESHOLD  " + juce::String (thresh.getValue(), 1) + " dB",
                630, 434, 366, 20, juce::Justification::centred);
}

void MaskBassEditor::resized()
{
    mix.setBounds (62, 148, 308, 308);
    band.setBounds (22, 528, 100, 38);
    listen.setBounds (134, 533, 36, 28);
    xover.setBounds (22, 580, 382, 22);

    for (int i = 0; i < (int) rate.boxes.size(); ++i) rate.boxes[(size_t) i]->setBounds (432 + i * 78, 16, 74, 38);
    for (int i = 0; i < (int) mode.boxes.size(); ++i) mode.boxes[(size_t) i]->setBounds (790 + i * 102, 16, 98, 38);

    view.setBounds (432, 66, 762, 358);

    prevRow.setBounds (432, 436, 90, 40);
    prev.setBounds    (528, 436, 90, 40);
    next.setBounds    (1008, 436, 90, 40);
    nextRow.setBounds (1104, 436, 90, 40);
    thresh.setBounds  (650, 458, 326, 18);

    selector.setBounds (432, 492, 762, 110);
}
