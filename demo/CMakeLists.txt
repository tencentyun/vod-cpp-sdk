# demo directory CMakeLists.txt


ADD_DEFINITIONS(-D_AFXDLL)
SET(CMAKE_MFC_FLAG 2)
#ADD_EXECUTABLE(detect WIN64 ${DIR_SRCS})

SET(RC_FILE ./demo.rc ./res/demo.ico ./res/demo.rc2)

ADD_EXECUTABLE(vod_demo WIN32 demo.cpp demoDlg.cpp stdafx.cpp ${RC_FILE})

TARGET_LINK_LIBRARIES(vod_demo vodsdk)


