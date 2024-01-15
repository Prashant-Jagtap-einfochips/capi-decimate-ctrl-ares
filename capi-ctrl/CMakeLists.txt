# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.14.3)

project(control_tx C CXX ASM)

enable_language(ASM)

macro (list2string out in)
    #string(REPLACE ";" " " ${out} "${in}")
    #message(STATUS "OUT:${${out}}")
    set(list ${ARGV})
    list(REMOVE_ITEM list ${out})
    foreach(item ${list})
        if(NOT ${out})
            set(${out} "${item}")
        else()
            set(${out} "${${out}} ${item}")
        endif()
    endforeach()
endmacro(list2string)

set(CMAKE_DEBUG_TARGET_PROPERTIES
        INCLUDE_DIRECTORIES
        COMPILE_DEFINITIONS
        POSITION_INDEPENDENT_CODE
        CONTAINER_SIZE_REQUIRED
        LIB_VERSION
    )

#Dependent library Paths
set(test_main_src ${HEXAGON_SDK_ROOT}/utils/test_main)
set(test_main_inc ${HEXAGON_SDK_ROOT}/utils/test_main/inc)
set(test_profile_src ${HEXAGON_SDK_ROOT}/utils/sim_profiler)
set(test_profile_inc ${HEXAGON_SDK_ROOT}/utils/sim_profiler/inc)
set(test_utils_src ${HEXAGON_SDK_ROOT}/utils/sim_utils)

set(test_capi_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/test/test_capi)
set(test_capi_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/test/test_capi/inc)
set(spf_ref_counter_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/utils/cmn)
set(spf_ref_counter_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/utils/cmn/inc)
set(spf_list_utils_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/utils/list)
set(spf_list_utils_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/utils/list/inc)	
set(ctrl_link_utils_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/capi_cmn)
set(ctrl_link_utils_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/capi_cmn/inc)
set(capi_utils_props_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/libs/capi_utils_props)
set(capi_utils_props_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/libs/capi_utils_props/inc)	
set(posal_utils_src ${CMAKE_CURRENT_SOURCE_DIR}/spf/libs/posal_utils)
set(posal_utils_inc ${CMAKE_CURRENT_SOURCE_DIR}/spf/libs/posal_utils/inc)		


include(${HEXAGON_CMAKE_ROOT}/hexagon_fun.cmake)
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)


set(EXE_LD_FLAGS)
list2string(EXE_LD_FLAGS
    -march=hexagon
    -mcpu=hexagon${V_ARCH}
    "-o <TARGET>"
    -G0
    -Wl,-L${HEXAGON_TOOLCHAIN}/Tools/target/hexagon/lib/${HEXAGON_ARCH}/G0/
    -Wl,-L${HEXAGON_TOOLCHAIN}/Tools/target/hexagon/lib/${HEXAGON_ARCH}/
    -Wl,-L${HEXAGON_TOOLCHAIN}/Tools/target/hexagon/lib/
    -Wl,--no-threads -Wl,--dynamic-linker= -Wl,-E -Wl,--force-dynamic,-u,main
    -Wl,--start-group
    "<OBJECTS>"
    "<LINK_LIBRARIES>"
    ${HEXAGON_TOOLCHAIN}/Tools/target/hexagon/lib/${HEXAGON_ARCH}/G0/libhexagon.a
    -Wl,--end-group
    -Wl,--start-group -lstandalone -lc -lgcc -Wl,--end-group
    )


