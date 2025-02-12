import threading
import time
import struct
import serial

RX_MAGIC_WORD = 0x2D2D2D2D  # The magic word we send from "panel" to "load"

def _build_panel_to_load_message(enable, mode, cc, cv, cr, cp):
    """
    Build a 64-byte 'panel_to_load_t' packet:
      - enable (uint32_t)
      - mode   (uint32_t)
      - cc, cv, cr, cp each are dictionaries with:
            {
              "value_milli": <int>,
              "min_value_milli": <int>,
              "max_value_milli": <int>
            }
      - The final 4 bytes is a checksum over the first 60 bytes (magic + data).
    """

    # 1) Magic word (4 bytes)
    magic_word_packed = struct.pack("<I", RX_MAGIC_WORD)

    # 2) 14x uint32_t (56 bytes total):
    #    (enable, mode,
    #     cc.value_milli, cc.min_value_milli, cc.max_value_milli,
    #     cv.value_milli, cv.min_value_milli, cv.max_value_milli,
    #     cr.value_milli, cr.min_value_milli, cr.max_value_milli,
    #     cp.value_milli, cp.min_value_milli, cp.max_value_milli)
    data_packed = struct.pack(
        "<14I",
        enable,
        mode,
        cc["value_milli"], cc["min_value_milli"], cc["max_value_milli"],
        cv["value_milli"], cv["min_value_milli"], cv["max_value_milli"],
        cr["value_milli"], cr["min_value_milli"], cr["max_value_milli"],
        cp["value_milli"], cp["min_value_milli"], cp["max_value_milli"]
    )

    partial_msg = magic_word_packed + data_packed

    # 3) Compute checksum (sum of the first 60 bytes)
    chksum = sum(partial_msg) & 0xFFFFFFFF
    chksum_packed = struct.pack("<I", chksum)

    # Final 64 bytes
    return partial_msg + chksum_packed

class LoadParser:
    """
    Parser to handle inbound data from the load. It looks for:
      - Magic word  (4 bytes, 0x2B2B2B2B)
      - Data block  (20 bytes => 5 x uint32_t)
      - Checksum    (4 bytes)
    Total: 28 bytes.

    Once a valid packet is found, it returns a dict with fields:
        "cc_milli", "cv_milli", "cr_milli", "cp_milli", "temp_milli"
    """

    # Parser states
    PARSER_WAIT_START = 0
    PARSER_WAIT_DATA  = 1
    PARSER_WAIT_CS    = 2

    # The load’s TX magic word (0x2B2B2B2B)
    RX_MAGIC_WORD = 0x2B2B2B2B
    RX_DATA_SIZE  = 20  # size of load_measurement_t (5 x uint32_t)
    RX_MSG_SIZE   = 28  # 4 (magic) + 20 (data) + 4 (checksum)

    def __init__(self):
        self.parser_state = self.PARSER_WAIT_START
        self.current_byte = 0
        self.buffer = bytearray(self.RX_MSG_SIZE)

        # The top byte of 0x2B2B2B2B is 0x2B
        self.magic_first_byte = (self.RX_MAGIC_WORD >> 24) & 0xFF

    def parse_byte(self, incoming_byte: int):
        """
        Feed a single byte into the parser state machine.
        Returns:
            - None if we are still waiting for a full packet
            - 28-byte `bytes` if a full packet is found
        """
        if self.parser_state == self.PARSER_WAIT_START:
            # Looking for the first byte of the magic word (0x2B)
            if incoming_byte == self.magic_first_byte:
                self.buffer[self.current_byte] = incoming_byte
                self.current_byte += 1
                if self.current_byte == 4:
                    # We might have matched 4 bytes of magic
                    self.parser_state = self.PARSER_WAIT_DATA
            else:
                self.current_byte = 0  # restart

        elif self.parser_state == self.PARSER_WAIT_DATA:
            self.buffer[self.current_byte] = incoming_byte
            self.current_byte += 1
            if self.current_byte == 4 + self.RX_DATA_SIZE:
                # Next we expect the checksum
                self.parser_state = self.PARSER_WAIT_CS

        elif self.parser_state == self.PARSER_WAIT_CS:
            self.buffer[self.current_byte] = incoming_byte
            self.current_byte += 1
            if self.current_byte == self.RX_MSG_SIZE:
                # We have all 28 bytes
                complete_message = bytes(self.buffer)

                # Reset parser for next time
                self.parser_state = self.PARSER_WAIT_START
                self.current_byte = 0

                return complete_message

        else:
            # Shouldn't happen, reset
            self.parser_state = self.PARSER_WAIT_START
            self.current_byte = 0

        return None

    def parse_message(self, msg: bytes):
        """
        Given a 28-byte message, validate magic & checksum,
        then parse out the load_measurement_t data.
        Returns:
            dict with keys cc_milli, cv_milli, cr_milli, cp_milli, temp_milli
            or None if invalid.
        """
        if len(msg) != self.RX_MSG_SIZE:
            return None

        magic_word, = struct.unpack_from("<I", msg, 0)
        if magic_word != self.RX_MAGIC_WORD:
            return None

        data_section = msg[4 : 4 + self.RX_DATA_SIZE]  # next 20 bytes
        checksum, = struct.unpack_from("<I", msg, 4 + self.RX_DATA_SIZE)

        # Recompute checksum over the first 24 bytes
        calc_sum = sum(msg[0 : 4 + self.RX_DATA_SIZE]) & 0xFFFFFFFF
        if calc_sum != checksum:
            return None

        # Unpack the 5 measurement fields
        cc_milli, cv_milli, cr_milli, cp_milli, temp_milli = struct.unpack("<5I", data_section)

        return {
            "cc_milli": cc_milli,
            "cv_milli": cv_milli,
            "cr_milli": cr_milli,
            "cp_milli": cp_milli,
            "temp_milli": temp_milli,
        }


