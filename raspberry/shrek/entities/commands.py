from typing import List, Union

from pydantic import BaseModel


class Command(BaseModel):
    angle: Union[List["Command"], int]


Command.update_forward_refs()


def from_dict(commands: dict):
    comm = {}
    for k, v in commands.items():
        comm[k] = Command(**v)
    return comm
