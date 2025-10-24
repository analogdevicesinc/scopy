/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #define MAP_FAILED NULL
    #define PROT_READ 0x1
    #define PROT_WRITE 0x2
    #define MAP_PRIVATE 0x01
    #define MAP_SHARED 0x02

    typedef long long ssize_t;
    // Windows doesn't have getline, so we provide an implementation
    ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#else
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
#endif

#include "cjson/cJSON.h"

typedef struct {
    char input_file[256];
    int sampling_frequency;
    int frequency_offset;
    int channel_count;
    char input_file_format[64];
    char **channel_format;
    int sample_count;
} InputConfig;

typedef struct {
    char analysis_type[32];
    int samples_size;
    int gain;
} AnalysisConfig;

typedef struct {
    char output_file[256];
    char output_file_format[64];
    char **enabled_analysis;
    int enabled_analysis_count;
} OutputConfig;

typedef struct {
    InputConfig input_config;
    AnalysisConfig analysis_config;
    OutputConfig output_config;
    void *readMap;
    void *writeMap;
    size_t inputFileSize;
#ifdef _WIN32
    HANDLE inputFileHandle;
    HANDLE inputMapHandle;
    HANDLE outputFileHandle;
    HANDLE outputMapHandle;
#else
    int inputFd;
    int outputFd;
#endif
    int configured;
} AppState;

AppState g_state = {0};

#ifdef _WIN32

// Windows implementation of getline
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL) {
        return -1;
    }

    if (*lineptr == NULL) {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL) {
            return -1;
        }
    }

    ssize_t pos = 0;
    int c;
    
    while ((c = fgetc(stream)) != EOF) {
        if (pos >= (ssize_t)(*n - 1)) {
            *n *= 2;
            char *new_ptr = realloc(*lineptr, *n);
            if (new_ptr == NULL) {
                return -1;
            }
            *lineptr = new_ptr;
        }
        
        (*lineptr)[pos++] = c;
        
        if (c == '\n') {
            break;
        }
    }
    
    if (pos == 0 && c == EOF) {
        return -1;
    }
    
    (*lineptr)[pos] = '\0';
    return pos;
}

void* map_file_for_reading(const char* path, size_t* size, HANDLE* file_handle, HANDLE* map_handle) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[READ] The file cannot be opened: %s (Error: %lu)\n", path, GetLastError());
        return MAP_FAILED;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        fprintf(stderr, "GetFileSizeEx error: %lu\n", GetLastError());
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    *size = (size_t)fileSize.QuadPart;
    
    if (*size == 0) {
        fprintf(stderr, "[READ] File is empty: %s\n", path);
        CloseHandle(hFile);
        return MAP_FAILED;
    }
    
    HANDLE hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMapFile == NULL) {
        fprintf(stderr, "[READ] The file cannot be mapped (Error: %lu)\n", GetLastError());
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    void* addr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, *size);
    if (addr == NULL) {
        fprintf(stderr, "[READ] MapViewOfFile failed (Error: %lu)\n", GetLastError());
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    if (file_handle) *file_handle = hFile;
    if (map_handle) *map_handle = hMapFile;
    
    return addr;
}

void* map_file_for_writing(const char* path, size_t size, HANDLE* file_handle, HANDLE* map_handle) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, 
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[WRITE] The file cannot be opened: %s (Error: %lu)\n", path, GetLastError());
        return MAP_FAILED;
    }

    // Set file size properly for large files
    LARGE_INTEGER fileSize;
    fileSize.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(hFile, fileSize, NULL, FILE_BEGIN)) {
        fprintf(stderr, "[WRITE] SetFilePointerEx error: %lu\n", GetLastError());
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    // For large files, we need to specify the high and low parts separately
    DWORD dwMaximumSizeHigh = (DWORD)(size >> 32);
    DWORD dwMaximumSizeLow = (DWORD)(size & 0xFFFFFFFF);
    
    HANDLE hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 
                                        dwMaximumSizeHigh, dwMaximumSizeLow, NULL);
    if (hMapFile == NULL) {
        fprintf(stderr, "[WRITE] The file cannot be mapped (Error: %lu)\n", GetLastError());
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    void* addr = MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
    if (addr == NULL) {
        fprintf(stderr, "[WRITE] MapViewOfFile failed (Error: %lu)\n", GetLastError());
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return MAP_FAILED;
    }

    if (file_handle) *file_handle = hFile;
    if (map_handle) *map_handle = hMapFile;
    
    return addr;
}

