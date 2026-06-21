#pragma once

#include <juce_dsp/juce_dsp.h>

namespace indie
{

/**
    Mono fractional delay line with optional feedback.

    A circular buffer, sized once in prepare() for a maximum delay time. Input is written
    at the head; reads tap `delaySamples` behind it with linear interpolation, so the delay
    time can be set — and modulated per sample — in fractional samples without zipper steps.
    An optional feedback coefficient routes the (caller-supplied) delayed signal back into
    the line, which is what turns a single echo into a decaying train of repeats.

    This is deliberately just the storage-and-tap primitive. Tone shaping / damping,
    saturation, pitch wobble, stereo placement and dry/wet mixing all belong in the
    consumer — for a tape/analog slapback the consumer typically filters and saturates the
    delayed signal before handing it back as the feedback sample, so each successive repeat
    gets progressively darker and warmer.

    Realtime-safe: the buffer is allocated once in prepare(); the per-sample read()/write()
    do only scalar math and never allocate or grow.
*/
class DelayLine
{
public:
    DelayLine() = default;

    /** Allocates the buffer for up to `maxDelaySeconds` of delay at `newSampleRate`. */
    void prepare (double newSampleRate, double maxDelaySeconds)
    {
        sampleRate = newSampleRate;
        // +4 guard samples so the interpolation read just past the longest tap stays in range.
        bufferLength = juce::jmax (4, (int) std::ceil (maxDelaySeconds * sampleRate) + 4);
        buffer.assign ((size_t) bufferLength, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    /** Sets the base delay time in seconds, clamped to what prepare() allocated. */
    void setDelaySeconds (float seconds)
    {
        setDelaySamples ((float) (seconds * sampleRate));
    }

    /** Sets the base delay time directly in (fractional) samples. */
    void setDelaySamples (float samples)
    {
        // Stay >= 1 sample (never read the slot we're about to overwrite) and leave room
        // for the +1 interpolation tap at the far end.
        delaySamples = juce::jlimit (1.0f, (float) (bufferLength - 2), samples);
    }

    /** Feedback fraction folded back into the line. 0 = a single tap (one echo);
        approaching 1 = a long decaying train. Clamped below 1 to stay stable. */
    void setFeedback (float newFeedback)
    {
        feedback = juce::jlimit (0.0f, 0.999f, newFeedback);
    }

    /** Reads the current base tap (fractional, linearly interpolated) without advancing. */
    float read() const
    {
        return readAt (delaySamples);
    }

    /** Reads `samples` behind the write head (fractional, interpolated) without advancing.
        Lets the consumer add per-sample modulation/wobble on top of the base delay time. */
    float readAt (float samples) const
    {
        float pos = (float) writePos - juce::jlimit (1.0f, (float) (bufferLength - 2), samples);
        while (pos < 0.0f)
            pos += (float) bufferLength;

        const int   i0   = (int) pos;
        const float frac = pos - (float) i0;
        const int   i1   = (i0 + 1) % bufferLength;

        return buffer[(size_t) i0] + frac * (buffer[(size_t) i1] - buffer[(size_t) i0]);
    }

    /** Writes one input sample and advances the write head. `feedbackSample` is the
        (possibly tone-shaped/saturated) delayed signal to fold back in — pass 0 for a
        pure single tap, or the delayed output for a feedback train. */
    void write (float x, float feedbackSample)
    {
        buffer[(size_t) writePos] = x + feedback * feedbackSample;
        if (++writePos >= bufferLength)
            writePos = 0;
    }

    /** Convenience: the standard tap-then-feed-back-then-write in one call. Reads the base
        tap, folds it straight back, writes the input and returns the delayed sample. Use
        the separate read()/readAt()/write() calls when the feedback path needs shaping. */
    float processSample (float x)
    {
        const float delayed = read();
        write (x, delayed);
        return delayed;
    }

private:
    double sampleRate = 44100.0;

    std::vector<float> buffer;
    int   bufferLength = 1;

    int   writePos     = 0;
    float delaySamples = 1.0f;
    float feedback     = 0.0f;
};

} // namespace indie