set(HEXAGON_C_LINK_EXECUTABLE_LINK_OPTIONS "${EXE_LD_FLAGS}" )
set(HEXAGON_CXX_LINK_EXECUTABLE_LINK_OPTIONS "${EXE_LD_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "${HEXAGON_LINKER} ${HEXAGON_C_LINK_EXECUTABLE_LINK_OPTIONS}")
set(CMAKE_CXX_LINK_EXECUTABLE "${HEXAGON_LINKER} ${HEXAGON_CXX_LINK_EXECUTABLE_LINK_OPTIONS}")



#################################################
#Hexagon Dynamic Library
#################################################
#if(${BUILD_NAME} MATCHES "_q")

    #################################################
    # Static elf
    #################################################

    set(incs
        ${CMAKE_CURRENT_SOURCE_DIR}/api/
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/cmn/api/
        ${CMAKE_CURRENT_SOURCE_DIR}/capi/inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/test/test_capi/inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/api/ar_utils/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/api/apm/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/capi/adv/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spm/api/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spm/common/internal_api/imcl/api
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/capi/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/shared_lib_api/inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/api/ar_utils/hexagon/sim/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/metadata/api/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/api/modules/
	${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/spf/interfaces/module/capi_cmn/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/platform/posal/inc/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/platform/posal/inc/hexagon/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/gpr/platform/qurt/osal/api/
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/incs/core/api/services/

     )


    add_executable(control_tx_q
        ${CMAKE_CURRENT_SOURCE_DIR}/spf/test/test_capi/src/capi_unit_test
        ${CMAKE_CURRENT_SOURCE_DIR}/capi_entry_functions
        )

    add_library(control_tx
            SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/capi/src/control_tx
            ${CMAKE_CURRENT_SOURCE_DIR}/capi/src/control_tx_utils
        )

    add_library(control_tx_static
            STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/capi/src/control_tx
            ${CMAKE_CURRENT_SOURCE_DIR}/capi/src/control_tx_utils
        )


    # Set some of the flags
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions ")

    ##############################
    #
    # prepare_libraries_hexagon (<hexagon_targets> <hexagon_incs>  <hexagon_libs>
    #   libName [libNames...])
    #
    # The first 3 arguments will be the output arguments.  And the
    # following arguments will be the library names.  Without surfix, it is
    # treated as a dynamic library. surfixed with ".a" will be processed as
    # static library. And surffixed with ".so" will be processed as dynamic
    # library.
    #
    # This function will do the following:
    #
    # (1) For all libraries that specified, it will search Hexagon SDK tree
    # to find the corresponding library, and add a target into the
    # <hexagon_targets> list.  The custom_target will specify what to do
    # for that target.  It can be going into that corresponding directory to
    # build the directory or do nothing if it's prebuilt library. Caller of
    # this function can add this target as the dependency of their own
    # target or multiple targets
    #
    # (2) This call will add the "library specific" include directories
    # into <hexagon_incs> list. Caller of the function can add this include
    # path list into their corresponding include list
    #
    # (3) This library call will also return a "library-target" list
    # <hexagon_libs> so that it can be added into the linker dependency
    # list from target_link_libraries call
    #
    ##############################
    prepare_libraries_hexagon(hexagonTargets hexagonIncs hexagonLibs hexagonSharedLibs
        atomic.a		
        test_util.a
        test_main.a 
        test_profile.a
        test_capi.a
        posal_utils.a
        spf_ref_counter.a
        spf_list_utils.a
        ctrl_link_utils.a
    )

    prepare_libraries_hexagon(hexagonCommonTargets hexagonCommonIncs hexagonCommonLibs hexagonCommonSharedLibs
        capi_utils_props.a
    )

    #Import the Required RTLD Lib
    add_library(rtld STATIC IMPORTED) # or STATIC instead of SHARED
    set_target_properties(rtld PROPERTIES
        IMPORTED_LOCATION "${HEXAGON_SDK_ROOT}/ipc/fastrpc/rtld/ship/${PREBUILT_LIB_DIR}/rtld.a"
        INTERFACE_INCLUDE_DIRECTORIES "${HEXAGON_SDK_ROOT}/ipc/fastrpc/rtld/ship/${PREBUILT_LIB_DIR}/"
    )

    #Import the Required libstdc++ Lib
    #add_library(libstdc++ STATIC IMPORTED) # or STATIC instead of SHARED
    #set_target_properties(libstdc++ PROPERTIES
    #    IMPORTED_LOCATION "${HEXAGON_SDK_ROOT}/tools/HEXAGON_Tools/8.4.04/Tools/target/hexagon/lib/v68/G0/libstdc++.a"
    #)

    #Import the Required voice_imc_utils Lib
    #add_library(voice_imc_utils STATIC IMPORTED) # or STATIC instead of SHARED
    #set_target_properties(voice_imc_utils PROPERTIES
    #    IMPORTED_LOCATION "${HEXAGON_SDK_ROOT}/addons/audio/libs/voice_imc_utils/ship/${PREBUILT_LIB_DIR}/voice_imc_utils.a"
    #)

    #Include necessary dependent lib directory
    include_directories(
        ${incs}
        ${hexagonIncs}
        ${hexagonCommonIncs}
    )

    #Add hexagonTargets as one of your dependencies
    add_dependencies(control_tx ${hexagonCommonTargets})
    add_dependencies(control_tx_q control_tx_static ${hexagonTargets} ${hexagonCommonTargets} rtld)
    #add_dependencies(control_tx_q control_tx ${hexagonTargets} ${hexagonCommonTargets} rtld)    

    #Add hexagonLibs as your target link dependency
    target_link_libraries(control_tx ${hexagonCommonLibs})
    target_link_libraries(control_tx_q control_tx_static ${hexagonLibs} ${hexagonCommonLibs} rtld)
    #target_link_libraries(control_tx_q ${hexagonLibs} ${hexagonCommonLibs} rtld)    

    set(HEXAGON_EXEC control_tx_q)
    set(HEXAGON_EXEC_CMD_OPTIONS ${HEXAGON_EXEC_CMD_OPTIONS} -f epsilon -w 256 -h 64)

    copy_binaries(control_tx_q control_tx_static control_tx)
#endif()

