project(dev_sandbox)

set(SRC
  main.cpp
)

if(${PLATFORM_MAC})
  append(SRC
    main.mm
  )
endif()

omim_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC})

target_link_libraries(${PROJECT_NAME}
  generator  # For borders::LoadBorders
  qt_common
  map
  gflags::gflags
)

target_include_directories(${PROJECT_NAME} PUBLIC ${OMIM_ROOT}/3party/glfw/include)

set(BUNDLE_NAME "OMapsDevSandbox")
set(BUNDLE_DISPLAY_NAME "Organic Maps: Developer Sandbox")

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BUNDLE_NAME})

set(BUNDLE_EXECUTABLE ${BUNDLE_NAME})

set(BUNDLE_FOLDER ${CMAKE_BINARY_DIR}/${BUNDLE_NAME}.app)
set(RESOURCES_FOLDER ${BUNDLE_FOLDER}/Contents/Resources)
set(DATA_DIR ${OMIM_ROOT}/data)

execute_process(
  COMMAND mkdir -p ${RESOURCES_FOLDER}/shaders_compiler
)

function(copy_resources)
  foreach(file ${ARGN})
    execute_process(
      COMMAND cp -r ${DATA_DIR}/${file} ${RESOURCES_FOLDER}
    )
  endforeach()
endfunction()

copy_resources(
  countries-strings
  resources-default
  resources-mdpi_light
  resources-hdpi_light
  resources-xhdpi_light
  resources-xxhdpi_light
  resources-xxxhdpi_light
  resources-6plus_light

  categories.txt
  categories_cuisines.txt
  classificator.txt
  colors.txt
  countries.txt
  drules_proto_default_light.bin
  drules_proto_default_dark.bin
  drules_proto_vehicle_light.bin
  drules_proto_vehicle_dark.bin
  editor.config
  fonts_blacklist.txt
  fonts_whitelist.txt
  packed_polygons.bin
  patterns.txt
  transit_colors.txt
  types.txt
  unicode_blocks.txt
  World.mwm
  WorldCoasts.mwm

  00_NotoNaskhArabic-Regular.ttf
  00_NotoSansBengali-Regular.ttf
  00_NotoSansHebrew-Regular.ttf
  00_NotoSansMalayalam-Regular.ttf
  00_NotoSansThai-Regular.ttf
  00_NotoSerifDevanagari-Regular.ttf
  01_dejavusans.ttf
  02_droidsans-fallback.ttf
  03_jomolhari-id-a3d.ttf
  04_padauk.ttf
  05_khmeros.ttf
  06_code2000.ttf
  07_roboto_medium.ttf
)

if (NOT PLATFORM_LINUX)
  # On Linux, ICU data is loaded from the shared library.
  copy_resources(icudt75l.dat)
endif()

if (PLATFORM_MAC)
  execute_process(
    COMMAND cp -r ${OMIM_ROOT}/tools/shaders_compiler/macos ${RESOURCES_FOLDER}/shaders_compiler
  )
elseif (PLATFORM_LINUX)
  execute_process(
    COMMAND cp -r ${OMIM_ROOT}/tools/shaders_compiler/linux ${RESOURCES_FOLDER}/shaders_compiler
  )
endif()
