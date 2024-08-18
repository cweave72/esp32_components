import sys
import os.path as osp
import atexit
import logging
import click
import hexdump
from pathlib import Path

from rich.console import Console

# ProtoRpc modules
from protorpc.cli import get_params
from protorpc.cli.common_opts import cli_common_opts, cli_init
from protorpc.cli.common_opts import CONTEXT_SETTINGS
from protorpc.util import ProtoRpcException

# Callset classes
from lfs_part import LfsPart


logger = logging.getLogger(__name__)

connections = []


def on_exit():
    """Cleanup actions on program exit.
    """
    logger.debug("Closing connections on exit.")
    for con in connections:
        con.close()


@click.group(context_settings=CONTEXT_SETTINGS)
@cli_common_opts
@click.pass_context
def cli(ctx, **kwargs):
    """CLI application for calling LfsPart RPCs.
    """
    global connections

    params = get_params(**kwargs)

    try:
        api, conn = cli_init(ctx, params)
    except Exception as e:
        logger.error(f"Exiting due to error: {str(e)}")
        sys.exit(1)

    ctx.obj['lfs'] = LfsPart(api)
    ctx.obj['conn'] = conn

    connections.append(conn)
    atexit.register(on_exit)


@cli.command
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def fsinfo(ctx, **kwargs):
    """Prints a table of filesystem information.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        tbl = lfs.get_fsinfo_table(label=params.part)
        con = Console()
        con.print(tbl)
    except ProtoRpcException:
        sys.exit()


@cli.command
@click.argument('path')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def ls(ctx, **kwargs):
    """Prints a directory listing.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        tbl = lfs.ls_table(path=params.path, label=params.part)
        con = Console()
        con.print(tbl)
    except ProtoRpcException:
        sys.exit()


@cli.command
@click.argument('remotepath')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.option("--local", type=str, help="Path to local file to write.")
@click.option("-p", "--print", is_flag=True, help="Prints file to console.")
@click.pass_context
def get(ctx, **kwargs):
    """Gets a remote file.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        data = lfs.get_file(path=params.remotepath, label=params.part)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    if len(data) > 0:
        localfile = params.local if params.local is not None else osp.basename(params.remotepath)
        logger.info(f"Writing {len(data)} bytes to {localfile}.")
        Path(localfile).write_bytes(data)

        if params.print:
            console = Console()
            console.print(f"Contents of {params.remotepath}:")
            console.print(data)


@cli.command
@click.argument('remotepath')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def cat(ctx, **kwargs):
    """Prints a remote file to the console.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        data = lfs.get_file(path=params.remotepath, label=params.part)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    console = Console()
    console.print(f"{data.decode('utf-8')}")


@cli.command
@click.argument('remotepath')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def dump(ctx, **kwargs):
    """Hexdumps a remote file to the console.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        data = lfs.get_file(path=params.remotepath, label=params.part)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    if len(data) > 0:
        print(f"{params.remotepath} len={len(data)}")
        hexdump.hexdump(data)


@cli.command
@click.argument('local')
@click.argument('remote')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def put(ctx, **kwargs):
    """Puts a file to the remote filesystem.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        data = Path(params.local).read_bytes()
        data = lfs.put_file(data, path=params.remote, label=params.part)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    con = Console()
    con.print(f"Wrote: {params.local} --> remote: {params.remote}")


@cli.command
@click.argument('path')
@click.option("-p", "--part",
              type=str,
              default="littlefs",
              show_default=True,
              help="Partition label")
@click.pass_context
def rm(ctx, **kwargs):
    """Remove a file system path.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lfs = ctx.obj['lfs']
    try:
        status = lfs.remove(path=params.path, label=params.part)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    if status == 0:
        con = Console()
        con.print(f"Path {params.path} removed successfully.")


def main():
    cli(obj={})


if __name__ == "__main__":
    main()
