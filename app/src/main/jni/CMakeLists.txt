cmake_minimum_required(VERSION 3.22)
project(scan)
add_library(scan SHARED scan.c)
target_link_options(scan PRIVATE -Wl,-z,max-page-size=16384)
