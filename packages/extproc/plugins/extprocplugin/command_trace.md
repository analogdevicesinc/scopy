# Scopy - extproc CLI command trace

### Newline-delimited JSON exchanged over the CLI child process:
- REQUEST = written to the CLI stdin
- RESPONSE = read from the CLI stdout
- Bulk sample data is exchanged separately via memory-mapped `data.in`/`data.out`
and is not shown here

### Commands used in this protocol:
- `get_analysis_types` - query the CLI for the analysis types it can perform.
- `get_analysis_info` - get the full description of one analysis type (parameters, measurements, output channels, plots).
- `set_input_config` - configure how the CLI reads the input samples (`data.in`, format, channels, sample count, sampling frequency).
- `set_output_config` - configure where and how the CLI writes results (`data.out`, format, enabled analyses).
- `set_analysis_config` - set the user-chosen values for the analysis parameters before running.
- `run` - trigger one processing pass (read `data.in`, run analysis, write `data.out`, return measurements).

## Session started 2026-07-03 15:39:37.076

### REQUEST - GET_ANALYSIS_TYPES  15:39:37.082

Query the CLI for the analysis types it is able to perform.

```json
{
    "command": "get_analysis_types"
}
```

### RESPONSE - GET_ANALYSIS_TYPES  15:39:37.084

The CLI replies with the list of analysis types it supports (here just `test`).


```json
{
    "command": "get_analysis_types",
    "status": "success",
    "supported_types": [
        "test"
    ]
}
```

### REQUEST - GET_ANALYSIS_INFO  15:39:37.084

Ask the CLI for the full description of one analysis type: its configurable
parameters, the measurements it produces, the output channels it generates, and
how those channels should be plotted.

```json
{
    "analysis_type": "test",
    "command": "get_analysis_info"
}
```

### RESPONSE - GET_ANALYSIS_INFO  15:39:37.085

The CLI describes the `test` analysis: its `parameters` (gain, samples_size),
the `measurements` it computes, the `output_info` (channel count, formats and
names) and the `plot_info` telling Scopy which plots to build and what to draw
on each.

```json
{
    "analysis_type": "test",
    "command": "get_analysis_info",
    "measurements": [
        "peak_power",
        "snr"
    ],
    "output_info": {
        "channel_count": 3,
        "channel_format": [
            "float32",
            "float32",
            "float32"
        ],
        "channel_names": [
            "add_ch",
            "sub_ch",
            "gain_ch"
        ]
    },
    "parameters": {
        "gain": {
            "default": 1,
            "description": "Gain",
            "type": "int"
        },
        "samples_size": {
            "default": 1024,
            "description": "Number of samples to process",
            "type": "int"
        }
    },
    "plot_info": [
        {
            "ch": [
                [
                    "time",
                    "add_ch"
                ],
                [
                    "time",
                    "sub_ch"
                ]
            ],
            "flags": [
                "labels"
            ],
            "id": 1,
            "title": "Add, Sub",
            "type": "plot",
            "xLabel": "time[s]",
            "yLabel": "amplitude[V]"
        },
        {
            "ch": [
                [
                    "input0",
                    "input1"
                ]
            ],
            "flags": [
                "points",
                "labels"
            ],
            "id": 2,
            "title": "X-Y",
            "type": "plot",
            "xLabel": "amplitude[V]",
            "yLabel": "amplitude[V]"
        },
        {
            "ch": [
                [
                    "time",
                    "gain_ch"
                ]
            ],
            "flags": [
                "labels"
            ],
            "id": 3,
            "title": "Mul",
            "type": "plot",
            "xLabel": "time[s]",
            "yLabel": "amplitude[V]"
        },
        {
            "ch": [
                [
                    "fft_frequency",
                    "fft_magnitude_db"
                ]
            ],
            "flags": [
                "labels"
            ],
            "id": 4,
            "title": "FFT",
            "type": "plot",
            "xLabel": "frequency[Hz]",
            "yLabel": "magnitude[dB]"
        }
    ],
    "status": "success"
}
```

### REQUEST - SET_INPUT_CONFIG  15:40:07.250

Tell the CLI where and how to read the input samples: the memory-mapped
`data.in` file, its format, the number and format of channels, the sample count
and the sampling frequency.

```json
{
    "command": "set_input_config",
    "config": {
        "channel_count": 1,
        "channel_format": [
            "float32"
        ],
        "frequency_offset": 0,
        "input_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.in",
        "input_file_format": "binary-interleaved",
        "sample_count": 1024,
        "sampling_frequency": 30719999
    }
}
```

### RESPONSE - SET_INPUT_CONFIG  15:40:07.253

The CLI echoes back the accepted input configuration to confirm it was applied.

```json
{
    "command": "set_input_config",
    "config": {
        "channel_count": 1,
        "channel_format": [
            "float32"
        ],
        "frequency_offset": 0,
        "input_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.in",
        "input_file_format": "binary-interleaved",
        "sample_count": 1024,
        "sampling_frequency": 30719999
    },
    "status": "success"
}
```

