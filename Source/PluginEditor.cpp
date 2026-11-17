#include "PluginEditor.h"
using namespace yl;
static const char* kSharp[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
static const char* kFlat[12]  = {"C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B"};
static bool isBlack (int n) { return n==1||n==3||n==6||n==8||n==10; }
static juce::Font serif (float h, int style = juce::Font::plain) { return juce::FontOptions (juce::Font::getDefaultSerifFontName(), h, style); }
static const float PI = juce::MathConstants<float>::pi;
static float sstep (float a, float b, float x) { const float t = juce::jlimit (0.f, 1.f, (x - a) / (b - a)); return t * t * (3.f - 2.f * t); }

YoloLook::YoloLook()
{
    setColour (juce::PopupMenu::backgroundColourId, panel); setColour (juce::PopupMenu::textColourId, pink);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, plum); setColour (juce::PopupMenu::highlightedTextColourId, cream);
}

void YoloLook::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (.5f);
    g.setColour (ink); g.fillRoundedRectangle (r, 6.f); g.setColour (line); g.drawRoundedRectangle (r, 6.f, 1.f);
    juce::Path a; a.addTriangle (w - 16.f, h * .40f, w - 8.f, h * .40f, w - 12.f, h * .62f); g.setColour (mute); g.fillPath (a);
}

void YoloLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.f);
    const float R = juce::jmin (b.getWidth(), b.getHeight()) * .5f; auto c = b.getCentre();
    { const float gr = R * 1.15f; g.setGradientFill (juce::ColourGradient (pink2.withAlpha (.06f + .16f * pos), c.x, c.y, pink2.withAlpha (0.f), c.x + gr, c.y, true)); g.fillEllipse (c.x - gr, c.y - gr, 2 * gr, 2 * gr); }
    const int N = 52; const float rIn = R * .84f, rOut = R;
    for (int i = 0; i < N; ++i)
    {
        const float t = (float) i / (N - 1), a = a0 + t * (a1 - a0) - PI * .5f;
        const bool on = t <= pos + 0.001f;
        const float len = on ? (rOut - rIn) : (rOut - rIn) * .7f;
        juce::Path tk; tk.addRectangle (-R * .012f - .5f, -rOut, R * .024f + 1.f, len);
        if (on) { juce::Path gl; gl.addRectangle (-2.2f, -rOut - 1.f, 4.4f, len + 2.f); g.setColour (pink2.withAlpha (.14f)); g.fillPath (gl, juce::AffineTransform::rotation (a + PI * .5f).translated (c)); }
        g.setColour (on ? pink.interpolatedWith (pink2, t) : line.brighter (.25f));
        g.fillPath (tk, juce::AffineTransform::rotation (a + PI * .5f).translated (c));
    }
    const float rb = rIn - R * .06f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e2227), c.x, c.y - rb, ink, c.x, c.y + rb, false));
    g.fillEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb);
    g.setColour (line); g.drawEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb, 1.2f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (.12f), c.x - rb * .5f, c.y - rb * .8f, juce::Colours::transparentWhite, c.x, c.y, false));
    g.fillEllipse (c.x - rb * .86f, c.y - rb * .93f, rb * 1.4f, rb * .85f);
    const float ang = a0 + pos * (a1 - a0);
    g.setColour (cream); g.fillEllipse (c.x + std::sin (ang) * (rIn + 1) - 3.2f, c.y - std::cos (ang) * (rIn + 1) - 3.2f, 6.4f, 6.4f);
    const float big = juce::jmax (12.f, rb * .40f);
    g.setColour (cream); g.setFont (serif (big, juce::Font::bold));
    g.drawText (s.getTextFromValue (s.getValue()).upToFirstOccurrenceOf (" ", false, false), juce::Rectangle<float> (c.x - rb, c.y - big * .85f, 2 * rb, big * 1.2f), juce::Justification::centred);
    g.setColour (mute); g.setFont (juce::FontOptions (juce::jmax (8.f, rb * .15f), juce::Font::bold));
    g.drawText (s.getName(), juce::Rectangle<float> (c.x - rb, c.y + big * .42f, 2 * rb, 14.f), juce::Justification::centred);
}

void YoloLook::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const float ty = y + h - 7.f;
    if (s.getName().isNotEmpty())
    {
        g.setColour (cream.withAlpha (.85f)); g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.drawText (s.getName(), 0, y, s.getWidth() - 20, 12, juce::Justification::centredLeft);
        g.setColour (mute); g.setFont (juce::FontOptions (7.5f, juce::Font::bold));
        g.drawText (s.getTextFromValue (s.getValue()) + "%", s.getWidth() - 24, y, 24, 12, juce::Justification::centredRight);
    }
    g.setColour (line); g.fillRoundedRectangle ((float) x + 3, ty - 1.5f, (float) w - 6, 3.f, 1.5f);
    g.setGradientFill (juce::ColourGradient (pink2, (float) x, ty, pink, (float) x + w, ty, false));
    g.fillRoundedRectangle ((float) x + 3, ty - 1.5f, juce::jmax (0.f, pos - x - 3), 3.f, 1.5f);
    g.setColour (cream); g.fillEllipse (pos - 5.5f, ty - 5.5f, 11, 11);
    g.setColour (pink2); g.drawEllipse (pos - 5.5f, ty - 5.5f, 11, 11, 1.4f);
}

