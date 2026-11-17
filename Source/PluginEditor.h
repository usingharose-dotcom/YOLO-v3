#pragma once
#include "PluginProcessor.h"

namespace yl {
    const juce::Colour bg (0xff171012), frame (0xff1d1519), panel (0xff241a1f), line (0xff3a2b32), pink (0xffffc9dd), pink2 (0xffff9fc4),
                       mute (0xff9a8a92), plum (0xff7a3558), ink (0xff0f0a0c), cream (0xfffff4f7), gold (0xffe8b86a);
}

class YoloLook : public juce::LookAndFeel_V4
{
public:
    YoloLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (11.f); }
};

class Seg : public juce::Component
{
public:
    Seg (juce::StringArray l, int columns, std::function<int()> g, std::function<void (int)> s)
        : labels (std::move (l)), cols (columns), get (std::move (g)), set (std::move (s)) {}
    juce::StringArray labels; int cols; float fs = 11.f; bool chips = false;
    std::function<int()> get; std::function<void (int)> set;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::Rectangle<float> cell (int i) const;
};

class Link : public juce::Component   // clickable text
{
public:
    Link (juce::String t, std::function<void()> c, bool boldText = true) : text (std::move (t)), click (std::move (c)), bold (boldText) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    juce::String text; std::function<void()> click; bool bold; juce::Colour col { yl::cream }; juce::Justification just { juce::Justification::centred };
    bool pill = false, pillOn = true;
    void paint (juce::Graphics& g) override { if (pill) { auto r = getLocalBounds().toFloat().reduced (.5f); g.setColour (pillOn ? yl::pink : yl::ink); g.fillRoundedRectangle (r, r.getHeight() / 2); g.setColour (pillOn ? yl::pink2 : yl::line); g.drawRoundedRectangle (r, r.getHeight() / 2, 1.f); } g.setColour (col); g.setFont (juce::FontOptions (11.f, bold ? juce::Font::bold : juce::Font::plain)); g.drawText (text, getLocalBounds(), just); }
    void mouseDown (const juce::MouseEvent&) override { if (click) click(); }
};

class IconBtn : public juce::Component
{
public:
    enum Kind { Undo, Redo, Up, Down };
    IconBtn (Kind k, std::function<void()> c) : kind (k), click (std::move (c)) {}
    Kind kind; std::function<void()> click; bool enabledLook = true;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (enabledLook && click) click(); }
};

class Piano : public juce::Component, private juce::Timer
{
public:
    Piano (YoloProcessor& p, const bool& flatsRef) : proc (p), flats (flatsRef) { startTimerHz (20); }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { proc.toneNote.store (-1); }
    int detectKey() const;                    // best major key from what has been sung, -1 if not enough data
    float heatIn[12] {}, heatOut[12] {}, keyHist[12] {};
    bool showHeat = true, showDetected = true;
    static constexpr int heatH = 34, chipH = 24, toneH = 22;
private:
    int keyAt (juce::Point<float>) const;
    juce::Rectangle<float> keyRegion() const { return getLocalBounds().toFloat().withTrimmedTop ((float) (heatH + chipH + toneH)); }
    void timerCallback() override;
    YoloProcessor& proc; const bool& flats;
};

class Scope : public juce::Component
{
public:
    explicit Scope (YoloProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
private:
    YoloProcessor& proc;
};

class Slots : public juce::Component
{
public:
    explicit Slots (YoloProcessor& p) : proc (p) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent& e) override { proc.switchSlot (juce::jlimit (0, 3, (int) (e.position.y / (getHeight() / 4.f)))); repaint(); }
private:
    YoloProcessor& proc;
};

class Scene : public juce::Component, private juce::Timer
{
public:
    explicit Scene (YoloProcessor&);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; }
    void mouseExit (const juce::MouseEvent&) override { hover = false; }
    bool effects = true;
private:
    float dragStart = 0; bool dragging = false, hover = false;
    void setDoubler (float v01);
    struct Petal { float x, y, vx, vy, rot, vr, s; };
    struct Drop { float x, y, len, v; };
    struct Spark { float x, y, ph; };
    void timerCallback() override;
    YoloProcessor& proc; float angle = 0, level = 0, t = 0;
    std::vector<Petal> petals; std::vector<Drop> rain; std::vector<Spark> sparks; juce::Random rnd;
};

class YoloEditor : public juce::AudioProcessorEditor
{
public:
    explicit YoloEditor (YoloProcessor&);
    ~YoloEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    struct Content; std::unique_ptr<Content> content;
    juce::ComponentBoundsConstrainer constrainer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YoloEditor)
};
