from setuptools import setup, find_packages

NAME = "lfs_part"
DESC = "RPC api for Lfs_Part."
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
            'lfs_cli=lfs_part.cli:main'
        ],
    },
    packages=find_packages(),
    install_requires=required
)
