# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2006 Blender Foundation. All rights reserved.

set(INC
  .
  gizmo
  gizmo/intern
  ../blenfont
  ../blenkernel
  ../blenlib
  ../blenloader
  ../blentranslation
  ../compositor
  ../depsgraph
  ../editors/include
  ../draw
  ../gpu
  ../imbuf
  ../makesdna
  ../makesrna
  ../nodes
  ../render
  ../sequencer
  ../../../intern/clog
  ../../../intern/ghost
  ../../../intern/glew-mx
  ../../../intern/guardedalloc
  ../../../intern/memutil

  # for writefile.c: dna_type_offsets.h
  ${CMAKE_BINARY_DIR}/source/blender/makesdna/intern
)

set(SRC
  intern/wm.c
  intern/wm_cursors.c
  intern/wm_dragdrop.c
  intern/wm_draw.c
  intern/wm_event_query.c
  intern/wm_event_system.c
  intern/wm_files.c
  intern/wm_files_link.c
  intern/wm_gesture.c
  intern/wm_gesture_ops.c
  intern/wm_init_exit.c
  intern/wm_jobs.c
  intern/wm_keymap.c
  intern/wm_keymap_utils.c
  intern/wm_menu_type.c
  intern/wm_operator_props.c
  intern/wm_operator_type.c
  intern/wm_operator_utils.c
  intern/wm_operators.c
  intern/wm_panel_type.c
  intern/wm_platform_support.c
  intern/wm_playanim.c
  intern/wm_splash_screen.c
  intern/wm_stereo.c
  intern/wm_subwindow.c
  intern/wm_surface.c
  intern/wm_toolsystem.c
  intern/wm_tooltip.c
  intern/wm_uilist_type.c
  intern/wm_utils.c
  intern/wm_window.c
  gizmo/intern/wm_gizmo.c
  gizmo/intern/wm_gizmo_group.c
  gizmo/intern/wm_gizmo_group_type.c
  gizmo/intern/wm_gizmo_map.c
  gizmo/intern/wm_gizmo_target_props.c
  gizmo/intern/wm_gizmo_type.c
  message_bus/intern/wm_message_bus.c
  message_bus/intern/wm_message_bus_rna.c
  message_bus/intern/wm_message_bus_static.c

  WM_api.h
  WM_keymap.h
  WM_message.h
  WM_toolsystem.h
  WM_types.h
  wm.h
  wm_cursors.h
  wm_draw.h
  wm_event_system.h
  wm_event_types.h
  wm_files.h
  wm_surface.h
  wm_window.h
  intern/wm_platform_support.h
  intern/wm_window_private.h
  gizmo/WM_gizmo_api.h
  gizmo/WM_gizmo_types.h
  gizmo/wm_gizmo_fn.h
  gizmo/wm_gizmo_wmapi.h
  gizmo/intern/wm_gizmo_intern.h
  message_bus/intern/wm_message_bus_intern.h
  message_bus/wm_message_bus.h
)

set(LIB
  bf_editor_screen
  bf_sequencer
)

if(WITH_AUDASPACE)
  add_definitions(-DWITH_AUDASPACE)

  list(APPEND INC_SYS
    ${AUDASPACE_C_INCLUDE_DIRS}
  )
  list(APPEND LIB
    ${AUDASPACE_C_LIBRARIES}
    ${AUDASPACE_PY_LIBRARIES}
  )
endif()

if(WITH_CYCLES)
  add_definitions(-DWITH_CYCLES)
endif()

if(WITH_OPENCOLLADA)
  add_definitions(-DWITH_COLLADA)
endif()

if(WITH_CODEC_FFMPEG)
  list(APPEND INC_SYS
    ${FFMPEG_INCLUDE_DIRS}
  )
  list(APPEND LIB
    ${FFMPEG_LIBRARIES}
  )
  add_definitions(-DWITH_FFMPEG)
endif()

if(WITH_HEADLESS)
  add_definitions(-DWITH_HEADLESS)
elseif(WITH_GHOST_X11)
  add_definitions(-DWITH_GHOST_X11)
endif()

if(WITH_PYTHON)
  list(APPEND INC
    ../python
  )
  add_definitions(-DWITH_PYTHON)
  if(WITH_PYTHON_MODULE)
    add_definitions(-DWITH_PYTHON_MODULE)
  endif()
endif()

if(WITH_BUILDINFO)
  add_definitions(-DWITH_BUILDINFO)
endif()

if(WITH_INPUT_NDOF)
  add_definitions(-DWITH_INPUT_NDOF)
endif()

if(WIN32 OR APPLE)
  if(WITH_INPUT_IME)
    add_definitions(-DWITH_INPUT_IME)
  endif()
endif()

if(WITH_COMPOSITOR)
  list(APPEND LIB
    bf_compositor
  )
  add_definitions(-DWITH_COMPOSITOR)
endif()

if(WITH_XR_OPENXR)
  add_definitions(-DWITH_XR_OPENXR)

  list(APPEND INC
    xr
  )

  list(APPEND SRC
    xr/intern/wm_xr.c
    xr/intern/wm_xr_action.c
    xr/intern/wm_xr_actionmap.c
    xr/intern/wm_xr_draw.c
    xr/intern/wm_xr_operators.c
    xr/intern/wm_xr_session.c

    xr/wm_xr.h
    xr/intern/wm_xr_intern.h
  )
endif()

blender_add_lib_nolist(bf_windowmanager "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

# Needed so we can use dna_type_offsets.h for defaults initialization.
add_dependencies(bf_windowmanager bf_dna)
