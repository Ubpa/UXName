cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UXName VERSION 0.0.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_AddDep(UTemplate 0.4.9)
Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  CPM
  DIRECTORIES
    "include"
)