void cleanup_mapping(void* addr, HANDLE map_handle, HANDLE file_handle) {
    if (addr && addr != MAP_FAILED) {
        if (!UnmapViewOfFile(addr)) {
            fprintf(stderr, "UnmapViewOfFile failed (Error: %lu)\n", GetLastError());
        }
    }
    if (map_handle && map_handle != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(map_handle)) {
            fprintf(stderr, "CloseHandle (map) failed (Error: %lu)\n", GetLastError());
        }
    }
    if (file_handle && file_handle != INVALID_HANDLE_VALUE) {
        // Force flush before closing
        FlushFileBuffers(file_handle);
        if (!CloseHandle(file_handle)) {
            fprintf(stderr, "CloseHandle (file) failed (Error: %lu)\n", GetLastError());
        }
    }
}

#else
// Linux implementations (unchanged)
void* map_file_for_reading(const char* path, size_t* size, int* fd_out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[READ] The file cannot be opened: %s\n", path);
        return MAP_FAILED;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "fstat error\n");
        close(fd);
        return MAP_FAILED;
    }

    *size = sb.st_size;
    void* addr = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "[READ] The file cannot be mapped\n");
        close(fd);
        return MAP_FAILED;
    }

    if (fd_out) {
        *fd_out = fd;
    } else {
        close(fd);
    }
    return addr;
}

void* map_file_for_writing(const char* path, size_t size, int* fd_out) {
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        fprintf(stderr, "[WRITE] The file cannot be opened: %s\n", path);
        return MAP_FAILED;
    }

    if (ftruncate(fd, size) == -1) {
        fprintf(stderr, "[WRITE] Truncate error\n");
        close(fd);
        return MAP_FAILED;
    }

    void* addr = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "[WRITE] The file cannot be mapped\n");
        close(fd);
        return MAP_FAILED;
    }

    if (fd_out) {
        *fd_out = fd;
    } else {
        close(fd);
    }
    return addr;
}
#endif

// Channel format parsing functions (unchanged)
int parse_channel_format(const char *format, int *bits, int *is_signed, int *shift) {
    *bits = 16; // default
    *is_signed = 0; // default unsigned
    *shift = 0; // default no shift
    
    if (strstr(format, "u12")) {
        *bits = 12;
        *is_signed = 0;
    } else if (strstr(format, "s16")) {
        *bits = 16;
        *is_signed = 1;
    } else if (strstr(format, "u16")) {
        *bits = 16;
        *is_signed = 0;
    }
    
    // Extract shift if present
    char *shift_pos = strstr(format, ">>");
    if (shift_pos) {
        *shift = atoi(shift_pos + 2);
    }
    
    return 0;
}

float convert_sample(void *data, int sample_idx, int channel_idx, const char *format) {
    (void)format; // channel format parsing not needed for raw float data

    if (sample_idx < 0 || channel_idx < 0 || channel_idx >= g_state.input_config.channel_count) {
        return 0.0f;
    }

    float *samples = (float *)data;
    size_t total_samples = g_state.inputFileSize / sizeof(float);
    size_t sample_offset = (size_t)sample_idx * (size_t)g_state.input_config.channel_count + (size_t)channel_idx;

    if (sample_offset >= total_samples) {
        return 0.0f;
    }

    return samples[sample_offset];
}

