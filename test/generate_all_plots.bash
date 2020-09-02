#!/bin/bash
mapfilter=""

./order_statistic_map_small_data_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Erase" | python3 ./plot.py --logx --title "Integer key insertion-erasure" --ylabel "time [ns]" --xlabel "size" --save insert_erase_integer.svg

./order_statistic_map_string_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Erase" | python3 ./plot.py --logx --title "String key insertion-erasure" --ylabel "time [ns]" --xlabel "size" --save insert_erase_string.svg

./order_statistic_map_big_data_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Erase" | python3 ./plot.py --logx --title "Large data insertion-erasure" --ylabel "time [ns]" --xlabel "size" --save insert_erase_big.svg

./order_statistic_map_small_data_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Find" | python3 ./plot.py --logx --title "Find integer key" --ylabel "time [ns]" --xlabel "size" --save find_integer.svg

./order_statistic_map_string_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Find" | python3 ./plot.py --logx --title "Find string key" --ylabel "time [ns]" --xlabel "size" --save find_string.svg

./order_statistic_map_big_data_perftest "--benchmark_format=csv" "--benchmark_filter=${mapfilter}Find" | python3 ./plot.py --logx --title "Find large data" --ylabel "time [ns]" --xlabel "size" --save find_big.svg
