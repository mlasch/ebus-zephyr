# SPDX-License-Identifier: Apache-2.0

if(CONFIG_EBUS)
  zephyr_library()

  zephyr_library_sources(
    ebus.c
  )
endif()
