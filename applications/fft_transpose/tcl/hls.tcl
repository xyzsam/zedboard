open_project fft_syn

set_top fft_transpose

add_files -cflags "-I../../../common/src" fft.c
add_files -cflags "-I../../../common/src" local_support.c
add_files input.data
add_files check.data
add_files -cflags "-I../../../common/src" ../../../common/src/support.c
add_files -cflags "-I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

set clock 10
set part {xc7z020clg484-1}

open_solution -reset solution_baseline_3
set_part $part
create_clock -period $clock
#set_clock_uncertainty 0
source ./fft_dir
config_rtl -reset all -reset_level low
csynth_design
#cosim_design -tool modelsim -rtl verilog -trace_level all
#export_design -format ip_catalog
exit
