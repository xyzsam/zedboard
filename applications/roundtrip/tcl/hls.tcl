open_project roundtrip_syn

add_files -cflags "-D_SYNTHESIS_" roundtrip.c
add_files -cflags "-D_SYNTHESIS_ -DHLS_TB" -tb roundtrip.c

set_top roundtrip

open_solution -reset static_reg_plus_1
set_part {xc7z020clg484-1}
create_clock -period 2.5
config_rtl -reset all -reset_level low
csynth_design
cosim_design -rtl verilog -tool modelsim -trace_level all
export_design -format ip_catalog

exit
