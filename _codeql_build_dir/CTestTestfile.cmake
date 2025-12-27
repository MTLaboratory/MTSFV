# CMake generated Testfile for 
# Source directory: /home/runner/work/MTSFV/MTSFV
# Build directory: /home/runner/work/MTSFV/MTSFV/_codeql_build_dir
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[QuickerSFV Unit Tests]=] "/home/runner/work/MTSFV/MTSFV/_codeql_build_dir/quicker_sfv_test")
set_tests_properties([=[QuickerSFV Unit Tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/MTSFV/MTSFV/CMakeLists.txt;195;add_test;/home/runner/work/MTSFV/MTSFV/CMakeLists.txt;0;")
add_test([=[QuickerSFV UI Unit Tests]=] "/home/runner/work/MTSFV/MTSFV/_codeql_build_dir/quicker_sfv_ui_tests")
set_tests_properties([=[QuickerSFV UI Unit Tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/MTSFV/MTSFV/CMakeLists.txt;275;add_test;/home/runner/work/MTSFV/MTSFV/CMakeLists.txt;0;")
subdirs("external/catch2")
subdirs("external/chromium_zlib")
