open_project hls

add_files -cflags "-D_SYNTHESIS_" accel.c
add_files -cflags "-D_SYNTHESIS_ -DENA_SIDECHANNELS -DHLS_TB" -tb main.c

set_top copy_axi

open_solution copy_axi
set_part {xc7z020clg484-1}
create_clock -period 10
csynth_design
cosim_design -rtl verilog -tool modelsim -trace_level all
export_design -format ip_catalog

exit