class ElectronicLoadSDK:
    """
    High-level SDK that:
      - Connects to the load via a serial port.
      - Spawns two threads:
          1) Writer: periodically sends “panel_to_load_t” with the SDK’s current
             settings (enable, mode, and setpoints for CC, CV, CR, CP).
          2) Reader: continuously parses inbound “load_measurement_t” messages
             to update the latest measured values (cc, cv, cr, cp, temperature).
      - Provides API methods to enable/disable, set modes and setpoints, and get
        measured current/voltage/resistance/power.
    """

    MODE_CC = 0  # Constant Current
    MODE_CV = 1  # Constant Voltage
    MODE_CR = 2  # Constant Resistance
    MODE_CP = 3  # Constant Power

    def __init__(self, port="/dev/ttyACM0", baud=115200,
                 write_interval=1.0, read_interval=0.01):
        # Connection params
        self._port = port
        self._baud = baud

        # Thread intervals
        self._write_interval = write_interval
        self._read_interval = read_interval

        # Internal objects
        self._ser = None
        self._stop_event = threading.Event()
        self._lock = threading.Lock()
        self._parser = LoadParser()

        # Worker threads
        self._writer_thread = None
        self._reader_thread = None

        # Last known measurement from the device
        # (cc_milli, cv_milli, cr_milli, cp_milli, temp_milli)
        self._last_measurement = {
            "cc_milli": 0,
            "cv_milli": 0,
            "cr_milli": 0,
            "cp_milli": 0,
            "temp_milli": 0,
        }

        # Outbound controls:
        #   - enable (0 or 1)
        #   - mode   (MODE_CC/CV/CR/CP)
        #   - setpoints (in milli-units)
        self._enable = 0
        self._mode = self.MODE_CC  # default to CC
        # CC
        self._cc_value = 1000
        self._cc_min = 0
        self._cc_max = 0
        # CV
        self._cv_value = 5000
        self._cv_min = 0
        self._cv_max = 0
        # CR (in milliohms)
        self._cr_value = 10000
        self._cr_min = 0
        self._cr_max = 0
        # CP (in mW)
        self._cp_value = 2000
        self._cp_min = 0
        self._cp_max = 0

    def connect(self):
        """Open serial port and start background threads."""
        with self._lock:
            if self._ser is not None:
                return  # already connected

            self._ser = serial.Serial(self._port, self._baud, timeout=0.1)
            self._stop_event.clear()

        # Start the reader/writer threads
        self._reader_thread = threading.Thread(target=self._reader_loop, daemon=True)
        self._writer_thread = threading.Thread(target=self._writer_loop, daemon=True)

        self._reader_thread.start()
        self._writer_thread.start()

    def disconnect(self):
        """Signal threads to stop, then close the serial port."""
        with self._lock:
            if self._ser is None:
                return  # already disconnected
            self._stop_event.set()

        if self._reader_thread:
            self._reader_thread.join()
        if self._writer_thread:
            self._writer_thread.join()

        with self._lock:
            if self._ser:
                self._ser.close()
                self._ser = None

        self._reader_thread = None
        self._writer_thread = None

    def enable_load(self):
        """Enable the load (set enable=1)."""
        with self._lock:
            self._enable = 1

    def disable_load(self):
        """Disable the load (set enable=0)."""
        with self._lock:
            self._enable = 0

    def set_cc(self, milli_amp):
        """
        Set the load to Constant Current (CC) mode, specifying the setpoint in mA.
        """
        with self._lock:
            self._mode = self.MODE_CC
            self._cc_value = int(milli_amp)

    def set_cv(self, milli_volt):
        """
        Set the load to Constant Voltage (CV) mode, specifying the setpoint in mV.
        """
        with self._lock:
            self._mode = self.MODE_CV
            self._cv_value = int(milli_volt)

    def set_cr(self, milli_ohm):
        """
        Set the load to Constant Resistance (CR) mode, specifying the setpoint in milliohms.
        """
        with self._lock:
            self._mode = self.MODE_CR
            self._cr_value = int(milli_ohm)

    def set_cp(self, milli_watt):
        """
        Set the load to Constant Power (CP) mode, specifying the setpoint in mW.
        """
        with self._lock:
            self._mode = self.MODE_CP
            self._cp_value = int(milli_watt)

    def get_current(self):
        """Return the last measured CC (in mA) from the device."""
        with self._lock:
            return self._last_measurement["cc_milli"]

    def get_voltage(self):
        """Return the last measured CV (in mV) from the device."""
        with self._lock:
            return self._last_measurement["cv_milli"]

    def get_resistance(self):
        """Return the last measured CR (in milliohms) from the device."""
        with self._lock:
            return self._last_measurement["cr_milli"]

    def get_power(self):
        """Return the last measured CP (in mW) from the device."""
        with self._lock:
            return self._last_measurement["cp_milli"]

    def get_temperature(self):
        """Return the last measured temperature (in milli-degC, if device provides it)."""
        with self._lock:
            return self._last_measurement["temp_milli"]

    def get_measurements(self):
        """
        Returns a dict with all the last-known measured values:
            {
                "cc_milli": ...,
                "cv_milli": ...,
                "cr_milli": ...,
                "cp_milli": ...,
                "temp_milli": ...
            }
        """
        with self._lock:
            return dict(self._last_measurement)

    def _writer_loop(self):
        """
        Periodically build and send the 64-byte "panel_to_load_t" packet
        based on the current enable, mode, and setpoint values.
        """
        while True:
            with self._lock:
                if self._stop_event.is_set():
                    break
                if not self._ser:
                    break

                # Build the sub-structs for CC, CV, CR, CP
                cc_struct = {
                    "value_milli": self._cc_value,
                    "min_value_milli": self._cc_min,
                    "max_value_milli": self._cc_max,
                }
                cv_struct = {
                    "value_milli": self._cv_value,
                    "min_value_milli": self._cv_min,
                    "max_value_milli": self._cv_max,
                }
                cr_struct = {
                    "value_milli": self._cr_value,
                    "min_value_milli": self._cr_min,
                    "max_value_milli": self._cr_max,
                }
                cp_struct = {
                    "value_milli": self._cp_value,
                    "min_value_milli": self._cp_min,
                    "max_value_milli": self._cp_max,
                }

                # Build the 64-byte payload
                msg = _build_panel_to_load_message(
                    self._enable,
                    self._mode,
                    cc_struct,
                    cv_struct,
                    cr_struct,
                    cp_struct
                )

                # Send out over serial
                self._ser.write(msg)

            time.sleep(self._write_interval)

    def _reader_loop(self):
        """
        Continuously read from the serial port and parse any inbound 
        "load_measurement_t" packets. Update self._last_measurement.
        """
        while True:
            with self._lock:
                if self._stop_event.is_set():
                    break
                if not self._ser:
                    break

                incoming_data = self._ser.read(32)  # read up to 32 bytes each time

            # Parse each byte
            for b in incoming_data:
                complete_msg = self._parser.parse_byte(b)
                if complete_msg:
                    result = self._parser.parse_message(complete_msg)
                    if result is not None:
                        with self._lock:
                            self._last_measurement = result

            time.sleep(self._read_interval)
