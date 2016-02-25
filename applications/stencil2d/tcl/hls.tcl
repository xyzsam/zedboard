open_project stencil_syn

add_files -cflags "-D_SYNTHESIS_" stencil.c
add_files input.data
add_files check.data
add_files -cflags "-D_SYNTHESIS_" -tb harness.c

set_top stencil
open_solution -reset solution

set_part {xc7z020clg484-1}
create_clock -period 15
source ./stencil_dir

config_rtl -reset all -reset_level low
csynth_design
cosim_design -rtl verilog -tool modelsim -trace_level all -argv "input.data check.data"
export_design -format ip_catalog

exit
