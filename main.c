#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cyaml/cyaml.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "presets.h"
#include "directories/string_cleanser.c"
#include "directories/maker.c"

static const cyaml_config_t config = {
	.log_fn = cyaml_log,
	.mem_fn = cyaml_mem,
	.log_level = CYAML_LOG_WARNING,
};

int main(void) {
  #include "schema/yamlet_yml.c"
  struct yamlet_configuration *init_config;
  puts("Loading "INIT_FNAME"...");
  cyaml_err_t err = cyaml_load_file(INIT_FNAME, &config, &top_schema, (cyaml_data_t **)&init_config, NULL);
  if (err != CYAML_OK) {
    fprintf(stderr, "Error when loading "INIT_FNAME"!");
    return 1;
  }

  printf("Starting server on port %u\n", init_config -> port);
  char current_endpoint[MAX_ROUTE_LENGTH];
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
  return 0;
}
