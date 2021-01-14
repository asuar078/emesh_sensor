#nrfjprog --program build/zephyr/zephyr.hex -f nrf52 --sectorerase
#nrfjprog --program cmake-build-debug/zephyr/zephyr.hex -f nrf52 --sectorerase
west build
west flash
nrfjprog --reset -f nrf52