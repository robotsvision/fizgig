# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/.drivers/esp-idf-v5.2.2/components/bootloader/subproject"
  "C:/Projects/fizgig/build/bootloader"
  "C:/Projects/fizgig/build/bootloader-prefix"
  "C:/Projects/fizgig/build/bootloader-prefix/tmp"
  "C:/Projects/fizgig/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Projects/fizgig/build/bootloader-prefix/src"
  "C:/Projects/fizgig/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Projects/fizgig/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Projects/fizgig/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
