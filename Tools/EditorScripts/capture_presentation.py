"""List the six Presentation Pass viewpoints.

UnrealEditor-Cmd cannot take these shots. AutomationLibrary.take_high_res_screenshot
crashes that process (null RHI, and again with -AllowCommandletRendering). The
viewpoints live here so a later task can capture Before_ and After_ from the game
window. This script does not call the screenshot API.
"""
import unreal

# loc cm, pitch, yaw, roll. +X is out the boathouse door. The lake is -Y.
SHOTS = [
    ("Interior", (220.0, 0.0, 170.0), (0.0, 70.0, 0.0)),
    ("Door", (760.0, 0.0, 170.0), (0.0, 0.0, 0.0)),
    ("Camp", (2100.0, -500.0, 180.0), (0.0, 20.0, 0.0)),
    ("Lookout", (2900.0, 1100.0, 180.0), (0.0, 40.0, 0.0)),
    ("Wreck", (-900.0, -200.0, 180.0), (0.0, 180.0, 0.0)),
    ("Water", (-1040.0, -700.0, 160.0), (-8.0, -100.0, 0.0)),
]


def log(msg):
    unreal.log_warning("[DCSHOT] " + msg)


def main():
    for name, loc, rot in SHOTS:
        log(f"{name} loc={loc} rot={rot}")
    log("headless capture is not available; shots are taken from the game window")


main()
