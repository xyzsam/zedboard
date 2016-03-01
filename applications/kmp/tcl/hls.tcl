open_project kmp_syn

add_files -cflags "-DZYNQ -I../../../common/src" kmp.c
add_files -cflags "-DZYNQ -I../../../common/src" local_support.c
add_files input.data
add_files check.data
add_files -cflags "-DZYNQ -I../../../common/src"  ../../../common/src/support.c
add_files -cflags "-DZYNQ -I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

#add_files -tb kmp_test.c

set_top kmp

open_solution -reset solution
set_part {xc7z020clg484-1}
create_clock -period 10
source ./kmp_dir
config_rtl -reset all -reset_level low
csynth_design
#cosim_design -rtl verilog -tool modelsim
export_design -format ip_catalog

exit
