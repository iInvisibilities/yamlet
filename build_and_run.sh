MAIN_SCRIPT="main.c"
OUT_EX_NAME="main"
DEV_TEST_PROJ_DIR="test_project"

CYAML_DEP=$(pkg-config --cflags --libs libcyaml)
DEPS=$CYAML_DEP

gcc $MAIN_SCRIPT $DEPS -o $DEV_TEST_PROJ_DIR/$OUT_EX_NAME && (cd $DEV_TEST_PROJ_DIR && ./$OUT_EX_NAME)
