import csv
import sys
import time
import pyxhook
# from sshkeyboard import listen_keyboard

USER_KEYSTROKES_PATH = '/lib64/security/keystrokes_models/{0}/{0}_keystrokes.csv'
# username = sys.argv[1]
class KeystrokesReader:

    def __init__(self, username):
        self.keystrokes_features = []
        self.username = username
        # Create hookmanager
        self.hookman = pyxhook.HookManager()
        # Define our callback to fire when a key is pressed down
        self.hookman.KeyDown = self.kb_down_event
        # Define our callback to fire when a key is pressed down
        self.hookman.KeyUp = self.kb_up_event
        # Hook the keyboard
        self.hookman.HookKeyboard()
        # Start our listener. Threads can only be started once.
        try:
            self.hookman.start()
        except KeyboardInterrupt as e:
            print("Error reading password!\n")


    def find_entrance(self, code):
        for i in range(len(self.keystrokes_features))[::-1]:
            if self.keystrokes_features[i][0] == code:
                return i

    def finalize(self, time):
        for i in range(len(self.keystrokes_features))[::-1]:
            if self.keystrokes_features[i][1]['pressed'] is None:
                self.keystrokes_features[i][1]['pressed'] = time
            if self.keystrokes_features[i][1]['released'] is None:
                self.keystrokes_features[i][1]['released'] = time


    def kb_down_event(self, event):
        try:
            if event.ScanCode == 36:
                self.hookman.cancel()
                self.finalize(time.time())
                self.calculate_row_data2(self.keystrokes_features)
                return
            self.keystrokes_features.append([event.ScanCode, {'pressed': time.time(), 'released': None}])
        except KeyError:
            pass


    def kb_up_event(self, event):
        try:
            if event.ScanCode == 36:
                return
            entrance = self.find_entrance(event.ScanCode)
            self.keystrokes_features[entrance][1]['released'] = time.time()
        except KeyError:
            pass


    def calculate_row_data2(self, data):
        row_data = []
        for i in range(len(data) - 1):
            HL1 = (data[i][1]['released'] - data[i][1]['pressed'])  # / 1000.0  # hold latency
            HL2 = (data[i + 1][1]['released'] - data[i + 1][1]['pressed'])  # / 1000.0  # hold latency
            IL = (data[i + 1][1]['pressed'] - data[i][1]['released'])  # / 1000.0  # inter-key latency
            PL = (data[i + 1][1]['pressed'] - data[i][1]['pressed'])  # / 1000.0  # press latency
            RL = (data[i + 1][1]['released'] - data[i][1]['released'])  # / 1000.0  # release latency
            row_data.extend([data[i][0], data[i + 1][0], HL1, HL2, IL, PL, RL])

        try:
            with open(USER_KEYSTROKES_PATH.format(self.username), 'w', newline='') as f:
                writer = csv.writer(f)

                # write the data
                writer.writerow(row_data)

                f.close()
        except FileNotFoundError as e:
            pass



if __name__ == '__main__':
    try:
        KeystrokesReader(sys.argv[1])
    except KeyboardInterrupt as e:
        print("Finish")
