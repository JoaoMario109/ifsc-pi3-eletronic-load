idf_component_register(
  SRCS
    "bus/spi.c"
    "bus/i2c.c"

    "peripherals/encoder.c"
    "peripherals/buttons.c"
    "peripherals/lcd.c"
    "peripherals/led.c"

    "ui/index.c"

    "control/load.c"

    "main.c"
  INCLUDE_DIRS "."
)
