cmake_minimum_required(VERSION 3.23.3)
project(mmv_test)
add_executable(${PROJECT_NAME} ./src/mmv_test.c  ../src/mmv.c ../unity/unity.c)
