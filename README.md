# indie_shared_dsp

Reusable DSP building blocks shared across the Indie plugin suite (by Dr Fix Audio), packaged as
a [JUCE module](https://docs.juce.com/master/tutorial_create_projucer_basic_plugin.html).

## Contents

- `OnePoleFilter` — single-pole low/high-pass filter
- `DelayLine` — fractional delay line with feedback (echo/slapback building block)
- `EnvelopeFollower` — attack/release envelope follower
- `StereoField` — stereo width/pan utilities
- `SaturationStage` — waveshaping saturation/drive stage
- `Modulators` — LFO and modulation source utilities
- `PitchShifter` — pitch shifting
- `Diffuser` — diffusion/decorrelation
- `OnsetDetector` — transient/onset detection

## Usage

Add this repo as a git submodule under `modules/` in a consuming plugin, then wire it up in
`CMakeLists.txt`:

```cmake
juce_add_module(modules/indie_shared_dsp/indie_shared_dsp)
target_link_libraries(SharedCode INTERFACE indie_shared_dsp)
```

Include the umbrella header:

```cpp
#include <indie_shared_dsp/indie_shared_dsp.h>
```

## Roadmap

This module is part of the wider Indie suite — see
[indie-doubler's docs/ROADMAP.md](https://github.com/FXDecroix/indie-doubler/blob/main/docs/ROADMAP.md)
for the multi-repo plan (indie-doubler, indie-shared-dsp, indie-plugin-template) and current
phase status.
