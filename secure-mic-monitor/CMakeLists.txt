cmake_minimum_required(VERSION 3.12)

#### Load config file
set(CONFIG_FILE "${CMAKE_CURRENT_LIST_DIR}/config/config.ini")
get_filename_component(CONFIG_FILE ${CONFIG_FILE} ABSOLUTE)
file(STRINGS ${CONFIG_FILE} CONFIG_CONTENTS)
foreach(config ${CONFIG_CONTENTS})
  # Strip leading spaces
  string(REGEX REPLACE "^[ ]+" "" config ${config})
  # Find variable name
  string(REGEX MATCH "^[^=]+" name ${config})
  # Find the value
  string(REPLACE "${name}=" "" value ${config})
	# Remove ""
  string(REPLACE "\"" "" value ${value})
  # Set the variable
  set(${name} ${value})
endforeach()

#### Project
set(PROJECT_NAME secure-mic-monitor)
set(PROJECT_BINARY ${PROJECT_NAME})
project(${PROJECT_NAME} CXX)

#### Project sources
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE PROJECT_SOURCE_FILES ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE PROJECT_HEADER_FILES ${SRC_DIR}/*.hpp)
add_executable(${PROJECT_BINARY}
		${PROJECT_SOURCE_FILES}
		${PROJECT_HEADER_FILES}
)
target_include_directories(${PROJECT_BINARY} PRIVATE ${SRC_DIR} ${OpenSSL})

#### OpenSSL
find_package(OpenSSL REQUIRED)
if( OPENSSL_FOUND )
	# include_directories(${OPENSSL_INCLUDE_DIRS})
	message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
	message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
	message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
	target_include_directories(${PROJECT_BINARY} PRIVATE ${OPENSSL_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_BINARY} ${OPENSSL_LIBRARIES})
else()
	message(FATAL_ERROR "OpenSSL not found") 
endif()

#### Definitions
# set(SSL_CERT_FILE "${CMAKE_CURRENT_LIST_DIR}/cert/cert.pem")
# set(SSL_KEY_FILE "${CMAKE_CURRENT_LIST_DIR}/cert/key.pem")
set(SSL_CERT_DIR ${CMAKE_CURRENT_LIST_DIR}/cert)
get_filename_component(SSL_CERT_FILE "${SSL_CERT_DIR}/${SSL_CERT_FILE}" ABSOLUTE)
get_filename_component(SSL_KEY_FILE "${SSL_CERT_DIR}/${SSL_KEY_FILE}" ABSOLUTE)
message(STATUS "SSL cert file: ${SSL_CERT_FILE}")
message(STATUS "SSL key file: ${SSL_KEY_FILE}")
# if (NOT DEFINED SERVER_PORT)
# 	set(SERVER_PORT 33412)
# endif()
# message(STATUS "Netlog monitor port: ${SERVER_PORT}")
# message(STATUS "Netlog enable ssl: ${NETLOG_SSL}")
target_compile_definitions(${PROJECT_BINARY} PRIVATE
	# NETLOG_PROTOCOL=${NETLOG_PROTOCOL}
	# NETLOG_PORT=${NETLOG_PORT}
	# NETLOG_SSL=${NETLOG_SSL}
	SSL_CERT_FILE="${SSL_CERT_FILE}" 
	SSL_KEY_FILE="${SSL_KEY_FILE}"
	SERVER_PORT=${SERVER_PORT}
	SERVER_SSL=${SERVER_SSL}
)
