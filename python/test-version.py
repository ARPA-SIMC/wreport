#!/usr/bin/python3
import wreport
import unittest


class TestVersion(unittest.TestCase):
    def test_version(self):
        version = wreport.__version__


if __name__ == "__main__":
    from testlib import main
    main("test-version")