void YoloLook::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat(); const bool on = b.getToggleState();
    if (b.getComponentID() == "cell")
    {
        g.setColour (on ? pink2 : mute); g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        auto c = juce::Point<float> (r.getCentreX(), r.getCentreY() - 14);
        g.drawEllipse (c.x - 5, c.y - 3.5f, 10, 7, 1.2f); g.fillEllipse (c.x - 1.8f, c.y - 1.8f, 3.6f, 3.6f);
        g.drawFittedText (b.getButtonText(), r.withTrimmedTop (r.getHeight() * .5f).toNearestInt(), juce::Justification::centredTop, 2);
        return;
    }
    if (b.getComponentID() == "byp")
    {
        auto c = juce::Point<float> (r.getCentreX(), r.getCentreY() - 12);
        g.setColour (on ? pink2 : mute); juce::Path a; a.addCentredArc (c.x, c.y, 5, 5, 0, .6f, 2 * PI - .6f, true); a.startNewSubPath (c.x, c.y - 6.5f); a.lineTo (c.x, c.y - 1.f);
        g.strokePath (a, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.drawFittedText (b.getButtonText(), r.withTrimmedTop (r.getHeight() * .5f).toNearestInt(), juce::Justification::centredTop, 2);
        return;
    }
    auto sw = juce::Rectangle<float> (r.getX() + 1, r.getCentreY() - 7, 26, 14);
    g.setColour (on ? pink2 : ink); g.fillRoundedRectangle (sw, 7.f); g.setColour (on ? pink : line); g.drawRoundedRectangle (sw, 7.f, 1.2f);
    g.setColour (cream); g.fillEllipse (on ? sw.getRight() - 12.f : sw.getX() + 2.f, sw.getY() + 2.f, 10, 10);
    const juce::String sub = b.getProperties()["sub"].toString();
    auto tr = r.withTrimmedLeft (32).toNearestInt();
    g.setColour (cream); g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    auto top = tr.removeFromTop (sub.isEmpty() ? tr.getHeight() : 16);
    g.drawText (b.getButtonText(), top, juce::Justification::centredLeft);
    if (sub.isNotEmpty()) { g.setColour (pink2.withAlpha (.8f)); g.setFont (juce::FontOptions (7.5f)); g.drawText (sub, tr, juce::Justification::topLeft); }
}

// ---------- small widgets ----------
juce::Rectangle<float> Seg::cell (int i) const
{
    const int n = labels.size(), rows = (n + cols - 1) / cols;
    const float cw = (float) getWidth() / cols, ch = (float) getHeight() / rows;
    return { (i % cols) * cw, (i / cols) * ch, cw, ch };
}
void Seg::paint (juce::Graphics& g)
{
    const int sel = get ? get() : -1;
    if (! chips) { auto all = getLocalBounds().toFloat().reduced (.5f); g.setColour (ink); g.fillRoundedRectangle (all, all.getHeight() / 2); g.setColour (line); g.drawRoundedRectangle (all, all.getHeight() / 2, 1.f); }
    for (int i = 0; i < labels.size(); ++i)
    {
        auto r = cell (i).reduced (chips ? 1.5f : 2.f);
        if (i == sel) { g.setColour (pink); g.fillRoundedRectangle (r, chips ? 7.f : r.getHeight() / 2); }
        else if (chips) { g.setColour (ink); g.fillRoundedRectangle (r, 7.f); g.setColour (line); g.drawRoundedRectangle (r, 7.f, 1.f); }
        g.setColour (i == sel ? ink : cream.withAlpha (.85f)); g.setFont (juce::FontOptions (fs, juce::Font::bold));
        g.drawText (labels[i], r.toNearestInt(), juce::Justification::centred);
    }
}
void Seg::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < labels.size(); ++i) if (cell (i).contains (e.position)) { if (set) set (i); repaint(); return; }
}

void IconBtn::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (2); g.setColour (enabledLook ? cream.withAlpha (.8f) : line);
    juce::Path p;
    if (kind == Up) p.addTriangle (r.getCentreX() - 3.5f, r.getCentreY() + 1.5f, r.getCentreX() + 3.5f, r.getCentreY() + 1.5f, r.getCentreX(), r.getCentreY() - 2.5f);
    else if (kind == Down) p.addTriangle (r.getCentreX() - 3.5f, r.getCentreY() - 1.5f, r.getCentreX() + 3.5f, r.getCentreY() - 1.5f, r.getCentreX(), r.getCentreY() + 2.5f);
    else
    {
        const bool u = kind == Undo; const float d = u ? 1.f : -1.f;
        p.addCentredArc (r.getCentreX(), r.getCentreY() + 1, 5.f, 4.5f, 0, u ? -1.2f : 1.2f, u ? 2.2f : -2.2f, true);
        g.strokePath (p, juce::PathStrokeType (1.4f)); juce::Path ah;
        ah.addTriangle (r.getCentreX() - d * 6.f, r.getCentreY() - 3.f, r.getCentreX() - d * 1.5f, r.getCentreY() - 5.f, r.getCentreX() - d * 1.5f, r.getCentreY() - .5f);
        g.fillPath (ah); return;
    }
    g.fillPath (p);
}

