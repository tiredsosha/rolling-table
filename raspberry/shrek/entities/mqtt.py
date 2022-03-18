from typing import Optional
from pydantic import BaseModel, validator


class Mqtt(BaseModel):
    uid: str
    host: str
    port: Optional[int] = 1883
    username: Optional[str]
    password: Optional[str]

    @validator("port")
    def port_range(cls, value: int):
        if 0 < value < 65536:
            return value
        raise ValueError(
            f"Broker port value={value} must be in range (1..65535)")
