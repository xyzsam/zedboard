open_project gemm_hls

add_files gemm.c
add_files input.data
add_files check.data
add_files -tb -cflags "-DHLS_TB" ../../../common/src/machsuite_harness.c

set_top gemm

open_solution solution_baseline
set_part {xc7z020clg484-1}
#source ./gemm_dir
create_clock -period 10
csynth_design
# cosim_design -rtl verilog -tool modelsim -trace_level all -argv "input8.data check.data"
export_design -format ip_catalog

exit
