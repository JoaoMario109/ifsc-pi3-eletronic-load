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
  "ui/stream.c"

  "control/load.c"
  "control/menu.c"
  "control/stream.c"

  "main.c"
  INCLUDE_DIRS "."
)
