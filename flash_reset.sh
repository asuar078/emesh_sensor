#nrfjprog --program build/zephyr/zephyr.hex -f nrf52 --sectorerase
nrfjprog --program cmake-build-debug/zephyr/zephyr.hex -f nrf52 --sectorerase
nrfjprog --reset -f nrf52