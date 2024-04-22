# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1/components/bootloader/subproject"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/tmp"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/src/bootloader-stamp"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/src"
  "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/ESP_Projects/ESP_IDF_5.1/power_save/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