void processData() {
    if (!g_state.readMap || !g_state.writeMap) {
        fprintf(stderr, "Error: Files not mapped\n");
        return;
    }
    
    size_t samplesPerChnl;
    size_t inputSamplesPerChnl = 0;
    if (g_state.input_config.channel_count > 0) {
        inputSamplesPerChnl = g_state.inputFileSize / (g_state.input_config.channel_count * sizeof(float));
    }
    if (g_state.analysis_config.samples_size > 0 && g_state.analysis_config.samples_size <= (int)inputSamplesPerChnl) {
        samplesPerChnl = (size_t)g_state.analysis_config.samples_size;
    } else {
        samplesPerChnl = inputSamplesPerChnl;
    }
    
    float *writeMap = (float *)g_state.writeMap;
    
    for (size_t i = 0; i < samplesPerChnl; ++i) {
        float *currentWrite = writeMap + (i * 3); 
        
        // Operation 1: Addition
        float sum = 0.0f;
        for (int j = 0; j < g_state.input_config.channel_count; j++) {
            float sample = convert_sample(g_state.readMap, (int)i, j, 
                                          g_state.input_config.channel_format ? g_state.input_config.channel_format[j] : NULL);
            sum += sample;
        }
        currentWrite[0] = sum;
        
        float sub = convert_sample(g_state.readMap, (int)i, 0, 
                                   g_state.input_config.channel_format ? g_state.input_config.channel_format[0] : NULL);
        for (int j = 1; j < g_state.input_config.channel_count; j++) {
            float sample = convert_sample(g_state.readMap, (int)i, j, 
                                          g_state.input_config.channel_format ? g_state.input_config.channel_format[j] : NULL);
            sub -= sample;
        }
        currentWrite[1] = sub;
        
        float mul = 1.0f;
        if (g_state.input_config.channel_count >= 1) {
            float sample0 = convert_sample(g_state.readMap, (int)i, 0, 
                                           g_state.input_config.channel_format ? g_state.input_config.channel_format[0] : NULL);
            mul = sample0 * (float)g_state.analysis_config.gain;
        }
        currentWrite[2] = mul;
    }
}

void send_json_response(cJSON *response) {
    char *json_string = cJSON_Print(response);
    if (json_string) {
        printf("%s\n", json_string);
        fflush(stdout);
        free(json_string);
    }
    cJSON_Delete(response);
}

void handle_set_input_config(cJSON *request) {
    cJSON *config = cJSON_GetObjectItem(request, "config");
    if (!config) {
        fprintf(stderr, "Error: Missing config in set_input_config\n");
        return;
    }

    // Parse input config
    cJSON *input_file = cJSON_GetObjectItem(config, "input_file");
    cJSON *sampling_freq = cJSON_GetObjectItem(config, "sampling_frequency");
    cJSON *freq_offset = cJSON_GetObjectItem(config, "frequency_offset");
    cJSON *channel_count = cJSON_GetObjectItem(config, "channel_count");
    cJSON *input_format = cJSON_GetObjectItem(config, "input_file_format");
    cJSON *channel_format = cJSON_GetObjectItem(config, "channel_format");
    cJSON *sample_count = cJSON_GetObjectItem(config, "sample_count");

    if (input_file) strcpy(g_state.input_config.input_file, input_file->valuestring);
    if (sampling_freq) g_state.input_config.sampling_frequency = sampling_freq->valueint;
    if (freq_offset) g_state.input_config.frequency_offset = freq_offset->valueint;
    if (channel_count) g_state.input_config.channel_count = channel_count->valueint;
    if (input_format) strcpy(g_state.input_config.input_file_format, input_format->valuestring);
    if (sample_count) g_state.input_config.sample_count = sample_count->valueint;

    // Parse channel formats
    if (channel_format && cJSON_IsArray(channel_format)) {
        int array_size = cJSON_GetArraySize(channel_format);
        g_state.input_config.channel_format = malloc(array_size * sizeof(char*));
        for (int i = 0; i < array_size; i++) {
            cJSON *format_item = cJSON_GetArrayItem(channel_format, i);
            g_state.input_config.channel_format[i] = malloc(64);
            strcpy(g_state.input_config.channel_format[i], format_item->valuestring);
        }
    }

    // Map input file
    if (g_state.readMap && g_state.readMap != MAP_FAILED) {
#ifdef _WIN32
        cleanup_mapping(g_state.readMap, g_state.inputMapHandle, g_state.inputFileHandle);
#else
        munmap(g_state.readMap, g_state.inputFileSize);
        close(g_state.inputFd);
#endif
    }
    
#ifdef _WIN32
    g_state.readMap = map_file_for_reading(g_state.input_config.input_file, 
                                          &g_state.inputFileSize, 
                                          &g_state.inputFileHandle,
                                          &g_state.inputMapHandle);
#else
    g_state.readMap = map_file_for_reading(g_state.input_config.input_file, 
                                          &g_state.inputFileSize, &g_state.inputFd);
#endif

    if (!g_state.readMap) {
        fprintf(stderr, "Error: map file for reading: %d\n", g_state.inputFileSize);
        return;
    }

    // Create response
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "set_input_config");
    cJSON_AddItemToObject(response, "config", cJSON_Duplicate(config, 1));
    
    send_json_response(response);
}

