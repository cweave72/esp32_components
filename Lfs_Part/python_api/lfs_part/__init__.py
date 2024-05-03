import logging
from dataclasses import dataclass

from rich import inspect
from rich.table import Table
from rich.text import Text
from rich.pretty import Pretty

from protorpc.util import CallsetBase

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())


class LfsPartException(Exception):
    pass


@dataclass
class FileItem:
    _ftype: int
    size: int
    name: str

    @property
    def ftype(self):
        return "f" if self._ftype == 1 else "d"


class LfsPart(CallsetBase):
    """Class which provides access to the Lfs_PartRpc callset.
    """
    name = "lfs"

    def __init__(self, api):
        super().__init__(api)

    def get_fsinfo_table(self, label='littlefs'):
        """Prints a table for file system information.
        label : the patition label.
        """

        info = self.get_fsinfo(label)

        table = Table(title='', box=None)
        table.add_column('', style='yellow')
        table.add_column('')

        table.add_row('partition', Text(f": {label}"))
        table.add_row('base address', f": 0x{info.address:08x}")
        table.add_row('block_size', f": {info.block_size}")
        table.add_row('block_count', f": {info.block_count}")
        table.add_row('size (bytes)', f": {info.size}")
        return table

    def ls_table(self, path, label='littlefs'):
        """Prints a table for directory listing.
        label : the patition label.
        """
        idx = 0
        entries = []
        while True:
            # Due to limited size of the array of entries in the RPC, it might
            # take multiple calls to get all the entries.
            results = self.dirlist(path, start_idx=idx, label=label)
            new = [FileItem(r.type, r.size, r.name) for r in results.info_array]
            entries += new
            if len(entries) < results.num_entries:
                # Adjust start_idx to fetch the next group of entries.
                idx += len(entries)
                continue
            # Got them all
            break

        table = Table(title=f"{path}", box=None)
        table.add_column('Type', style='yellow')
        table.add_column('Size')
        table.add_column('Name')

        for entry in entries:
            row = []
            row.append(entry.ftype)
            row.append(Pretty(entry.size))
            row.append(entry.name)
            table.add_row(*row)
        return table

    def get_fsinfo(self, label='littlefs'):
        """Gets FS information.
        Params:
        label : Partition label.
        """
        reply = self.api.getfsinfo(part_label=label)
        self.check_reply(reply)
        return reply.result

    def dirlist(self, path, start_idx=0, label='littlefs'):
        """Lists contents of a directory.
        """
        reply = self.api.dirlist(part_label=label,
                                 path=path,
                                 start_idx=start_idx)
        self.check_reply(reply)
        return reply.result
