#include "process.h"
#include "config.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool open_process(ProcessHandle *handle, pid_t pid)
{
  if (handle == NULL)
  {
    errorm_print("ProcessHandle is NULL\n");
    return false;
  }
  if (handle->memory)
  {
    close(handle->memory);
    handle->memory = 0;
  }
  memset(handle, 0, sizeof(ProcessHandle));
  handle->pid = pid;
  if (!is_valid_pid(pid))
  {
    error_print("Invalid PID: %d\n", pid);
    return false;
  }
  char memory_path[32];
  snprintf(memory_path, sizeof(memory_path), "/proc/%d/mem", pid);
  handle->memory = open(memory_path, O_RDONLY);
  if (!handle->memory)
  {
    errorm_print("Failed to open memory file");
    return false;
  }
  return true;
}

bool check_elf_header(const uint8_t *data, size_t size)
{
  if (size < 4)
  {
    return false;
  }
  return (data[0] == 0x7F && data[1] == 'E' && data[2] == 'L' &&
          data[3] == 'F');
}

bool get_module_base_address(ProcessHandle *handle, const char *module_name,
                             uint64_t *base_address)
{
  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    return false;
  }
  char maps_path[32];
  snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", handle->pid);
  FILE *maps_file = fopen(maps_path, "r");
  if (!maps_file)
  {
    errorm_print("Failed to open maps file");
    return false;
  }

  char line[256];
  while (fgets(line, sizeof(line), maps_file))
  {
    if (strstr(line, module_name))
    {
      uint64_t addr;
      if (sscanf(line, "%lx", &addr) == 1)
      {
        *base_address = addr;
        fclose(maps_file);
        return true;
      }
    }
  }

  fclose(maps_file);
  error_print("Module %s not found\n", module_name);
  return false;
}

pid_t get_pid(const char *process_name)
{
  char command[256];
  snprintf(command, sizeof(command), "pidof %s", process_name);
  FILE *pipe = popen(command, "r");
  if (!pipe)
  {
    return 0;
  }

  char buffer[128];
  if (fgets(buffer, sizeof(buffer), pipe) != NULL)
  {
    pclose(pipe);
    return atoi(buffer);
  }

  pclose(pipe);
  return 0;
}

bool is_valid_pid(pid_t pid)
{
  char path[32];
  snprintf(path, sizeof(path), "/proc/%d", pid);
  struct stat buffer;
  return stat(path, &buffer) == 0;
}