void handle_set_analysis_config(cJSON *request) {
    cJSON *analysis_type = cJSON_GetObjectItem(request, "analysis_type");
    cJSON *config = cJSON_GetObjectItem(request, "config");

    if (analysis_type) strcpy(g_state.analysis_config.analysis_type, analysis_type->valuestring);
    
    if (config) {
        cJSON *samples_size = cJSON_GetObjectItem(config, "samples_size");
        cJSON *gain_val = cJSON_GetObjectItem(config, "gain");
        
        if (samples_size) {
            if (samples_size->valuestring) {
                g_state.analysis_config.samples_size = atoi(samples_size->valuestring);
            }
        }
        
        if (gain_val) {
            if (gain_val->valuestring) {
                g_state.analysis_config.gain = atoi(gain_val->valuestring);
            }
        }
    }

    // Create response
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "set_analysis_config");
    if (analysis_type) cJSON_AddStringToObject(response, "analysis_type", analysis_type->valuestring);
    
    // Add config with possibly modified freq_offset
    cJSON *resp_config = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp_config, "samples_size", g_state.analysis_config.samples_size);
    cJSON_AddNumberToObject(resp_config, "gain", 1); // Dummy modified value
    cJSON_AddItemToObject(response, "config", resp_config);
    
    // Add output info
    cJSON *output_info = cJSON_CreateObject();
    cJSON_AddNumberToObject(output_info, "channel_count", 3);
    
    cJSON *channel_names = cJSON_CreateArray();
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("add_ch"));
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("sub_ch"));
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("gain_ch"));
    cJSON_AddItemToObject(output_info, "channel_names", channel_names);
    
    cJSON *channel_formats = cJSON_CreateArray();
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("int16"));
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("int16"));
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("int16"));
    cJSON_AddItemToObject(output_info, "channel_format", channel_formats);
    
    cJSON_AddItemToObject(response, "output_info", output_info);
    
    send_json_response(response);
}

void handle_set_output_config(cJSON *request) {
    cJSON *config = cJSON_GetObjectItem(request, "config");
    if (!config) {
        fprintf(stderr, "Error: Missing config in set_output_config\n");
        return;
    }

    cJSON *output_file = cJSON_GetObjectItem(config, "output_file");
    cJSON *output_format = cJSON_GetObjectItem(config, "output_file_format");
    cJSON *enabled_analysis = cJSON_GetObjectItem(config, "enabled_analysis");

    if (output_file) strcpy(g_state.output_config.output_file, output_file->valuestring);
    if (output_format) strcpy(g_state.output_config.output_file_format, output_format->valuestring);

    if (enabled_analysis && cJSON_IsArray(enabled_analysis)) {
        int array_size = cJSON_GetArraySize(enabled_analysis);
        g_state.output_config.enabled_analysis = malloc(array_size * sizeof(char*));
        g_state.output_config.enabled_analysis_count = array_size;
        for (int i = 0; i < array_size; i++) {
            cJSON *analysis_item = cJSON_GetArrayItem(enabled_analysis, i);
            g_state.output_config.enabled_analysis[i] = malloc(64);
            strcpy(g_state.output_config.enabled_analysis[i], analysis_item->valuestring);
        }
    }

    size_t samplesPerChnl;
    if (g_state.analysis_config.samples_size > 0) {
        samplesPerChnl = g_state.analysis_config.samples_size;
    } else if (g_state.input_config.channel_count > 0) {
        samplesPerChnl = g_state.inputFileSize / (g_state.input_config.channel_count * sizeof(float));
    } else {
        samplesPerChnl = 0;
    }
    
    size_t output_size = samplesPerChnl * 3 * sizeof(float);
    
    if (g_state.writeMap && g_state.writeMap != MAP_FAILED) {
#ifdef _WIN32
        cleanup_mapping(g_state.writeMap, g_state.outputMapHandle, g_state.outputFileHandle);
#else
        munmap(g_state.writeMap, output_size);
        close(g_state.outputFd);
#endif
    }

#ifdef _WIN32
    g_state.writeMap = map_file_for_writing(g_state.output_config.output_file, 
                                           output_size,
                                           &g_state.outputFileHandle,
                                           &g_state.outputMapHandle);
#else
    g_state.writeMap = map_file_for_writing(g_state.output_config.output_file, 
                                           output_size, &g_state.outputFd);
#endif

    g_state.configured = 1;

    // Create response
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "set_output_config");
    cJSON_AddItemToObject(response, "config", cJSON_Duplicate(config, 1));
    
    send_json_response(response);
}