// ---------- piano + heat ----------
static void currentMask (YoloProcessor& p, bool out[12])
{
    bool cu[12]; for (int i = 0; i < 12; ++i) cu[i] = p.apvts.getRawParameterValue ("n" + juce::String (i))->load() > .5f;
    YoloProcessor::scaleMask ((int) p.apvts.getRawParameterValue ("key")->load(), (int) p.apvts.getRawParameterValue ("scale")->load(), cu, out);
}
static juce::Rectangle<float> keyRect (juce::Rectangle<float> b, int n)
{
    static const int wi[12] = {0,-1,1,-1,2,3,-1,4,-1,5,-1,6};
    const float ww = b.getWidth() / 7.f;
    if (! isBlack (n)) return { b.getX() + wi[n] * ww, b.getY(), ww, b.getHeight() };
    return { b.getX() + (wi[n - 1] + 1) * ww - ww * .27f, b.getY(), ww * .54f, b.getHeight() * .62f };
}
void Piano::timerCallback()
{
    for (int i = 0; i < 12; ++i) { heatIn[i] *= .96f; heatOut[i] *= .96f; keyHist[i] *= .9995f; }
    if (proc.engine.voiced.load())
    {
        auto cls = [] (float m) { return ((int) std::lround (m) % 12 + 12) % 12; };
        heatIn[cls (proc.engine.detM.load())] += 1.f; heatOut[cls (proc.engine.outM.load())] += 1.f; keyHist[cls (proc.engine.detM.load())] += 1.f;
    }
    repaint();
}
int Piano::detectKey() const
{
    static const int maj[7] = {0,2,4,5,7,9,11}; float tot = 0; for (float v : keyHist) tot += v;
    if (tot < 8.f) return -1;
    int best = 0; float bs = -1;
    for (int k = 0; k < 12; ++k) { float s = 0; for (int d : maj) s += keyHist[(k + d) % 12]; if (s > bs) { bs = s; best = k; } }
    return best;
}
int Piano::keyAt (juce::Point<float> pt) const
{
    auto kb = keyRegion();
    for (int n = 0; n < 12; ++n) if (isBlack (n) && keyRect (kb, n).contains (pt)) return n;
    for (int n = 0; n < 12; ++n) if (! isBlack (n) && keyRect (kb, n).contains (pt)) return n;
    if (pt.y >= heatH && pt.y < heatH + chipH) return juce::jlimit (0, 11, (int) (pt.x / (getWidth() / 12.f)));
    return -1;
}
void Piano::mouseDown (const juce::MouseEvent& e)
{
    const int n = keyAt (e.position); if (n < 0) return;
    proc.toneNote.store (60 + n);
    bool al[12]; currentMask (proc, al);
    for (int i = 0; i < 12; ++i)
        if (auto* p = proc.apvts.getParameter ("n" + juce::String (i))) p->setValueNotifyingHost ((i == n ? ! al[i] : al[i]) ? 1.f : 0.f);
    if (auto* s = proc.apvts.getParameter ("scale")) s->setValueNotifyingHost (s->convertTo0to1 (3.f));
}
void Piano::paint (juce::Graphics& g)
{
    bool al[12]; currentMask (proc, al);
    const bool vc = proc.engine.voiced.load();
    const int act = ((int) std::lround (proc.engine.outM.load()) % 12 + 12) % 12;
    const float cw = getWidth() / 12.f;
    float mx = 6.f; for (int i = 0; i < 12; ++i) mx = juce::jmax (mx, heatIn[i], heatOut[i]);
    g.setColour (ink); g.fillRect (0, 0, getWidth(), heatH);
    for (int i = 0; i < 12; ++i)
    {
        auto col = juce::Rectangle<float> (i * cw + 3, 2, cw - 6, heatH - 4);
        g.setColour (line.withAlpha (.5f)); for (float yy = col.getY(); yy < col.getBottom(); yy += 3.f) g.fillRect (col.getX(), yy, col.getWidth(), .8f);
        if (showHeat)
        {
            const float hi = juce::jmin (1.f, heatIn[i] / mx) * (col.getHeight() * .5f - 1), ho = juce::jmin (1.f, heatOut[i] / mx) * (col.getHeight() * .5f - 1);
            g.setColour (cream); g.fillRect (col.getX(), col.getY() + col.getHeight() * .5f - 1 - hi, col.getWidth(), hi);
            g.setColour (pink2); g.fillRect (col.getX(), col.getBottom() - ho, col.getWidth(), ho);
        }
    }
    for (int i = 0; i < 12; ++i)
    {
        auto r = juce::Rectangle<float> (i * cw + 2.5f, (float) heatH + 2, cw - 5, (float) chipH - 4);
        const bool hot = showDetected && vc && i == act;
        g.setColour (hot ? pink2 : (al[i] ? cream : ink)); g.fillRoundedRectangle (r, 5.f);
        g.setColour (line); g.drawRoundedRectangle (r, 5.f, .8f);
        g.setColour (hot || al[i] ? ink : mute); g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText (flats ? kFlat[i] : kSharp[i], r.toNearestInt(), juce::Justification::centred);
    }
    auto kb = keyRegion();
    for (int pass = 0; pass < 2; ++pass)
        for (int n = 0; n < 12; ++n)
        {
            if (isBlack (n) != (pass == 1)) continue;
            auto r = keyRect (kb, n).reduced (.8f, 0);
            const bool hot = showDetected && vc && n == act;
            juce::Colour top = isBlack (n) ? juce::Colour (0xffa04770) : cream, bot = isBlack (n) ? juce::Colour (0xff6b2a4a) : pink2.brighter (.1f);
            if (! al[n]) { top = isBlack (n) ? juce::Colour (0xff2a1c22) : juce::Colour (0xff3a2a31); bot = isBlack (n) ? juce::Colour (0xff1a1216) : juce::Colour (0xff2a1d23); }
            if (hot) { top = pink; bot = pink2; }
            g.setGradientFill (juce::ColourGradient (top, 0, r.getY(), bot, 0, r.getBottom(), false));
            g.fillRoundedRectangle (r.withTrimmedTop (-5), 6.f);
            if (! isBlack (n) && al[n]) { g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (.3f), 0, r.getY(), juce::Colours::transparentWhite, 0, r.getY() + r.getHeight() * .3f, false)); g.fillRoundedRectangle (r.withTrimmedTop (-5), 6.f); }
            g.setColour (ink.withAlpha (.55f)); g.drawRoundedRectangle (r.withTrimmedTop (-5), 6.f, .8f);
        }
}

void Scope::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (ink); g.fillRoundedRectangle (r, 8.f); g.setColour (line); g.drawRoundedRectangle (r.reduced (.5f), 8.f, 1.f);
    juce::Path p; const int pos = proc.scopePos.load(); const float mid = r.getCentreY();
    for (int i = 0; i < 128; ++i)
    {
        const float v = juce::jmin (1.f, proc.scope[(size_t) ((pos + i) & 127)].load() * 2.4f) * (r.getHeight() * .42f);
        const float x = r.getX() + 4 + i * (r.getWidth() - 8) / 127.f, s = (i & 1) ? 1.f : -1.f;
        if (i == 0) p.startNewSubPath (x, mid + v * s); else p.lineTo (x, mid + v * s);
    }
    g.setColour (pink.withAlpha (.25f)); g.strokePath (p, juce::PathStrokeType (3.f));
    g.setColour (pink); g.strokePath (p, juce::PathStrokeType (1.1f));
}

