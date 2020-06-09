__all__ = ["YaqdWrightFilterWheelsContinuous"]

import asyncio
from typing import Dict, Any, List

from yaqd_core import ContinuousHardware, aserial

from .__version__ import __branch__

class YaqdWrightFilterWheelsContinuous(ContinuousHardware):
    _kind = "yaqd-wright-filter-wheels-continuous"
    _version = "0.1.0" + f"+{__branch__}" if __branch__ else ""
    traits: List[str] = ["uses-uart","uses-serial","is-homeable"]
    defaults: Dict[str, Any] = {"baud_rate": 57600}

    def __init__(self, name, config, config_filepath):
        super().__init__(name, config, config_filepath)
        self._motornum=config["motor"]
        self._serial_port = aserial.ASerial(config["serial_port"], config["baud_rate"])
        self.microstep=1
        self._steps_per_rotation=400


    def _load_state(self, state):
        """Load an initial state from a dictionary (typically read from the state.toml file).

        Must be tolerant of missing fields, including entirely empty initial states.

        Parameters
        ----------
        state: dict
            The saved state to load.
        """
        super()._load_state(state)
        # This is an example to show the symetry between load and get
        # If no persistent state is needed, these unctions can be deleted
        self.value = state.get("value", 0)

    def get_state(self):
        state = super().get_state()
        state["value"] = self.value
        return state

    def _set_position(self, position):
        step_position=int(round(self.microstep*position*self._steps_per_rotation/360,0))
        self._serial_port.write(f"M {self._motornum} {step_position}\n".encode())

    def direct_serial_write(self, message):
        self._busy = True
        self._serial_port.write(message.encode())

    def home(self):
        loop = asyncio.get_event_loop()
        loop.create_task(self._home())

    async def _home(self):
        self._busy = True
        # Initiate the home
        self._serial_port.write(b"H\n")
        await self._not_busy_sig.wait()
        self.set_position(self._destination)

    
    def set_microstep(self, microint):
        self._busy = True
        if (microint == 1 | microint == 2 | microint ==4 | microint ==8 | microint == 16 | microint ==32):
            self._serial_port.write(f"U {microint}\n".encode())
            self.microstep=microint
        

    async def update_state(self):
        while True:
            # Perform any updates to internal state
            self._serial_port.write(b"Q\n")
            line = await self._serial_port.areadline()
            self._busy = (line[0:1] != b"R")
            # self.logger.debug(line[0:1])
            # self._serial_port.write(b"G\n")
            # self._position = float(await self._serial_port.areadline())
            if self._busy:
                await asyncio.sleep(0.1)