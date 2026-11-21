#include "PluginEditor.h"

using namespace mbui;

static juce::Font boldFont (float h)  { return juce::Font (juce::FontOptions (h, juce::Font::bold)); }
static juce::Font plainFont (float h) { return juce::Font (juce::FontOptions (h)); }

// Trazo con brillo (capas semitransparentes, sin desenfoque = barato)
static void glowStroke (juce::Graphics& g, const juce::Path& p, float coreW, juce::Colour core)
{
    const float gw[3] = { coreW + 11.f, coreW + 7.f, coreW + 3.5f };
    const float ga[3] = { 0.06f, 0.10f, 0.18f };
    for (int i = 0; i < 3; ++i)
    {
        g.setColour (kAccent.withAlpha (ga[i]));
        g.strokePath (p, juce::PathStrokeType (gw[i], juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (core);
    g.strokePath (p, juce::PathStrokeType (coreW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

// Logo sencillo: engranaje + gota
static void drawLogo (juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto c = r.getCentre();
    const float R = r.getWidth() * 0.5f;

    juce::ColourGradient grad (kAccentHi, c.x - R, c.y - R, kAccentDim, c.x + R, c.y + R, false);

    juce::Path teeth;
    for (int i = 0; i < 8; ++i)
    {
        juce::Path t;
        t.addRoundedRectangle (c.x - R * 0.14f, c.y - R, R * 0.28f, R * 0.3f, R * 0.05f);
        teeth.addPath (t, juce::AffineTransform::rotation (juce::MathConstants<float>::twoPi * (float) i / 8.f, c.x, c.y));
    }
    juce::Path ring;
    ring.addEllipse (c.x - R * 0.86f, c.y - R * 0.86f, R * 1.72f, R * 1.72f);
    ring.addEllipse (c.x - R * 0.64f, c.y - R * 0.64f, R * 1.28f, R * 1.28f);
    ring.setUsingNonZeroWinding (false);

    g.setGradientFill (grad);
    g.fillPath (teeth);
    g.fillPath (ring);

    const float d = R * 0.46f;
    juce::Path drop;
    drop.addEllipse (c.x - 0.8f * d, c.y + 0.25f * d - 0.8f * d, 1.6f * d, 1.6f * d);
    drop.addTriangle (c.x, c.y - 1.35f * d, c.x - 0.7f * d, c.y + 0.05f * d, c.x + 0.7f * d, c.y + 0.05f * d);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9d0ff), c.x, c.y - d, kAccent, c.x, c.y + d, false));
    g.fillPath (drop);
}

void mbui::drawPill (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& text,
                     bool on, bool over, bool fillOff)
{
    auto pill = r.reduced (2.f);
    const float rad = pill.getHeight() * 0.5f;

    if (on)
    {
        g.setColour (kAccent.withAlpha (0.12f));
        g.drawRoundedRectangle (pill.expanded (2.f), rad + 2.f, 3.f);
        g.setColour (kAccent.withAlpha (0.20f));
        g.fillRoundedRectangle (pill, rad);
        g.setColour (kAccent);
        g.drawRoundedRectangle (pill, rad, 1.6f);
    }
    else if (fillOff)
    {
        g.setColour (juce::Colour (0xff17171a));
        g.fillRoundedRectangle (pill, rad);
        g.setColour (kAccentDim);
        g.drawRoundedRectangle (pill, rad, 1.3f);
    }
    else if (over)
    {
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRoundedRectangle (pill, rad);
    }

    g.setColour (on ? kAccentHi : (fillOff ? kAccent.withAlpha (0.85f) : juce::Colour (0xffc4c4cc)));
    g.setFont (boldFont (pill.getHeight() * 0.55f));
    g.drawText (text, pill, juce::Justification::centred);
}

// ---------------------------------------------------------------- LookAndFeel
void MaskLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float start, float end, juce::Slider&)
{
    auto b = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (4.f);
    const float r = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
    const auto c = b.getCentre();
    const float arcR = r - 9.f;
    const float angle = start + pos * (end - start);

    juce::Path track;  track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, start, end, true);
    g.setColour (juce::Colour (0xff1e1e24));
    g.strokePath (track, juce::PathStrokeType (11.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;    val.addCentredArc (c.x, c.y, arcR, arcR, 0.f, start, angle, true);
    glowStroke (g, val, 8.f, kAccent);

    // anillo metalico
    const float r1 = r - 27.f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb4b4bc), c.x - r1, c.y - r1,
                                             juce::Colour (0xff1e1e22), c.x + r1, c.y + r1, false));
    g.fillEllipse (c.x - r1, c.y - r1, r1 * 2.f, r1 * 2.f);

    const float r1b = r1 * 0.94f;
    g.setColour (juce::Colour (0xff111114));
    g.fillEllipse (c.x - r1b, c.y - r1b, r1b * 2.f, r1b * 2.f);

    // aro morado fino
    const float r1c = r1 * 0.87f;
    g.setColour (kAccent.withAlpha (0.25f));
    g.drawEllipse (c.x - r1c, c.y - r1c, r1c * 2.f, r1c * 2.f, 4.f);
    g.setColour (kAccent);
    g.drawEllipse (c.x - r1c, c.y - r1c, r1c * 2.f, r1c * 2.f, 1.5f);

    // cara cepillada
    const float r2 = r1 * 0.80f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff36363c), c.x, c.y,
                                             juce::Colour (0xff0d0d0f), c.x + r2, c.y, true));
    g.fillEllipse (c.x - r2, c.y - r2, r2 * 2.f, r2 * 2.f);

    g.setColour (juce::Colours::white.withAlpha (0.035f));
    for (int i = 0; i < 48; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / 48.f;
        g.drawLine (c.x + std::sin (a) * r2 * 0.08f, c.y - std::cos (a) * r2 * 0.08f,
                    c.x + std::sin (a) * r2,         c.y - std::cos (a) * r2, 1.f);
    }

    // indicador
    const float pr = r2 * 0.74f;
    const float px = c.x + std::sin (angle) * pr, py = c.y - std::cos (angle) * pr;
    g.setColour (kAccent.withAlpha (0.25f));
    g.fillEllipse (px - 9.f, py - 9.f, 18.f, 18.f);
    g.setColour (kAccentHi);
    g.fillEllipse (px - 5.f, py - 5.f, 10.f, 10.f);
}

void MaskLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                        float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto track = b.withSizeKeepingCentre (b.getWidth(), b.getHeight() * 0.64f);
    const float rad = track.getHeight() * 0.5f;

    g.setColour (juce::Colour (0xff0d0d10));
    g.fillRoundedRectangle (track, rad);
    g.setColour (juce::Colour (0xff2a2a30));
    g.drawRoundedRectangle (track.reduced (0.5f), rad, 1.f);

    auto filled = track.withRight (sliderPos);
    g.setColour (kAccent.withAlpha (0.22f));
    g.fillRoundedRectangle (filled.expanded (2.f), rad + 2.f);
    g.setGradientFill (juce::ColourGradient (kAccentDim, filled.getX(), 0.f, kAccent, filled.getRight(), 0.f, false));
    g.fillRoundedRectangle (filled, rad);

    // perilla metalica
    const float tw = 18.f, th = b.getHeight() * 1.5f;
    auto t = juce::Rectangle<float> (tw, th).withCentre ({ sliderPos, b.getCentreY() });
    juce::ColourGradient mg (juce::Colour (0xff45454c), t.getX(), 0.f, juce::Colour (0xff45454c), t.getRight(), 0.f, false);
    mg.addColour (0.5, juce::Colour (0xffc6c6ce));
    g.setGradientFill (mg);
    g.fillRoundedRectangle (t, 3.f);
    g.setColour (juce::Colour (0xff101012));
    g.drawRoundedRectangle (t, 3.f, 1.f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    for (int i = -1; i <= 1; ++i)
        g.drawLine (t.getCentreX() + (float) i * 4.f, t.getY() + 5.f, t.getCentreX() + (float) i * 4.f, t.getBottom() - 5.f, 1.2f);
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

    // marco metalico
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff77777e), 0.f, 0.f,
                                             juce::Colour (0xff1f1f23), 0.f, b.getHeight(), false));
    g.fillRoundedRectangle (b, 9.f);
    auto in = b.reduced (5.f);
    g.setColour (kPanel);
    g.fillRoundedRectangle (in, 6.f);

    g.saveState();
    g.reduceClipRegion (in.toNearestInt());

    // rejilla
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 1; i < 8; ++i)
        g.drawVerticalLine ((int) (in.getX() + (float) i * in.getWidth() / 8.f), in.getY(), in.getBottom());
    for (int i = 1; i < 4; ++i)
        g.drawHorizontalLine ((int) (in.getY() + (float) i * in.getHeight() / 4.f), in.getX(), in.getRight());

    // onda de entrada (real)
    {
        const int n = MaskBassProcessor::kScope;
        const int wr = proc.scopeWrite.load();
        const float colW = in.getWidth() / (float) n;
        const float midY = in.getCentreY();
        const float half = in.getHeight() * 0.40f;
        g.setColour (juce::Colour (0xffcfc8dc).withAlpha (0.42f));
        for (int i = 0; i < n; ++i)
        {
            const float v = std::sqrt (juce::jlimit (0.f, 1.f, proc.scope[(size_t) ((wr + i) % n)].load()));
            const float hh = v * half;
            g.fillRect (in.getX() + (float) i * colW, midY - hh, juce::jmax (1.f, colW - 0.4f), hh * 2.f);
        }
    }

    // curva
    const int   idx = (int) proc.apvts.getRawParameterValue ("curve")->load();
    const float mix = proc.apvts.getRawParameterValue ("mix")->load();
    const float top = in.getY() + 14.f, bot = in.getBottom() - 14.f;

    juce::Path line, area;
    const int steps = juce::jmax (2, (int) in.getWidth() / 2);
    for (int i = 0; i <= steps; ++i)
    {
        const float t = (float) i / (float) steps;
        const float gain = mb::curveValue (idx, t);
        const float duck = 1.f - mix * (1.f - gain);
        const float xx = in.getX() + t * in.getWidth();
        const float yy = bot - duck * (bot - top);
        if (i == 0) { line.startNewSubPath (xx, yy); area.startNewSubPath (xx, yy); }
        else        { line.lineTo (xx, yy);          area.lineTo (xx, yy); }
    }
    area.lineTo (in.getRight(), in.getBottom());
    area.lineTo (in.getX(), in.getBottom());
    area.closeSubPath();

    g.setGradientFill (juce::ColourGradient (kAccent.withAlpha (0.50f), 0.f, top,
                                             kAccent.withAlpha (0.06f), 0.f, bot, false));
    g.fillPath (area);
    glowStroke (g, line, 3.f, juce::Colour (0xffc060ff));

    // cabezal
    if (proc.uiActive.load())
    {
        const float xx = in.getX() + proc.uiPhase.load() * in.getWidth();
        g.setColour (juce::Colours::black.withAlpha (0.50f));
        g.fillRect (xx, in.getY(), in.getRight() - xx, in.getHeight());
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.fillRect (xx - 1.f, in.getY(), 2.f, in.getHeight());
    }

    g.restoreState();
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

        if (sel)
        {
            g.setColour (kAccent.withAlpha (0.25f));
            g.fillRoundedRectangle (r.expanded (2.f), 6.f);
            g.setColour (kAccent);
            g.fillRoundedRectangle (r, 4.f);
        }
        else
        {
            g.setColour (kCell);
            g.fillRoundedRectangle (r, 4.f);
            g.setColour (juce::Colour (0xff2a2a30));
            g.drawRoundedRectangle (r.reduced (0.5f), 4.f, 1.f);
        }

        auto a = r.reduced (r.getWidth() * 0.16f, r.getHeight() * 0.22f);
        juce::Path p;
        for (int k = 0; k <= 30; ++k)
        {
            const float t = (float) k / 30.f;
            const float xx = a.getX() + t * a.getWidth();
            const float yy = a.getBottom() - mb::curveValue (i, t) * a.getHeight();
            if (k == 0) p.startNewSubPath (xx, yy); else p.lineTo (xx, yy);
        }
        g.setColour (sel ? juce::Colour (0xff0b0b0d) : juce::Colour (0xff9a62d6));
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
        g.setGradientFill (juce::ColourGradient (over ? juce::Colour (0xff5a5a62) : juce::Colour (0xff4a4a50), 0.f, r.getY(),
                                                 juce::Colour (0xff27272b), 0.f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 5.f);
        g.setColour (juce::Colour (0xff6c6c74));
        g.drawRoundedRectangle (r.reduced (0.5f), 5.f, 1.f);

        const auto c = r.getCentre();
        const float s = left ? 1.f : -1.f;
        juce::Path p;
        g.setColour (juce::Colour (0xffbcbcc6));
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

    band.draw = [] (juce::Graphics& g, juce::Rectangle<float> r, bool on, bool over)
                { drawPill (g, r, "BAND", on, over, true); };
    band.onClick = [this] { bandAtt.setValueAsCompleteGesture (band.on ? 0.f : 1.f); };

    listen.draw = [] (juce::Graphics& g, juce::Rectangle<float> r, bool on, bool)
    {
        const auto c = r.getCentre();
        g.setColour (on ? kAccentHi : juce::Colour (0xff8f72b8));
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
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1e1e23), 0.f, 0.f,
                                             juce::Colour (0xff111113), 0.f, (float) getHeight(), false));
    g.fillAll();

    // logo y titulo
    drawLogo (g, { 24.f, 28.f, 62.f, 62.f });
    g.setColour (juce::Colour (0xffa88bd6));
    g.setFont (plainFont (23.f));
    g.drawText ("DRIP INDUSTRIES", 100, 30, 320, 26, juce::Justification::left);
    g.setColour (kAccent);
    g.setFont (boldFont (50.f));
    g.drawFittedText ("MASKBASS", 98, 54, 320, 56, juce::Justification::left, 1);

    // MIX
    juce::AttributedString as;
    as.setJustification (juce::Justification::centred);
    as.append ("MIX ", boldFont (34.f), juce::Colour (0xfff0f0f5));
    as.append (juce::String (juce::roundToInt (mix.getValue() * 100.0)) + "%", boldFont (34.f), kAccentHi);
    as.draw (g, juce::Rectangle<float> (62.f, 462.f, 308.f, 44.f));

    // frecuencia de corte
    g.setColour (juce::Colour (0xffb9a6d8));
    g.setFont (boldFont (18.f));
    g.drawText (juce::String ((int) xover.getValue()) + " Hz", 184, 528, 140, 38, juce::Justification::left);

    // umbral
    g.setColour (kGrey);
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
