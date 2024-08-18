import logging

from rich import inspect
from rich.table import Table
from rich.text import Text
from rich.pretty import Pretty

from protorpc.util import CallsetBase

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())


class LuaException(Exception):
    pass


class Lua(CallsetBase):
    """Class which provides access to the lua_thread_rpc callset.
    """
    name = "lua"

    def __init__(self, api):
        super().__init__(api)

    def runscript(self, path):
        """Runs a script residing in the flash filesystem.
        return: 0=success, -1=failure
        """
        reply = self.api.runscript(filename=path)
        self.check_reply(reply)
        return reply.result.status
