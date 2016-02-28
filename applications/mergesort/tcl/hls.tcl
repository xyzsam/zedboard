open_project merge_syn

add_files merge.c
add_files input.data
add_files check.data
add_files -tb ../../../common/src/harness.c

set_top mergesort

open_solution -reset solution
set_part {xc7z020clg484-1}
create_clock -period 10
source ./merge_dir
csynth_design
cosim_design -rtl verilog -tool modelsim -trace_level all -argv "input.data check.data"
export_design -format ip_catalog


exit
