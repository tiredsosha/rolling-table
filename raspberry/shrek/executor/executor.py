from entities.commands import Command


async def execute(command: Command):
    stack = [command]
    while stack:
        current = stack.pop()
        if isinstance(current, Command):
            stack.append(current.angle)
        elif isinstance(current, list):
            for action in current[::-1]:
                stack.append(action.angle)
        elif isinstance(current, int):
            yield current