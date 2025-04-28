#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#define ELF_PROGRAM_HEADER_OFFSET 0x20
#define ELF_PROGRAM_HEADER_ENTRY_SIZE 0x36
#define ELF_PROGRAM_HEADER_NUM_ENTRIES 0x38

#define ELF_SECTION_HEADER_OFFSET 0x28
#define ELF_SECTION_HEADER_ENTRY_SIZE 0x3A
#define ELF_SECTION_HEADER_NUM_ENTRIES 0x3C

#define ELF_DYNAMIC_SECTION_PHT_TYPE 0x02

typedef struct ProcessHandle {
  pid_t pid;
  int memory;
} ProcessHandle;

bool open_process(ProcessHandle *handle, pid_t pid);
bool get_module_base_address(ProcessHandle *handle, const char *module_name,
                             uint64_t *base_address);
bool get_module_size(ProcessHandle *handle, const char *module_name,
                     size_t *size);

pid_t get_pid(const char *process_name);
bool is_valid_pid(pid_t pid);

int8_t read_i8(ProcessHandle *handle, uint64_t address);
int16_t read_i16(ProcessHandle *handle, uint64_t address);
int32_t read_i32(ProcessHandle *handle, uint64_t address);
int64_t read_i64(ProcessHandle *handle, uint64_t address);

uint8_t read_u8(ProcessHandle *handle, uint64_t address);
uint16_t read_u16(ProcessHandle *handle, uint64_t address);
uint32_t read_u32(ProcessHandle *handle, uint64_t address);
uint64_t read_u64(ProcessHandle *handle, uint64_t address);

float read_f32(ProcessHandle *handle, uint64_t address);
double read_f64(ProcessHandle *handle, uint64_t address);

uint8_t *read_bytes(ProcessHandle *handle, uint64_t address, size_t size);
char *read_string(ProcessHandle *handle, uint64_t address);

bool dump_module(ProcessHandle *handle, uint64_t offset, size_t *size,
                 uint8_t *result);
bool get_module_export_address(ProcessHandle *handle, const char *module_name,
                               const char *export_name, uint64_t *address);
bool scan_pattern(ProcessHandle *handle, uint64_t offset, size_t size,
                  const uint8_t pattern[], const bool mask[], uint64_t *result);
uint64_t get_rel_address(ProcessHandle *handle, uint64_t address,
                         uint64_t offset, uint64_t size);
bool get_module_export(ProcessHandle *handle, uint64_t offset,
                       const char *export_name, uint64_t *result);
bool get_segment_from_pht(ProcessHandle *handle, uint64_t offset, uint64_t tag,
                          uint64_t *result);
bool get_address_from_dynamic_section(ProcessHandle *handle, uint64_t offset,
                                      uint64_t tag, uint64_t *result);
bool get_interface_offset(ProcessHandle *handle, uint64_t address, char *name,
                          uint64_t *offset);
bool get_convar(ProcessHandle *handle, uint64_t offset, char *name,
                uint64_t *result);