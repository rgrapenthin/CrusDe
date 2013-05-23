#!/bin/tcsh

#define order in which plugins are to be installed to fulfill dependencies
set crustal_decay = ('crustal_decay/exponential' 'crustal_decay/exponential_rate');

set data_handler = ('data_handler/netcdf' 'data_handler/table');

set green = ('green/pinel_hs_elastic' 'green/pinel_hs_thickplate' 'green/pinel_hs_final_relaxed' 'green/pinel_inverse_thickplate' 'green/pinel_hs_elastic_minus_thickplate');

set load = ('load/disk_load'  'load/irregular_load');

set load_history = ('load_history/dirac_impulse' 'load_history/boxcar' 'load_history/boxcar_rate' 'load_history/fourier_series' 'load_history/fourier_series_rate'  'load_history/heaviside_step' 'load_history/sinusoidal' 'load_history/sinusoidal_rate');

set operator = ('operator/fast_conv' 'operator/fast_conv_time_space'); 

set postprocessors = ('postprocess/factor' 'postprocess/hori2vert_ratio' 'postprocess/xy2r');

#and now install them all ... 
foreach plugin ($crustal_decay $data_handler $green $load $load_history $operator $postprocessors)
    crusde -p install `pwd`/plugin_src/$plugin.so;
end

echo "We're done. Looks like a success. Note that there may be other plugins"
echo "in the /src/plugin_src/* directories. When not explicitly listed here,"
echo "Those are to be considered experimental. They may or may not work."
echo "Most likely they are not documented. If a name sounds intriguing and"
echo "you're interested in the promised functionality, I invite you to go "
echo "to work. Or drop me a note and I make it a priority. Cheers!"
 

