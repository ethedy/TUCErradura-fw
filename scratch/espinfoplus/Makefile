#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := esp-info

EXCLUDE_COMPONENTS := freemodbus libsodium esp-tls esp_http_client esp_http_server fatfs esp_https_ota wear_levelling openssl tcp_transport mqtt 

CXXFLAGS := -Wpedantic -std=c++17

#	
#LDLIBS += -u_printf_float

include $(IDF_PATH)/make/project.mk

