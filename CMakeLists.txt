#Usage => cmake -DCMAKE_BUILD_TYPE=Debug .

cmake_minimum_required(VERSION 2.8.0)
project( ScanVanUtils )
set(CMAKE_CXX_FLAGS "-std=c++11") # docopt needs this, per https://github.com/Qihoo360/logkafka/issues/1

set(ROOT ./)
include_directories(${ROOT})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package( OpenCV REQUIRED )


SET(GCC_COVERAGE_COMPILE_FLAGS "")
SET(GCC_COVERAGE_LINK_FLAGS "-lv4l1 -lv4l2")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )


add_executable( cmosCalibration src/cmosCalibration.cpp )
target_link_libraries( cmosCalibration ${OpenCV_LIBS} )

add_executable( omniCalibration src/omniCalibration.cpp )
target_link_libraries( omniCalibration ${OpenCV_LIBS} )

add_executable( camCapture src/camCapture.cpp src/v4ldevice.cpp)
target_compile_definitions(camCapture PRIVATE DOCOPT_HEADER_ONLY=1)
target_link_libraries( camCapture ${OpenCV_LIBS} )

add_executable( positionCalibration src/positionCalibration.cpp src/v4ldevice.cpp)
target_compile_definitions(positionCalibration PRIVATE DOCOPT_HEADER_ONLY=1)
target_link_libraries( positionCalibration ${OpenCV_LIBS} )


add_custom_target(cmosCalibration_demo
    COMMAND bin/cmosCalibration img/calib_dmk72buc02_C/in_calib.xml
    DEPENDS cmosCalibration
)

add_custom_target(omniCalibration_demo
    COMMAND bin/omniCalibration -w 9 -h 6 -sw 17.5 -sh 17.5 img/calib_dmk72buc02_omni/calib.xml
    DEPENDS cmosCalibration
)

add_custom_target(camCapture_demoCmos
    COMMAND bin/camCapture vc 0 cmos out_camera_data.xml
    DEPENDS camCapture
)

add_custom_target(camCapture_demoOmni
    COMMAND bin/camCapture vc 0 omni out_camera_omni.xml
    DEPENDS camCapture
)

add_custom_target(camCapture_demoOmniV4l
    COMMAND bin/camCapture v4l 0 omni out_camera_omni.xml
    DEPENDS camCapture
)

add_custom_target(positionCalibration_demo
    COMMAND bin/positionCalibration vc 0 out_camera_data.xml
    DEPENDS positionCalibration
)


