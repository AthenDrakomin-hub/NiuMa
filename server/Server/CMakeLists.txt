
file(GLOB SERVER_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${SERVER_SRC_LIST})

add_executable(Server ${SERVER_SRC_LIST})

target_compile_options(Server PRIVATE ${COMPILE_OPTIONS_DEFAULT})

target_link_libraries(Server PRIVATE 
    ${FRAMEWORK_LIBRARY} 
    ${MAHJONG_LIBRARY} 
    ${POKER_LIBRARY}
    ${3RDPART_LIBRARIES}
    ${MYSQL_LIBRARIES}
    ${RABBITMQ_LIBRARIES}
    /usr/lib/x86_64-linux-gnu/libboost_log.so
    /usr/lib/x86_64-linux-gnu/libboost_log_setup.so
    /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
    /usr/lib/x86_64-linux-gnu/libboost_system.so
    /usr/lib/x86_64-linux-gnu/libboost_thread.so
    /usr/lib/x86_64-linux-gnu/libboost_chrono.so
    /usr/lib/x86_64-linux-gnu/libboost_regex.so
    /usr/lib/x86_64-linux-gnu/libboost_atomic.so
    /usr/lib/x86_64-linux-gnu/libboost_date_time.so
    ${FRAMEWORK_LIBRARY}
    /usr/lib/x86_64-linux-gnu/libboost_log.so
    /usr/lib/x86_64-linux-gnu/libboost_log_setup.so
    ${OPENSSL_LIBRARIES}
    -lpthread
    -lrt
    -ldl
)

if(WIN32)
  target_link_directories(Server PRIVATE "${BOOST_ROOT}/lib")
else()
  target_link_directories(Server PRIVATE "${Boost_INCLUDE_DIRS}/../lib")
endif()
