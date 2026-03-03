#!/bin/bash

if [ -z "$CRUSDE_HOME" ]; then
    echo "Error: Environment variable CRUSDE_HOME is not set!"
    echo "Please set this in your shell profile (~/.bashrc etc) to the directory in which CrusDe lives."
    exit 1
fi

if [ ! -f "$CRUSDE_HOME/plugins/plugins.xml" ]; then
    echo "Error: Could not find plugin database at '$CRUSDE_HOME/plugins/plugins.xml'"
    echo "Make sure CRUSDE_HOME is set to the right path."
    exit 1
fi

for dir in crustal_decay data_handler green load load_history operator postprocess; do
    if [ ! -d "$CRUSDE_HOME/plugins/$dir" ]; then
        echo "Error: Could not find plugin directory '$CRUSDE_HOME/plugins/$dir'"
        echo "Make sure CRUSDE_HOME is set to the right path."
        exit 1
    fi
done

# Define order in which plugins are to be installed to fulfill dependencies
crustal_decay=(
    crustal_decay/exponential
    crustal_decay/exponential_rate
)

data_handler=(
    data_handler/netcdf
    data_handler/table
)

green=(
    green/pinel_hs_elastic
    green/pinel_hs_thickplate
    green/pinel_hs_final_relaxed
    green/pinel_inverse_thickplate
    green/pinel_hs_elastic_minus_thickplate
)

load=(
    load/disk_load
    load/irregular_load
)

load_history=(
    load_history/dirac_impulse
    load_history/boxcar
    load_history/boxcar_rate
    load_history/fourier_series
    load_history/fourier_series_rate
    load_history/heaviside_step
    load_history/sinusoidal
    load_history/sinusoidal_rate
)

operator=(
    operator/fast_conv
    operator/fast_conv_time_space
)

postprocessors=(
    postprocess/factor
    postprocess/hori2vert_ratio
    postprocess/xy2r
)

# Create a backup of plugins.xml in case of abortion along the way
cp "$CRUSDE_HOME/plugins/plugins.xml" "$CRUSDE_HOME/plugins/plugins.xml.save"

all_plugins=(
    "${crustal_decay[@]}"
    "${data_handler[@]}"
    "${green[@]}"
    "${load[@]}"
    "${load_history[@]}"
    "${operator[@]}"
    "${postprocessors[@]}"
)

for plugin in "${all_plugins[@]}"; do
    crusde -p install "$(pwd)/plugin_src/${plugin}.so"
    exitcode=$?

    if [ $exitcode -ne 0 ] && [ $exitcode -ne 139 ]; then   # segfault is OK (see INSTALL)
        echo "Exit code was: $exitcode"
        cp "$CRUSDE_HOME/plugins/plugins.xml" "$CRUSDE_HOME/plugins/plugins.xml.$$"
        mv "$CRUSDE_HOME/plugins/plugins.xml.save" "$CRUSDE_HOME/plugins/plugins.xml"
        echo "There was an issue. I restored the previous plugin database at:"
        echo "     $CRUSDE_HOME/plugins/plugins.xml"
        echo "and saved the corrupted one to:"
        echo "     $CRUSDE_HOME/plugins/plugins.xml.$$"
        echo ""
        exit 1
    else
        echo " "
        echo "-------------------------------------------------------------------------------------------"
        echo " "
    fi
done

# Remove backup
rm "$CRUSDE_HOME/plugins/plugins.xml.save"

echo "We're done. Looks like a success. Note that there may be other plugins"
echo "in the /src/plugin_src/* directories. When not explicitly listed here,"
echo "those are to be considered experimental. They may or may not work."
echo "Most likely they are not documented. If a name sounds intriguing and"
echo "you're interested in the promised functionality, I invite you to go"
echo "to work. Or drop me a note at ronni@seismo.berkeley.edu"
echo ""
