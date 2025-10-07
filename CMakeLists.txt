cmake_minimum_required(VERSION 3.20)
project(mini_os C CXX ASM)

# ---- Options / tools ------------------------------------------------------
# Build as a freestanding 32-bit kernel using host GCC/Clang with -m32.
# (Install multilib: e.g. on Debian/Ubuntu: gcc-multilib g++-multilib)
# Alternatively, switch your compilers before configure to i686-elf-gcc/g++.

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

# ---- Sources --------------------------------------------------------------
set(SRCS
  src/kernel.cpp
  src/keyboard.cpp
  src/vga.cpp
  src/boot/loaders/Multiboot2.cpp
  src/runtime/mem.cpp
  src/runtime/cxx_shim.cpp
  boot/boot.S
)

# ---- Compile & link flags -------------------------------------------------
# Freestanding, no host CRT, 32-bit, no stack protector/PIE, etc.
add_compile_options(
  -ffreestanding -fno-exceptions -fno-rtti -fno-pic -fno-plt
  -fno-stack-protector -fno-builtin
  -O2 -Wall -Wextra
  -m32
)

# Linker script + i386 ELF, no default libs/PIE/build-id
add_link_options(
  -nostdlib
  -Wl,-m,elf_i386
  -Wl,--build-id=none
  -no-pie
  -m32
)

add_executable(kernel.elf ${SRCS})

# Ensure the linker uses our script
set(LDS ${CMAKE_SOURCE_DIR}/boot/linker.ld)
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${LDS})
target_link_options(kernel.elf PRIVATE -T ${LDS})

# ---- objcopy to raw binary ------------------------------------------------
find_program(OBJCOPY objcopy REQUIRED)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/kernel.bin
  COMMAND ${OBJCOPY} -O binary $<TARGET_FILE:kernel.elf> ${CMAKE_BINARY_DIR}/kernel.bin
  DEPENDS kernel.elf
  VERBATIM
)
add_custom_target(bin ALL DEPENDS ${CMAKE_BINARY_DIR}/kernel.bin)

# ---- ISO with GRUB --------------------------------------------------------
# Requires: grub-mkrescue and xorriso installed.
find_program(GRUB_MKRESCUE grub-mkrescue REQUIRED)

set(ISOROOT ${CMAKE_BINARY_DIR}/iso)
set(ISOFILE ${CMAKE_BINARY_DIR}/os.iso)

add_custom_command(
  OUTPUT ${ISOFILE}
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${ISOROOT}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${ISOROOT}/boot/grub
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel.elf> ${ISOROOT}/boot/kernel.elf  # <-- ELF
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/boot/grub.cfg ${ISOROOT}/boot/grub/grub.cfg
  COMMAND ${GRUB_MKRESCUE} -o ${ISOFILE} ${ISOROOT}
  DEPENDS kernel.elf ${CMAKE_SOURCE_DIR}/boot/grub.cfg
)

add_custom_target(iso ALL DEPENDS ${ISOFILE})

# ---- Run in QEMU ----------------------------------------------------------
find_program(QEMU qemu-system-i386)

if(QEMU)
  add_custom_target(run
    COMMAND ${QEMU} -cdrom ${ISOFILE} -serial mon:stdio -m 16000
    DEPENDS iso
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )

  add_custom_target(run2G
    COMMAND ${QEMU} -cdrom ${ISOFILE} -serial mon:stdio -m 2000
    DEPENDS iso
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )

  add_custom_target(debug
    COMMAND ${QEMU} -cdrom ${ISOFILE} -serial mon:stdio -m 16000 -s -S -device VGA,vgamem_mb=256
    DEPENDS iso
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )

endif()