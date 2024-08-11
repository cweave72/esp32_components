import logging
import itertools
from dataclasses import dataclass

from rich import inspect
from rich.table import Table
from rich.text import Text
from rich.pretty import Pretty

from protorpc.util import CallsetBase

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())

DATA_CHUNK_SIZE = 1000

LFS_SEEK_SET = 0
LFS_SEEK_CUR = 1
LFS_SEEK_END = 2

LFS_O_RDONLY = 1         # Open a file as read only
LFS_O_WRONLY = 2         # Open a file as write only
LFS_O_RDWR   = 3         # Open a file as read and write
LFS_O_CREAT  = 0x0100    # Create a file if it does not exist
LFS_O_EXCL   = 0x0200    # Fail if a file already exists
LFS_O_TRUNC  = 0x0400    # Truncate the existing file to zero size
LFS_O_APPEND = 0x0800    # Move to end of file on every write

lfs_error = {
    0: "LFS_ERR_OK",
    -5:  "LFS_ERR_IO",             # Error during device operation
    -84: "LFS_ERR_CORRUPT",        # Corrupted
    -2:  "LFS_ERR_NOENT",          # No directory entry
    -17: "LFS_ERR_EXIST",          # Entry already exists
    -20: "LFS_ERR_NOTDIR",         # Entry is not a dir
    -21: "LFS_ERR_ISDIR",          # Entry is a dir
    -39: "LFS_ERR_NOTEMPTY",       # Dir is not empty
    -9:  "LFS_ERR_BADF",           # Bad file number
    -27: "LFS_ERR_FBIG",           # File too large
    -22: "LFS_ERR_INVAL",          # Invalid parameter
    -28: "LFS_ERR_NOSPC",          # No space left on device
    -12: "LFS_ERR_NOMEM",          # No more memory available
    -61: "LFS_ERR_NOATTR",         # No data/attr available
    -36: "LFS_ERR_NAMETOOLONG"     # File name too long
}


def lfs_error_str(err: int) -> str:
    return lfs_error.get(err, "Unknown")


def chunked(iterable, size):
    it = iter(iterable)
    while True:
        chunk = tuple(itertools.islice(it, size))
        if len(chunk) != size or not chunk:
            break
        yield bytearray(chunk)
                      

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

    def file_open(self, path, flags, label='littlefs') -> (int, int):
        """File open.
        Params:
        path: Path to file.
        label : Partition label.
        Return (fd, file size)
        """
        reply = self.api.fileopen(path=path, flags=flags, part_label=label)
        self.check_reply(reply)

        status = reply.result.status
        if status < 0:
            logger.error(f"File open: {status} ({lfs_error_str(status)})")

        return reply.result.fd

    def file_close(self, fd) -> None:
        """File close.
        Params:
        fd: File desciptor returned from file_open().
        """
        reply = self.api.fileclose(fd)
        self.check_reply(reply)

    def file_read(
        self,
        fd,
        size,
        use_offset=False,
        offset=0,
        seek=LFS_SEEK_SET):
        """File read.
        Params:
        fd: File desciptor returned from file_open().
        size: size to read.
        use_offset: boolean indicating the offset should be used.
        offset: byte offset in file.
        seek: seek flag
        Returns data.
        """
        logger.debug(f"file_read: fd={fd}; offset={offset}; size={size}")
        reply = self.api.fileread(fd=fd,
                                  use_offset=use_offset,
                                  offset=offset,
                                  seek_flag=seek,
                                  read_size=size)
        self.check_reply(reply)
        status = reply.result.status
        if status < 0:
            logger.error(f"File read: {status} ({lfs_error_str(status)})")
            return (status, bytearray())

        if status == 0:
            logger.debug("EOF reached during read.")
            return (status, bytearray())

        logger.debug(f"file read: {status} bytes")
        return (status, reply.result.data)

    def file_write(
        self,
        fd,
        data,
        use_offset=False,
        offset=0,
        seek=LFS_SEEK_SET):
        """File write.
        Params:
        fd: File desciptor returned from file_open().
        use_offset: boolean indicating the offset should be used.
        offset: byte offset in file.
        seek: seek flag
        """
        logger.debug(f"file_write: fd={fd}; offset={offset}; size={len(data)} seek=0x{seek:08x}")
        reply = self.api.filewrite(fd=fd,
                                   use_offset=use_offset,
                                   offset=offset,
                                   seek_flag=seek,
                                   data=data)
        self.check_reply(reply)
        status = reply.result.status
        if status < 0:
            logger.error(f"File write: {status} ({lfs_error_str(status)})")
            return None

        logger.debug(f"file write: {status} bytes")
        return status

    def remove(self, path, label='littlefs') -> int:
        """Remove dir or file.
        Params:
        path: Path
        label : Partition label.
        Return (fd, file size)
        """
        reply = self.api.remove(path=path, part_label=label)
        self.check_reply(reply)

        status = reply.result.status
        if status < 0:
            logger.error(f"remove: {status} ({lfs_error_str(status)})")

        return status

    def get_file(self, path, label='littlefs'):
        """File read.
        Params:
        path: Path to file.
        label : Partition label.
        Returns a bytearray.
        """
        fd = self.file_open(path, LFS_O_RDONLY, label)

        if fd < 0:
            logger.error(f"Error opening remote path {path}")
            return bytearray()

        logger.debug(f"Opened file {path}. fd={fd}")
        filedata = bytearray()
        while True:
            status, data = self.file_read(fd, DATA_CHUNK_SIZE)
            if status < 0:
                self.file_close(fd)
                raise LfsPartException("Error during file read")

            if status > 0:
                logger.debug(f"Read ({status}) data={data}")
                filedata += data
            else:
                break

        self.file_close(fd)
        return filedata

    def put_file(self, data, path, label='littlefs'):
        """File write.
        Params:
        data: bytearray to write.
        path: Path to remote file.
        label : Partition label.
        """
        fd = self.file_open(path, LFS_O_CREAT|LFS_O_RDWR, label)
        logger.debug(f"put_file: Opened file {path}. fd={fd} size={len(data)}")

        if fd < 0:
            raise LfsPartException("Error during file open")

        chunks = len(data)//DATA_CHUNK_SIZE
        leftover = len(data) % DATA_CHUNK_SIZE

        logger.debug(f"put_file: Writing {chunks} chunks and {leftover} bytes leftover")

        for i, chunk in enumerate(chunked(data, size=DATA_CHUNK_SIZE)):
            logger.debug(f"put_file: Writing chunk {i} at offset={i*DATA_CHUNK_SIZE}")
            logger.debug(f"({len(chunk)}) chunk={chunk}")
            #status = self.file_write(fd, chunk, i*DATA_CHUNK_SIZE)
            status = self.file_write(fd, chunk)
            if status is None:
                logger.error(f"put_file: Error writing file chunk {i}: {path}")
                self.file_close(fd)
                raise LfsPartException("Error during file write")

        if leftover > 0:
            left = data[-leftover:]
            logger.debug(f"put_file: Writing leftover ({len(left)})")
            logger.debug(f"leftover={left}")
            #status = self.file_write(fd, left, chunks*DATA_CHUNK_SIZE)
            status = self.file_write(fd, left)
            if status is None:
                logger.error(f"put_file: Error writing leftover chunk: {path}")
                self.file_close(fd)
                raise LfsPartException("Error during file read")

        self.file_close(fd)
