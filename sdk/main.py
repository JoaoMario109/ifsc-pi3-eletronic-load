# test_sdk.py

from load_sdk import ElectronicLoadSDK
import time

def main():
    load = ElectronicLoadSDK(port="/dev/ttyACM0", baud=115200)

    load.connect()
    time.sleep(2)

    load.enable_load()

    for i in [100, 200, 300, 400, 500]:
        load.set_cc(i)
        print("Set to CC mode, {} mA".format(i))
        print(load.get_measurements())
        time.sleep(2)


    for _ in range(5):
        meas = load.get_measurements()
        print("Measurements:", meas)
        time.sleep(1)

    load.set_cv(5000)
    print("Set to CV mode, 5 V")

    time.sleep(3)
    meas = load.get_measurements()
    print("Measurements in CV mode:", meas)

    load.disable_load()
    load.disconnect()

if __name__ == "__main__":
    main()