void Slots::paint (juce::Graphics& g)
{
    g.setColour (ink.withAlpha (.7f)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.f);
    const float h = getHeight() / 4.f;
    for (int i = 0; i < 4; ++i)
    {
        auto r = juce::Rectangle<float> (0, i * h, (float) getWidth(), h);
        const bool sel = proc.currentSlot() == i;
        if (sel) { g.setColour (pink.withAlpha (.16f)); g.fillRect (r); g.setColour (pink2); g.fillRect (r.removeFromLeft (2.f)); }
        g.setColour (mute); g.setFont (juce::FontOptions (6.5f)); g.drawText (proc.slotUsed (i) ? "1" : "0", r.withTrimmedTop (3).removeFromTop (8).toNearestInt(), juce::Justification::centred);
        g.setColour (sel ? cream : mute); g.setFont (juce::FontOptions (11.f, juce::Font::bold)); g.drawText (juce::String (i + 1), r.withTrimmedTop (9).toNearestInt(), juce::Justification::centred);
    }
}

// ---------- credits (click the logo) ----------
struct LogoHit : juce::Component
{
    std::function<void()> onClick;
    LogoHit() { setMouseCursor (juce::MouseCursor::PointingHandCursor); setInterceptsMouseClicks (true, false); }
    void paint (juce::Graphics&) override {}           // fully transparent
    void mouseUp (const juce::MouseEvent& e) override { if (e.mouseWasClicked() || getLocalBounds().contains (e.getPosition())) { if (onClick) onClick(); } }
};
struct CreditsPanel : juce::Component
{
    CreditsPanel() { setVisible (false); }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.f);
        g.setColour (juce::Colour (0xf0180f13)); g.fillRoundedRectangle (r, 12.f);
        g.setColour (juce::Colour (0xffff5fa8).withAlpha (.6f)); g.drawRoundedRectangle (r, 12.f, 1.2f);
        g.setColour (juce::Colour (0xfff2e4d6)); g.setFont (juce::FontOptions (11.f, juce::Font::bold)); g.drawText ("MADE BY", 18, 12, 200, 14, juce::Justification::centredLeft);
        g.setFont (juce::FontOptions (16.f, juce::Font::bold));
        const char* names[] = { "Kinok9", "Ohjokr", "Razee", "17daess" };
        for (int i = 0; i < 4; ++i)
        {
            const int y = 34 + i * 26;
            g.setColour (juce::Colour (0xffff5fa8)); g.drawText (names[i], 18, y, 90, 22, juce::Justification::centredLeft);
            if (i == 3) { g.setColour (juce::Colour (0xff9c8a8f)); g.setFont (juce::FontOptions (11.f, juce::Font::italic)); g.drawText ("even tho he ain't do shi", 100, y, 200, 22, juce::Justification::centredLeft); g.setFont (juce::FontOptions (16.f, juce::Font::bold)); }
        }
    }
    void mouseDown (const juce::MouseEvent&) override { setVisible (false); }
};

