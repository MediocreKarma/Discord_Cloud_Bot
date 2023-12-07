cmake_minimum_required(VERSION 3.10)

project(smtpclientlibrary-download NONE)

include(ExternalProject)
ExternalProject_Add(smtpclientlibrary
  GIT_REPOSITORY    https://github.com/jeremydumais/CPP-SMTPClient-library.git
  GIT_TAG           master
  SOURCE_DIR        "/home/karma/Desktop/Retele/Project/Server/smtpclientlibrary-src"
  BINARY_DIR        "/home/karma/Desktop/Retele/Project/Server/smtpclientlibrary-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
