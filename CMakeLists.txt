cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(recvx C)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Resident Evil Code: Veronica X")
set(VITA_TITLEID  "RECVX0001")
set(VITA_VERSION  "00.01")

# ---------------------------------------------------------------- game code
set(GAME_PROG ${CMAKE_SOURCE_DIR}/src/ps2/veronica/prog)
set(GAME_ADX  ${CMAKE_SOURCE_DIR}/src/cri/mwlib/ee/lib/libadxe)

file(GLOB GAME_SOURCES ${GAME_PROG}/*.c ${GAME_ADX}/*.c)
# Replaced by the platform layer:
#   gcc_wrapper.c   EE libgcc helpers (the Vita toolchain has its own)
#   ps2_MovieFunc.c IPU/libmpeg movie decoder -> platform/vita/movie.c
# Built as part of another file:
#   ps2_Vu1Scissor2.c  #included by ps2_Vu1Strip.c
list(REMOVE_ITEM GAME_SOURCES
  ${GAME_PROG}/gcc_wrapper.c
  ${GAME_PROG}/ps2_MovieFunc.c
  ${GAME_PROG}/ps2_Vu1Scissor2.c
)

file(GLOB PLATFORM_SOURCES
  ${CMAKE_SOURCE_DIR}/platform/ps2compat/src/*.c
  ${CMAKE_SOURCE_DIR}/platform/gs/*.c
  ${CMAKE_SOURCE_DIR}/platform/iop/*.c
  ${CMAKE_SOURCE_DIR}/platform/vita/*.c
)

add_executable(recvx ${GAME_SOURCES} ${PLATFORM_SOURCES})
# EE code writes to string literals and const data: keep them writable.
set_target_properties(recvx PROPERTIES C_COMPILER_LAUNCHER ${CMAKE_SOURCE_DIR}/cmake/writable_rodata.sh)

set(PS2COMPAT_INC ${CMAKE_SOURCE_DIR}/platform/ps2compat/include)
target_include_directories(recvx PRIVATE
  ${PS2COMPAT_INC}
  ${CMAKE_SOURCE_DIR}/include/ps2/veronica/prog
  ${CMAKE_SOURCE_DIR}/include/cri/mwlib/ee/lib/libadxe
  ${GAME_PROG}
  ${GAME_ADX}
  ${CMAKE_SOURCE_DIR}/include/recvx-decomp-cri/cri/mwlib/include
  ${CMAKE_SOURCE_DIR}/include/recvx-decomp-cri/cri/mwlib/ee/include
  ${CMAKE_SOURCE_DIR}/platform/vita
  ${CMAKE_SOURCE_DIR}/platform
)
# The game's main() runs on an EE thread; the Vita main() is in platform/vita.
set_source_files_properties(${GAME_PROG}/njloop.c PROPERTIES COMPILE_DEFINITIONS "main=recvx_game_main")
# The Katana SDK directory ships its own stdio.h/math.h: search it last.
target_compile_options(recvx PRIVATE
  "SHELL:-idirafter ${CMAKE_SOURCE_DIR}/include/recvx-decomp-katana/KATANA/Include"
)

target_compile_definitions(recvx PRIVATE RECVX_VITA=1)
target_compile_options(recvx PRIVATE
  "SHELL:-include ${PS2COMPAT_INC}/recvx_prelude.h"
  "SHELL:-include ${PS2COMPAT_INC}/ee_asm.h"
  -std=gnu99 -O2 -g
  -fno-strict-aliasing -fwrapv -fsigned-char
  # EE code reads past arrays and through null-ish pointers the way the MIPS
  # tolerated; don't let GCC turn that undefined behaviour into hangs/traps.
  -fno-aggressive-loop-optimizations -fno-delete-null-pointer-checks
  -fno-isolate-erroneous-paths-dereference
  # decompiled code reads locals it never set on some paths (KazuSet's loop
  # bounds); on the EE stale registers happened to work, here make them zero
  -ftrivial-auto-var-init=zero
  -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard
  -Wall -Wno-unused -Wno-unknown-pragmas -Wno-parentheses -Wno-missing-braces
  -Werror=implicit-function-declaration -Werror=return-type
  -Wno-error=incompatible-pointer-types -Wno-incompatible-pointer-types
  -Wshift-count-overflow -Wno-pointer-sign -Wno-format
)

target_link_libraries(recvx
  vitaGL vitashark SceShaccCgExt taihen_stub SceShaccCg_stub
  mathneon
  SceAvPlayer_stub SceAudio_stub SceCtrl_stub SceDisplay_stub SceGxm_stub
  SceSysmodule_stub SceAppMgr_stub SceAppUtil_stub SceCommonDialog_stub
  SceKernelDmacMgr_stub SceIme_stub SceTouch_stub SceMotion_stub ScePower_stub
  SceLibKernel_stub SceKernelThreadMgr_stub SceIofilemgr_stub SceRtc_stub
  stdc++ m z c
)

vita_create_self(eboot.bin recvx UNSAFE)
vita_create_vpk(recvx.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
