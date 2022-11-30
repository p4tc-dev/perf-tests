SHELL := /bin/bash
ROOT_DIR := $(abspath $(lastword $(MAKEFILE_LIST)/../../../))
COMMON_SRC_DIR := ${ROOT_DIR}/src
COMMON_DIR := ${ROOT_DIR}/include
LIB_DIR := ${ROOT_DIR}/src
APP_DIR := ./