// ---------- scene (flat spinning yin yang with pink effects) ----------
void Scene::setDoubler (float v01)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) p->setValueNotifyingHost (juce::jlimit (0.f, 1.f, v01));
}
void Scene::mouseDown (const juce::MouseEvent&)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { dragStart = p->getValue(); p->beginChangeGesture(); dragging = true; }
}
void Scene::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging) return;
    setDoubler (dragStart + (float) (-e.getDistanceFromDragStartY() + e.getDistanceFromDragStartX()) / 220.f);   // up or right = more
}
void Scene::mouseUp (const juce::MouseEvent&)
{
    if (dragging) { if (auto* p = proc.apvts.getParameter ("doubler")) p->endChangeGesture(); dragging = false; }
}
void Scene::mouseDoubleClick (const juce::MouseEvent&)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { p->beginChangeGesture(); p->setValueNotifyingHost (0.f); p->endChangeGesture(); }
}
void Scene::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (auto* p = proc.apvts.getParameter ("doubler")) { p->beginChangeGesture(); p->setValueNotifyingHost (juce::jlimit (0.f, 1.f, p->getValue() + w.deltaY * .25f)); p->endChangeGesture(); }
}
Scene::Scene (YoloProcessor& p) : proc (p)
{
    for (int i = 0; i < 20; ++i) petals.push_back ({ rnd.nextFloat() * 390, rnd.nextFloat() * 292, rnd.nextFloat() - .5f, .3f + rnd.nextFloat() * .6f, rnd.nextFloat() * 6, (rnd.nextFloat() - .5f) * .05f, 3.f + rnd.nextFloat() * 3.5f });
    for (int i = 0; i < 26; ++i) rain.push_back ({ rnd.nextFloat() * 390, rnd.nextFloat() * 292, 14.f + rnd.nextFloat() * 40.f, 1.2f + rnd.nextFloat() * 2.f });
    for (int i = 0; i < 14; ++i) sparks.push_back ({ 8.f + i * 28.f + rnd.nextFloat() * 14.f, 14.f + rnd.nextFloat() * 18.f, rnd.nextFloat() * 6.f });
    setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    startTimerHz (30);
}
void Scene::timerCallback()
{
    t += 1.f / 30.f;
    level += (juce::jlimit (0.f, 1.f, proc.engine.peakIn.load() * 3.f) - level) * .25f;
    angle += .008f + level * .09f;
    const float w = (float) getWidth(), h = (float) getHeight();
    if (effects)
    {
        for (auto& q : petals) { q.x += q.vx + std::sin (q.y * .03f) * .25f; q.y += q.vy; q.rot += q.vr; if (q.y > h + 10) { q.y = -10; q.x = rnd.nextFloat() * w; } if (q.x > w + 10) q.x = -10; if (q.x < -10) q.x = w; }
        for (auto& d : rain) { d.y += d.v * (1.f + level * 2.f); if (d.y - d.len > h) { d.y = -4; d.x = rnd.nextFloat() * w; } }
    }
    repaint();
}
void Scene::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat(); const juce::Point<float> c (b.getWidth() * .53f, b.getHeight() * .48f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff22171c), c.x, c.y, juce::Colour (0xff120c0f), 0, 0, true)); g.fillAll();
    if (effects)
    {
        for (auto& d : rain)
        {
            g.setGradientFill (juce::ColourGradient (pink.withAlpha (0.f), d.x, d.y - d.len, pink.withAlpha (.28f), d.x, d.y, false));
            g.fillRect (d.x, d.y - d.len, 1.f, d.len);
            g.setColour (pink.withAlpha (.5f)); g.fillEllipse (d.x - 1.2f, d.y - 1.5f, 2.4f, 3.4f);
        }
        for (auto& s : sparks) { const float a = .35f + .65f * (.5f + .5f * std::sin (t * 2.2f + s.ph)); g.setColour (gold.withAlpha (a)); g.fillEllipse (s.x - 1.4f, s.y - 1.4f, 2.8f, 2.8f); }
        for (int ring = 0; ring < 3; ++ring)
        {
            const float rr = 108.f + ring * 20.f; const int nD = 70 + ring * 26;
            for (int k = 0; k < nD; ++k)
            {
                const float a = (ring % 2 ? -1.f : 1.f) * t * .12f + k * 2.f * PI / nD;
                g.setColour (pink.withAlpha ((.25f + .35f * (k % 3 == 0)) * (1.f - ring * .22f)));
                g.fillEllipse (c.x + std::cos (a) * rr - .8f, c.y + std::sin (a) * rr - .8f, 1.6f, 1.6f);
            }
        }
        for (auto& q : petals) { juce::Path p; p.addEllipse (-q.s, -q.s * .55f, q.s * 2, q.s * 1.1f); g.setColour (pink.withAlpha (.6f)); g.fillPath (p, juce::AffineTransform::rotation (q.rot).translated (q.x, q.y)); }
    }
    const float R = 58.f;
    const float dbl = proc.apvts.getParameter ("doubler")->getValue();
    g.setColour (pink.withAlpha (.10f + level * .25f + dbl * .1f)); g.fillEllipse (c.x - 90, c.y - 90, 180, 180);
    g.setColour (ink); g.fillEllipse (c.x - 78, c.y - 78, 156, 156);
    g.setColour (pink.withAlpha (.35f)); g.drawEllipse (c.x - 78, c.y - 78, 156, 156, 3.5f);
    // doubler amount as a glowing arc around the ring
    if (dbl > 0.001f)
    {
        juce::Path arc; arc.addCentredArc (c.x, c.y, 78.f, 78.f, 0.f, -PI * .5f, -PI * .5f + dbl * 2.f * PI, true);
        g.setColour (pink2.withAlpha (.25f)); g.strokePath (arc, juce::PathStrokeType (8.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (pink); g.strokePath (arc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else { g.setColour (pink); g.drawEllipse (c.x - 78, c.y - 78, 156, 156, 3.5f); }
    auto yy = [&] (float cx, float cy, float a, float alpha)
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.addTransform (juce::AffineTransform::rotation (a, cx, cy));
        g.setOpacity (alpha);
        g.beginTransparencyLayer (alpha);
        g.setColour (cream); g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
        juce::Path half; half.addPieSegment (cx - R, cy - R, 2 * R, 2 * R, 0.f, PI, 0.f);
        g.setColour (ink); g.fillPath (half);
        g.setColour (cream); g.fillEllipse (cx - R / 2, cy - R, R, R);
        g.setColour (ink);   g.fillEllipse (cx - R / 2, cy, R, R);
        g.fillEllipse (cx - R * .12f, cy - R * .62f, R * .24f, R * .24f);
        g.setColour (cream); g.fillEllipse (cx - R * .12f, cy + R * .38f, R * .24f, R * .24f);
        g.endTransparencyLayer();
    };
    // stereo "doubled" ghosts that spread apart as the doubler goes up
    if (dbl > 0.01f) { const float off = dbl * 20.f; yy (c.x - off, c.y, angle * .9f - .15f * dbl, .38f * dbl + .1f); yy (c.x + off, c.y, angle * 1.1f + .15f * dbl, .38f * dbl + .1f); }
    yy (c.x, c.y, angle, 1.f);
    // label
    const bool show = hover || dragging || dbl > 0.001f;
    if (show)
    {
        g.setColour (juce::Colours::black.withAlpha (.45f)); g.fillRoundedRectangle (c.x - 46, c.y + 92, 92, 18, 9.f);
        g.setColour (dragging || hover ? cream : pink); g.setFont (juce::FontOptions (9.5f, juce::Font::bold));
        g.drawText ("Doubler  " + juce::String ((int) std::lround (dbl * 100.f)) + "%", c.x - 46, c.y + 92, 92, 18, juce::Justification::centred);
    }
}

// ---------- editor ----------
struct YoloEditor::Content : public juce::Component, private juce::Timer
{
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit Content (YoloProcessor& p)
        : proc (p), scene (p), piano (p, flats), scope (p), slots (p)
    {
        auto& a = proc.apvts;
        for (auto* c : std::initializer_list<juce::Component*> { &scene, &piano, &scope, &slots }) addAndMakeVisible (c);
        auto dial = [this] (juce::Slider& s, const juce::String& name, bool rotary)
        {
            s.setName (name); s.setSliderStyle (rotary ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::LinearHorizontal);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            if (rotary) s.setRotaryParameters (PI * 1.25f, PI * 2.75f, true);
            addAndMakeVisible (s);
        };
        dial (speed, "Speed (ms)", true); dial (sustain, "Sustain", true); dial (amount, "Amount", true);
        dial (inG, "Input", false); dial (outG, "Output", false); dial (flex, "Flex-Tune", false); dial (hum, "Humanize", false);
        dial (tone, "", false); dial (ref, "", false);
        aSp = std::make_unique<SA> (a, "speed", speed); aSu = std::make_unique<SA> (a, "sustain", sustain); aAm = std::make_unique<SA> (a, "amount", amount);
        aIn = std::make_unique<SA> (a, "inGain", inG); aOut = std::make_unique<SA> (a, "outGain", outG);         aFl = std::make_unique<SA> (a, "flex", flex); aHu = std::make_unique<SA> (a, "humanize", hum); aTo = std::make_unique<SA> (a, "tone", tone); aRef = std::make_unique<SA> (a, "ref", ref);

        auto par = [this] (const char* id) { return proc.apvts.getParameter (id); };
        auto idx = [par] (const char* id) { auto* p = par (id); return (int) std::lround (p->convertFrom0to1 (p->getValue())); };
        auto setIdx = [par] (const char* id, int i) { auto* p = par (id); p->setValueNotifyingHost (p->convertTo0to1 ((float) i)); };
        // Note / Major / Minor
        modeSeg = std::make_unique<Seg> (juce::StringArray {"Note", "Major", "Minor"}, 3,
            [idx] { const int s = idx ("scale"); return s == 3 ? 0 : (s == 1 ? 1 : (s == 2 ? 2 : -1)); },
            [setIdx] (int i) { setIdx ("scale", i == 0 ? 3 : i); });
        modeSeg->chips = true;
        accSeg = std::make_unique<Seg> (juce::StringArray {"#", "b"}, 2, [this] { return flats ? 1 : 0; }, [this] (int i) { flats = i == 1; repaintKeys(); });
        stabSeg = std::make_unique<Seg> (juce::StringArray {"None", "Short", "Mid", "Long"}, 4, [idx] { return idx ("stab"); }, [setIdx] (int i) { setIdx ("stab", i); });
        keySeg = std::make_unique<Seg> (juce::StringArray (kSharp, 12), 4, [idx] { return idx ("key"); }, [setIdx] (int i) { setIdx ("key", i); });
        keySeg->chips = true; keySeg->fs = 8.5f;
        for (auto* s : { modeSeg.get(), accSeg.get(), stabSeg.get(), keySeg.get() }) addAndMakeVisible (*s);

        detectLink = std::make_unique<Link> ("Detect key", [this, setIdx]
        {
            const int k = piano.detectKey(); if (k < 0) return;
            setIdx ("key", k); setIdx ("scale", 1);
        });
        chromLink = std::make_unique<Link> ("Chromatic", [setIdx] { setIdx ("scale", 0); });
        chromLink->col = pink;
        addAndMakeVisible (*detectLink); addAndMakeVisible (*chromLink);

        for (auto* t : { &inst, &lowlat, &heatT, &detT, &bypass }) addAndMakeVisible (t);
        inst.setButtonText ("Instrument"); lowlat.setButtonText ("Low Latency");
        heatT.setButtonText ("In/Out HeatMaps"); detT.setButtonText ("Detected Notes"); bypass.setButtonText ("Global Bypass");
        heatT.setComponentID ("cell"); detT.setComponentID ("cell"); bypass.setComponentID ("byp");
        heatT.setToggleState (true, juce::dontSendNotification); detT.setToggleState (true, juce::dontSendNotification);
        heatT.onClick = [this] { piano.showHeat = heatT.getToggleState(); }; detT.onClick = [this] { piano.showDetected = detT.getToggleState(); };
        aInst = std::make_unique<BA> (a, "inst", inst); aLow = std::make_unique<BA> (a, "lowlat", lowlat); aByp = std::make_unique<BA> (a, "bypass", bypass);

        // header
        for (int i = 0; i < proc.getNumPrograms(); ++i) preset.addItem (proc.getProgramName (i), i + 1);
        preset.setTextWhenNothingSelected ("No preset selected"); preset.setJustificationType (juce::Justification::centred);
        preset.setColour (juce::ComboBox::textColourId, mute);
        preset.onChange = [this] { proc.setCurrentProgram (preset.getSelectedId() - 1); };
        addAndMakeVisible (preset);
        upBtn = std::make_unique<IconBtn> (IconBtn::Up, [this] { stepPreset (-1); });
        dnBtn = std::make_unique<IconBtn> (IconBtn::Down, [this] { stepPreset (1); });
        undoBtn = std::make_unique<IconBtn> (IconBtn::Undo, [this] { proc.undoManager.undo(); });
        redoBtn = std::make_unique<IconBtn> (IconBtn::Redo, [this] { proc.undoManager.redo(); });
        for (auto* c : { (juce::Component*) upBtn.get(), (juce::Component*) dnBtn.get(), (juce::Component*) undoBtn.get(), (juce::Component*) redoBtn.get() }) addAndMakeVisible (c);
        abSeg = std::make_unique<Seg> (juce::StringArray {"A", ">", "B"}, 3, [this] { return proc.currentAB() == 0 ? 0 : 2; },
            [this] (int i) { if (i == 1) proc.copyAB(); else if ((i == 0 ? 0 : 1) != proc.currentAB()) proc.switchAB(); });
        abSeg->fs = 10.f; addAndMakeVisible (*abSeg);
        effBtn = std::make_unique<Link> ("Effects", [this] { scene.effects = ! scene.effects; effBtn->pillOn = scene.effects; effBtn->col = scene.effects ? ink : mute; repaint(); }); effBtn->col = ink; effBtn->pill = true;

        // pitch reference: Hz + cents, both clickable
        auto styleLabel = [] (juce::Label& l, juce::Component& parent)
        {
            l.setEditable (true, true, false); l.setJustificationType (juce::Justification::centred);
            l.setFont (juce::FontOptions (10.f, juce::Font::bold)); l.setColour (juce::Label::textColourId, pink); l.setMouseCursor (juce::MouseCursor::IBeamCursor); l.setTooltip ("Click and type a value, then press Enter");
            l.setColour (juce::Label::backgroundWhenEditingColourId, ink); l.setColour (juce::Label::textWhenEditingColourId, cream); l.setColour (juce::Label::outlineWhenEditingColourId, pink2);
            parent.addAndMakeVisible (l);
        };
        styleLabel (hzLabel, *this); styleLabel (ctLabel, *this);
        hzLabel.onEditorShow = [this] { if (auto* e = hzLabel.getCurrentTextEditor()) { e->setText (juce::String (440.0 * std::pow (2.0, ref.getValue() / 1200.0), 1), false); e->selectAll(); } };
        ctLabel.onEditorShow = [this] { if (auto* e = ctLabel.getCurrentTextEditor()) { e->setText (juce::String ((int) std::lround (ref.getValue())), false); e->selectAll(); } };
        hzLabel.onTextChange = [this]
        {
            const double hz = hzLabel.getText().retainCharacters ("0123456789.").getDoubleValue();
            if (hz >= 300.0 && hz <= 600.0) ref.setValue (juce::jlimit (-100.0, 100.0, 1200.0 * std::log2 (hz / 440.0)), juce::sendNotificationSync);
            refreshRef (true);
        };
        ctLabel.onTextChange = [this]
        {
            ref.setValue (juce::jlimit (-100.0, 100.0, ctLabel.getText().retainCharacters ("0123456789.-+").getDoubleValue()), juce::sendNotificationSync);
            refreshRef (true);
        };
        addAndMakeVisible (*effBtn);
        addAndMakeVisible (logoHit); addChildComponent (credits);
        logoHit.onClick = [this] { credits.setVisible (! credits.isVisible()); credits.toFront (false); };
        refreshRef (true);
        startTimerHz (10);
    }

    void stepPreset (int d)
    {
        const int n = proc.getNumPrograms(); const int cur = preset.getSelectedId() > 0 ? preset.getSelectedId() - 1 : (d > 0 ? -1 : n);
        const int nx = juce::jlimit (0, n - 1, cur + d); preset.setSelectedId (nx + 1, juce::sendNotificationSync);
    }
    void repaintKeys() { for (int i = 0; i < 12; ++i) keySeg->labels.set (i, flats ? kFlat[i] : kSharp[i]); keySeg->repaint(); piano.repaint(); }
    void refreshRef (bool force = false)
    {
        const double hz = 440.0 * std::pow (2.0, ref.getValue() / 1200.0); const int c = (int) std::lround (ref.getValue());
        if (force || ! ctLabel.isBeingEdited()) ctLabel.setText (juce::String (c) + "cts", juce::dontSendNotification);
        if (force || ! hzLabel.isBeingEdited()) hzLabel.setText (juce::String (hz, 1) + "Hz", juce::dontSendNotification);
    }
    void timerCallback() override
    {
        refreshRef();
        if ((++flushTick & 3) == 0) (void) proc.apvts.copyState();   // makes parameter edits reach the undo history promptly
        if (proc.undoManager.getNumActionsInCurrentTransaction() > 0 && ! juce::Desktop::getInstance().getMainMouseSource().isDragging()) proc.undoManager.beginNewTransaction();
        undoBtn->enabledLook = proc.undoManager.canUndo(); redoBtn->enabledLook = proc.undoManager.canRedo();
        const double ms = proc.getSampleRate() > 0 ? 1000.0 * proc.getLatencySamples() / proc.getSampleRate() : 0.0;
        lowlat.getProperties().set ("sub", juce::String (ms, 1) + " ms");
        inst.getProperties().set ("sub", inst.getToggleState() ? "instrument" : "voice");
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff120c0e));
        auto fr = getLocalBounds().toFloat().reduced (.5f);
        g.setColour (frame); g.fillRoundedRectangle (fr, 16.f);
        g.setColour (pink.withAlpha (.35f)); g.drawRoundedRectangle (fr, 16.f, 1.f);
        g.setColour (juce::Colour (0xff231a1e)); g.fillRoundedRectangle (0, 0, (float) getWidth(), 46.f, 16.f); g.fillRect (0, 24, getWidth(), 22);
        g.setColour (ink); g.fillRect (0, 45, getWidth(), 1);
        g.setGradientFill (juce::ColourGradient (pink, 24, 13, pink2, 48, 37, false)); g.fillRoundedRectangle (24, 13, 24, 24, 6.f);
        { juce::Path hp; hp.addPieSegment (29, 18, 14, 14, 0.f, PI, 0.f); g.setColour (ink); g.fillEllipse (29, 18, 14, 14); g.setColour (cream); g.fillEllipse (29, 18, 14, 14); g.setColour (ink); g.fillPath (hp); g.setColour (cream); g.fillEllipse (32.5f, 18, 7, 7); g.setColour (ink); g.fillEllipse (32.5f, 25, 7, 7); }
        g.setColour (cream); g.setFont (serif (25, juce::Font::italic)); g.drawText ("Yolo", 300, 6, 200, 34, juce::Justification::centred);
        g.setColour (line); g.fillRect (240, 24, 56, 1); g.fillRect (506, 24, 56, 1);
        // dial panel
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b2025), 0, 57, panel.darker (.3f), 0, 338, false)); g.fillRoundedRectangle (406, 57, 284, 281, 16.f);
        g.setColour (line); g.drawRoundedRectangle (406, 57, 284, 281, 16.f, 1.f);
        // rows
        g.setColour (ink.withAlpha (.5f)); g.fillRect (14, 345, 672, 1); g.fillRect (14, 384, 672, 1);
        g.setColour (mute); g.setFont (juce::FontOptions (8.f, juce::Font::bold)); g.drawText ("Note Stabilizer", 372, 355, 76, 24, juce::Justification::centred);
        g.setColour (juce::Colour (0xff1b1317)); g.fillRect (14, 386, 77, 410); g.fillRect (622, 386, 64, 410);
        g.setColour (cream.withAlpha (.8f)); g.setFont (juce::FontOptions (8.f, juce::Font::bold)); g.drawText ("Input", 18, 389, 40, 12, juce::Justification::centredLeft); g.drawText ("Output", 18, 403, 40, 12, juce::Justification::centredLeft);
        g.setFont (serif (10, juce::Font::italic | juce::Font::bold)); g.drawText ("Levels", 18, 466, 60, 13, juce::Justification::centredLeft);
        g.setColour (pink); { juce::Path cur; cur.addTriangle (46, 640, 46, 656, 56, 652); g.fillPath (cur, juce::AffineTransform::rotation (-.4f, 50, 648)); }
        g.setColour (mute); g.setFont (juce::FontOptions (8.f)); g.drawFittedText ("Click a key - " + juce::String (scaleName()), 14, 660, 77, 24, juce::Justification::centredTop, 2);
        g.setColour (line); g.fillRect (622, 507, 64, 1); g.fillRect (622, 627, 64, 1); g.fillRect (622, 747, 64, 1);
        g.setColour (cream.withAlpha (.85f)); g.setFont (juce::FontOptions (8.5f, juce::Font::bold)); g.drawText ("Pitch Reference", 622, 749, 64, 12, juce::Justification::centred);
        g.setColour (ink); g.fillRoundedRectangle (626, 769, 56, 26, 6.f); g.setColour (line); g.fillRect (630, 782, 48, 1);
        g.setColour (cream.withAlpha (.8f)); g.setFont (juce::FontOptions (8.5f, juce::Font::bold));
        g.drawText ("Tone Volume", 98, 386 + Piano::heatH + Piano::chipH + 3, 72, 16, juce::Justification::centredLeft);
        g.setColour (mute); g.drawText (juce::String ((int) std::lround (tone.getValue())) + "%", 576, 386 + Piano::heatH + Piano::chipH + 3, 40, 16, juce::Justification::centredRight);
        // status
        g.setColour (juce::Colour (0xff1b1317)); g.fillRect (14, 796, 672, 18);
        g.setColour (mute); g.setFont (juce::FontOptions (8.f));
        juce::String det = "listening";
        if (proc.engine.voiced.load())
        { auto nm = [this] (float m) { const int c = ((int) std::lround (m) % 12 + 12) % 12; return juce::String (flats ? kFlat[c] : kSharp[c]); }; det = nm (proc.engine.detM.load()) + " -> " + nm (proc.engine.outM.load()); }
        g.drawText ("Live", 22, 796, 40, 18, juce::Justification::centredLeft);
        g.drawText (juce::String (proc.getSampleRate() / 1000.0, 1) + " kHz", 66, 796, 60, 18, juce::Justification::centredLeft);
        g.drawText (juce::String (inst.getToggleState() ? "Instrument" : "Voice"), 130, 796, 70, 18, juce::Justification::centredLeft);
        g.setColour (pink2.withAlpha (.8f)); g.drawText (det, 220, 796, 120, 18, juce::Justification::centredLeft);
        g.setColour (pink.withAlpha (.8f)); g.drawText (juce::String (kSharp[(int) proc.apvts.getRawParameterValue ("key")->load()]) + " " + scaleName(), 500, 796, 110, 18, juce::Justification::centredRight);
        g.setColour (mute); g.drawText ("YOLO 1.0", 620, 796, 60, 18, juce::Justification::centredRight);
    }
    juce::String scaleName() const { static const char* n[4] = {"Chromatic", "Major", "Minor", "Custom"}; return n[juce::jlimit (0, 3, (int) proc.apvts.getRawParameterValue ("scale")->load())]; }

    void resized() override
    {
        logoHit.setBounds (24, 13, 24, 24); credits.setBounds (14, 50, 270, 146);
        preset.setBounds (54, 13, 128, 20); upBtn->setBounds (186, 13, 16, 10); dnBtn->setBounds (186, 24, 16, 10);
        undoBtn->setBounds (571, 13, 20, 20); redoBtn->setBounds (595, 13, 20, 20); abSeg->setBounds (626, 13, 62, 20);
        scene.setBounds (14, 46, 390, 292); slots.setBounds (14, 85, 26, 124); effBtn->setBounds (48, 316, 44, 18);
        speed.setBounds (417, 60, 160, 160); sustain.setBounds (569, 120, 70, 70); amount.setBounds (473, 212, 126, 126);
        modeSeg->setBounds (24, 354, 122, 24); accSeg->setBounds (158, 354, 44, 24); stabSeg->setBounds (211, 354, 160, 24);
        inst.setBounds (452, 352, 100, 28); lowlat.setBounds (566, 352, 110, 28);
        piano.setBounds (93, 386, 529, 410);
        tone.setBounds (172, 386 + Piano::heatH + Piano::chipH + 3, 400, 16);
        scope.setBounds (21, 420, 66, 40);
        int y = 488; for (auto* s : { &inG, &outG, &flex, &hum }) { s->setBounds (20, y, 66, 24); y += 36; }
        keySeg->setBounds (18, 690, 70, 54); detectLink->setBounds (14, 748, 77, 14); chromLink->setBounds (14, 768, 77, 14);
        heatT.setBounds (622, 387, 64, 120); detT.setBounds (622, 507, 64, 120); bypass.setBounds (622, 627, 64, 120);
        ref.setBounds (625, 760, 58, 8); hzLabel.setBounds (626, 769, 56, 13); ctLabel.setBounds (626, 782, 56, 13);
    }

    YoloProcessor& proc; bool flats = false;
    Scene scene; Piano piano; Scope scope; Slots slots; LogoHit logoHit; CreditsPanel credits; int flushTick = 0;
    juce::Slider speed, sustain, amount, inG, outG, flex, hum, tone, ref;
    juce::ToggleButton inst, lowlat, heatT, detT, bypass;
    juce::ComboBox preset; juce::Label hzLabel, ctLabel;
    std::unique_ptr<Seg> modeSeg, accSeg, stabSeg, keySeg, abSeg;
    std::unique_ptr<Link> detectLink, chromLink, effBtn;
    std::unique_ptr<IconBtn> upBtn, dnBtn, undoBtn, redoBtn;
    std::unique_ptr<SA> aSp, aSu, aAm, aIn, aOut, aFl, aHu, aTo, aRef;
    std::unique_ptr<BA> aInst, aLow, aByp;
};

YoloEditor::YoloEditor (YoloProcessor& p) : AudioProcessorEditor (&p)
{
    static YoloLook look; juce::ignoreUnused (look);
    content = std::make_unique<Content> (p);
    content->setLookAndFeel (&look);
    content->setSize (700, 815);
    addAndMakeVisible (*content);
    constrainer.setFixedAspectRatio (700.0 / 815.0);
    constrainer.setSizeLimits (525, 611, 1400, 1630);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (700, 815);
}
YoloEditor::~YoloEditor() { content->setLookAndFeel (nullptr); }
void YoloEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colour (0xff120c0e)); }
void YoloEditor::resized()
{
    content->setTransform (juce::AffineTransform::scale ((float) getWidth() / 700.f));
    content->setBounds (0, 0, 700, 815);
}
