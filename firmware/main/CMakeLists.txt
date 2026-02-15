if(NOT DEFINED PIXIE_APP)
  set(PIXIE_APP "handshake")
endif()

if(PIXIE_APP STREQUAL "handshake")
  set(PIXIE_APP_SOURCE "apps/app_handshake.c")
elseif(PIXIE_APP STREQUAL "template")
  set(PIXIE_APP_SOURCE "apps/app_template.c")
elseif(PIXIE_APP STREQUAL "valentine")
  set(PIXIE_APP_SOURCE "apps/app_valentine.c")
elseif(PIXIE_APP STREQUAL "ble_demo")
  set(PIXIE_APP_SOURCE "apps/app_ble_demo.c")
else()
  message(FATAL_ERROR "Unknown PIXIE_APP='${PIXIE_APP}'. Supported: handshake, template, valentine, ble_demo")
endif()

idf_component_register(
  SRCS
    "main.c"
    "apps/pixie_runtime.c"
    "${PIXIE_APP_SOURCE}"
  INCLUDE_DIRS
    "."
    "apps"
)
