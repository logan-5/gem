#!/usr/bin/env python

import os


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == os.errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def safe_open_w(path):
    mkdir_p(os.path.dirname(path))
    return open(path, 'w')


def safe_touch(path):
    mkdir_p(os.path.dirname(path))
    open(path, 'w').close()


def safe_rm(path):
    try:
        os.remove(path)
    except OSError as exc:
        if exc.errno == os.errno.ENOENT:
            pass
        else:
            raise


def is_newer(file_name, than):
    try:
        return os.path.getmtime(file_name) > os.path.getmtime(than)
    except OSError:
        return True
