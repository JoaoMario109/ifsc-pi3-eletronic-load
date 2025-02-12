import tkinter as tk
from tkinter import ttk
from load_sdk import ElectronicLoadSDK


class LoadControlUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Electronic Load Control")

        self.load = None

        self.port_var = tk.StringVar(value="/dev/ttyACM0")
        self.baud_var = tk.StringVar(value="115200")

        # We'll keep track of whether we are connected or not
        self.connected = False

        # For the load mode (CC=0, CV=1, CR=2, CP=3)
        self.mode_var = tk.IntVar(value=ElectronicLoadSDK.MODE_CC)

        # Setpoints for each mode (in milli-units)
        self.cc_var = tk.StringVar(value="1000")    # mA
        self.cv_var = tk.StringVar(value="5000")    # mV
        self.cr_var = tk.StringVar(value="10000")   # milliohms
        self.cp_var = tk.StringVar(value="2000")    # mW

        # Measurement labels
        self.cc_measured_var = tk.StringVar(value="0 mA")
        self.cv_measured_var = tk.StringVar(value="0 mV")
        self.cr_measured_var = tk.StringVar(value="0 mΩ")
        self.cp_measured_var = tk.StringVar(value="0 mW")
        self.temp_measured_var = tk.StringVar(value="0 m°C")

        conn_frame = ttk.LabelFrame(root, text="Connection")
        conn_frame.pack(padx=10, pady=5, fill="x")

        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, padx=5, pady=5, sticky="e")
        ttk.Entry(conn_frame, textvariable=self.port_var, width=15).grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(conn_frame, text="Baud:").grid(row=0, column=2, padx=5, pady=5, sticky="e")
        ttk.Entry(conn_frame, textvariable=self.baud_var, width=10).grid(row=0, column=3, padx=5, pady=5)

        self.connect_button = ttk.Button(conn_frame, text="Connect", command=self.toggle_connection)
        self.connect_button.grid(row=0, column=4, padx=5, pady=5)

        # 2) Enable/Disable frame
        enable_frame = ttk.LabelFrame(root, text="Enable/Disable")
        enable_frame.pack(padx=10, pady=5, fill="x")

        self.enable_button = ttk.Button(enable_frame, text="Enable Load", command=self.enable_disable_load)
        self.enable_button.pack(padx=5, pady=5)

        # 3) Mode & Setpoints frame
        mode_frame = ttk.LabelFrame(root, text="Mode & Setpoint")
        mode_frame.pack(padx=10, pady=5, fill="x")

        # Mode radio buttons
        modes = [("CC (mA)", ElectronicLoadSDK.MODE_CC),
                 ("CV (mV)", ElectronicLoadSDK.MODE_CV),
                 ("CR (mΩ)", ElectronicLoadSDK.MODE_CR),
                 ("CP (mW)", ElectronicLoadSDK.MODE_CP)]

        for i, (label, value) in enumerate(modes):
            rb = ttk.Radiobutton(mode_frame, text=label, variable=self.mode_var, value=value)
            rb.grid(row=0, column=i, padx=5, pady=5, sticky="w")

        # Setpoint entries for each mode
        sp_frame = ttk.Frame(mode_frame)
        sp_frame.grid(row=1, column=0, columnspan=4, padx=5, pady=5, sticky="nsew")

        ttk.Label(sp_frame, text="CC (mA):").grid(row=0, column=0, sticky="e")
        ttk.Entry(sp_frame, textvariable=self.cc_var, width=8).grid(row=0, column=1, sticky="w")

        ttk.Label(sp_frame, text="CV (mV):").grid(row=0, column=2, sticky="e")
        ttk.Entry(sp_frame, textvariable=self.cv_var, width=8).grid(row=0, column=3, sticky="w")

        ttk.Label(sp_frame, text="CR (mΩ):").grid(row=0, column=4, sticky="e")
        ttk.Entry(sp_frame, textvariable=self.cr_var, width=8).grid(row=0, column=5, sticky="w")

        ttk.Label(sp_frame, text="CP (mW):").grid(row=0, column=6, sticky="e")
        ttk.Entry(sp_frame, textvariable=self.cp_var, width=8).grid(row=0, column=7, sticky="w")

        self.set_button = ttk.Button(sp_frame, text="Apply Setpoint", command=self.apply_setpoint)
        self.set_button.grid(row=0, column=8, padx=5, pady=5)

        # 4) Measurements frame
        meas_frame = ttk.LabelFrame(root, text="Measurements")
        meas_frame.pack(padx=10, pady=5, fill="x")

        row = 0
        ttk.Label(meas_frame, text="Current:").grid(row=row, column=0, padx=5, pady=5, sticky="e")
        ttk.Label(meas_frame, textvariable=self.cc_measured_var).grid(row=row, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(meas_frame, text="Voltage:").grid(row=row, column=2, padx=5, pady=5, sticky="e")
        ttk.Label(meas_frame, textvariable=self.cv_measured_var).grid(row=row, column=3, padx=5, pady=5, sticky="w")

        row = 1
        ttk.Label(meas_frame, text="Resistance:").grid(row=row, column=0, padx=5, pady=5, sticky="e")
        ttk.Label(meas_frame, textvariable=self.cr_measured_var).grid(row=row, column=1, padx=5, pady=5, sticky="w")

        ttk.Label(meas_frame, text="Power:").grid(row=row, column=2, padx=5, pady=5, sticky="e")
        ttk.Label(meas_frame, textvariable=self.cp_measured_var).grid(row=row, column=3, padx=5, pady=5, sticky="w")

        row = 2
        ttk.Label(meas_frame, text="Temperature:").grid(row=row, column=0, padx=5, pady=5, sticky="e")
        ttk.Label(meas_frame, textvariable=self.temp_measured_var).grid(row=row, column=1, padx=5, pady=5, sticky="w")

        # Schedule periodic measurement updates
        self.update_measurements()

    def toggle_connection(self):
        """Connect or disconnect from the load based on current state."""
        if not self.connected:
            # Connect
            try:
                port = self.port_var.get()
                baud = int(self.baud_var.get())
                self.load = ElectronicLoadSDK(port=port, baud=baud)
                self.load.connect()
                self.connected = True
                self.connect_button.configure(text="Disconnect")
            except Exception as e:
                print(f"Failed to connect: {e}")
        else:
            # Disconnect
            try:
                if self.load:
                    self.load.disconnect()
                self.load = None
                self.connected = False
                self.connect_button.configure(text="Connect")
            except Exception as e:
                print(f"Failed to disconnect: {e}")

    def enable_disable_load(self):
        if not self.load or not self.connected:
            return

        # Check if load is currently enabled by reading our local sdk's internal var
        # We can't directly read `_enable`, so let's guess by checking the button text
        current_text = self.enable_button.cget("text")
        if current_text == "Enable Load":
            self.load.enable_load()
            self.enable_button.configure(text="Disable Load")
        else:
            self.load.disable_load()
            self.enable_button.configure(text="Enable Load")

    def apply_setpoint(self):
        if not self.load or not self.connected:
            return

        mode = self.mode_var.get()
        try:
            if mode == ElectronicLoadSDK.MODE_CC:
                value = int(self.cc_var.get())
                self.load.set_cc(value)
            elif mode == ElectronicLoadSDK.MODE_CV:
                value = int(self.cv_var.get())
                self.load.set_cv(value)
            elif mode == ElectronicLoadSDK.MODE_CR:
                value = int(self.cr_var.get())
                self.load.set_cr(value)
            elif mode == ElectronicLoadSDK.MODE_CP:
                value = int(self.cp_var.get())
                self.load.set_cp(value)
        except ValueError:
            print("Invalid setpoint value.")

    def update_measurements(self):
        """
        Periodically fetch the latest measurements from the load
        and update the labels.
        """
        if self.load and self.connected:
            meas = self.load.get_measurements()
            self.cc_measured_var.set(f"{meas['cc_milli']} mA")
            self.cv_measured_var.set(f"{meas['cv_milli']} mV")
            self.cr_measured_var.set(f"{meas['cr_milli']} mΩ")
            self.cp_measured_var.set(f"{meas['cp_milli']} mW")
            self.temp_measured_var.set(f"{meas['temp_milli']} m°C")

        # Schedule the next update in 1 second
        self.root.after(1000, self.update_measurements)


def main():
    root = tk.Tk()
    app = LoadControlUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