int8_t read_i8(ProcessHandle *handle, uint64_t address)
{
  int8_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(int8_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

int16_t read_i16(ProcessHandle *handle, uint64_t address)
{
  int16_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(int16_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

int32_t read_i32(ProcessHandle *handle, uint64_t address)
{
  int32_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(int32_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

int64_t read_i64(ProcessHandle *handle, uint64_t address)
{
  int64_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(int64_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

uint8_t read_u8(ProcessHandle *handle, uint64_t address)
{
  uint8_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(uint8_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

uint16_t read_u16(ProcessHandle *handle, uint64_t address)
{
  uint16_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(uint16_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

uint32_t read_u32(ProcessHandle *handle, uint64_t address)
{
  uint32_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(uint32_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

uint64_t read_u64(ProcessHandle *handle, uint64_t address)
{
  uint64_t value = 0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(uint64_t), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

float read_f32(ProcessHandle *handle, uint64_t address)
{
  float value = 0.0f;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(float), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

double read_f64(ProcessHandle *handle, uint64_t address)
{
  double value = 0.0;
  if (handle->memory)
  {
    pread(handle->memory, &value, sizeof(double), address);
  }
  else
  {
    errorm_print("Memory file is not open\n");
  }
  return value;
}

uint8_t *read_bytes(ProcessHandle *handle, uint64_t address, size_t size)
{
  debug_print("Reading %zu bytes from address 0x%lx\n", size, address);

  uint8_t *buffer = (uint8_t *)malloc(size);
  if (buffer == NULL)
  {
    errorm_print("Failed to allocate memory for read_bytes\n");
    return NULL;
  }

  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    free(buffer);
    return NULL;
  }

  pread(handle->memory, buffer, size, address);
  return buffer;
}

// shits the bed if the string is longer than 255 chars
char *read_string(ProcessHandle *handle, uint64_t address)
{
  char *buffer = (char *)malloc(256);
  if (buffer && handle->memory)
  {
    pread(handle->memory, buffer, 255, address);
    buffer[255] = '\0';
  }
  else
  {
    errorm_print("Memory file is not open or allocation failed\n");
    free(buffer);
    buffer = NULL;
  }
  return buffer;
}

bool dump_module(ProcessHandle *handle, uint64_t offset, size_t *result_size,
                 uint8_t *result)
{
  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    return false;
  }

  if (!check_elf_header(read_bytes(handle, offset, 4), 4))
  { // Check ELF header
    errorm_print("Invalid ELF header\n");
    return false;
  }

  const uint64_t section_header_offset =
      read_u64(handle, offset + ELF_SECTION_HEADER_OFFSET);
  const uint16_t section_header_entry_size =
      read_u16(handle, offset + ELF_SECTION_HEADER_ENTRY_SIZE);
  const uint16_t section_header_num_entries =
      read_u16(handle, offset + ELF_SECTION_HEADER_NUM_ENTRIES);

  const uint64_t module_size =
      section_header_offset +
      section_header_num_entries * section_header_entry_size;

  uint8_t *buffer = read_bytes(handle, offset, module_size);
  if (buffer == NULL)
  {
    errorm_print("Failed to read module memory\n");
    free(buffer);
    return false;
  }

  *result_size = module_size;
  if (result != NULL)
  {
    memcpy(result, buffer, module_size);
  }
  free(buffer);
  return true;
}

bool scan_pattern(ProcessHandle *handle, uint64_t offset, size_t pattern_length,
                  const uint8_t pattern[], const bool mask[],
                  uint64_t *result)
{
  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    return false;
  }

  size_t mem_size = 0;
  if (!dump_module(handle, offset, &mem_size, NULL))
  {
    errorm_print("Failed to get module size\n");
    return false;
  }

  uint8_t *mem = malloc(mem_size);
  if (mem == NULL)
  {
    errorm_print("Failed to allocate memory for pattern scanning\n");
    return false;
  }

  if (!dump_module(handle, offset, &mem_size, mem))
  {
    errorm_print("Failed to dump module\n");
    free(mem);
    return false;
  }

  for (size_t i = 0; i < mem_size - pattern_length; i++)
  {
    bool found = true;
    for (size_t j = 0; j < pattern_length; j++)
    {
      if (mask[j] && mem[i + j] != pattern[j])
      {
        found = false;
        break;
      }
    }
    if (found)
    {
      *result = offset + i;
      free(mem);
      return true;
    }
  }

  free(mem);
  return false;
}

uint64_t get_rel_address(ProcessHandle *handle, uint64_t address,
                         uint64_t offset, uint64_t instruction_size)
{
  const int32_t relative_offset = read_i32(handle, address + offset);
  return address + instruction_size + relative_offset;
}

bool get_segment_from_pht(ProcessHandle *handle, uint64_t offset, uint64_t tag,
                          uint64_t *result)
{
  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    return false;
  }

  const uint64_t first_entry =
      read_u32(handle, offset + ELF_PROGRAM_HEADER_OFFSET) + offset;

  const uint16_t pht_entry_size =
      read_u16(handle, offset + ELF_PROGRAM_HEADER_ENTRY_SIZE);

  const uint16_t num_entries =
      read_u16(handle, offset + ELF_PROGRAM_HEADER_NUM_ENTRIES);

  for (size_t i = 0; i < num_entries; i++)
  {
    const uint64_t entry = first_entry + i * pht_entry_size;
    if (read_u32(handle, entry) == tag)
    {
      *result = entry;
      return true;
    }
  }

  return false;
}

bool get_address_from_dynamic_section(ProcessHandle *handle, uint64_t offset,
                                      uint64_t tag, uint64_t *result)
{
  if (handle == NULL || !handle->memory)
  {
    errorm_print("ProcessHandle is NULL or memory file is not open\n");
    return false;
  }

  uint64_t dynamic_section_offset = 0;
  if (!get_segment_from_pht(handle, offset, ELF_DYNAMIC_SECTION_PHT_TYPE,
                            &dynamic_section_offset))
  {
    errorm_print("Failed to get dynamic section offset\n");
    return false;
  }

  const uint8_t register_size = 8;
  uint64_t address =
      read_u64(handle, dynamic_section_offset + 2 * register_size) + offset;

  while (true)
  {
    uint64_t tag_address = address;
    const uint64_t tag_value = read_u64(handle, tag_address);

    if (tag_value == 0)
    {
      break;
    }

    if (tag_value == tag)
    {
      *result = read_u64(handle, tag_address + register_size);
      return true;
    }

    address += register_size * 2;
  }

  return false;
}

bool get_module_export(ProcessHandle *handle, uint64_t offset,
                       const char *export_name, uint64_t *result)
{
  if (handle == NULL || !handle->memory || export_name == NULL)
  {
    errorm_print("Invalid parameters\n");
    return false;
  }

  // TODO: +1 to the unfreak tally
  size_t data_size = 0;
  uint8_t *data = malloc(1);
  if (!dump_module(handle, offset, &data_size, NULL))
  {
    errorm_print("Failed to get module size\n");
    free(data);
    return false;
  }

  free(data);
  data = malloc(data_size);
  if (data == NULL)
  {
    errorm_print("Failed to allocate memory for module dump\n");
    return false;
  }

  if (!dump_module(handle, offset, &data_size, data))
  {
    errorm_print("Failed to dump module\n");
    free(data);
    return false;
  }

  if (!check_elf_header(data, data_size))
  {
    errorm_print("Invalid ELF header\n");
    free(data);
    return false;
  }

  free(data);

  const uint8_t add = 0x18;
  const uint8_t length = 0x08;

  uint64_t string_table = 0;
  uint64_t symbol_table = 0;

  if (!get_address_from_dynamic_section(handle, offset, 0x05, &string_table))
  {
    errorm_print("Failed to get string table address\n");
    return false;
  }

  if (!get_address_from_dynamic_section(handle, offset, 0x06, &symbol_table))
  {
    errorm_print("Failed to get symbol table address\n");
    return false;
  }

  symbol_table += add;

  uint32_t st_name;
  while ((st_name = read_u32(handle, symbol_table)) != 0)
  {
    char *name = read_string(handle, string_table + st_name);

    if (name && strcmp(name, export_name) == 0)
    {
      uint64_t address_value = read_u64(handle, symbol_table + length);
      *result = address_value + offset;
      free(name);
      return true;
    }

    if (name)
    {
      free(name);
    }

    symbol_table += add;
  }

  return false;
}

bool get_interface_offset(ProcessHandle *handle, uint64_t address, char *name,
                          uint64_t *offset)
{
  if (handle == NULL || !handle->memory || name == NULL)
  {
    errorm_print("Invalid parameters\n");
    return false;
  }

  uint64_t interface_export = 0;
  if (!get_module_export(handle, address, "CreateInterface",
                         &interface_export))
  {
    errorm_print("Failed to get CreateInterface export\n");
    return false;
  }

  uint64_t export_address =
      get_rel_address(handle, interface_export, 0x01, 0x05) + 0x10;
  uint32_t interface_offset = read_u32(handle, export_address + 0x03);
  uint64_t interface_entry =
      read_u64(handle, export_address + 0x07 + interface_offset);

  size_t name_length = strlen(name);

  while (interface_entry != 0)
  {
    uint64_t interface_name_address = read_u64(handle, interface_entry + 8);

    uint8_t *name_bytes =
        read_bytes(handle, interface_name_address, name_length + 1);
    if (name_bytes != NULL)
    {
      name_bytes[name_length] = '\0';

      if (strcmp((char *)name_bytes, name) == 0)
      {
        uint64_t vfunc_address = read_u64(handle, interface_entry);
        *offset = read_u32(handle, vfunc_address + 0x03) + vfunc_address + 0x07;
        free(name_bytes);
        return true;
      }
      free(name_bytes);
    }

    interface_entry = read_u64(handle, interface_entry + 0x10);
  }

  debug_print("Failed to find interface %s\n", name);
  return false;
}

bool get_convar(ProcessHandle *handle, uint64_t convar_offset,
                char *convar_name, uint64_t *result)
{
  if (handle == NULL || !handle->memory || convar_name == NULL)
  {
    errorm_print("Invalid parameters\n");
    return false;
  }

  const uint64_t objects = read_u64(handle, convar_offset + 64);
  const uint32_t count = read_u32(handle, convar_offset + 160);

  for (size_t i = 0; i < count; i++)
  {
    const uint64_t object = read_u64(handle, objects + i * 16);
    if (object == 0)
    {
      break;
    }

    char *name = read_string(handle, read_u64(handle, object));
    if (name != NULL)
    {
      if (strcmp(name, convar_name) == 0)
      {
        *result = object;
        free(name);
        return true;
      }
      free(name);
    }
  }

  return false;
}