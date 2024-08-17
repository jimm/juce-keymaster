#pragma once
#ifdef KM_TEST

#include "../km/consts.h"
#include "../km/connection.h"
#include "../km/keymaster.h"
#include "../km/cursor.h"

#define TEST_DATA_PATH "Source/test/testdata.kmst"

KeyMaster *load_test_data();
Connection *create_conn();

#endif /* KM_TEST */
