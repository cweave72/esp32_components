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
from lua import Lua
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

    ctx.obj['lua'] = Lua(api)
    ctx.obj['lfs'] = LfsPart(api)
    ctx.obj['conn'] = conn

    connections.append(conn)
    atexit.register(on_exit)


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
@click.argument('remote')
@click.pass_context
def exec(ctx, **kwargs):
    """Executes a script on the remote filesystem.
    """
    params = get_params(**kwargs)
    cli_params = ctx.obj['cli_params']

    lua = ctx.obj['lua']
    try:
        status = lua.run_script(params.remote)
    except Exception as e:
        logger.exception(f"Error: {str(e)}")
        sys.exit()

    con = Console()
    if status < 0:
        con.print(f"Error: Script execution returned error: {status}.")
    else:
        con.print("Script execution returned success.")


def main():
    cli(obj={})


if __name__ == "__main__":
    main()
