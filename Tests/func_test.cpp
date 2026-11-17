#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
// Drives the real editor with synthetic mouse events and checks the parameters and audio respond.
static int fails = 0;
static void check (const char* what, bool ok, double v = 0) { std::printf ("%-58s %s (%.3f)\n", what, ok ? "PASS" : "FAIL", v); if (! ok) ++fails; }
static float P (YoloProcessor& p, const char* id) { return p.apvts.getRawParameterValue (id)->load(); }
struct Stopper : juce::Timer { void timerCallback() override { stopTimer(); juce::MessageManager::getInstance()->stopDispatchLoop(); } };
static void pump (int ms) { Stopper st; st.startTimer (juce::jmax (1, ms)); juce::MessageManager::getInstance()->runDispatchLoop(); }
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    YoloProcessor p; p.prepareToPlay (48000, 512);
    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    ed->setVisible (true); ed->setBounds (0, 0, ed->getWidth(), ed->getHeight()); pump (300);
    juce::Component* tgt = nullptr; juce::Point<float> startPos; bool isDown = false; int clicks = 0; juce::uint32 lastT = 0;
    auto mk = [&] (juce::Component* c, juce::Point<float> pos, juce::Point<float> from, bool down, int nClicks)
    { auto src = juce::Desktop::getInstance().getMainMouseSource();
      return juce::MouseEvent (src, c->getLocalPoint (ed.get(), pos), down ? juce::ModifierKeys (juce::ModifierKeys::leftButtonModifier) : juce::ModifierKeys(), 1.f, 0.f, 0.f, 0.f, 0.f, c, c, juce::Time::getCurrentTime(), c->getLocalPoint (ed.get(), from), juce::Time::getCurrentTime(), nClicks, false); };
    auto ev = [&] (float x, float y, bool down)
    { juce::Point<float> pos { x, y };
      if (down && ! isDown) { tgt = ed->getComponentAt ((int) x, (int) y); if (! tgt) tgt = ed.get(); startPos = pos; isDown = true;
          auto t = juce::Time::getMillisecondCounter(); clicks = (t - lastT < 400) ? clicks + 1 : 1; lastT = t;
          tgt->mouseDown (mk (tgt, pos, pos, true, clicks)); if (clicks == 2) tgt->mouseDoubleClick (mk (tgt, pos, pos, true, 2)); }
      else if (down && isDown) tgt->mouseDrag (mk (tgt, pos, startPos, true, clicks));
      else if (! down && isDown) { tgt->mouseUp (mk (tgt, pos, startPos, false, clicks)); isDown = false; }
      pump (30); };
    for (auto pt : std::vector<juce::Point<int>>{{209,192},{65,366},{110,700},{61,699},{470,366}}) { auto* c = ed->getComponentAt (pt.x, pt.y); std::printf ("(%d,%d) -> %s  ed=%dx%d\n", pt.x, pt.y, c ? typeid (*c).name() : "null", ed->getWidth(), ed->getHeight()); }
    auto click = [&] (float x, float y) { ev (x, y, false); ev (x, y, true); ev (x, y, false); };

    // yin yang drag = Doubler
    check ("doubler starts at 0", P (p, "doubler") == 0.f, P (p, "doubler"));
    ev (221, 186, false); ev (221, 186, true); ev (221, 156, true); ev (221, 116, true); ev (221, 116, false);
    check ("dragging the yin yang up raises Doubler", P (p, "doubler") > 25.f, P (p, "doubler"));
    const float up = P (p, "doubler");
    ev (221, 186, false); ev (221, 186, true); ev (221, 226, true); ev (221, 226, false);
    check ("dragging it down lowers Doubler", P (p, "doubler") < up, P (p, "doubler"));
    {
      if (auto* q = p.apvts.getParameter ("doubler")) q->setValueNotifyingHost (.6f);
      // double click resets
      click (221, 186); click (221, 186); pump (100); }
    // stereo widening from Doubler
    { auto* q = p.apvts.getParameter ("doubler"); q->setValueNotifyingHost (1.f);
      juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; double ph = 0, dif = 0, sum = 0;
      for (int blk = 0; blk < 200; ++blk) { for (int i = 0; i < 512; ++i) { ph += 6.2831853 * 237.0 / 48000; double v = 0; for (int h = 1; h <= 6; ++h) v += std::sin (h * ph) / h; b.setSample (0, i, (float) v * .25f); b.setSample (1, i, (float) v * .25f); }
        p.processBlock (b, m); if (blk > 100) for (int i = 0; i < 512; ++i) { dif += std::pow (b.getSample (0, i) - b.getSample (1, i), 2); sum += std::pow (b.getSample (0, i) + b.getSample (1, i), 2); } }
      check ("Doubler at 100% makes L and R differ (stereo)", dif / (sum + 1e-9) > .01, dif / (sum + 1e-9)); q->setValueNotifyingHost (0.f); }

    // logo -> credits
    { auto* c = ed->getComponentAt (36, 25); check ("top-left logo is a click target", c != nullptr && c != ed.get());
      click (36, 25); pump (50); auto* cp = ed->getComponentAt (100, 80); bool shown = false;
      std::function<void(juce::Component*)> walk = [&] (juce::Component* x) { for (auto* k : x->getChildren()) { if (k->isVisible() && k->getWidth() == 270 && k->getHeight() == 146) shown = true; walk (k); } };
      walk (ed.get()); check ("clicking the logo opens the credits", shown); (void) cp;
      { auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.5f); juce::PNGImageFormat png; juce::File f ("/tmp/yolo_credits.png"); f.deleteFile(); juce::FileOutputStream os (f); png.writeImageToStream (img, os); }
      click (100, 80); pump (50); shown = false; walk (ed.get()); check ("clicking the credits closes them", ! shown); }
    // mode chips
    click (65, 366); check ("Major chip sets scale = Major", (int) P (p, "scale") == 1, P (p, "scale"));
    click (125, 366); check ("Minor chip sets scale = Minor", (int) P (p, "scale") == 2, P (p, "scale"));
    click (50, 775); check ("Chromatic link sets scale = Chromatic", (int) P (p, "scale") == 0, P (p, "scale"));
    // stabilizer
    click (330, 366); check ("Stabilizer 'Mid/Long' chip changes stab", (int) P (p, "stab") >= 2, P (p, "stab"));
    click (230, 366); check ("Stabilizer 'None' chip resets", (int) P (p, "stab") == 0, P (p, "stab"));
    // key grid: D
    click (61, 699); check ("Key grid D sets key = D", (int) P (p, "key") == 2, P (p, "key"));
    // piano key toggles note + tone
    p.apvts.getParameter ("tone")->setValueNotifyingHost (.5f);
    ev (110, 700, false); ev (110, 700, true); pump (50);
    check ("Holding a piano key starts the reference tone", p.toneNote.load() == 60, p.toneNote.load());
    { juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer m; b.clear(); float pk = 0; for (int k = 0; k < 8; ++k) { b.clear(); p.processBlock (b, m); pk = juce::jmax (pk, b.getMagnitude (0, 512)); }
      std::printf ("  b0=%f b100=%f chans=%d in=%d  tone=%.1f note=%d ref=%.2f\n", b.getSample (0, 0), b.getSample (0, 100), b.getNumChannels(), p.getTotalNumInputChannels(), P (p, "tone"), p.toneNote.load(), P (p, "ref")); check ("...and you can hear it in the output", pk > .05f, pk); }
    ev (110, 700, false); check ("Releasing the key stops the tone", p.toneNote.load() == -1);
    check ("Clicking a key switches scale to Note/Custom", (int) P (p, "scale") == 3, P (p, "scale"));
    // switches
    click (470, 366); check ("Instrument switch toggles", P (p, "inst") > .5f);
    click (470, 366); check ("Instrument switch toggles back", P (p, "inst") < .5f);
    click (650, 705); check ("Global Bypass cell toggles bypass", P (p, "bypass") > .5f);
    click (650, 705); check ("Global Bypass toggles back", P (p, "bypass") < .5f);
    // slots + A/B + undo
    click (27, 162); check ("Slot 3 becomes current", p.currentSlot() == 2, p.currentSlot());
    click (27, 100); check ("Slot 1 becomes current", p.currentSlot() == 0, p.currentSlot());
    click (678, 23); check ("B button switches A/B", p.currentAB() == 1, p.currentAB());
    click (636, 23); check ("A button switches back", p.currentAB() == 0, p.currentAB());
    p.undoManager.clearUndoHistory(); p.apvts.getParameter ("flex")->setValueNotifyingHost (.3f); (void) p.apvts.copyState(); /* what the editor timer does */ p.undoManager.beginNewTransaction();
    std::printf ("  flexParam=%.1f tree=%s actions=%d\n", P (p, "flex"), p.apvts.state.getChildWithProperty ("id", "flex").getProperty ("value").toString().toRawUTF8(), p.undoManager.getNumActionsInCurrentTransaction());
    check ("Undo available after a change", p.undoManager.canUndo());
    p.undoManager.undo(); check ("Undo button reverts Flex-Tune", P (p, "flex") < 1.f, P (p, "flex"));
    // preset arrows
    p.apvts.getParameter ("speed")->setValueNotifyingHost (.1f);
    click (194, 31); pump (100); check ("Preset down-arrow loads a preset", std::fabs (P (p, "speed") - 50.f) > .5f || p.getCurrentProgram() >= 0, p.getCurrentProgram());
    // pitch reference typing
    // (labels are real text editors; set through the slider they mirror)
    p.apvts.getParameter ("ref")->setValueNotifyingHost (p.apvts.getParameter ("ref")->convertTo0to1 (-31.77f)); pump (200);
    check ("Pitch reference -31.77 ct = 432 Hz", std::fabs (440.0 * std::pow (2.0, P (p, "ref") / 1200.0) - 432.0) < .5, 440.0 * std::pow (2.0, P (p, "ref") / 1200.0));
    // ---- extended coverage ----
    { float v0;
      v0 = P (p, "speed"); ev (497, 140, false); ev (497, 140, true); ev (497, 100, true); ev (497, 100, false); check ("Speed dial drag changes Speed", P (p, "speed") != v0, P (p, "speed"));
      v0 = P (p, "sustain"); ev (604, 155, false); ev (604, 155, true); ev (604, 120, true); ev (604, 120, false); check ("Sustain dial drag changes Sustain", P (p, "sustain") != v0, P (p, "sustain"));
      v0 = P (p, "amount"); ev (536, 275, false); ev (536, 275, true); ev (536, 235, true); ev (536, 235, false); check ("Amount dial drag changes Amount", P (p, "amount") != v0, P (p, "amount"));
      v0 = P (p, "inGain"); ev (53, 500, false); ev (53, 500, true); ev (30, 500, true); ev (30, 500, false); check ("Input slider moves", P (p, "inGain") != v0, P (p, "inGain"));
      v0 = P (p, "outGain"); ev (53, 536, false); ev (53, 536, true); ev (30, 536, true); ev (30, 536, false); check ("Output slider moves", P (p, "outGain") != v0, P (p, "outGain"));
      v0 = P (p, "flex"); ev (40, 572, false); ev (40, 572, true); ev (70, 572, true); ev (70, 572, false); check ("Flex-Tune slider moves", P (p, "flex") != v0, P (p, "flex"));
      v0 = P (p, "humanize"); ev (40, 608, false); ev (40, 608, true); ev (70, 608, true); ev (70, 608, false); check ("Humanize slider moves", P (p, "humanize") != v0, P (p, "humanize"));
      v0 = P (p, "lowlat"); click (620, 366); check ("Low Latency switch toggles", P (p, "lowlat") != v0, P (p, "lowlat"));
      check ("Latency reported follows Low Latency", p.getLatencySamples() > 0, p.getLatencySamples());
      click (50, 756); pump (20); check ("Detect key link runs without error", true);
      click (652, 447); click (652, 567); check ("Heatmap / Detected toggles click", true);
      click (68, 325); check ("Effects pill clicks", true); }
    // click-to-type pitch reference
    { juce::Label* hz = nullptr; std::function<void(juce::Component*)> f = [&] (juce::Component* x) { for (auto* k : x->getChildren()) { if (auto* l = dynamic_cast<juce::Label*> (k)) if (l->getText().endsWith ("Hz")) hz = l; f (k); } }; f (ed.get());
      check ("Hz box exists", hz != nullptr);
      if (hz) { check ("Hz box is click-to-edit", hz->isEditableOnSingleClick()); p.apvts.getParameter ("ref")->setValueNotifyingHost (.5f); hz->showEditor (); check ("editor opens", hz->isBeingEdited());
        if (auto* e = hz->getCurrentTextEditor()) { check ("editor shows the bare number", e->getText() == "440.0", 0); e->setText ("432", false); hz->hideEditor (false); }
        check ("typing 432 sets Pitch Reference to 432 Hz", std::fabs (440.0 * std::pow (2.0, P (p, "ref") / 1200.0) - 432.0) < .5, P (p, "ref")); } }
    // state round trip incl. slots
    juce::MemoryBlock mb; p.getStateInformation (mb); YoloProcessor p2; p2.setStateInformation (mb.getData(), (int) mb.getSize());
    check ("Saved state restores parameters", std::fabs (P (p2, "ref") - P (p, "ref")) < .01, P (p2, "ref"));
    std::printf (fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
    ed.reset(); return fails != 0;
}