void handle_run(cJSON *request) {
    if (!g_state.readMap) {
        fprintf(stderr, "Error: read map\n");
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "command", "run");
        cJSON_AddStringToObject(response, "message", "System not configured");
        send_json_response(response);
        return;
    }

    if (!g_state.writeMap) {
        fprintf(stderr, "Error: write map\n");
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "command", "run");
        cJSON_AddStringToObject(response, "message", "System not configured");
        send_json_response(response);
        return;
    }

    if (!g_state.configured) {
        fprintf(stderr, "Error: System not properly configured\n");
        cJSON *response = cJSON_CreateObject();
        cJSON_AddStringToObject(response, "status", "error");
        cJSON_AddStringToObject(response, "command", "run");
        cJSON_AddStringToObject(response, "message", "System not configured");
        send_json_response(response);
        return;
    }

    // Process the data
    processData();

    // Create response
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "run");
    cJSON_AddStringToObject(response, "output_file", g_state.output_config.output_file);
    
    cJSON *results = cJSON_CreateObject();
    cJSON_AddStringToObject(results, "status", "completed");
    cJSON_AddNumberToObject(results, "offset", 0);
    cJSON_AddNumberToObject(results, "samples_size", g_state.analysis_config.samples_size);
    cJSON_AddItemToObject(response, "results", results);
    
    // Add dummy measurements
    cJSON *measurements = cJSON_CreateObject();
    
    cJSON *peak_power = cJSON_CreateObject();
    cJSON_AddNumberToObject(peak_power, "value", 10.5);
    cJSON_AddStringToObject(peak_power, "units", "dBm");
    cJSON_AddNumberToObject(peak_power, "channel", 0);
    cJSON_AddItemToObject(measurements, "peak_power", peak_power);
    
    cJSON *snr = cJSON_CreateObject();
    cJSON_AddNumberToObject(snr, "value", 25.3);
    cJSON_AddStringToObject(snr, "units", "dB");
    cJSON_AddNumberToObject(snr, "channel", 1);
    cJSON_AddItemToObject(measurements, "snr", snr);
    
    cJSON_AddItemToObject(response, "measurements", measurements);
    
    send_json_response(response);
}

void handle_get_analysis_types(cJSON *request) {
    (void)request; // Suppress unused parameter warning
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "get_analysis_types");
    
    cJSON *supported_types = cJSON_CreateArray();
    cJSON_AddItemToArray(supported_types, cJSON_CreateString("test"));
    cJSON_AddItemToObject(response, "supported_types", supported_types);
    
    send_json_response(response);
}

