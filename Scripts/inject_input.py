# -*- coding: utf-8 -*-
# Injects real keyboard/mouse input into the Salamander game window (test only).
# Usage: python inject_input.py   (run while the -game client window exists)
import ctypes, time, sys

user32 = ctypes.windll.user32

VK_LSHIFT = 0xA0
VK_LCONTROL = 0xA2
VK_LMB = 0x01
KEYEVENTF_KEYUP = 0x0002
MOUSEEVENTF_MOVE = 0x0001

class MOUSEINPUT(ctypes.Structure):
    _fields_ = [("dx", ctypes.c_long), ("dy", ctypes.c_long), ("mouseData", ctypes.c_ulong),
                ("dwFlags", ctypes.c_ulong), ("time", ctypes.c_ulong), ("dwExtraInfo", ctypes.c_ulong)]

class KEYBDINPUT(ctypes.Structure):
    _fields_ = [("wVk", ctypes.c_ushort), ("wScan", ctypes.c_ushort), ("dwFlags", ctypes.c_ulong),
                ("time", ctypes.c_ulong), ("dwExtraInfo", ctypes.c_ulong)]

class INPUT_union(ctypes.Union):
    _fields_ = [("ki", KEYBDINPUT), ("mi", MOUSEINPUT)]

class INPUT(ctypes.Structure):
    _fields_ = [("type", ctypes.c_ulong), ("union", INPUT_union)]
INPUT_KEY = 1
INPUT_MOUSE = 0

def send_key(vk, down):
    ki = KEYBDINPUT(vk, 0, 0 if down else KEYEVENTF_KEYUP, 0, 0)
    x = INPUT(INPUT_KEY, INPUT_union(ki=ki))
    user32.SendInput(1, ctypes.byref(x), ctypes.sizeof(INPUT))

def send_mouse_move(dx, dy):
    mi = MOUSEINPUT(dx, dy, 0, MOUSEEVENTF_MOVE, 0, 0)
    x = INPUT(INPUT_MOUSE, INPUT_union(mi=mi))
    user32.SendInput(1, ctypes.byref(x), ctypes.sizeof(INPUT))

def find_window():
    result = []
    def cb(hwnd, lparam):
        if user32.IsWindowVisible(hwnd):
            buf = ctypes.create_unicode_buffer(256)
            user32.GetWindowTextW(hwnd, buf, 256)
            if buf.value.startswith('Salamander'):
                result.append((hwnd, buf.value))
        return True
    WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)
    user32.EnumWindows(WNDENUMPROC(cb), 0)
    return result

wins = find_window()
print('windows:', wins)
if not wins:
    sys.exit(1)
hwnd = wins[0][0]
user32.SetForegroundWindow(hwnd)
time.sleep(0.5)

# sprint: hold LSHIFT 1.2s
send_key(VK_LSHIFT, True)
time.sleep(1.2)
send_key(VK_LSHIFT, False)
time.sleep(0.5)
# crouch: tap LCTRL
send_key(VK_LCONTROL, True)
time.sleep(0.4)
send_key(VK_LCONTROL, False)
time.sleep(0.5)
# attack: LMB
send_key(VK_LMB, True)
time.sleep(0.15)
send_key(VK_LMB, False)
time.sleep(0.5)
# mouse move (camera)
for i in range(10):
    send_mouse_move(25, 3)
    time.sleep(0.08)
print('input injected')
