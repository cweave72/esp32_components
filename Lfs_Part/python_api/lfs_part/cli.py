import sys
import atexit
import logging
import click

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


def main():
    cli(obj={})


if __name__ == "__main__":
    main()
