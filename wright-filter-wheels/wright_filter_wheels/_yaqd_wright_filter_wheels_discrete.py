__all__ = ["YaqdWrightFilterWheelsDiscrete"]

import asyncio
from typing import Dict, Any, List
from yaqd_core import DiscreteHardware, aserial
import time
from .__version__ import __branch__

class YaqdWrightFilterWheelsDiscrete(DiscreteHardware):
    _kind = "wright-filter-wheels-discrete"
    _version = "0.1.0" + f"+{__branch__}" if __branch__ else ""
    traits: List[str] = ["uses-uart","uses-serial","is-homeable"]
    defaults: Dict[str, Any] = {"baud_rate": 57600}
    
    def __init__(self, name, config, config_filepath):
        super().__init__(name, config, config_filepath)
        self._motornum=config["motor"]
        self._serial_port = aserial.ASerial(config["serial_port"], config["baud_rate"])
        self._microstep=config["microstep"]
        self._units=config["units"]
        self._set_microstep(self._microstep)
        time.sleep(0.2)
        self._steps_per_rotation=400
        #self._position=0  # this statement soon to be removed
        #self._destination=0
        #self.home()   # this statement also soon to be removed
        #self._position_identifier = None # "
                               

    def _set_position(self, position):
        step_position=round(self._microstep*(position-self._position+0)*self._steps_per_rotation/360)   # 0 is placeholder for poss. offset
        self._serial_port.write(f"M {self._motornum} {step_position}\n".encode())
        self._position=position
 
    def direct_serial_write(self, message):
        self._busy = True
        self._serial_port.write(message.encode())

    def home(self):
        loop = asyncio.get_event_loop()
        loop.create_task(self._home())
        
    async def _home(self):
        self._busy = True
        self._serial_port.write(f"H {self._motornum}\n".encode())
        await self._not_busy_sig.wait()
        self._position=0
        self.set_position(self._destination)

    def _set_microstep(self, microint):
        self._busy = True
        if microint in [2**i for i in range(0,6)]:
            self._serial_port.write(f"U {microint}\n".encode())
            self._microstep=microint

    async def update_state(self):
        while True:
            # Perform any updates to internal state
            self._serial_port.write(f"Q {self._motornum}\n".encode())
            line = await self._serial_port.areadline()
            self._busy = (line[0:1] != b"R")
            #self.logger.debug(self._destination)
            await asyncio.sleep(1)
            if self._busy:
                self._position_identifier = None
            else:
                k1=None
                for k,v in self._position_identifiers.items():
                    self.logger.debug(self._position)
                    self.logger.debug(v)
                    if round(self._position) == round(v):
                        k1 = k
                self._position_identifier=k1        