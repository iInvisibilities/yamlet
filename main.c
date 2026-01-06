#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cyaml/cyaml.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "presets.h"
#include "directories/string_cleanser.c"
#include "directories/maker.c"
#include "schema/yamlet_yml.c"

void load_shared_buffer(void);
void unload_shared_buffer(void);
void load_modules(void);
void load_endpoints(struct yamlet_configuration *init_config);

static const cyaml_config_t config = {
	.log_fn = cyaml_log,
	.mem_fn = cyaml_mem,
	.log_level = CYAML_LOG_WARNING,
};
void *shared_memory_buffer;

int main(void) {
  struct yamlet_configuration *init_config;
  puts("Loading "INIT_FNAME"...");
  cyaml_err_t err = cyaml_load_file(INIT_FNAME, &config, &top_schema, (cyaml_data_t **)&init_config, NULL);
  if (err != CYAML_OK) {
    fprintf(stderr, "Error when loading "INIT_FNAME"!");
    return 1;
  }

  printf("Starting server on port %u\n", init_config -> port);
  load_shared_buffer();
  load_modules();
  load_endpoints(init_config);

  // Server main loop would go here

  unload_shared_buffer();
  return 0;
}

void load_shared_buffer(void) {
  puts("Loading shared buffer...");
  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
  if (shm_fd == -1) {
    fprintf(stderr, "Failed to create shared memory segment: %s\n", strerror(errno));
    exit(1);
  }

  shared_memory_buffer = mmap(NULL, sizeof(struct yamlet_configuration), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_memory_buffer == MAP_FAILED) {
    fprintf(stderr, "Failed to map shared memory segment: %s\n", strerror(errno));
    exit(1);
  }
}

void unload_shared_buffer(void) {
  puts("Unloading shared buffer...");
  if (munmap(shared_memory_buffer, sizeof(struct yamlet_configuration)) == -1) {
    fprintf(stderr, "Failed to unmap shared memory segment: %s\n", strerror(errno));
  }
  if (shm_unlink(SHM_NAME) == -1) {
    fprintf(stderr, "Failed to unlink shared memory segment: %s\n", strerror(errno));
  }
}

void load_modules(void) {
  puts("Loading modules...");
  
}

void load_endpoints(struct yamlet_configuration *init_config) {
  char current_endpoint[MAX_ROUTE_LENGTH];
  puts("Loading endpoints...");
  for (int route_n = 0; route_n < init_config -> routes_count; route_n++) {
    (init_config -> routes)[route_n] = cleanse((init_config -> routes)[route_n]);
    strncpy(current_endpoint, (init_config -> routes)[route_n], MAX_ROUTE_LENGTH - 1);
    printf("Loading endpoint: %s\n", current_endpoint);
    int mkres = mkdir_r(current_endpoint, 0755);
    if (mkres != 0) {
      fprintf(stderr, "Error creating directory %s: %s\n", current_endpoint, strerror(mkres));
      fprintf(stderr, "Skipping to next route...\n");
      continue;
    }
  }
}