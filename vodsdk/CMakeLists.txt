cmake_minimum_required(VERSION 2.8)
CMAKE_policy(SET CMP0015 NEW)

# CMakeLists for src directory
PROJECT(VOD_CPP_SDK)

if (OPENSSL_VERSION VERSION_LESS 1.1.0)
    message("old openssl version less than 1.1.0")
    set(VODSDK_SOURCE_FILES apply_upload.cpp commit_upload.cpp vod.cpp vodapi.cpp)
ELSE()
    message("new version upper than 1.1.0")
    set(VODSDK_SOURCE_FILES apply_upload.cpp commit_upload.cpp vod.cpp vodapi.cpp)
ENDIF()


add_library(vodsdk STATIC ${VODSDK_SOURCE_FILES})

# When use the on windows need change the boost library according to the local name
if(WIN32)
    target_link_libraries(vodsdk cossdk PocoFoundation PocoNet PocoNetSSL PocoCrypto PocoUtil PocoJSON PocoXML ssl crypto jsoncpp libboost_system-vc141-mt-x64-1_69 libboost_thread-vc141-mt-x64-1_69)
else()
	target_link_libraries(vodsdk cossdk PocoNetSSL PocoNet PocoCrypto PocoUtil PocoJSON PocoXML PocoFoundation ssl crypto rt stdc++ pthread jsoncpp boost_thread boost_system)
endif()

set_target_properties(vodsdk PROPERTIES OUTPUT_NAME "vodsdk")
