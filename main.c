#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <cyaml/cyaml.h>
#include <yaml.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "presets.h"
#include "directories/string_cleanser.c"
#include "directories/maker.c"
#include "schema/yamlet_yml.c"

void unload_cyaml(void);
void load_shared_buffer(void);
void unload_shared_buffer(void);
void load_modules(void);
void load_endpoints(struct yamlet_configuration*);

struct yamlet_configuration *init_config;
static const cyaml_config_t config = {
	.log_fn = cyaml_log,
	.mem_fn = cyaml_mem,
	.log_level = CYAML_LOG_WARNING,
};

int shm_fd;
void *shared_memory_buffer;

int main(void) {
  atexit(unload_cyaml);
  atexit(unload_shared_buffer);

  #ifdef DEBUG
  puts("Loading "INIT_FNAME"...");
  #endif
  cyaml_err_t err = cyaml_load_file(INIT_FNAME, &config, &top_schema, (cyaml_data_t **)&init_config, NULL);
  if (err != CYAML_OK) {
    fprintf(stderr, "Error when loading "INIT_FNAME"!");
    return 1;
  }

  #ifdef DEBUG
  printf("Starting server on port %u\n", init_config -> port);
  #endif
  load_shared_buffer();
  load_modules();
  load_endpoints(init_config);

  // Server main loop would go here
  #ifdef DEBUG
  printf("Entering main server loop...\n");
  #endif
  int i = 0;
  while (i++ < 20) {
    // sleep for .5 s and write to the shared buffer the time and a hello world message
    usleep(500000);
    printf("Main loop iteration %d\n", i);
    sprintf(shared_memory_buffer, "Hello, world! Current time: %ld\n", time(NULL));
    #ifdef DEBUG
    //printf("Updated shared buffer: %s", (char *)shared_memory_buffer);
    #endif
  }
  return 0;
}

void load_shared_buffer(void) {
  #ifdef DEBUG
  puts("Loading shared buffer...");
  #endif
  shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
  if (shm_fd == -1) {
    fprintf(stderr, "Failed to create shared memory segment: %s\n", strerror(errno));
    exit(1);
  }

  shared_memory_buffer = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shared_memory_buffer == MAP_FAILED) {
    fprintf(stderr, "Failed to map shared memory segment: %s\n", strerror(errno));
    exit(1);
  }
}

void unload_cyaml(void) {
  #ifdef DEBUG
  puts("Unloading cyaml...");
  #endif
  free(init_config);
}

void unload_shared_buffer(void) {
  #ifdef DEBUG
  puts("Unloading shared buffer...");
  #endif
  if (munmap(shared_memory_buffer, sizeof(struct yamlet_configuration)) == -1) {
    fprintf(stderr, "Failed to unmap shared memory segment: %s\n", strerror(errno));
  }
  if (shm_unlink(SHM_NAME) == -1) {
    fprintf(stderr, "Failed to unlink shared memory segment: %s\n", strerror(errno));
  }
}

void load_modules(void) {
  #ifdef DEBUG
  puts("Loading modules...");
  #endif
  yaml_parser_t parser;
  yaml_event_t event;

  int done = 0;
  yaml_parser_initialize(&parser);

  #ifdef DEBUG
  puts("-- parsing modules.yml --");
  #endif
  FILE *input = fopen("modules.yml", "rb");
  yaml_parser_set_input_file(&parser, input);

  int r_type = 2;
  typedef struct {
    char *module_name;
    char *init_command;
  } yamlet_module;
  yamlet_module current_module = {0};
  while (!done) {
    if (!yaml_parser_parse(&parser, &event)) {
      fprintf(stderr, "Failed to parse YAML: %s\n", parser.problem);
      break;
    }

    if (event.type == YAML_SCALAR_EVENT) {
      if (r_type % 2 == 0) {
        current_module.module_name = strdup((char *)event.data.scalar.value);
        r_type++;
      } else {
        current_module.init_command = strdup((char *)event.data.scalar.value);
        // execute the init command to run the module
        #ifdef DEBUG
        printf("Initializing module %s with command: %s\n", current_module.module_name, current_module.init_command);
        #endif
        int pid = fork();
        if (pid == -1) {
          fprintf(stderr, "Failed to fork process for module %s: %s\n", current_module.module_name, strerror(errno));
          exit(1);
          return;
        } else if (pid == 0) {
          if (execv("/bin/sh", (char *const []){ "sh", "-c", current_module.init_command, NULL }) == -1) {
            fprintf(stderr, "Failed to execute module %s: %s\n", current_module.module_name, strerror(errno));
            continue;
          }
          memset(&current_module, 0, sizeof(yamlet_module));
        }
        else r_type = 2;
      }
    }

    done = (event.type == YAML_STREAM_END_EVENT);
    yaml_event_delete(&event);
  }

  yaml_parser_delete(&parser);
}

void load_endpoints(struct yamlet_configuration *init_config) {
  char current_endpoint[MAX_ROUTE_LENGTH];
  #ifdef DEBUG
  puts("Loading endpoints...");
  #endif
  for (int route_n = 0; route_n < init_config -> routes_count; route_n++) {
    (init_config -> routes)[route_n] = cleanse((init_config -> routes)[route_n]);
    strncpy(current_endpoint, (init_config -> routes)[route_n], MAX_ROUTE_LENGTH - 1);
    #ifdef DEBUG
    printf("Loading endpoint: %s\n", current_endpoint);
    #endif
    int mkres = mkdir_r(current_endpoint, 0755);
    if (mkres != 0) {
      fprintf(stderr, "Error creating directory %s: %s\n", current_endpoint, strerror(mkres));
      fprintf(stderr, "Skipping to next route...\n");
      continue;
    }
  }
}