void handle_get_analysis_info(cJSON *request) {
    cJSON *analysis_type = cJSON_GetObjectItem(request, "analysis_type");
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "status", "success");
    cJSON_AddStringToObject(response, "command", "get_analysis_info");
    if (analysis_type) cJSON_AddStringToObject(response, "analysis_type", analysis_type->valuestring);
    
    // Add parameters
    cJSON *parameters = cJSON_CreateObject();
    
    cJSON *samples_size_param = cJSON_CreateObject();
    cJSON_AddStringToObject(samples_size_param, "type", "int");
    cJSON_AddNumberToObject(samples_size_param, "default", 1024);
    cJSON_AddStringToObject(samples_size_param, "description", "Number of samples to process");
    cJSON_AddItemToObject(parameters, "samples_size", samples_size_param);
    
    cJSON *freq_offset_param = cJSON_CreateObject();
    cJSON_AddStringToObject(freq_offset_param, "type", "int");
    cJSON_AddNumberToObject(freq_offset_param, "default", 1);
    cJSON_AddStringToObject(freq_offset_param, "description", "Gain");
    cJSON_AddItemToObject(parameters, "gain", freq_offset_param);
    
    cJSON_AddItemToObject(response, "parameters", parameters);
    
    // Add output info
    cJSON *output_info = cJSON_CreateObject();
    cJSON_AddNumberToObject(output_info, "channel_count", 3);
    
    cJSON *channel_names = cJSON_CreateArray();
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("add_ch"));
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("sub_ch"));
    cJSON_AddItemToArray(channel_names, cJSON_CreateString("gain_ch"));
    cJSON_AddItemToObject(output_info, "channel_names", channel_names);
    
    cJSON *channel_formats = cJSON_CreateArray();
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("float32"));
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("float32"));
    cJSON_AddItemToArray(channel_formats, cJSON_CreateString("float32"));
    cJSON_AddItemToObject(output_info, "channel_format", channel_formats);
    
    cJSON_AddItemToObject(response, "output_info", output_info);
    
    // Add plot info
    cJSON *plot_info = cJSON_CreateArray();
    
    // Plot 0 - Add
    cJSON *plot0 = cJSON_CreateObject();
    cJSON_AddNumberToObject(plot0, "id", 1);
    cJSON_AddStringToObject(plot0, "title", "Add, Sub");
    cJSON_AddStringToObject(plot0, "xLabel", "time[s]");
    cJSON_AddStringToObject(plot0, "yLabel", "amplitude[V]");
    cJSON_AddStringToObject(plot0, "type", "plot");
    cJSON *ch0 = cJSON_CreateArray();
    cJSON *ch0_pair = cJSON_CreateArray();
    cJSON_AddItemToArray(ch0_pair, cJSON_CreateString("time"));
    cJSON_AddItemToArray(ch0_pair, cJSON_CreateString("add_ch"));
    cJSON *ch0_pair1 = cJSON_CreateArray();
    cJSON_AddItemToArray(ch0_pair1, cJSON_CreateString("time"));
    cJSON_AddItemToArray(ch0_pair1, cJSON_CreateString("sub_ch"));
    cJSON_AddItemToArray(ch0, ch0_pair);
    cJSON_AddItemToArray(ch0, ch0_pair1);
    cJSON_AddItemToObject(plot0, "ch", ch0);

    cJSON *flag0 = cJSON_CreateArray();
    cJSON_AddItemToArray(flag0, cJSON_CreateString("labels"));
    cJSON_AddItemToObject(plot0, "flags", flag0);

    cJSON_AddItemToArray(plot_info, plot0);
    
    // Plot 1 - Sub
    cJSON *plot1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(plot1, "id", 2);
    cJSON_AddStringToObject(plot1, "title", "X-Y");
    cJSON_AddStringToObject(plot1, "xLabel", "amplitude[V]");
    cJSON_AddStringToObject(plot1, "yLabel", "amplitude[V]");
    cJSON_AddStringToObject(plot1, "type", "plot");
    cJSON *ch1 = cJSON_CreateArray();
    cJSON *ch1_pair = cJSON_CreateArray(); 
    cJSON_AddItemToArray(ch1_pair, cJSON_CreateString("input0"));     
    cJSON_AddItemToArray(ch1_pair, cJSON_CreateString("input1"));   
    cJSON_AddItemToArray(ch1, ch1_pair);
    cJSON_AddItemToObject(plot1, "ch", ch1);

    cJSON *flag1 = cJSON_CreateArray();
    cJSON_AddItemToArray(flag1, cJSON_CreateString("points"));
    cJSON_AddItemToArray(flag1, cJSON_CreateString("labels"));
    cJSON_AddItemToObject(plot1, "flags", flag1);

    cJSON_AddItemToArray(plot_info, plot1);
    
    // Plot 2 - Mul
    cJSON *plot2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(plot2, "id", 3);
    cJSON_AddStringToObject(plot2, "title", "Mul");
    cJSON_AddStringToObject(plot2, "xLabel", "time[s]");
    cJSON_AddStringToObject(plot2, "yLabel", "amplitude[V]");
    cJSON_AddStringToObject(plot2, "type", "plot");
    cJSON *ch2 = cJSON_CreateArray();
    cJSON *ch2_pair = cJSON_CreateArray();
    cJSON_AddItemToArray(ch2_pair, cJSON_CreateString("time"));
    cJSON_AddItemToArray(ch2_pair, cJSON_CreateString("gain_ch"));
    cJSON_AddItemToArray(ch2, ch2_pair);
    cJSON_AddItemToObject(plot2, "ch", ch2);

    cJSON *flag2 = cJSON_CreateArray();
    cJSON_AddItemToArray(flag2, cJSON_CreateString("labels"));
    cJSON_AddItemToObject(plot2, "flags", flag2);

    cJSON_AddItemToArray(plot_info, plot2);

    // Plot 3 - FFT

    cJSON *plot3 = cJSON_CreateObject();
    cJSON_AddNumberToObject(plot3, "id", 4);
    cJSON_AddStringToObject(plot3, "title", "FFT");
    cJSON_AddStringToObject(plot3, "xLabel", "frequency[Hz]");
    cJSON_AddStringToObject(plot3, "yLabel", "magnitude[dB]");
    cJSON_AddStringToObject(plot3, "type", "plot");
    cJSON *ch3 = cJSON_CreateArray();
    cJSON *ch3_pair = cJSON_CreateArray();
    cJSON_AddItemToArray(ch3_pair, cJSON_CreateString("fft_frequency"));
    cJSON_AddItemToArray(ch3_pair, cJSON_CreateString("fft_magnitude_db"));
    cJSON_AddItemToArray(ch3, ch3_pair);
    cJSON_AddItemToObject(plot3, "ch", ch3);

    cJSON *flag3 = cJSON_CreateArray();
    cJSON_AddItemToArray(flag3, cJSON_CreateString("labels"));
    cJSON_AddItemToObject(plot3, "flags", flag3);

    cJSON_AddItemToArray(plot_info, plot3);

    cJSON_AddItemToObject(response, "plot_info", plot_info);


    // Add measurement info
    cJSON *measurements_info = cJSON_CreateArray();
    cJSON_AddItemToArray(measurements_info, cJSON_CreateString("peak_power"));
    cJSON_AddItemToArray(measurements_info, cJSON_CreateString("snr"));
    
    cJSON_AddItemToObject(response, "measurements", measurements_info);
    
    send_json_response(response);
}

