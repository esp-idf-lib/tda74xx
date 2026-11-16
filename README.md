# esp-idf-lib/tda74xx

[![Build examples](https://github.com/esp-idf-lib/tda74xx/actions/workflows/build.yml/badge.svg)](https://github.com/esp-idf-lib/tda74xx/actions/workflows/build.yml)
[![Build docs](https://github.com/esp-idf-lib/tda74xx/actions/workflows/build-docs.yml/badge.svg)](https://github.com/esp-idf-lib/tda74xx/actions/workflows/build-docs.yml)
[![Validation](https://github.com/esp-idf-lib/tda74xx/actions/workflows/validate-component.yml/badge.svg)](https://github.com/esp-idf-lib/tda74xx/actions/workflows/validate-component.yml)

Driver for the TDA7439, TDA7439DS and TDA7440 audioprocessors.

* [Documentation](https://esp-idf-lib.github.io/tda74xx/)
* [Repository](https://github.com/esp-idf-lib/tda74xx)
* [Issues](https://github.com/esp-idf-lib/tda74xx/issues)
* [Discussions and questions](https://github.com/esp-idf-lib/core/discussions)
* [Component page at the ESP Component Registry](https://components.espressif.com/components/esp-idf-lib/tda74xx)

## Features

The chips sit between the audio sources and the power amplifier and contain, in one package: an input selector for four stereo inputs, an input gain stage, the master volume, a three-band tone control and an attenuator for each speaker output.

- Input selector: `IN1`..`IN4`, the chip powers up with `IN2`.
- Input gain: 0 to 30 dB in 2 dB steps.
- Master volume: 0 to -47 dB in 1 dB steps, and mute.
- Bass, middle and treble: -14 to +14 dB in 2 dB steps. The TDA7440 has no middle band; the driver still writes the register, which the chip ignores.
- Speaker attenuation, left and right: 0 to -79 dB in 1 dB steps, and mute.
- The I2C address is `0x44`, fixed, and there is no way to change it.

## Wiring

| Signal | Notes |
|--------|-------|
| `SDA`, `SCL` | The I2C bus, with the usual pull-up resistors. The chip drives them as a slave only. |
| `IN1`..`IN4` | Analog inputs, AC-coupled with a capacitor. |
| `OUTL`, `OUTR` | Analog outputs to the power amplifier. |
| `VDD`, `GND` | Supply and ground. |

The component uses 100 kHz, the speed the chip is specified for. The example reads `CONFIG_EXAMPLE_I2C_MASTER_SDA` and `CONFIG_EXAMPLE_I2C_MASTER_SCL`; their defaults are the usual pins of the target (18 and 19 on ESP32, S2 and S3; 5 and 6 on the C-series).

## Volume and mute

The volume control covers 0 to -47 dB in 1 dB steps: the register counts attenuation, so a value of 1 is -1 dB and 47 is -47 dB. The codes between the last level and the mute codes are not defined by the datasheet, and the driver rejects them. Muting is a separate call, `tda74xx_set_volume_mute()`; unmuting sets the volume to 0 dB, so set the level again if it was lower.

## Tone control

The tone registers are not a plain offset. Below zero dB the register counts up from 0: 0 is -14 dB, 7 is 0 dB. Above zero dB it counts down from 0x08: 0x08 is +14 dB, 0x0e is +2 dB, and 0x0f is 0 dB again. The driver converts between decibels and the register value in both directions, and rejects values that are not a multiple of 2 dB.

## Inputs

The values of `tda74xx_input_t` are the codes the chip expects, and they run backwards: `TDA74XX_INPUT_1` is 3 and `TDA74XX_INPUT_4` is 0. Use the enum, not numbers.

## Speaker attenuation and mute

`tda74xx_set_speaker_attenuation()` takes the attenuation in dB, 0 to 79, and `tda74xx_set_speaker_mute()` mutes or unmutes one channel. Unmuting sets the attenuation to 0 dB; the previous level is not restored, so call the attenuation function afterwards if it is needed.

## API

| Function | Purpose |
|----------|---------|
| `tda74xx_init_desc()` | Fill the descriptor and create its mutex |
| `tda74xx_free_desc()` | Delete the mutex |
| `tda74xx_set_input()` | Select one of the four inputs |
| `tda74xx_set_input_gain()` | Set the input gain, 0..30 dB, 2 dB steps |
| `tda74xx_set_volume()` | Set the volume, -47..0 dB |
| `tda74xx_set_volume_mute()` | Mute or unmute the output |
| `tda74xx_set_equalizer_gain()` | Set bass, middle or treble, -14..14 dB, 2 dB steps |
| `tda74xx_set_speaker_attenuation()` | Attenuate a speaker output, 0..79 dB |
| `tda74xx_set_speaker_mute()` | Mute or unmute a speaker output |

Every descriptor has its own mutex, so calls from different tasks do not interleave. Two descriptors for the same chip are not synchronized.

## Example

`examples/default` switches the input, sets the gains and sweeps the volume from mute to 0 dB. See its own README for the wiring and the configuration.

## Installation

```sh
idf.py add-dependency esp-idf-lib/tda74xx
```

## Support

For questions and discussions about the component, please use
[Discussions](https://github.com/esp-idf-lib/core/discussions)
at [esp-idf-lib/core](https://github.com/esp-idf-lib/core).

## Contributing

Please read [CONTRIBUTING.md](https://github.com/esp-idf-lib/core/blob/main/CONTRIBUTING.md)
at [esp-idf-lib/core](https://github.com/esp-idf-lib/core).
