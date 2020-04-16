# This file is here to initial Xcode to better default settings.

macro(CONFIGURE_XCODE_DEFAULTS _EXE_NAME)

# This iOS toolchain provides this convenience macro, but this is not in the mainline Mac, so let's define it.
if(APPLE AND NOT IOS)
	macro (set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
		set_property (TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
	endmacro (set_xcode_property)
endif()


if(APPLE)
	if(IOS)

		# We need to make sure dead code stripping is off for "plugins" since we must statically link.
#		set_xcode_property(${_EXE_NAME} DEAD_CODE_STRIPPING "No")

		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=RelWithDebInfo] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Release] YES)

		set_xcode_property(${_EXE_NAME} IPHONEOS_DEPLOYMENT_TARGET 5.1)
		set_xcode_property(${_EXE_NAME} TARGETED_DEVICE_FAMILY "1,2")

	# OSX
	else()
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=RelWithDebInfo] YES)
		set_xcode_property(${_EXE_NAME} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Release] YES)

		set_xcode_property(${_EXE_NAME} MACOSX_DEPLOYMENT_TARGET 10.6)
	

	endif(IOS)

	# Common to both
	# Use Apple's recommended standard architectures
	set_xcode_property(${_EXE_NAME} ARCHS "$(ARCHS_STANDARD)")

	
	
		set_xcode_property(${_EXE_NAME} GCC_OPTIMIZATION_LEVEL[variant=Debug] "0")
		set_xcode_property(${_EXE_NAME} GCC_OPTIMIZATION_LEVEL[variant=MinSizeRel] "s")
		set_xcode_property(${_EXE_NAME} GCC_OPTIMIZATION_LEVEL[variant=RelWithDebInfo] "s")
		set_xcode_property(${_EXE_NAME} GCC_OPTIMIZATION_LEVEL[variant=Release] "s")


	set_xcode_property(${_EXE_NAME} ONLY_ACTIVE_ARCH NO)
	set_xcode_property(${_EXE_NAME} DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

endif(APPLE)
endmacro(CONFIGURE_XCODE_DEFAULTS)
