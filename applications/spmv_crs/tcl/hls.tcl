open_project spmv_syn

add_files -cflags "-DZYNQ -I../../../common/src" spmv.c
add_files -cflags "-DZYNQ -I../../../common/src" local_support.c
add_files input.data
add_files check.data
add_files -cflags "-DZYNQ -I../../../common/src"  ../../../common/src/support.c
add_files -cflags "-DZYNQ -I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

set_top spmv

open_solution -reset solution
set_part {xc7z020clg484-1}
create_clock -period 10
#source ./spmv_dir
csynth_design
#cosim_design -rtl verilog -tool modelsim
export_design -format ip_catalog

exit