### REQUEST - SET_OUTPUT_CONFIG  15:40:07.253

Tell the CLI where to write its results (the memory-mapped `data.out` file), in
which format, and which analyses should be enabled to produce that output.

```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    }
}
```

### RESPONSE - SET_OUTPUT_CONFIG  15:40:07.253

The CLI echoes back the accepted output configuration to confirm it was applied.

```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    },
    "status": "success"
}
```

### REQUEST - SET_INPUT_CONFIG  15:40:08.056
```json
{
    "command": "set_input_config",
    "config": {
        "channel_count": 2,
        "channel_format": [
            "float32",
            "float32"
        ],
        "frequency_offset": 0,
        "input_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.in",
        "input_file_format": "binary-interleaved",
        "sample_count": 1024,
        "sampling_frequency": 30719999
    }
}
```

### RESPONSE - SET_INPUT_CONFIG  15:40:08.058
```json
{
    "command": "set_input_config",
    "config": {
        "channel_count": 2,
        "channel_format": [
            "float32",
            "float32"
        ],
        "frequency_offset": 0,
        "input_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.in",
        "input_file_format": "binary-interleaved",
        "sample_count": 1024,
        "sampling_frequency": 30719999
    },
    "status": "success"
}
```

### REQUEST - SET_OUTPUT_CONFIG  15:40:08.058
```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    }
}
```

### RESPONSE - SET_OUTPUT_CONFIG  15:40:08.059
```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    },
    "status": "success"
}
```

### REQUEST - SET_ANALYSIS_CONFIG  15:40:12.811

Send the values chosen by the user for the analysis parameters (gain,
samples_size) so the CLI configures the selected analysis before running.

```json
{
    "analysis_type": "test",
    "command": "set_analysis_config",
    "config": {
        "gain": "2",
        "samples_size": "1024"
    }
}
```

### RESPONSE - SET_ANALYSIS_CONFIG  15:40:12.812

The CLI confirms the applied parameters and returns the resulting `output_info`
(the channels, formats and names the analysis will produce with this config).

```json
{
    "analysis_type": "test",
    "command": "set_analysis_config",
    "config": {
        "gain": 2,
        "samples_size": 1024
    },
    "output_info": {
        "channel_count": 3,
        "channel_format": [
            "int16",
            "int16",
            "int16"
        ],
        "channel_names": [
            "add_ch",
            "sub_ch",
            "gain_ch"
        ]
    },
    "status": "success"
}
```

### REQUEST - SET_OUTPUT_CONFIG  15:40:12.812
```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    }
}
```

### RESPONSE - SET_OUTPUT_CONFIG  15:40:12.812
```json
{
    "command": "set_output_config",
    "config": {
        "enabled_analysis": [
            "test"
        ],
        "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
        "output_file_format": "binary-interleaved"
    },
    "status": "success"
}
```

### REQUEST - RUN  15:40:16.627

Trigger one processing pass: the CLI reads the samples from `data.in`, runs the
configured analysis, and writes the results to `data.out`. Sent repeatedly for
continuous/streaming acquisition.

```json
{
    "command": "run"
}
```

### RESPONSE - RUN  15:40:16.627

The CLI reports completion: the computed `measurements`, the `output_file` that
now holds the processed samples, and a `results` block with the sample offset,
size and status.

```json
{
    "command": "run",
    "measurements": {
        "peak_power": {
            "channel": 0,
            "units": "dBm",
            "value": 10.5
        },
        "snr": {
            "channel": 1,
            "units": "dB",
            "value": 25.3
        }
    },
    "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
    "results": {
        "offset": 0,
        "samples_size": 1024,
        "status": "completed"
    },
    "status": "success"
}
```

### REQUEST - RUN  15:40:16.635
```json
{
    "command": "run"
}
```

### RESPONSE - RUN  15:40:16.637
```json
{
    "command": "run",
    "measurements": {
        "peak_power": {
            "channel": 0,
            "units": "dBm",
            "value": 10.5
        },
        "snr": {
            "channel": 1,
            "units": "dB",
            "value": 25.3
        }
    },
    "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
    "results": {
        "offset": 0,
        "samples_size": 1024,
        "status": "completed"
    },
    "status": "success"
}
```

### REQUEST - RUN  15:40:16.640
```json
{
    "command": "run"
}
```

### RESPONSE - RUN  15:40:16.640
```json
{
    "command": "run",
    "measurements": {
        "peak_power": {
            "channel": 0,
            "units": "dBm",
            "value": 10.5
        },
        "snr": {
            "channel": 1,
            "units": "dB",
            "value": 25.3
        }
    },
    "output_file": "/home/andrei/git_repositories/scopy-dev/scopy/build/Desktop_Qt_5_15_2_GCC_64bit-Debug/data.out",
    "results": {
        "offset": 0,
        "samples_size": 1024,
        "status": "completed"
    },
    "status": "success"
}
```