void process_json_command(const char *json_str) {
    cJSON *json = cJSON_Parse(json_str);
    if (!json) {
        fprintf(stderr, "Error: Invalid JSON\n");
        return;
    }

    cJSON *command = cJSON_GetObjectItem(json, "command");
    if (!command || !cJSON_IsString(command)) {
        fprintf(stderr, "Error: Missing or invalid command\n");
        cJSON_Delete(json);
        return;
    }

    const char *cmd = command->valuestring;
    
    if (strcmp(cmd, "set_input_config") == 0) {
        handle_set_input_config(json);
    } else if (strcmp(cmd, "set_analysis_config") == 0) {
        handle_set_analysis_config(json);
    } else if (strcmp(cmd, "set_output_config") == 0) {
        handle_set_output_config(json);
    } else if (strcmp(cmd, "run") == 0) {
        handle_run(json);
    } else if (strcmp(cmd, "get_analysis_types") == 0) {
        handle_get_analysis_types(json);
    } else if (strcmp(cmd, "get_analysis_info") == 0) {
        handle_get_analysis_info(json);
    } else {
        fprintf(stderr, "Error: Unknown command: %s\n", cmd);
    }

    cJSON_Delete(json);
}

int main(int argc, char **argv) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Initialize state
    memset(&g_state, 0, sizeof(g_state));
    g_state.analysis_config.samples_size = 1024; // Default value
    g_state.analysis_config.gain = 1; // Default gain value

    while ((read = getline(&line, &len, stdin)) != -1) {
        // Remove newline
        if (line[read-1] == '\n') {
            line[read-1] = '\0';
        }
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Process JSON command
        process_json_command(line);
    }

    // Cleanup
    if (line) free(line);
    
    if (g_state.readMap && g_state.readMap != MAP_FAILED) {
#ifdef _WIN32
        cleanup_mapping(g_state.readMap, g_state.inputMapHandle, g_state.inputFileHandle);
#else
        munmap(g_state.readMap, g_state.inputFileSize);
        close(g_state.inputFd);
#endif
    }
    
    if (g_state.writeMap && g_state.writeMap != MAP_FAILED) {
        size_t samplesPerChnl = 0;
        if (g_state.analysis_config.samples_size > 0) {
            samplesPerChnl = g_state.analysis_config.samples_size;
        } else if (g_state.input_config.channel_count > 0) {
            samplesPerChnl = g_state.inputFileSize / (g_state.input_config.channel_count * sizeof(float));
        }
        size_t output_size = samplesPerChnl * 3 * sizeof(float);
#ifdef _WIN32
        cleanup_mapping(g_state.writeMap, g_state.outputMapHandle, g_state.outputFileHandle);
#else
        munmap(g_state.writeMap, output_size);
        close(g_state.outputFd);
#endif
    }

    return 0;
}
