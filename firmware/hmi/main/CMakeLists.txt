idf_component_register(
  SRCS
  "bus/spi.c"
  "bus/uart.c"

  "peripherals/encoder.c"
  "peripherals/buttons.c"
  "peripherals/lcd.c"
  "peripherals/led.c"
  "peripherals/sd.c"

  "server/server.c"

  "ui/index.c"
  "ui/menu.c"

  "control/load.c"

  "main.c"
  INCLUDE_DIRS "."
)
