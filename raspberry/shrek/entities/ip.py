from pydantic import BaseModel


class Ip(BaseModel):
    arduino: str
    raspberry: str
