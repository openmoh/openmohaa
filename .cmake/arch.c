
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#  error cmake_ARCH x86
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
#  error cmake_ARCH x86_64
#elif defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__) || defined(_ARCH_PPC64)
#  if __BIG_ENDIAN__
#    error cmake_ARCH ppc64
#  else
#    error cmake_ARCH ppc64el
#  endif
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__)
#  error cmake_ARCH ppc
#elif defined __s390__
#  error cmake_ARCH s390
#elif defined __s390x__
#  error cmake_ARCH s390x
#elif defined __ia64__
#  error cmake_ARCH ia64
#elif defined __e2k__
#  error cmake_ARCH e2k
#elif defined __alpha__
#  error cmake_ARCH alpha
#elif defined __sparc__
#  error cmake_ARCH sparc
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
#  error cmake_ARCH arm64
#elif defined __arm__ || defined (_M_ARM)
#  if defined(__ARM_PCS_VFP) && (__ARM_PCS_VFP)
#    error cmake_ARCH armhf
#  else
#    error cmake_ARCH armel
#  endif
#elif defined __cris__
#  error cmake_ARCH cris
#elif defined __hppa__
#  error cmake_ARCH hppa
#elif defined __mips__
#  error cmake_ARCH mips
#elif defined __sh__
#  error cmake_ARCH sh
#elif defined __riscv
#  if __UINTPTR_MAX__ == __UINT64_MAX__
#    error cmake_ARCH riscv64
#  else
#    error cmake_ARCH riscv
#  endif
#else
#  error cmake_ARCH unknown
#endif

