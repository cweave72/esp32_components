from setuptools import setup, find_packages

NAME = "lua"
DESC = "RPC api for lua_thread_rpc."
VERSION = "0.1.0"

required = [
    "click",
    "rich",
]

setup(
    name=NAME,
    version=VERSION,
    description=DESC,
    author='cdw',
    entry_points={
        'console_scripts': [
            'lua_cli=lua.cli:main'
        ],
    },
    packages=find_packages(),
    install_requires=required
)
