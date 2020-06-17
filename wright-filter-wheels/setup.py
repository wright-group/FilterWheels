#!/usr/bin/env python3

"""The setup script."""

import pathlib
from setuptools import setup, find_packages

here = pathlib.Path(__file__).parent

with open(here / "wright_filter_wheels" / "VERSION") as version_file:
    version = version_file.read().strip()


with open("README.md") as readme_file:
    readme = readme_file.read()


requirements = ["yaqd-core>=2020.06.1"]

extra_requirements = {"dev": ["black", "pre-commit"]}
extra_files = {"wright_filter_wheels": ["VERSION"]}

setup(
    author="yaq Developers",
    author_email="kentmeyer@wisc.edu",
    python_requires=">=3.7",
    classifiers=[
        "Development Status :: 2 - Pre-Alpha",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
        "Natural Language :: English",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Topic :: Scientific/Engineering",
    ],
    description="yaq daemon for Wright Group filter wheels",
    entry_points={
        "console_scripts": [
            "yaqd-wright-filter-wheels-continuous=wright_filter_wheels._yaqd_wright_filter_wheels_continuous:YaqdWrightFilterWheelsContinuous.main",
            "yaqd-wright-filter-wheels-discrete=wright_filter_wheels._yaqd_wright_filter_wheels_discrete:YaqdWrightFilterWheelsDiscrete.main",
        ],
    },
    install_requires=requirements,
    extras_require=extra_requirements,
    license="GNU Lesser General Public License v3 (LGPL)",
    long_description=readme,
    long_description_content_type="text/markdown",
    include_package_data=True,
    package_data=extra_files,
    keywords="wright-filter-wheels",
    name="wright-filter-wheels",
    packages=find_packages(include=["yaqd-wright-filter-wheels-continuous", "yaqd-wright-filter-wheels-continuous.*", "yaqd-wright-filter-wheels-discrete", "yaqd-wright-filter-wheels-discrete.*"]),
    # url="https://gitlab.com/yaq/wright-filter-wheels",
    version=version,
    zip_safe=False,
)
