struct yamlet_configuration {
  uint16_t port;
  char **routes;
  uint8_t routes_count;
};

static const cyaml_schema_value_t routes_entry = {
	CYAML_VALUE_STRING(CYAML_FLAG_POINTER, char*, 1, MAX_ROUTE_LENGTH)
};

static const cyaml_schema_field_t top_mapping_schema[] = {
	CYAML_FIELD_UINT(
		"port", CYAML_FLAG_DEFAULT, struct yamlet_configuration, port),
	CYAML_FIELD_SEQUENCE(
		"routes", CYAML_FLAG_POINTER, struct yamlet_configuration, routes, &routes_entry, 0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

static const cyaml_schema_value_t top_schema = {
	CYAML_VALUE_MAPPING(
		CYAML_FLAG_POINTER, struct yamlet_configuration, top_mapping_schema)
};
