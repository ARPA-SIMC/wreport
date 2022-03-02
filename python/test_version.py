#!/usr/bin/python3
import wreport
import unittest


class TestVersion(unittest.TestCase):
    def test_version(self):
        version = wreport.__version__
