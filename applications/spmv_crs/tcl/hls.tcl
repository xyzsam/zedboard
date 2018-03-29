open_project spmv_syn2

add_files -cflags "-DZYNQ -I../../../common/src" spmv.c
add_files -cflags "-DZYNQ -I../../../common/src" local_support.c
add_files input.data
add_files output.data
add_files -cflags "-DZYNQ -I../../../common/src"  ../../../common/src/support.c
add_files -cflags "-DZYNQ -I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

set_top spmv

#open_solution -reset solution
open_solution -reset solution2
set_part {xc7z020clg484-1}
# set_part virtex7
create_clock -period 10

set_directive_resource -core RAM_1P_BRAM "spmv" val
set_directive_resource -core RAM_1P_BRAM "spmv" vec
set_directive_resource -core RAM_1P_BRAM "spmv" cols
set_directive_resource -core RAM_1P_BRAM "spmv" rowDelimiters
set_directive_resource -core RAM_1P_BRAM "spmv" out
set_directive_unroll -factor 1  spmv/spmv_1
set_directive_unroll -factor 1  spmv/spmv_2

#source ./spmv_dir
config_rtl -reset all -reset_level low
csynth_design
#cosim_design -rtl verilog -tool modelsim
export_design -format ip_catalog

exit
