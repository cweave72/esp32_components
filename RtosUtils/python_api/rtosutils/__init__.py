import logging

from rich import inspect
from rich.table import Table
from rich.pretty import Pretty
from rich import box

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())


STATES_TABLE = {
    0: 'running',
    1: 'ready',
    2: 'blocked',
    3: 'suspended',
    4: 'deleted',
    5: 'invalid'
}


class RtosUtilsException(Exception):
    pass


class RtosUtils:
    """Class which provides access to the RtosUtilsRpc callset.
    """
    name = "rtosutils"

    def __init__(self, api):
        self.api = api[f"{self.name}_callset"]

    def get_system_tasks_table(self) -> Table:

        results = self.get_system_tasks()
        # Collect all task entries into a dict by task number as the key, so
        # then can be sorted.
        tasks = {}
        for entry in results.task_info:
            tasks[entry.number] = entry

        sorted_tasks = dict(sorted(tasks.items()))

        table = Table(title='RTOS Tasks',
                      box=box.MINIMAL_DOUBLE_HEAD,
                      caption=f"total run time: {results.run_time}")

        table.add_column('#', style='magenta')
        table.add_column('Name', style='yellow')
        table.add_column('State', style='magenta')
        table.add_column('Prio')
        table.add_column('Run Time')
        table.add_column('Run %', style='blue')
        table.add_column('Stack Rem.')

        for num, entry in sorted_tasks.items():
            rows = []
            rows.append(Pretty(num))
            rows.append(entry.name)
            rows.append(STATES_TABLE.get(entry.state, 'unknown'))
            rows.append(Pretty(entry.prio))
            rows.append(Pretty(entry.rtc))
            run_pct = (entry.rtc / results.run_time)*100
            rows.append(f"{run_pct:.1f}")
            rows.append(Pretty(entry.stack_remaining))

            table.add_row(*rows)

        return table

    def get_system_tasks(self):
        reply = self.api.get_system_tasks()
        self.check_reply(reply)
        return reply.result

    def check_reply(self, reply):
        if not reply.success:
            msg = (f"RPC {self.name}: call={reply.call_msg} "
                   f"returned status={reply.status_str}")
            logger.error(msg)
            raise RtosUtilsException(msg)
