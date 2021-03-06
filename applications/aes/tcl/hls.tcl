open_project aes_syn

add_files -cflags "-DZYNQ -I../../../common/src" aes.c
add_files -cflags "-DZYNQ -I../../../common/src" local_support.c
add_files input.data
add_files check.data
add_files -cflags "-DZYNQ -I../../../common/src" ../../../common/src/support.c
add_files -cflags "-DZYNQ -I../../../common/src" -tb ../../../common/src/machsuite_new_harness.c

#add_files -tb aes_test.c

# set_top aes256_encrypt_ecb
set_top aes

open_solution -reset solution_baseline
set_part {xc7z020clg484-1}
create_clock -period 10
#source ./aes_dir
config_rtl -reset all -reset_level low
csynth_design
#cosim_design -rtl verilog -tool modelsim -trace_level all
#export_design -format ip_catalog

exit
