open_project knn_syn

add_files -cflags "-I../../../common/src" md.c
add_files -cflags "-I../../../common/src" local_support.c
add_files input.data
add_files check.data
add_files -cflags "-I../../../common/src" ../../../common/src/support.c
add_files -cflags "-I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

set_top mdknn

open_solution -reset solution_baseline_rerun
set_part {xc7z020clg484-1}
create_clock -period 10
# source ./knn_dir
config_rtl -reset all -reset_level low
csynth_design
#cosim_design -rtl verilog -tool modelsim -trace_level all
#export_design -format ip_catalog

exit
