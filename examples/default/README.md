# Example for the `tda74xx` driver

The datasheets are:

* [TDA7439](https://www.st.com/resource/en/datasheet/tda7439.pdf)
* [TDA7439DS](https://www.st.com/resource/en/datasheet/tda7439ds.pdf)
* [TDA7440](https://www.st.com/resource/en/datasheet/tda7440.pdf)

## What it does

The example configures a TDA7439, TDA7439DS or TDA7440: it selects the input,
sets the input gain, the bass, the middle and the treble, then sweeps the
volume from the mute level to 0 dB and back.

The example prints the values as it sets them.

## Wiring

Connect `SCL` and `SDA` to the pins below with the usual pull-up resistors.

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_EXAMPLE_I2C_MASTER_SCL` | GPIO number for `SCL` | 6 on the C-series, 19 on `esp32`, `esp32s2` and `esp32s3` |
| `CONFIG_EXAMPLE_I2C_MASTER_SDA` | GPIO number for `SDA` | 5 on the C-series, 18 on `esp32`, `esp32s2` and `esp32s3` |

Connect the audio source to one of the four inputs and the outputs to the
power amplifier. The inputs and the outputs are AC-coupled, so the capacitors
belong to the board of the chip, not to the example.

## Configuration

`idf.py menuconfig` → `Example configuration`:

| Option | Meaning | Default |
|--------|---------|---------|
| `EXAMPLE_I2C_MASTER_SCL`, `EXAMPLE_I2C_MASTER_SDA` | Pins of the I2C bus | Per target |
| `EXAMPLE_INPUT_CHANNEL` | Input, 1..4, as it is marked on the board | 2 |
| `EXAMPLE_INPUT_GAIN` | Input gain, 0..30 dB in 2 dB steps | 28 |
| `EXAMPLE_VOLUME` | Volume, -47..0 dB | 0 |
| `EXAMPLE_BASS`, `EXAMPLE_MIDDLE`, `EXAMPLE_TREBLE` | Tone, -14..14 dB in 2 dB steps | 0 |
| `EXAMPLE_USING_MIDDLE_RANGE_CONTROL` | Write the middle band; TDA7439DS only | On |

The chip powers up with `IN2`, 28 dB of input gain, the volume muted and a
flat tone control, which is what the defaults of the example repeat.

## Build and run

```sh
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Expected output

```
I (300) TDA74XX: Volume changed to 0 dB
I (300) TDA74XX: Volume changed to -1 dB
...
I (300) TDA74XX: Volume changed to -47 dB
```

## Notes

* The address is `0x44` and cannot be changed, so there can be only one
  TDA74xx on a bus.
* The TDA7440 has no middle band. The register is written anyway and the chip
  ignores it, so switch `EXAMPLE_USING_MIDDLE_RANGE_CONTROL` off to keep the example honest.
* Values that are not a multiple of 2 dB are rejected with
  `ESP_ERR_INVALID_ARG`, both for the gain and for the tone.
